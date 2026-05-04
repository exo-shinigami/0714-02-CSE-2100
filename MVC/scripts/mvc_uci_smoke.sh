#!/usr/bin/env bash
# Minimal UCI smoke: build, send handshake + position + quit; exit non-zero on failure.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
make -C "$ROOT" -j2
BIN="$ROOT/build/bin/gambit.exe"
(
  printf 'uci\nisready\nposition startpos moves e2e4 e7e5\nquit\n'
) | timeout 20 "$BIN" uci >/tmp/gambit_uci_smoke.log
grep -q 'readyok' /tmp/gambit_uci_smoke.log
echo "OK: UCI smoke passed (log: /tmp/gambit_uci_smoke.log)"
