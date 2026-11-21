#!/bin/bash
# Frama-C: EVA (абстрактная интерпретация) и WP (дедуктивная проверка ACSL-контрактов).
# usage: ./scripts/run_frama_c.sh [eva|wp|mutant|all]
#   FRAMAC  — команда запуска Frama-C (по умолчанию frama-c; для Docker см. README)
#   PROVER  — решатель для WP (по умолчанию alt-ergo; варианты: z3, cvc4 или имя из `why3 config list-provers`)
set -u
FRAMAC=${FRAMAC:-frama-c}
PROVER=${PROVER:-alt-ergo}
mkdir -p results
CPP="-cpp-extra-args=-Isrc"
eva() {  # $1 = имя, $2 = исходник
  echo "=== Frama-C EVA: $1 ==="
  $FRAMAC $CPP -eva -eva-precision 3 -main main harness/harness_eva.c "$2" \
          -then -report 2>&1 | tee "results/eva_$1.log" | grep -E "alarm|Alarm|Status Report|Completely|Considered|To Be Validated|Unreachable"
}
wp() {
  echo "=== Frama-C WP + RTE: fixed (решатель: $PROVER) ==="
  $FRAMAC $CPP -wp -wp-rte -wp-prover "$PROVER" -wp-timeout 10 src/fixed/vote_parser.c \
          -then -report 2>&1 | tee results/wp_fixed.log | grep -E "\[wp\] Proved goals|Status Report|Completely|To Be Validated|Unknown|Timeout"
  echo "=== Frama-C WP + RTE: legacy (контрактов нет, только цели RTE) ==="
  $FRAMAC $CPP -wp -wp-rte -wp-prover "$PROVER" -wp-timeout 10 src/legacy/vote_parser_legacy.c \
          2>&1 | tee results/wp_legacy.log | grep -E "\[wp\] Proved goals|Unknown|Timeout"
}
case "${1:-all}" in
  eva) eva legacy src/legacy/vote_parser_legacy.c; eva fixed src/fixed/vote_parser.c ;;
  wp)  wp ;;
  mutant)
    echo "=== Frama-C WP + RTE: mutant (удалена проверка n > MAX_AXES) ==="
    $FRAMAC $CPP -wp -wp-rte -wp-prover "$PROVER" -wp-timeout 10 src/mutant/vote_parser_mutant.c \
            2>&1 | tee results/wp_mutant.log | grep -E "\[wp\] Proved goals|Unknown|Timeout" ;;
  all) eva legacy src/legacy/vote_parser_legacy.c; eva fixed src/fixed/vote_parser.c; wp ;;
esac
