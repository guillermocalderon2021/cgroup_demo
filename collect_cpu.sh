#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

N="${1:-500000000}"
CG_REL=$(awk -F: '$1=="0" {print $3}' /proc/$$/cgroup)
CG="/sys/fs/cgroup${CG_REL}"

printf 'PID shell: %s\n' "$$"
printf 'cgroup: %s\n' "$CG_REL"
printf '\n--- cpu.max ---\n'
cat "$CG/cpu.max"
printf '\n--- cpu.stat ANTES ---\n'
cat "$CG/cpu.stat"
printf '\n--- workload ---\n'
/usr/bin/time -f 'elapsed=%e s\nuser=%U s\nsys=%S s' ./cpu_work "$N"
printf '\n--- cpu.stat DESPUÉS ---\n'
cat "$CG/cpu.stat"
