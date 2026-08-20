import threading

def tarea_paralela(id):
    print (f"Ejecutando tared {id}")

threads = [threading.Thread(target=tarea_paralela, args=(i,))
           for i in range(4)]

for t in threads:
    t.start()

for t in threads:
    t.join()