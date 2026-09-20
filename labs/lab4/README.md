# Laboratorio 4 — MPI: Matriz-Vector y Odd-Even Sort

Guía completa: cómo armar el clúster desde cero, compilar, ejecutar, y dónde
encontrar cada resultado que pide el PDF del laboratorio.

## Estructura del proyecto

```
lab4/
├── hosts.txt                          # hostfile para mpirun (4 nodos)
├── ejecutar_experimentos.sh           # automatiza todas las corridas
├── generar_graficas.py                # calcula speedup/eficiencia y arma las gráficas
├── generar_diagrama.py                # genera arquitectura_cluster.png
├── resultados_matriz.csv              # tiempos promedio matriz-vector
├── resultados_oddeven.csv             # tiempos promedio odd-even
├── arquitectura_cluster.png           # diagrama del clúster (host + 4 workers)
├── graficas_resultados/               # tablas y gráficas de Speedup/Eficiencia
│   ├── matriz_tiempo.png / _speedup.png / _eficiencia.png
│   ├── oddeven_tiempo.png / _speedup.png / _eficiencia.png
│   ├── parteH_escalabilidad.png / .csv     # Parte H (escalabilidad, N fijo)
│   ├── matriz_calculado.csv / oddeven_calculado.csv
│   └── tablas.txt
├── InformeLab4.odt / TareaLab4.odt    # informe final
├── MPI_Cheatsheet.md                  # repaso de comandos MPI
├── mulMatrices/
│   ├── mulMatrizSecuencial.cpp
│   ├── mulMatrizMPI.cpp
├── ordenamientoOddEven/
│   ├── ordenamientoSecuencial.cpp
│   ├── ordenamientoMPI.cpp
```

Todo esto vive también dentro de la carpeta compartida por NFS (ver más abajo),
por lo que compilar en un solo lugar alcanza para que los 4 clusters vean el
mismo binario sin copiar nada.

---

## 1. Arquitectura del clúster

- **PC host** (`AnthonyDesktop`, usuario `anthonybq`): servidor NFS, lanza
  `mpirun`, NO participa como nodo de cómputo.
- **4 VMs VirtualBox** (workers): `cluster01`, `cluster02`, `cluster03`,
  `cluster04`. Cada una con su propio usuario Linux (`cluster01`, `cluster02`,
  etc.), Ubuntu Server.
- **Red**: adaptador Host-Only de VirtualBox (`vboxnet0`), rango
  `192.168.56.0/24`. Cada VM tiene ADEMÁS un adaptador NAT (para salida a
  internet), que **no se usa** para las comunicaciones MPI (ver sección de
  problemas conocidos, al final).

| Nodo | IP | Usuario | Cores (slots) |
|---|---|---|---|
| cluster01 | 192.168.56.104 | cluster01 | 4 |
| cluster02 | 192.168.56.105 | cluster02 | 4 |
| cluster03 | 192.168.56.106 | cluster03 | 4 |
| cluster04 | 192.168.56.107 | cluster04 | 4 |
| **Total** | | | **16 slots** |

---

## 2. Cómo crear un nuevo cluster desde cero (VM en blanco)

Repetir estos pasos completos por cada nueva VM (ej. para agregar un cluster05).

### 2.1 — Requisitos de la VM en VirtualBox
- Ubuntu Server (24.04 usado en este lab).
- **2 adaptadores de red**:
  - Adaptador 1: NAT (para `apt install`, salida a internet).
  - Adaptador 2: Solo-anfitrión (Host-Only), conectado a `vboxnet0`.

### 2.2 — Dentro de la VM: habilitar el segundo adaptador con DHCP

```bash
sudo nano /etc/netplan/50-cloud-init.yaml
```

Contenido (ajustar nombres de interfaz si difieren, verificar con `ip a`):
```yaml
network:
  version: 2
  ethernets:
    enp0s3:
      dhcp4: true
    enp0s8:
      dhcp4: true
```

```bash
sudo netplan apply
ip a   # confirmar que enp0s8 (o el que corresponda) tiene IP 192.168.56.x
```

