#!/bin/bash
# Воспроизведение контрпримеров ESBMC на реальном запуске (AddressSanitizer + UBSan).
# usage: ./scripts/run_repro.sh [legacy|fixed]
V=${1:-legacy}
SRC=src/legacy/vote_parser_legacy.c; [ "$V" = fixed ] && SRC=src/fixed/vote_parser.c
mkdir -p build
gcc -g -O0 -fsanitize=address,undefined -fno-sanitize-recover=all -Isrc tests/repro.c "$SRC" -o build/repro_$V || exit 1
for t in 1 2 3 4; do
  echo "----- тест $t ($V) -----"
  ./build/repro_$V $t 2>&1 | grep -E "^test|^  [a-z_]+ =|ERROR: AddressSanitizer|SUMMARY|runtime error"
done
