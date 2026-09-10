# Experimentos de cgroups v2

Material auxiliar para la sesión **Cgroups y administración de recursos**.

## 1. Compilar

```bash
chmod +x build.sh collect_cpu.sh collect_memory.sh
./build.sh
```

## 2. Baseline de CPU

```bash
/usr/bin/time -f 'elapsed=%e s\nuser=%U s\nsys=%S s' \
  ./cpu_work 500000000
```

## 3. CPU con cuota

```bash
sudo systemd-run --scope \
  --unit=cpu-demo \
  -p CPUQuota=50% \
  -p CPUQuotaPeriodSec=100ms \
  bash
```

Dentro de esa shell:

```bash
./collect_cpu.sh 500000000
```

Conservar `cpu.max`, `cpu.stat` antes y después y el tiempo del workload. Salir con `exit`.

## 4. Baseline de memoria

```bash
./mem_work 256 200
```

Debe completar normalmente antes de aplicar límites.

## 5. Experimento A: `memory.high`

Este experimento estudia presión de memoria y reclaim. No se configura un límite duro deliberadamente.

```bash
sudo systemd-run --scope \
  --unit=mem-high-demo \
  -p MemoryHigh=128M \
  bash
```

Dentro de esa shell:

```bash
./collect_memory.sh 160 200
```

Si el proceso reduce mucho su velocidad al superar el umbral, dejarlo algunos segundos para acumular evidencia y presionar `Ctrl+C`. El script registrará el estado posterior. Interesan especialmente `memory.current` y el contador `high` de `memory.events`.

Salir con:

```bash
exit
```

## 6. Experimento B: `memory.max`

Este experimento estudia el límite duro y OOM. No se configura `MemoryHigh`, para evitar que el throttling del experimento anterior impida alcanzar el límite duro.

```bash
sudo systemd-run --scope \
  --unit=mem-max-demo \
  -p MemoryMax=192M \
  -p MemorySwapMax=0 \
  bash
```

Dentro de esa shell:

```bash
./collect_memory.sh 256 200
```

Conservar `memory.max`, `memory.swap.max`, `memory.events` antes y después y la última cantidad impresa por `mem_work`. Interesan especialmente `max`, `oom` y `oom_kill`.

Salir con:

```bash
exit
```

## 7. Relación entre PID y cgroup

Dentro de cualquiera de las shells:

```bash
cat /proc/$$/cgroup
```

Para obtener la ruta absoluta:

```bash
CG=/sys/fs/cgroup$(awk -F: '$1=="0" {print $3}' /proc/$$/cgroup)
echo "$CG"
```

Los valores experimentales deben registrarse desde la VM utilizada para la demostración y no sustituirse por valores inventados.
