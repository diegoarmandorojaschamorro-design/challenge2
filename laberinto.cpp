#include <algorithm>
#include <chrono>
#include <clocale>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

using std::cout;
using std::pair;
using std::string;
using std::vector;

// Estructura que modela el laberinto completo.
// "struct" es similar a clase, pero con miembros publicos por defecto.
struct Laberinto {
    // Cantidad de filas del tablero.
    int filas;
    // Cantidad de columnas del tablero.
    int columnas;
    // Matriz de celdas:
    // vector<string> = arreglo dinamico de filas (cada fila es un string).
    vector<string> grilla;

    // Constructor:
    // - f y c son dimensiones.
    // - grilla(f, string(c, '#')) crea f filas de longitud c llenas con '#'.
    //   '#' representa muro.
    Laberinto(int f, int c) : filas(f), columnas(c), grilla(f, string(c, '#')) {}
};

// Devuelve true si (f,c) esta dentro de [0..filas-1] y [0..columnas-1].
bool dentroLimites(int f, int c, int filas, int columnas) {
    return f >= 0 && f < filas && c >= 0 && c < columnas;
}

// Devuelve las 4 direcciones vecinas en orden aleatorio.
// "std::mt19937 &generador":
// - mt19937: motor de numeros pseudoaleatorios.
// - &: referencia (no copia el objeto, mas eficiente).
vector<pair<int, int>> vecinosAleatorios(std::mt19937 &generador) {
    // pair<int,int> guarda (deltaFila, deltaColumna).
    vector<pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    // Mezcla el orden para hacer la generacion distinta en cada corrida.
    std::shuffle(dirs.begin(), dirs.end(), generador);
    return dirs;
}

// Cuenta vecinos abiertos ('*') alrededor de la celda (f,c).
// "const Laberinto &lab":
// - const: no se puede modificar el laberinto dentro de esta funcion.
// - &: evita copiar todo el laberinto.
int contarVecinosAbiertos(const Laberinto &lab, int f, int c) {
    // "static const":
    // - static: se inicializa una sola vez y se reutiliza.
    // - const: no cambia durante el programa.
    // Son los desplazamientos de las 4 direcciones.
    static const int df[4] = {1, -1, 0, 0};
    static const int dc[4] = {0, 0, 1, -1};

    int abiertos = 0;
    for (int i = 0; i < 4; ++i) {
        int nf = f + df[i];
        int nc = c + dc[i];
        if (dentroLimites(nf, nc, lab.filas, lab.columnas) && lab.grilla[nf][nc] == '*') {
            ++abiertos;
        }
    }
    return abiertos;
}

// Generacion aleatoria del laberinto con DFS (Depth First Search) + poda.
void tallarLaberintoDFS(Laberinto &lab, int f, int c, std::mt19937 &generador) {
    // Abre la celda actual.
    lab.grilla[f][c] = '*';

    // Recorre vecinos en orden aleatorio.
    // "const auto &d": referencia constante al pair para no copiar.
    for (const auto &d : vecinosAleatorios(generador)) {
        int nf = f + d.first;
        int nc = c + d.second;
        // Solo abre si:
        // 1) esta dentro de limites
        // 2) sigue siendo muro
        // 3) no crea demasiada conexion local
        if (dentroLimites(nf, nc, lab.filas, lab.columnas) &&
            lab.grilla[nf][nc] == '#' &&
            contarVecinosAbiertos(lab, nf, nc) <= 1) {
            // Llamada recursiva (la funcion se llama a si misma).
            tallarLaberintoDFS(lab, nf, nc, generador);
        }
    }
}

// Refuerzo para garantizar al menos un camino de entrada a salida.
void tallarCaminoGarantizado(Laberinto &lab, std::mt19937 &generador) {
    int f = 0, c = 0;   // entrada
    lab.grilla[f][c] = '*';

    // Repite hasta llegar a la salida (abajo-derecha).
    while (f != lab.filas - 1 || c != lab.columnas - 1) {
        bool puedeBajar = f < lab.filas - 1;
        bool puedeDerecha = c < lab.columnas - 1;

        if (puedeBajar && puedeDerecha) {
            // Elige al azar entre bajar o derecha.
            (std::uniform_int_distribution<int>(0, 1)(generador) == 0) ? ++f : ++c;
        } else if (puedeBajar) {
            ++f;
        } else {
            ++c;
        }
        lab.grilla[f][c] = '*';
    }
}

// Resuelve por backtracking:
// intenta caminos; si una rama falla, retrocede y prueba otra.
bool resolverBacktracking(const Laberinto &lab, int f, int c, vector<vector<bool>> &visitado, vector<pair<int, int>> &camino) {
    // Casos de fallo:
    // - fuera de limites
    // - muro
    // - ya visitado
    if (!dentroLimites(f, c, lab.filas, lab.columnas) || lab.grilla[f][c] == '#' || visitado[f][c]) {
        return false;
    }

    // Marca estado actual.
    visitado[f][c] = true;
    camino.push_back({f, c});

    // Caso base de exito: llego a salida.
    if (f == lab.filas - 1 && c == lab.columnas - 1) {
        return true;
    }

    // Movimientos posibles.
    static const int df[4] = {1, -1, 0, 0};
    static const int dc[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; ++i) {
        if (resolverBacktracking(lab, f + df[i], c + dc[i], visitado, camino)) {
            return true;
        }
    }

    // Ninguna rama funciono: deshacer paso (backtrack).
    camino.pop_back();
    return false;
}

