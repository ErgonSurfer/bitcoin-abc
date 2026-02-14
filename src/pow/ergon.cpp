// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2017-2020 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow/ergon.h>

#include <arith_uint256.h>
#include <chain.h>
#include <consensus/activation.h>
#include <consensus/params.h>

#include <algorithm>
#include <cassert>

static const CBlockIndex *GetSuitableBlock(const CBlockIndex *pindex) {
    assert(pindex->nHeight >= 3);

    const CBlockIndex *blocks[3];
    blocks[2] = pindex;
    blocks[1] = pindex->pprev;
    blocks[0] = blocks[1]->pprev;

    if (blocks[0]->nTime > blocks[2]->nTime) {
        std::swap(blocks[0], blocks[2]);
    }

    if (blocks[0]->nTime > blocks[1]->nTime) {
        std::swap(blocks[0], blocks[1]);
    }

    if (blocks[1]->nTime > blocks[2]->nTime) {
        std::swap(blocks[1], blocks[2]);
    }

    return blocks[1];
}

static arith_uint256 ComputeEmaTarget(const CBlockIndex *pindexPrev,
                                      const Consensus::Params &params) {
    arith_uint256 work = pindexPrev->nChainWork - pindexPrev->pprev->nChainWork;
    const CBlockIndex *p1 = GetSuitableBlock(pindexPrev);
    const CBlockIndex *p0 = GetSuitableBlock(pindexPrev->pprev);

    int64_t t1 = p1->nTime;
    int64_t t0 = p0->nTime;
    int64_t t = t1 - t0;

    int64_t resistance = 1000;
    int minimum = resistance / 2 - 1;
    int normalized_time = t / params.nPowTargetSpacing;

    if (normalized_time > minimum) {
        work -= (work * minimum) / resistance - work / resistance -
                (work * minimum * minimum) / (resistance * resistance) +
                2 * (work * minimum) / (resistance * resistance) -
                work / (resistance * resistance);
    } else if (t >= 0) {
        work -= (work * t / params.nPowTargetSpacing) / resistance -
                work / resistance -
                (work * t * t /
                 (params.nPowTargetSpacing * params.nPowTargetSpacing)) /
                    (resistance * resistance) +
                2 * (work * t / params.nPowTargetSpacing) /
                    (resistance * resistance) -
                work / (resistance * resistance);
    } else {
        t = -t;
        work += (work * t / params.nPowTargetSpacing) / resistance +
                work / resistance +
                (work * t * t /
                 (params.nPowTargetSpacing * params.nPowTargetSpacing)) /
                    (resistance * resistance) +
                2 * (work * t / params.nPowTargetSpacing) /
                    (resistance * resistance) +
                work / (resistance * resistance);
    }

    return (-work) / work;
}

static arith_uint256 ComputeExpTarget(const CBlockIndex *pindexPrev,
                                      const Consensus::Params &params) {
    arith_uint256 work = pindexPrev->nChainWork - pindexPrev->pprev->nChainWork;
    const CBlockIndex *p1 = GetSuitableBlock(pindexPrev);
    const CBlockIndex *p0 = GetSuitableBlock(pindexPrev->pprev);

    int64_t t = p1->nTime - p0->nTime;

    int64_t resistance = 1000;
    int minimum = resistance / 2 - 1;
    int normalized_time = t / params.nPowTargetSpacing;

    if (normalized_time > minimum) {
        work -= (work * minimum) / resistance - work / resistance -
                (work * minimum * minimum) / (resistance * resistance) +
                2 * (work * minimum) / (resistance * resistance) -
                work / (resistance * resistance);
    } else {
        work -= (work * t / params.nPowTargetSpacing) / resistance -
                work / resistance -
                (work * (t * t) /
                 (params.nPowTargetSpacing * params.nPowTargetSpacing)) /
                    (resistance * resistance) +
                2 * (work * t / params.nPowTargetSpacing) /
                    (resistance * resistance) -
                work / (resistance * resistance);
    }

    return (-work) / work;
}

uint32_t GetNextExpWorkRequired(const CBlockIndex *pindexPrev,
                                const CBlockHeader *pblock,
                                const Consensus::Params &params) {
    (void)pblock;
    assert(pindexPrev);
    if (pindexPrev->nHeight < 4) {
        return 0x1a04b500;
    }
    arith_uint256 nextTarget = ComputeExpTarget(pindexPrev, params);
    if (!IsErgonEMAEnabled(params, pindexPrev)) {
        nextTarget = ComputeExpTarget(pindexPrev, params);
    } else if (IsErgonEMAEnabled(params, pindexPrev) &&
               !IsErgonEMAEnabled(params, pindexPrev->pprev)) {
        return 0x1a04b500;
    } else {
        nextTarget = ComputeEmaTarget(pindexPrev, params);
    }
    const arith_uint256 powLimit = UintToArith256(params.powLimit);
    if (nextTarget > powLimit) {
        return powLimit.GetCompact();
    }

    return nextTarget.GetCompact();
}
