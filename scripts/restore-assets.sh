#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
tar -xzf "$ROOT/assets/stage.tar.gz" -C "$ROOT"
tar -xzf "$ROOT/assets/distfiles.tar.gz" -C "$ROOT"