### 2.3 — Instalar SSH server

```bash
sudo apt update
sudo apt install -y openssh-server
sudo systemctl start ssh
sudo systemctl enable ssh
```

### 2.4 — Instalar MPI + compiladores + cliente NFS

```bash
sudo apt install -y openmpi-bin libopenmpi-dev g++ gcc nfs-common
```

Verificar:
```bash
which mpic++ mpicc g++ gcc mount.nfs4
```

### 2.5 — Desde la PC host: registrar el nuevo nodo

En **la PC** (`anthonybq`), editar `/etc/hosts`:
```bash
echo "192.168.56.XXX clusterNN" | sudo tee -a /etc/hosts
```

Editar `~/.ssh/config` agregando:
```
Host clusterNN
    User clusterNN
```

Copiar la llave SSH (pide password del cluster nuevo una sola vez):
```bash
ssh-copy-id clusterNN
```

**Nota importante**: si la primera vez SSH pregunta
`Are you sure you want to continue connecting (yes/no/[fingerprint])?`,
escribir **`yes`** explícitamente (no dejar vacío) — recién después pide el
password.

Verificar que quedó sin password:
```bash
ssh clusterNN hostname
```

### 2.6 — Montar el NFS compartido en el nuevo nodo

Ver sección 3 completa para el setup NFS del lado servidor. Del lado del
nuevo cliente:
```bash
sudo mkdir -p /mnt/lab4_paralela
sudo mount -t nfs 192.168.56.1:/home/anthonybq/lab4_paralela_nfs /mnt/lab4_paralela
```

Verificar:
```bash
ls /mnt/lab4_paralela/
```
Debe mostrar el mismo contenido que la carpeta `lab4/` de la PC host.

### 2.7 — Agregar el nodo a `hosts.txt`

En la PC host, editar `lab4/hosts.txt`:
```
cluster01 slots=4
cluster02 slots=4
cluster03 slots=4
cluster04 slots=4
clusterNN slots=N   <- agregar esta línea
```

`slots=N` = número de cores reales de esa VM (ver con `nproc` dentro de la VM).

---

## 3. Configuración de NFS (Network File System)

Permite compilar **una sola vez** y que los 4 (o más) clusters vean el mismo
binario automáticamente, sin `scp` manual.

### 3.1 — Servidor NFS (en la PC host, `anthonybq`)

El path real del proyecto (`.../Semestre 2026-2/Paralela/labs/lab4`) tiene un
espacio en el nombre, lo cual complica `/etc/exports`. Solución: un **bind
mount** hacia un path sin espacios.

```bash
sudo mkdir -p /home/anthonybq/lab4_paralela_nfs
sudo mount --bind "/home/anthonybq/Documentos/Semestre 2026-2/Paralela/labs/lab4" /home/anthonybq/lab4_paralela_nfs
```

Para que el bind mount sobreviva un reinicio de la PC, agregar a
`/etc/fstab` (única vez que hay que escapar el espacio, como `\040`):
```bash
echo '/home/anthonybq/Documentos/Semestre\0402026-2/Paralela/labs/lab4 /home/anthonybq/lab4_paralela_nfs none bind 0 0' | sudo tee -a /etc/fstab
```

Instalar el servidor NFS:
```bash
sudo apt update
sudo apt install -y nfs-kernel-server
```

Exportar la carpeta (path SIN espacios ya, gracias al bind mount):
```bash
echo '/home/anthonybq/lab4_paralela_nfs 192.168.56.0/24(rw,sync,no_subtree_check,no_root_squash)' | sudo tee -a /etc/exports
sudo exportfs -a
sudo systemctl restart nfs-kernel-server
```

**⚠️ Advertencia importante**: `sudo systemctl restart nfs-kernel-server`
**desconecta a todos los clientes ya montados**. Si agregás un nuevo cluster
y reiniciás el servicio NFS, hay que volver a montar en TODOS los clusters
existentes (ver sección "Problemas conocidos" más abajo).

