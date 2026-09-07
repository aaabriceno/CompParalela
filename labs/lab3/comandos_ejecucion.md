# Comandos de ejecución - Lab 3 MPI Cluster

Arquitectura: PC físico (AnthonyDesktop) = master que lanza mpirun.
Workers: cluster01 (192.168.56.104), cluster02 (192.168.56.105), cluster03 (192.168.56.106)
Cada worker: Ubuntu Server 24.04 LTS, 2 vCPU, Open MPI 4.1.6, red host-only VirtualBox (vboxnet0)

## Compilar (repetir en cada nodo tras copiar el .c)
```bash
mpicc hello_mpi.c -o hello_mpi
```

## Parte B - Local en un nodo
```bash
mpirun -np 4 ~/lab3/hello_mpi
```

## Parte C - Configuraciones distribuidas

Config A (2 procesos, 1 nodo):
```bash
mpirun --hostfile hosts.txt -np 2 --host cluster01:2 --path /home/cluster01/lab3 hello_mpi
```

Config B (4 procesos, 1 nodo, oversubscribe porque solo 2 cores):
```bash
mpirun --hostfile hosts.txt -np 4 --host cluster01:4 --oversubscribe --path /home/cluster01/lab3 hello_mpi
```

Config C (2 procesos, 2 nodos):
```bash
mpirun --hostfile hosts.txt -np 2 --host cluster01,cluster02 --path /home/cluster01/lab3:/home/cluster02/lab3 hello_mpi
```

Config D (4 procesos, 2 nodos):
```bash
mpirun --hostfile hosts.txt -np 4 --path /home/cluster01/lab3:/home/cluster02/lab3:/home/cluster03/lab3 hello_mpi
```

Extra (6 procesos, 3 nodos):
```bash
mpirun --hostfile hosts.txt -np 6 --path /home/cluster01/lab3:/home/cluster02/lab3:/home/cluster03/lab3 hello_mpi
```

## Nota sobre --path
Cada VM tiene un usuario distinto (cluster01, cluster02, cluster03), por eso el
binario vive en una ruta distinta ($HOME/lab3) en cada nodo. --path le dice a mpirun
dónde buscar el ejecutable en cada nodo remoto.
