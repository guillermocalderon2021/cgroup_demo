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

Este experimento estudia el comportamiento de un cgroup cuando alcanza un límite duro de memoria. No se configura `MemoryHigh`, para evitar que el throttling y el reclaim asociados a ese umbral dificulten alcanzar `memory.max`.

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

El workload reserva y utiliza memoria en incrementos de 4 MiB. Como `MemoryMax=192M` limita al cgroup completo y no únicamente a `mem_work`, el proceso puede ser terminado antes de imprimir `allocated=192 MiB`.

Si aparece un mensaje similar a:

```text
allocated=188 MiB
Terminado                 ./collect_memory.sh 256 200
```

**no salir todavía de la shell del cgroup**. El OOM killer puede terminar el script antes de que este alcance a imprimir el bloque `DESPUÉS`.

Obtener inmediatamente la ruta del cgroup:

```bash
CG=/sys/fs/cgroup$(awk -F: '$1=="0" {print $3}' /proc/$$/cgroup)
```

y consultar la evidencia restante:

```bash
cat "$CG/memory.events"
cat "$CG/memory.current"
cat "$CG/memory.peak"
```

Para interpretar el resultado interesan especialmente los siguientes campos de `memory.events`:

* `max`: número de veces que el cgroup intentó superar `memory.max`;
* `oom`: situaciones de Out Of Memory producidas al alcanzar el límite;
* `oom_kill`: procesos terminados por el OOM killer.

`memory.peak` permite observar el mayor consumo de memoria alcanzado por el cgroup durante la ejecución.

Un resultado con:

```text
max       > 0
oom       > 0
oom_kill  > 0
```

constituye evidencia de que el límite fue alcanzado y que el OOM killer terminó al menos un proceso del cgroup.

Una vez registrada la evidencia, salir de la shell:

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