### 3.2 — Cliente NFS (en cada VM cluster)

```bash
sudo mkdir -p /mnt/lab4_paralela
sudo mount -t nfs 192.168.56.1:/home/anthonybq/lab4_paralela_nfs /mnt/lab4_paralela
```

`192.168.56.1` es la IP de la PC host en la red Host-Only (`vboxnet0`).

### 3.3 — Verificar que todo está montado (correr desde la PC host)

```bash
for h in cluster01 cluster02 cluster03 cluster04; do
  echo "== $h =="
  ssh $h "mount | grep nfs && echo MONTADO || echo DESMONTADO"
done
```

Si alguno da `DESMONTADO`, repetir el paso 3.2 en esa VM específica.

---

## 4. Compilar los 4 programas

Compilar **una sola vez**, desde la PC host, directamente en la carpeta NFS
(o desde cualquier cluster — es la misma carpeta física):

```bash
cd /home/anthonybq/lab4_paralela_nfs/mulMatrices
g++ -O2 -o mulMatrizSecuencial mulMatrizSecuencial.cpp
mpic++ -O2 -o mulMatrizMPI mulMatrizMPI.cpp

cd /home/anthonybq/lab4_paralela_nfs/ordenamientoOddEven
g++ -O2 -o ordenamientoSecuencial ordenamientoSecuencial.cpp
mpic++ -O2 -o ordenamientoMPI ordenamientoMPI.cpp
```

Los 4 clusters ven los binarios recién compilados automáticamente en
`/mnt/lab4_paralela/...` (no hace falta copiar nada).

---

## 5. Cómo ejecutar cada programa

### 5.1 — Versión secuencial (corre local, sin MPI)

```bash
# Matriz-vector, N=1024 por defecto, o pasar N como argumento
./mulMatrizSecuencial [N] [quiet]

# Ejemplos
./mulMatrizSecuencial              # N=1024, imprime resultado completo
./mulMatrizSecuencial 4096 quiet   # N=4096, NO imprime el vector resultado (solo tiempo)
```

```bash
# Odd-Even sort, N=10000 por defecto
./ordenamientoSecuencial [N] [quiet]
```

### 5.2 — Versión MPI, local (un solo nodo, sin red distribuida)

```bash
mpirun -np <procesos> ./mulMatrizMPI [N] [quiet]
mpirun -np <procesos> ./ordenamientoMPI [N] [quiet]
```

### 5.3 — Versión MPI, distribuida en los 4 clusters (el caso real del lab)

**Comando base** (memorizar esta plantilla, es la que se usa siempre):

```bash
cd "/home/anthonybq/Documentos/Semestre 2026-2/Paralela/labs/lab4"

mpirun --hostfile hosts.txt --map-by node -np <PROCESOS> \
  --mca btl_tcp_if_include 192.168.56.0/24 \
  --mca oob_tcp_if_include 192.168.56.0/24 \
  /mnt/lab4_paralela/mulMatrices/mulMatrizMPI <N> [quiet]
```

Para Odd-Even, cambiar solo el path del ejecutable:
```bash
  /mnt/lab4_paralela/ordenamientoOddEven/ordenamientoMPI <N> [quiet]
```

**¿Por qué `--map-by node`?** Por defecto, Open MPI llena un nodo completo
(según sus `slots`) antes de pasar al siguiente (`--map-by slot`, el
default). Con 4 cores por VM, eso significa que `-np 10` ocupa solo 3 nodos
(4+4+2) y el 4to nunca se activa. `--map-by node` reparte los procesos en
**round-robin entre nodos** en vez de llenarlos uno por uno — con `-np 10`
en 4 nodos da 3+3+2+2, activando los 4 nodos simultáneamente. Usar este
flag siempre que se quiera ver a todos los nodos trabajando a la vez
(relevante para la Parte F del PDF).

