import subprocess
import pandas as pd

exponentes = [6, 7, 8]
repeticiones = 10

resultados = []

for exp in exponentes:
    print(f"Probando SECUENCIAL N=10^{exp}...")
    tiempos = []
    for _ in range(repeticiones):
        salida = subprocess.run(
            ["./main"],
            input=f"{exp}\n",
            capture_output=True, text=True
        )
        for linea in salida.stdout.splitlines():
            if "Tiempo transcurrido" in linea:
                tiempo_us = int(linea.split("=")[1].split("microsegundos")[0].strip())
                tiempos.append(tiempo_us)

    tiempo_mediana_us = pd.Series(tiempos).median()
    resultados.append({
        "N": 10 ** exp,
        "tiempo_mediana_us": tiempo_mediana_us,
        "tiempo_mediana_s": tiempo_mediana_us / 1_000_000
    })

df_secuencial = pd.DataFrame(resultados)
print(df_secuencial)

df_secuencial.to_csv("resultados_secuencial.csv", index=False)
