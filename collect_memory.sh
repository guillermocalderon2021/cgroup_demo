#!/usr/bin/env bash
set -u
cd "$(dirname "$0")" || exit 1

TARGET_MIB="${1:-256}"
PAUSE_MS="${2:-200}"

CG_REL=$(awk -F: '$1=="0" {print $3}' /proc/$$/cgroup)
CG="/sys/fs/cgroup${CG_REL}"

trap 'printf "\nSIGINT recibido: se interrumpe el workload y se conserva la evidencia.\n"' INT

snapshot() {
    local label="$1"

    printf '\n=== %s ===\n' "$label"

    printf '\n--- memory.current ---\n'
    cat "$CG/memory.current"

    for f in memory.high memory.max memory.swap.max; do
        printf '\n--- %s ---\n' "$f"
        cat "$CG/$f"
    done

    printf '\n--- memory.events ---\n'
    cat "$CG/memory.events"
}

printf 'PID shell: %s\n' "$$"
printf 'cgroup: %s\n' "$CG_REL"

snapshot "ANTES"

printf '\n=== WORKLOAD ===\n'
./mem_work "$TARGET_MIB" "$PAUSE_MS"
status=$?

printf '\nexit_status=%d\n' "$status"

snapshot "DESPUÉS"

exit 0
