import subprocess
import statistics
import csv

REPETICIONES = 10
TAMANOS_PARTE_E = [500, 1000, 2000, 4000]
TAMANOS_PARTE_G = [100, 250, 500, 1000, 2000, 4000]


def correr_binario(binario, n, repeticiones=REPETICIONES):
    """Corre el binario 'repeticiones' veces con tamaño n y devuelve lista de dicts parseados."""
    resultados = []
    for _ in range(repeticiones):
        salida = subprocess.run(
            [f"./{binario}", str(n)],
            capture_output=True, text=True, check=True
        ).stdout.strip()

        partes = salida.split(",")
        datos = dict(zip(partes[0::2], partes[1::2]))
        resultados.append({k: float(v) for k, v in datos.items()})
    return resultados


def medianas_mainlab2(n):
    corridas = correr_binario("mainLab2", n)
    filas = statistics.median(c["filas_us"] for c in corridas)
    columnas = statistics.median(c["columnas_us"] for c in corridas)
    return filas, columnas


def experimento_tamanos(tamanos, nombre_csv):
    filas_csv = []
    print(f"\n=== {nombre_csv} ===")
    for n in tamanos:
        print(f"Probando N = {n} ({REPETICIONES} repeticiones)...")
        filas_us, columnas_us = medianas_mainlab2(n)
        filas_ms = filas_us / 1000.0
        columnas_ms = columnas_us / 1000.0
        R = columnas_ms / filas_ms
        print(f"  Filas: {filas_ms:.4f} ms | Columnas: {columnas_ms:.4f} ms | R = {R:.3f}")
        filas_csv.append({"N": n, "filas_ms": filas_ms, "columnas_ms": columnas_ms, "R": R})

    with open(nombre_csv, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["N", "filas_ms", "columnas_ms", "R"])
        writer.writeheader()
        writer.writerows(filas_csv)
    print(f"Resultados guardados en {nombre_csv}")
    return filas_csv


def experimento_localidad_temporal(n=1000000):
    print(f"\n=== Parte H: Localidad temporal (N = {n}) ===")
    corridas = correr_binario("localidadTemporal", n)
    reutilizado = statistics.median(c["reutilizado_us"] for c in corridas)
    separado = statistics.median(c["separado_us"] for c in corridas)
    print(f"  Reutilizado (3 sumas seguidas): {reutilizado:.2f} us")
    print(f"  Separado (3 pasadas): {separado:.2f} us")

    with open("resultados_parte_h.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["N", "reutilizado_us", "separado_us"])
        writer.writerow([n, reutilizado, separado])
    print("Resultados guardados en resultados_parte_h.csv")


if __name__ == "__main__":
    experimento_tamanos(TAMANOS_PARTE_E, "resultados_parte_e.csv")
    experimento_tamanos(TAMANOS_PARTE_G, "resultados_parte_g.csv")
    experimento_localidad_temporal()
