# Comandos de ejecución - Lab 3 MPI Cluster

## Arquitectura

- Master: PC físico que lanza `mpirun` y coordina la ejecución.
- Workers: 3 máquinas virtuales (cluster01, cluster02, cluster03) sobre VirtualBox.
- Red: adaptador host-only de VirtualBox (vboxnet0), rango 192.168.56.0/24, exclusivo para el cluster (no tiene salida a internet).
- Cada worker: Ubuntu Server 24.04 LTS, 2 vCPU, Open MPI 4.1.6.

## 1. Instalación de Open MPI (en cada nodo: master y workers)

```bash
sudo apt update
sudo apt install openmpi-bin libopenmpi-dev
mpicc --version
mpirun --version
```

## 2. Configuración de red (host-only, en cada VM worker)

VirtualBox usa dos adaptadores por VM:
- Adaptador 1 (NAT): salida a internet.
- Adaptador 2 (host-only, vboxnet0): red interna del cluster.

Editar netplan para habilitar DHCP en la interfaz del adaptador 2:

```bash
sudo nano /etc/netplan/50-cloud-init.yaml
```

Contenido:
```yaml
network:
  version: 2
  ethernets:
    enp0s3:
      dhcp4: true
    enp0s8:
      dhcp4: true
```

Aplicar cambios y verificar IP asignada:
```bash
sudo netplan apply
ip a
```

## 3. Instalación y activación de SSH (en cada nodo)

```bash
sudo apt install openssh-server openssh-client
sudo systemctl start ssh
sudo systemctl enable ssh
```

## 4. SSH sin contraseña (desde el master hacia cada worker)

Generar par de llaves en el master:
```bash
ssh-keygen -t rsa -N ""
```

Copiar la llave pública a cada worker:
```bash
ssh-copy-id usuario@ip_worker1
ssh-copy-id usuario@ip_worker2
ssh-copy-id usuario@ip_worker3
```

Verificar acceso sin contraseña:
```bash
ssh worker1 hostname
ssh worker2 hostname
ssh worker3 hostname
```

## 5. Resolución de nombres (en el master, /etc/hosts)

```
ip_worker1 cluster01
ip_worker2 cluster02
ip_worker3 cluster03
```

## 6. Hostfile para mpirun (en el master, hosts.txt)

```
cluster01 slots=2
cluster02 slots=2
cluster03 slots=2
```

## 7. Verificación de conectividad

Desde el master hacia cada worker:
```bash
ping -c 3 cluster01
ping -c 3 cluster02
ping -c 3 cluster03
```

Entre workers (para confirmar que también se ven entre sí, no solo con el master):
```bash
ssh cluster01 "ping -c 3 ip_cluster02"
```

## 8. Compilar (en el master; el binario se copia luego a los workers)

```bash
mpicc hello_mpi.c -o hello_mpi
mpicc comunicacion_mpi.c -o comunicacion_mpi
mpicc memoria_mpi.c -o memoria_mpi
```

Copiar binario a cada worker (mismo directorio relativo que en el master):
```bash
scp hello_mpi cluster01:~/lab3/
scp hello_mpi cluster02:~/lab3/
scp hello_mpi cluster03:~/lab3/
```

## Parte B - Ejecución local en un único nodo

```bash
mpirun -np 4 ~/lab3/hello_mpi
```

## Parte C - Ejecución distribuida

Config A (2 procesos, 1 nodo):
```bash
mpirun --hostfile hosts.txt -np 2 --host cluster01:2 --path ~/lab3 hello_mpi
```

Config B (4 procesos, 1 nodo, oversubscribe porque el nodo solo tiene 2 cores):
```bash
mpirun --hostfile hosts.txt -np 4 --host cluster01:4 --oversubscribe --path ~/lab3 hello_mpi
```

Config C (2 procesos, 2 nodos):
```bash
mpirun --hostfile hosts.txt -np 2 --host cluster01:1,cluster02:1 --path ~/lab3 hello_mpi
```

Config D (4 procesos, 2 nodos):
```bash
mpirun --hostfile hosts.txt -np 4 --path ~/lab3 hello_mpi
```

Config E (3 procesos, 3 nodos):
```bash
mpirun --hostfile hosts.txt -np 3 --path ~/lab3 hello_mpi
```

Config F (6 procesos, 3 nodos):
```bash
mpirun --hostfile hosts.txt -np 6 --path ~/lab3 hello_mpi
```

Medición de tiempo (agregar `time` antes de cualquier comando anterior):
```bash
time mpirun --hostfile hosts.txt -np 4 --path ~/lab3 hello_mpi
```

## Parte D - Comunicación entre procesos (Send/Recv)

En un mismo nodo:
```bash
mpirun --hostfile hosts.txt -np 2 --host cluster01:2 --path ~/lab3 comunicacion_mpi
```

En nodos diferentes:
```bash
mpirun --hostfile hosts.txt -np 2 --host cluster01:1,cluster02:1 --path ~/lab3 comunicacion_mpi
```

## Parte E - Observación de memoria distribuida

En un mismo nodo:
```bash
mpirun -np 2 ~/lab3/memoria_mpi
```

En nodos diferentes:
```bash
mpirun --hostfile hosts.txt -np 2 --host cluster01:1,cluster02:1 --path ~/lab3 memoria_mpi
```

## Nota sobre --path

Cada worker puede tener un usuario distinto, por lo que el binario vive en una
ruta distinta (`$HOME/lab3`) en cada nodo. La opción `--path` le indica a `mpirun`
dónde buscar el ejecutable en cada nodo remoto.
