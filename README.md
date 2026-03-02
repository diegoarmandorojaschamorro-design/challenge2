# Laberinto en C++ (Backtracking)

## Como se genera el laberinto
- Se crea una grilla de muros.
- Se abre camino aleatoriamente con DFS desde la entrada `(0,0)`.
- Se garantiza conexion hasta la salida `(filas-1, columnas-1)` para que siempre exista al menos una solucion.

## Algoritmo para resolver
- Se usa **Backtracking recursivo**.
- Desde la entrada, prueba moverse en 4 direcciones.
- Marca celdas visitadas para no repetir.
- Cuando llega a la salida, guarda el camino solucion.

## Ejecucion
Compilar:
```powershell
& "C:\Users\User\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\g++.exe" -std=c++17 -O2 -Wall -Wextra -pedantic .\laberinto.cpp -o .\laberinto.exe
```

Ejecutar con tamaño por defecto (**10x10**):
```powershell
.\laberinto.exe
```

Ejecutar con tamaño personalizado (por ejemplo **15 filas y 20 columnas**):
```powershell
.\laberinto.exe 15 20
```

Si clonaste el repo y no tenes `laberinto.exe`:
```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -static -static-libstdc++ -static-libgcc .\laberinto.cpp -o .\laberinto.exe
.\laberinto.exe
.\laberinto.exe 15 20
```

## Tiempos y observaciones
- El programa imprime tiempo de generacion y resolucion en microsegundos al final.
- En pruebas, la resolucion suele ser muy rapida para tamaños chicos y medianos.

## Que haria distinto la proxima vez
- Implementar un generador de laberinto perfecto (sin forzar un camino final) para controlar mejor la forma del laberinto.
- Comparar Backtracking vs BFS y reportar diferencias de tiempo por tamaño.
