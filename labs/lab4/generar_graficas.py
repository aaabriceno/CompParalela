#!/usr/bin/env python3
"""Calcula speedup/eficiencia y genera tablas+graficas a partir de los CSV
de resultados_matriz.csv y resultados_oddeven.csv. Corre 100% local, sin internet.
Uso: python3 generar_graficas.py
"""
import pandas as pd
import matplotlib.pyplot as plt
import os

SALIDA = "graficas_resultados"
os.makedirs(SALIDA, exist_ok=True)

def calcular_speedup_eficiencia(df):
    filas = []
    for N in sorted(df["N"].unique()):
        sub = df[df["N"] == N]
        seq = sub[sub["algoritmo"] == "secuencial"]
        if seq.empty:
            continue
        t1 = seq["tiempo_ms"].values[0]
        mpi = sub[sub["algoritmo"] == "mpi"].sort_values("procesos")
        for _, row in mpi.iterrows():
            p = row["procesos"]
            tp = row["tiempo_ms"]
            speedup = t1 / tp if tp > 0 else float("nan")
            eficiencia = speedup / p
            filas.append({
                "N": N, "procesos": p, "nodos": row["nodos"],
                "T1_ms": t1, "Tp_ms": tp,
                "speedup": speedup, "eficiencia": eficiencia
            })
    return pd.DataFrame(filas)

def tabla_txt(df_calc, titulo):
    lineas = [f"\n=== {titulo} ===\n"]
    for N in sorted(df_calc["N"].unique()):
        sub = df_calc[df_calc["N"] == N]
        t1 = sub["T1_ms"].values[0]
        lineas.append(f"N = {N}  (T1 secuencial = {t1:.4f} ms)")
        lineas.append(f"{'procesos':>9} {'nodos':>6} {'Tp(ms)':>12} {'speedup':>9} {'eficiencia':>11}")
        for _, r in sub.iterrows():
            lineas.append(f"{int(r['procesos']):>9} {int(r['nodos']):>6} {r['Tp_ms']:>12.4f} {r['speedup']:>9.3f} {r['eficiencia']:>11.3f}")
        lineas.append("")
    return "\n".join(lineas)

def graficar(df_calc, nombre_base, titulo_prefijo):
    Ns = sorted(df_calc["N"].unique())

    # Procesos vs Tiempo
    plt.figure(figsize=(7,5))
    for N in Ns:
        sub = df_calc[df_calc["N"] == N].sort_values("procesos")
        plt.plot(sub["procesos"], sub["Tp_ms"], marker="o", label=f"N={N}")
    plt.xlabel("Número de procesos")
    plt.ylabel("Tiempo (ms)")
    plt.title(f"{titulo_prefijo}: Procesos vs Tiempo")
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f"{SALIDA}/{nombre_base}_tiempo.png", dpi=120)
    plt.close()

    # Procesos vs Speedup
    plt.figure(figsize=(7,5))
    for N in Ns:
        sub = df_calc[df_calc["N"] == N].sort_values("procesos")
        plt.plot(sub["procesos"], sub["speedup"], marker="o", label=f"N={N}")
    max_p = df_calc["procesos"].max()
    plt.plot([1, max_p], [1, max_p], "k--", alpha=0.4, label="Speedup ideal")
    plt.xlabel("Número de procesos")
    plt.ylabel("Speedup")
    plt.title(f"{titulo_prefijo}: Procesos vs Speedup")
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f"{SALIDA}/{nombre_base}_speedup.png", dpi=120)
    plt.close()

    # Procesos vs Eficiencia
    plt.figure(figsize=(7,5))
    for N in Ns:
        sub = df_calc[df_calc["N"] == N].sort_values("procesos")
        plt.plot(sub["procesos"], sub["eficiencia"], marker="o", label=f"N={N}")
    plt.axhline(1.0, color="k", linestyle="--", alpha=0.4, label="Eficiencia ideal")
    plt.xlabel("Número de procesos")
    plt.ylabel("Eficiencia")
    plt.title(f"{titulo_prefijo}: Procesos vs Eficiencia")
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f"{SALIDA}/{nombre_base}_eficiencia.png", dpi=120)
    plt.close()

def procesar(csv_path, nombre_base, titulo):
    df = pd.read_csv(csv_path, skipinitialspace=True)
    df.columns = df.columns.str.strip()
    for col in ["algoritmo"]:
        if col in df.columns:
            df[col] = df[col].str.strip()
    calc = calcular_speedup_eficiencia(df)
    calc.to_csv(f"{SALIDA}/{nombre_base}_calculado.csv", index=False)
    print(tabla_txt(calc, titulo))
    graficar(calc, nombre_base, titulo)
    print(f"Graficas guardadas en {SALIDA}/{nombre_base}_*.png")
    return calc

if __name__ == "__main__":
    calc_matriz = procesar("resultados_matriz.csv", "matriz", "Matriz-Vector")
    calc_oddeven = procesar("resultados_oddeven.csv", "oddeven", "Odd-Even Sort")

    with open(f"{SALIDA}/tablas.txt", "w") as f:
        f.write(tabla_txt(calc_matriz, "Matriz-Vector"))
        f.write(tabla_txt(calc_oddeven, "Odd-Even Sort"))

    print(f"\nTodo listo en la carpeta: {SALIDA}/")
