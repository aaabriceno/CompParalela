import csv
import matplotlib.pyplot as plt


def leer_csv(nombre_archivo):
    filas = []
    with open(nombre_archivo, newline="") as f:
        lector = csv.DictReader(f)
        for fila in lector:
            filas.append(fila)
    return filas


def graficar_tiempos_vs_n(nombre_csv, titulo, nombre_salida):
    datos = leer_csv(nombre_csv)

    n_valores = [int(fila["N"]) for fila in datos]
    tiempos_filas = [float(fila["filas_ms"]) for fila in datos]
    tiempos_columnas = [float(fila["columnas_ms"]) for fila in datos]

    plt.figure()
    plt.plot(n_valores, tiempos_filas, marker="o", label="Recorrido por filas")
    plt.plot(n_valores, tiempos_columnas, marker="o", label="Recorrido por columnas")
    plt.xlabel("Tamaño de la matriz (N)")
    plt.ylabel("Tiempo (ms)")
    plt.title(titulo)
    plt.legend()
    plt.grid(True)
    plt.savefig(nombre_salida)
    print(f"Gráfica guardada en {nombre_salida}")


if __name__ == "__main__":
    graficar_tiempos_vs_n(
        "resultados_parte_e.csv",
        "Parte E: Tiempo de acceso por filas vs columnas",
        "grafica_parte_e.png"
    )
    graficar_tiempos_vs_n(
        "resultados_parte_g.csv",
        "Parte G: Influencia del tamaño de la matriz",
        "grafica_parte_g.png"
    )
