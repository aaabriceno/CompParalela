#!/usr/bin/env python3
"""Genera un diagrama de la arquitectura del clúster: 1 nodo host (master)
+ 4 nodos worker (VMs), conectados por red Host-Only de VirtualBox."""
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

fig, ax = plt.subplots(figsize=(11, 8))
ax.set_xlim(0, 10)
ax.set_ylim(0, 8)
ax.axis("off")

COLOR_HOST = "#2c5f8a"
COLOR_WORKER = "#3d8b5f"
COLOR_RED = "#555555"
COLOR_TEXT = "white"

def caja(ax, x, y, w, h, color, titulo, lineas, fontsize_titulo=11, fontsize_lineas=9):
    box = FancyBboxPatch((x, y), w, h,
                          boxstyle="round,pad=0.05,rounding_size=0.08",
                          linewidth=1.5, edgecolor="black", facecolor=color)
    ax.add_patch(box)
    ax.text(x + w/2, y + h - 0.28, titulo, ha="center", va="top",
             fontsize=fontsize_titulo, fontweight="bold", color=COLOR_TEXT)
    for i, linea in enumerate(lineas):
        ax.text(x + w/2, y + h - 0.62 - i*0.32, linea, ha="center", va="top",
                 fontsize=fontsize_lineas, color=COLOR_TEXT)

# --- Nodo maestro (PC host) ---
caja(ax, 3.5, 6.1, 3.0, 1.75, COLOR_HOST, "PC HOST (Maestro)",
     ["AnthonyDesktop", "Lanza mpirun", "Servidor NFS", "192.168.56.1"])

# --- Red Host-Only ---
ax.text(5, 5.55, "Red Host-Only (VirtualBox)\nvboxnet0 — 192.168.56.0/24",
        ha="center", va="center", fontsize=10, style="italic", color=COLOR_RED)
ax.plot([1.2, 8.8], [5.3, 5.3], color=COLOR_RED, linewidth=2, linestyle="--")

# Línea vertical del maestro a la red
ax.add_patch(FancyArrowPatch((5, 6.1), (5, 5.3), arrowstyle="-", color="black", linewidth=1.5))

# --- Nodos worker ---
workers = [
    ("cluster01", "192.168.56.104", "4 cores"),
    ("cluster02", "192.168.56.105", "4 cores"),
    ("cluster03", "192.168.56.106", "4 cores"),
    ("cluster04", "192.168.56.107", "4 cores"),
]

xs = [0.4, 2.9, 5.4, 7.9]
for (nombre, ip, cores), x in zip(workers, xs):
    caja(ax, x, 2.6, 2.0, 1.9, COLOR_WORKER, nombre.upper(),
         ["VM Ubuntu Server", ip, cores, "MPI + NFS client"], fontsize_titulo=10, fontsize_lineas=8.5)
    # línea desde la red hasta el worker
    cx = x + 1.0
    ax.add_patch(FancyArrowPatch((cx, 5.3), (cx, 4.5), arrowstyle="-", color="black", linewidth=1.5))

# --- Leyenda de totales ---
ax.text(5, 1.9, "Total del clúster: 4 nodos worker × 4 cores = 16 cores (slots MPI)",
        ha="center", va="center", fontsize=10.5, fontweight="bold")
ax.text(5, 1.5, "Carpeta compartida vía NFS: /mnt/lab4_paralela (mismo path en los 4 workers)",
        ha="center", va="center", fontsize=9.5, style="italic", color=COLOR_RED)
ax.text(5, 1.1, "mpirun --hostfile hosts.txt -np <p> --mca btl_tcp_if_include 192.168.56.0/24 ...",
        ha="center", va="center", fontsize=8.5, family="monospace", color="#333333")

ax.set_title("Arquitectura del Clúster MPI — Laboratorio 4", fontsize=14, fontweight="bold", pad=15)

plt.tight_layout()
plt.savefig("arquitectura_cluster.png", dpi=150, bbox_inches="tight")
print("Guardado: arquitectura_cluster.png")
