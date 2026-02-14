Ergon Node Commands
===================

This is a practical command reference for running the Ergon-enabled node from this repo on macOS.

Paths used below:

```bash
NODE_DIR="/Users/jonathanklein/Documents/moom/bitcoin-abc-ergon-new"
DATA_DIR="/Users/jonathanklein/Documents/moom/.ergon-node"
BITCOIND="$NODE_DIR/build/src/bitcoind"
BTC_CLI="$NODE_DIR/build/src/bitcoin-cli"
```

Build
-----

Configure (example flags used in this setup):

```bash
cmake -S "$NODE_DIR" \
  -B "$NODE_DIR/build" \
  -GNinja \
  -DCMAKE_PREFIX_PATH=/opt/homebrew \
  -DBoost_ROOT=/opt/homebrew/opt/boost \
  -DENABLE_UPNP=OFF \
  -DENABLE_NATPMP=OFF \
  -DBUILD_WALLET=OFF \
  -DBUILD_ZMQ=OFF \
  -DBUILD_QT=OFF
```

Build:

```bash
cmake --build "$NODE_DIR/build" -j
```

Run
---

Create data dir and minimal config:

```bash
mkdir -p "$DATA_DIR"
cat > "$DATA_DIR/bitcoin.conf" <<'EOF'
chain=ergon
server=1
listen=1
txindex=1
EOF
```

Run in foreground:

```bash
"$BITCOIND" -datadir="$DATA_DIR" -chain=ergon
```

Run as daemon:

```bash
"$BITCOIND" -datadir="$DATA_DIR" -chain=ergon -daemon=1
```

Stop daemon:

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon stop
```

Health and Sync
---------------

Basic chain status:

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon getblockchaininfo
```

Network status:

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon getnetworkinfo
```

Current block count:

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon getblockcount
```

Peers and Hashrate
------------------

Connected peer count:

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon getconnectioncount
```

Detailed peer list:

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon getpeerinfo
```

Estimated network hashrate:

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon getnetworkhashps
```

Mining info (includes `networkhashps`):

```bash
"$BTC_CLI" -datadir="$DATA_DIR" -chain=ergon getmininginfo
```

Useful Ops
----------

Tail debug log:

```bash
tail -f "$DATA_DIR/debug.log"
```

Check local listening ports:

```bash
lsof -nP -iTCP -sTCP:LISTEN | rg '2135|2136|2137'
```

Ergon default ports in this setup:

- P2P: `2137`
- RPC: `2136`
- Chronik: `2135` (if enabled)

Quick Troubleshooting
---------------------

If config flags changed and CMake cache is stale:

```bash
rm -rf "$NODE_DIR/build"
```

Then configure and build again.