**¿Por qué los `--mca`?** Cada VM tiene 2 interfaces de red: la NAT
(`10.0.2.15`, **idéntica en las 4 VMs**) y la Host-Only (`192.168.56.x`,
única por VM). Sin estos flags, Open MPI puede intentar usar la interfaz NAT
duplicada para comunicación entre procesos y el programa se cuelga
(deadlock silencioso, sin mensaje de error). Los flags fuerzan a usar
solamente la red Host-Only, que es la única con IPs distintas entre nodos.

**Antes de cada corrida**, si el proceso anterior no terminó limpio, matar
procesos colgados en los 4 nodos (previene cuelgues intermitentes):
```bash
for h in cluster01 cluster02 cluster03 cluster04; do
  ssh $h "pkill -9 -f mulMatrizMPI; pkill -9 -f ordenamientoMPI"
done
```

### 5.4 — Ejecutar automáticamente TODOS los experimentos

```bash
cd "/home/anthonybq/Documentos/Semestre 2026-2/Paralela/labs/lab4"
bash ejecutar_experimentos.sh
```

Corre matriz-vector y odd-even, con N=(1024,2048,4096) y N=(1000,5000,10000)
respectivamente, cada uno con procesos=(1,2,4,6,8,10), **10 repeticiones por
combinación** (promedia el tiempo para reducir ruido de medición). Guarda
todo en `resultados_matriz.csv` y `resultados_oddeven.csv`.

⚠️ Tarda bastante (son ~10 repeticiones × 7 configuraciones × 3 tamaños × 2
programas = cientos de corridas). Dejarlo correr sin interrumpir.

---

## 6. Formato de los CSV de resultados

```
algoritmo,N,procesos,nodos,tiempo_ms
secuencial,1024,1,1,0.543666
mpi,1024,1,1,0.470988
mpi,1024,2,1,0.253470
mpi,1024,4,2,0.133383
...
```

- `algoritmo`: `secuencial` o `mpi`.
- `N`: tamaño del problema (orden de la matriz, o cantidad de claves a ordenar).
- `procesos`: cantidad de procesos MPI (1 para el secuencial, por convención).
- `nodos`: cantidad de VMs distintas donde corrieron los procesos (útil para
  la Parte F: comparar 1 nodo vs varios nodos).
- `tiempo_ms`: tiempo promedio de 10 corridas, en milisegundos.

### Cómo calcular Speedup y Eficiencia a partir del CSV

```
T1 = tiempo de la fila "secuencial" para ese N
Tp = tiempo de la fila "mpi" con p procesos, mismo N

Speedup(p)    = T1 / Tp
Eficiencia(p) = Speedup(p) / p
```

Ejemplo rápido en Python (o Excel/Sheets con las mismas columnas):
```python
import pandas as pd
df = pd.read_csv("resultados_matriz.csv")
for N in df["N"].unique():
    t1 = df[(df["algoritmo"]=="secuencial") & (df["N"]==N)]["tiempo_ms"].values[0]
    mpi = df[(df["algoritmo"]=="mpi") & (df["N"]==N)]
    for _, row in mpi.iterrows():
        speedup = t1 / row["tiempo_ms"]
        eficiencia = speedup / row["procesos"]
        print(N, row["procesos"], speedup, eficiencia)
```

---

## 7. Mapeo a cada parte del PDF del laboratorio

