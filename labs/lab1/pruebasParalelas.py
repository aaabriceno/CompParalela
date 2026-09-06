import subprocess
import pandas as pd
import matplotlib.pyplot as plt

exponentes = [6, 7, 8]
threads_valores = [2, 4, 8, 16, 32]
repeticiones = 10

resultados = []

for exp in exponentes:
    for threads in threads_valores:
        print(f"Probando N=10^{exp} con {threads} threads...")
        tiempos = []
        for _ in range(repeticiones):
            salida = subprocess.run(
                ["./main2"],
                input=f"{exp}\n{threads}\n",
                capture_output=True, text=True
            )
            for linea in salida.stdout.splitlines():
                if "Tiempo transcurrido" in linea:
                    tiempo_us = int(linea.split("=")[1].split("microsegundos")[0].strip())
                    tiempos.append(tiempo_us)

        tiempo_mediana_us = pd.Series(tiempos).median()
        resultados.append({
            "N": 10 ** exp,
            "threads": threads,
            "tiempo_mediana_us": tiempo_mediana_us,
            "tiempo_mediana_s": tiempo_mediana_us / 1_000_000
        })

df = pd.DataFrame(resultados)

df_seq = pd.read_csv("resultados_secuencial.csv")
T1_por_N = dict(zip(df_seq["N"], df_seq["tiempo_mediana_us"]))

df["speedup"] = df.apply(lambda fila: T1_por_N[fila["N"]] / fila["tiempo_mediana_us"], axis=1)
df["eficiencia"] = df["speedup"] / df["threads"]

fila_base = pd.DataFrame([
    {"N": N, "threads": 1, "tiempo_mediana_us": T1_por_N[N],
     "tiempo_mediana_s": T1_por_N[N] / 1_000_000, "speedup": 1.0, "eficiencia": 1.0}
    for N in T1_por_N
])
df = pd.concat([fila_base, df], ignore_index=True).sort_values(["N", "threads"]).reset_index(drop=True)

pd.set_option("display.max_columns", None)
pd.set_option("display.width", None)
print(df)

df.to_csv("resultados_paralelo.csv", index=False)

for N in sorted(df["N"].unique()):
    subset = df[df["N"] == N]
    plt.plot(subset["threads"], subset["speedup"], marker="o", label=f"N=10^{len(str(N))-1}")

plt.plot([1, 32], [1, 32], linestyle="--", color="gray", label="Speedup ideal")
plt.xlabel("Cantidad de threads")
plt.ylabel("Speedup")
plt.title("Speedup en funcion del numero de threads")
plt.legend()
plt.grid(True)
plt.savefig("grafico_speedup.png", dpi=150)
plt.show()