// Imprime el laberinto y opcionalmente superpone el camino solucion.
// El puntero "camino" es opcional:
// - nullptr => no dibuja solucion
// - no-null => dibuja solucion
void imprimeLaberinto(const Laberinto &lab, const vector<pair<int, int>> *camino = nullptr) {
    // Copia visual para no tocar la grilla original.
    vector<string> vista = lab.grilla;

    if (camino != nullptr) {
        for (const auto &p : *camino) {
            // Evita sobrescribir entrada y salida.
            if (!(p.first == 0 && p.second == 0) &&
                !(p.first == lab.filas - 1 && p.second == lab.columnas - 1)) {
                vista[p.first][p.second] = '.';
            }
        }
    }

    // Marca entrada y salida para mostrar.
    vista[0][0] = 'E';
    vista[lab.filas - 1][lab.columnas - 1] = 'S';

    // Recorre fila por fila y celda por celda.
    for (const auto &fila : vista) {
        for (char celda : fila) {
            if (celda == '#') cout << u8"🧱";
            else if (celda == '*') cout << u8"⬜";
            else if (celda == '.') cout << u8"🟨";
            else if (celda == 'E') cout << u8"🚪";
            else if (celda == 'S') cout << u8"🏁";
            else cout << u8"❓";
        }
        cout << '\n';
    }
}

// Convierte texto a entero positivo validado.
// Si no puede convertir, devuelve porDefecto.
int parsearPositivo(const char *texto, int porDefecto) {
    if (texto == nullptr) return porDefecto;

    char *fin = nullptr;
    long valor = std::strtol(texto, &fin, 10);
    if (*texto == '\0' || *fin != '\0' || valor <= 0 || valor > 1000) {
        return porDefecto;
    }
    // static_cast<int>: conversion explicita y segura de long a int.
    return static_cast<int>(valor);
}

#ifdef _WIN32
// Configura consola de Windows para UTF-8 (necesario para emojis).
void configurarConsolaUtf8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::setlocale(LC_ALL, ".UTF-8");
}
#endif

int main(int argc, char **argv) {
    // Tamaño por defecto del laberinto.
    int filas = 10;
    int columnas = 10;
    int cantidadNumeros = 0;

#ifdef _WIN32
    configurarConsolaUtf8();
#endif

    // Lee argumentos:
    // primer numero valido => filas
    // segundo numero valido => columnas
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--emoji") continue;

        int valor = parsearPositivo(argv[i], -1);
        if (valor != -1) {
            if (cantidadNumeros == 0) filas = valor;
            else if (cantidadNumeros == 1) columnas = valor;
            ++cantidadNumeros;
        }
    }

    // Inicializa generador aleatorio.
    std::random_device semilla;
    std::mt19937 generador(semilla());
    Laberinto lab(filas, columnas);

    // Cronometra generacion.
    auto inicioGen = std::chrono::high_resolution_clock::now();
    tallarLaberintoDFS(lab, 0, 0, generador);
    tallarCaminoGarantizado(lab, generador);
    auto finGen = std::chrono::high_resolution_clock::now();

    // Estructuras para resolver.
    vector<vector<bool>> visitado(filas, vector<bool>(columnas, false));
    vector<pair<int, int>> camino;

    // Cronometra resolucion.
    auto inicioSol = std::chrono::high_resolution_clock::now();
    bool resuelto = resolverBacktracking(lab, 0, 0, visitado, camino);
    auto finSol = std::chrono::high_resolution_clock::now();

    // Convierte tiempos a microsegundos.
    auto tiempoGen = std::chrono::duration_cast<std::chrono::microseconds>(finGen - inicioGen).count();
    auto tiempoSol = std::chrono::duration_cast<std::chrono::microseconds>(finSol - inicioSol).count();

    // Muestra laberinto y leyenda.
    cout << "Laberinto generado (" << filas << "x" << columnas << "):\n";
    cout << u8"Leyenda: 🧱 muro | ⬜ camino | 🚪 entrada | 🏁 salida | 🟨 solucion\n";
    imprimeLaberinto(lab);
    cout << '\n';

    // Muestra solucion si existe.
    if (resuelto) {
        cout << "Laberinto resuelto con Backtracking:\n";
        imprimeLaberinto(lab, &camino);
    } else {
        cout << "No se encontro camino desde la entrada hasta la salida.\n";
    }

    cout << "\nTiempo de generacion: " << tiempoGen << " microsegundos\n";
    cout << "Tiempo de resolucion: " << tiempoSol << " microsegundos\n";
    return 0;
}