| Parte del PDF | Qué pide | Dónde está / cómo verlo |
|---|---|---|
| **3.1 / 3.2** — Problemas a resolver | Matriz-vector y Odd-Even | Carpetas `mulMatrices/` y `ordenamientoOddEven/` |
| **Parte A** — Secuencial | Versión sin MPI, medir tiempo, 3+ tamaños | `mulMatrizSecuencial.cpp`, `ordenamientoSecuencial.cpp`. Correr con N=1024/2048/4096 (matriz) o N=1000/5000/10000 (odd-even). Tiempo se imprime en cada corrida (`Tiempo: X microsegundos/milisegundos`). |
| **Parte B** — MPI matriz-vector | Bcast/Scatterv/Gatherv, verificar contra secuencial | `mulMatrizMPI.cpp`. Usa `MPI_Bcast` (vector), `MPI_Scatterv`/`MPI_Gatherv` (filas de la matriz, soporta N no divisible por procesos). |
| **Parte C** — MPI Odd-Even | Sendrecv, comunicación con vecinos | `ordenamientoMPI.cpp`. Usa `Compute_partner` + `MPI_Sendrecv` + `Merge_low`/`Merge_high`, tal como en el libro Pacheco cap. 3. ⚠️ N debe ser divisible por la cantidad de procesos (limitación conocida del algoritmo). |
| **Parte D** — Correctitud | Correr con 1 proceso, luego varios, comparar con secuencial | Correr `mpirun -np 1 ...` y comparar el resultado impreso contra la versión secuencial (sin `quiet`, para ver el array/vector completo). Con N chico (ej. N=8 en matriz, N=16 en odd-even) se puede verificar a mano. |
| **Parte E** — Rendimiento (1,2,4,8,10 procesos) | Medir tiempo con esas cantidades | `hosts.txt` tiene 16 slots totales (4+4+4+4). `ejecutar_experimentos.sh` corre exactamente 1,2,4,6,8,10. Resultados en los CSV. |
| **Parte F** — Ejecución en clúster (nodos) | Registrar qué proceso corrió en qué nodo | Los programas MPI imprimen `Rank X ejecutando en nodo clusterNN` en cada corrida (vía `MPI_Get_processor_name`). La columna `nodos` del CSV cuenta cuántos nodos distintos participaron en cada corrida. |
| **Parte G** — Análisis comparativo | 10 preguntas sobre speedup/comunicación/eficiencia | Responder usando los datos de los CSV + lo observado (ver sección 8, "Hallazgos ya confirmados", más abajo — ahorra tiempo de análisis). |
| **Parte H** — Escalabilidad | 1 algoritmo, N fijo, graficar procesos vs tiempo/speedup/eficiencia | Se usó Matriz-Vector con N=4096 fijo, corrido con 1,2,4,8,10,12,14,16 procesos (el máximo real del clúster con sus 16 cores). Resultados en `graficas_resultados/parteH_escalabilidad.csv` y `parteH_escalabilidad.png`. |
| **Métricas (sección 9)** | Fórmulas Speedup/Eficiencia | Ver sección 6 de este README. |

---

## 8. Hallazgos ya confirmados durante las pruebas (útiles para Parte G)

Estos datos ya se observaron en corridas reales del clúster, documentarlos
directamente en el análisis:

1. **Con N chico y varios nodos, el paralelo puede ser MÁS LENTO que el
   secuencial.** Ejemplo confirmado: Odd-Even con N=1000, 4 procesos
   distribuidos en 2 nodos (cluster01+cluster02) tomó ~10ms, mientras que
   con 1-2 procesos en un solo nodo tomó <0.1-0.3ms. La razón: el costo de
   comunicación por red entre VMs domina completamente sobre el cómputo
   real cuando cada proceso tiene pocos elementos que procesar.

2. **El mismo algoritmo, dentro de un solo nodo, sí escala bien.** Prueba:
   Odd-Even con N=1000 y 4 procesos, TODOS en cluster01 (sin cruzar red), dio
   0.057ms — coherente con el paralelismo esperado. La diferencia brutal
   viene exclusivamente de cruzar la red entre VMs distintas.

3. **Matriz-vector escala mejor que Odd-Even a mayor N.** Con N=4096, el
   speedup con 6 procesos ronda 4.3x (eficiencia ~72%). Odd-Even, al ser
   O(n²) en el ordenamiento local de cada proceso, tiene mucho más cómputo
   por proceso y por eso se beneficia más del paralelismo cuando N es grande
   — pero es más sensible a comunicación cuando N es chico.

4. **Bug corregido durante el desarrollo**: la primera versión de
   `ordenamientoMPI.cpp` usaba `std::sort` (O(n log n)) para el ordenamiento
   local de cada proceso, en vez de `Odd_even_sort` manual (O(n²), el mismo
   algoritmo que la versión secuencial). Esto hacía que la comparación de
   tiempos fuera inválida (comparaba dos algoritmos distintos). Se corrigió
   agregando `Odd_even_sort_local` — ambas versiones (secuencial y MPI)
   usan ahora el mismo algoritmo de ordenamiento base.

