#!/bin/bash
# Automatiza las corridas de matriz-vector y odd-even sort (secuencial + MPI)
# Cada combinación se corre REPETICIONES veces y se guarda el promedio en CSV.

set -u

LAB4_MPI="/mnt/lab4_paralela"          # path visto por los clusters (clientes NFS), usado en mpirun
LAB4_LOCAL="/home/anthonybq/lab4_paralela_nfs" # path local en esta PC (servidor NFS), usado para el secuencial
HOSTS="hosts.txt"
MCA_FLAGS="--mca btl_tcp_if_include 192.168.56.0/24 --mca oob_tcp_if_include 192.168.56.0/24"
REPETICIONES=10

CSV_MATRIZ="resultados_matriz.csv"
CSV_ODDEVEN="resultados_oddeven.csv"

TAMANOS_MATRIZ=(1024 2048 4096)
TAMANOS_ODDEVEN=(1000 5000 10000)
PROCESOS=(1 2 4 8 10)   # clúster tiene 16 slots reales (4+4+4+4), 10 cubre lo pedido por el PDF sin oversubscribe

limpiar_procesos_colgados() {
    for h in cluster01 cluster02 cluster03 cluster04; do
        ssh "$h" "pkill -9 -f mulMatrizMPI; pkill -9 -f ordenamientoMPI" >/dev/null 2>&1
    done
    sleep 1
}

# Recibe unidad ("milisegundos"/"microsegundos") y valor, devuelve siempre en milisegundos
normalizar_a_ms() {
    local valor="$1" unidad="$2"
    if [ "$unidad" = "microsegundos" ]; then
        echo "scale=6; $valor / 1000" | bc
    else
        echo "$valor"
    fi
}

# Corre un binario N veces, promedia el "Tiempo: X unidad" de cada corrida, imprime nodos vistos y ms promedio
correr_promedio() {
    local comando="$1"
    local suma=0
    local nodos_vistos=""
    for i in $(seq 1 $REPETICIONES); do
        salida=$(eval timeout 60 "$comando" 2>&1)
        valor=$(echo "$salida" | grep -oP 'Tiempo:\s*\K[0-9.]+')
        unidad=$(echo "$salida" | grep -oP 'Tiempo:\s*[0-9.]+\s*\K\w+')
        ms=$(normalizar_a_ms "$valor" "$unidad")
        suma=$(echo "$suma + $ms" | bc)
        nodos_vistos="$nodos_vistos $(echo "$salida" | grep -oP 'nodo \K\S+')"
    done
    promedio=$(echo "scale=6; $suma / $REPETICIONES" | bc)
    n_nodos=$(echo "$nodos_vistos" | tr ' ' '\n' | sort -u | grep -v '^$' | wc -l)
    echo "$promedio|$n_nodos"
}

echo "===================================================="
echo " Matriz-Vector: secuencial + MPI (promedio de $REPETICIONES corridas)"
echo "===================================================="
echo "algoritmo,N,procesos,nodos,tiempo_ms" > "$CSV_MATRIZ"

for N in "${TAMANOS_MATRIZ[@]}"; do
    echo ">> Secuencial N=$N"
    resultado=$(correr_promedio "\"$LAB4_LOCAL/mulMatrices/mulMatrizSecuencial\" $N quiet")
    tiempo=$(echo "$resultado" | cut -d'|' -f1)
    echo "secuencial,$N,1,1,$tiempo" >> "$CSV_MATRIZ"

    for P in "${PROCESOS[@]}"; do
        limpiar_procesos_colgados
        echo ">> MPI N=$N procesos=$P"
        resultado=$(correr_promedio "mpirun --hostfile \"$HOSTS\" --map-by node -np $P $MCA_FLAGS \"$LAB4_MPI/mulMatrices/mulMatrizMPI\" $N quiet")
        tiempo=$(echo "$resultado" | cut -d'|' -f1)
        nodos=$(echo "$resultado" | cut -d'|' -f2)
        echo "mpi,$N,$P,$nodos,$tiempo" >> "$CSV_MATRIZ"
    done
done

echo ""
echo "===================================================="
echo " Odd-Even Sort: secuencial + MPI (promedio de $REPETICIONES corridas)"
echo "===================================================="
echo "algoritmo,N,procesos,nodos,tiempo_ms" > "$CSV_ODDEVEN"

for N in "${TAMANOS_ODDEVEN[@]}"; do
    echo ">> Secuencial N=$N"
    resultado=$(correr_promedio "\"$LAB4_LOCAL/ordenamientoOddEven/ordenamientoSecuencial\" $N quiet")
    tiempo=$(echo "$resultado" | cut -d'|' -f1)
    echo "secuencial,$N,1,1,$tiempo" >> "$CSV_ODDEVEN"

    for P in "${PROCESOS[@]}"; do
        if [ $((N % P)) -ne 0 ]; then
            echo ">> MPI N=$N procesos=$P -- SALTADO (N no divisible por P)"
            continue
        fi
        limpiar_procesos_colgados
        echo ">> MPI N=$N procesos=$P"
        resultado=$(correr_promedio "mpirun --hostfile \"$HOSTS\" --map-by node -np $P $MCA_FLAGS \"$LAB4_MPI/ordenamientoOddEven/ordenamientoMPI\" $N quiet")
        tiempo=$(echo "$resultado" | cut -d'|' -f1)
        nodos=$(echo "$resultado" | cut -d'|' -f2)
        echo "mpi,$N,$P,$nodos,$tiempo" >> "$CSV_ODDEVEN"
    done
done

echo ""
echo "Listo. Resultados en:"
echo "  $CSV_MATRIZ"
echo "  $CSV_ODDEVEN"
