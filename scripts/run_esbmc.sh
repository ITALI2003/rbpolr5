#!/bin/bash
# Ограниченная проверка моделей (BMC) легаси- и исправленной версии модуля голосования.
# usage: ./scripts/run_esbmc.sh [legacy|fixed|mutant|all]
set -u
ESBMC=${ESBMC:-esbmc}
OPTS="--unwind 65 --overflow-check --unsigned-overflow-check --multi-property"
mkdir -p results
run() {   # $1 = имя, $2 = исходник, $3 = доп. опции
  echo "=== ESBMC: $1 ($2) ==="
  t0=$(date +%s.%N)
  $ESBMC harness/harness.c "$2" -I src $OPTS $3 > "results/esbmc_$1.log" 2>&1
  t1=$(date +%s.%N)
  grep -E "✗ FAILED|Properties:|VERIFICATION" "results/esbmc_$1.log"
  awk -v a=$t0 -v b=$t1 'BEGIN{printf "Время проверки: %.1f с\n", b-a}' 
}
case "${1:-all}" in
  legacy) run legacy src/legacy/vote_parser_legacy.c "--no-unwinding-assertions" ;;
  fixed)  run fixed  src/fixed/vote_parser.c "" ;;
  mutant) run mutant src/mutant/vote_parser_mutant.c "" ;;
  all)    run legacy src/legacy/vote_parser_legacy.c "--no-unwinding-assertions"
          run fixed  src/fixed/vote_parser.c "" ;;
esac
# код возврата = результат проверки исправленной версии (для CI)
grep -q "VERIFICATION SUCCESSFUL" results/esbmc_fixed.log 2>/dev/null