---

## 9. Problemas conocidos y cómo resolverlos

### "mpirun se cuelga sin mensaje de error"
Causa: Open MPI eligió la interfaz NAT duplicada en vez de Host-Only.
Solución: siempre usar los flags `--mca btl_tcp_if_include 192.168.56.0/24
--mca oob_tcp_if_include 192.168.56.0/24` (ver sección 5.3).

### "No existe el archivo o directorio" al correr el binario MPI
Causa: el mount de NFS se cayó en ese nodo específico (pasa después de
reiniciar `nfs-kernel-server` en el servidor — desconecta a TODOS los
clientes ya montados, incluso los que no tocaste).
Solución: verificar con la sección 3.3, y remontar el NFS en cada nodo
afectado (sección 3.2).

### "mpirun was unable to launch... could not access executable" con appfile
Si en algún momento se usa un `appfile` (rutas distintas por nodo porque
cada cluster tiene usuario distinto) en vez de `hosts.txt` + NFS: verificar
que cada línea del appfile tiene el path CORRECTO para ESE usuario
específico. Desde que se usa NFS con el mismo path (`/mnt/lab4_paralela/...`)
en los 4 nodos, ya no se necesita appfile.

### "sudo: a password is required" al intentar instalar/configurar remoto
Los clusters no tienen sudo sin password configurado (correcto, por
seguridad). Cualquier paso que empiece con `sudo` en una VM remota hay que
correrlo manualmente DENTRO de esa VM (por consola VirtualBox o SSH
interactivo), no se puede automatizar desde la PC host.

### N no divisible por la cantidad de procesos (Odd-Even)
Limitación conocida y aceptada del algoritmo (igual que el libro Pacheco):
`ordenamientoMPI.cpp` asume `N % comm_sz == 0`. El script
`ejecutar_experimentos.sh` salta automáticamente las combinaciones
inválidas (ej. N=1000 con 6 procesos). Elegir siempre N y cantidades de
proceso que sean múltiplos entre sí (1000/5000/10000 con 1,2,4,5,8,10
funcionan; con 3,6,7,9 no).

### Al agregar un nuevo cluster, los demás pierden el mount NFS
Ya explicado en sección 3.1 — reiniciar el servicio NFS del servidor
desconecta a los clientes existentes. Siempre remontar en TODOS los nodos
después de agregar uno nuevo (sección 3.3 para verificar cuáles quedaron
desmontados).

---

## 10. Comandos de referencia rápida (cheat sheet de esta sesión)

```bash
# Ver estado de todos los mounts NFS
for h in cluster01 cluster02 cluster03 cluster04; do
  echo "== $h =="; ssh $h "mount | grep nfs || echo DESMONTADO"
done

# Matar procesos MPI colgados en todos los nodos
for h in cluster01 cluster02 cluster03 cluster04; do
  ssh $h "pkill -9 -f mulMatrizMPI; pkill -9 -f ordenamientoMPI"
done

# Recompilar todo de una (desde la carpeta NFS, propaga a los 4 nodos)
cd /home/anthonybq/lab4_paralela_nfs/mulMatrices && mpic++ -O2 -o mulMatrizMPI mulMatrizMPI.cpp
cd /home/anthonybq/lab4_paralela_nfs/ordenamientoOddEven && mpic++ -O2 -o ordenamientoMPI ordenamientoMPI.cpp

# Corrida rápida de prueba (matriz, 10 procesos repartidos en los 4 nodos, N=1024)
mpirun --hostfile hosts.txt --map-by node -np 10 \
  --mca btl_tcp_if_include 192.168.56.0/24 --mca oob_tcp_if_include 192.168.56.0/24 \
  /mnt/lab4_paralela/mulMatrices/mulMatrizMPI 1024

# Ver cuántos cores tiene un cluster
ssh clusterNN nproc
```
