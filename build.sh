#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

gcc -O2 -Wall -Wextra -Wpedantic -std=c17 cpu_work.c -o cpu_work
gcc -O2 -Wall -Wextra -Wpedantic -std=c17 mem_work.c -o mem_work

echo "Compilación completada: cpu_work y mem_work"
