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

struct Laberinto {
    int filas;
    int columnas;
    vector<string> grilla;

    Laberinto(int f, int c) : filas(f), columnas(c), grilla(f, string(c, '#')) {}
};

bool dentroLimites(int f, int c, int filas, int columnas) {
    return f >= 0 && f < filas && c >= 0 && c < columnas;
}

vector<pair<int, int>> vecinosAleatorios(std::mt19937 &generador) {
    vector<pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    std::shuffle(dirs.begin(), dirs.end(), generador);
    return dirs;
}

int contarVecinosAbiertos(const Laberinto &lab, int f, int c) {
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

// Generacion aleatoria del laberinto: DFS con poda para no abrir demasiado.
void tallarLaberintoDFS(Laberinto &lab, int f, int c, std::mt19937 &generador) {
    lab.grilla[f][c] = '*';

    for (const auto &d : vecinosAleatorios(generador)) {
        int nf = f + d.first;
        int nc = c + d.second;
        if (dentroLimites(nf, nc, lab.filas, lab.columnas) &&
            lab.grilla[nf][nc] == '#' &&
            contarVecinosAbiertos(lab, nf, nc) <= 1) {
            tallarLaberintoDFS(lab, nf, nc, generador);
        }
    }
}

// Refuerzo de conectividad: garantiza al menos un camino entrada -> salida.
void tallarCaminoGarantizado(Laberinto &lab, std::mt19937 &generador) {
    int f = 0, c = 0;
    lab.grilla[f][c] = '*';

    while (f != lab.filas - 1 || c != lab.columnas - 1) {
        bool puedeBajar = f < lab.filas - 1;
        bool puedeDerecha = c < lab.columnas - 1;

        if (puedeBajar && puedeDerecha) {
            (std::uniform_int_distribution<int>(0, 1)(generador) == 0) ? ++f : ++c;
        } else if (puedeBajar) {
            ++f;
        } else {
            ++c;
        }
        lab.grilla[f][c] = '*';
    }
}

// Backtracking clasico: intenta caminos y retrocede al atascarse.
bool resolverBacktracking(const Laberinto &lab, int f, int c, vector<vector<bool>> &visitado, vector<pair<int, int>> &camino) {
    if (!dentroLimites(f, c, lab.filas, lab.columnas) || lab.grilla[f][c] == '#' || visitado[f][c]) {
        return false;
    }

    visitado[f][c] = true;
    camino.push_back({f, c});

    if (f == lab.filas - 1 && c == lab.columnas - 1) {
        return true;
    }

    static const int df[4] = {1, -1, 0, 0};
    static const int dc[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; ++i) {
        if (resolverBacktracking(lab, f + df[i], c + dc[i], visitado, camino)) {
            return true;
        }
    }

    camino.pop_back();
    return false;
}

// Muestra el laberinto con emojis y opcionalmente superpone el camino solucion.
void imprimeLaberinto(const Laberinto &lab, const vector<pair<int, int>> *camino = nullptr) {
    vector<string> vista = lab.grilla;

    if (camino != nullptr) {
        for (const auto &p : *camino) {
            if (!(p.first == 0 && p.second == 0) &&
                !(p.first == lab.filas - 1 && p.second == lab.columnas - 1)) {
                vista[p.first][p.second] = '.';
            }
        }
    }

    vista[0][0] = 'E';
    vista[lab.filas - 1][lab.columnas - 1] = 'S';

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

int parsearPositivo(const char *texto, int porDefecto) {
    if (texto == nullptr) return porDefecto;

    char *fin = nullptr;
    long valor = std::strtol(texto, &fin, 10);
    if (*texto == '\0' || *fin != '\0' || valor <= 0 || valor > 1000) {
        return porDefecto;
    }
    return static_cast<int>(valor);
}

#ifdef _WIN32
void configurarConsolaUtf8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::setlocale(LC_ALL, ".UTF-8");
}
#endif

int main(int argc, char **argv) {
    int filas = 10;
    int columnas = 10;
    int cantidadNumeros = 0;

#ifdef _WIN32
    configurarConsolaUtf8();
#endif

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

    std::random_device semilla;
    std::mt19937 generador(semilla());
    Laberinto lab(filas, columnas);

    auto inicioGen = std::chrono::high_resolution_clock::now();
    tallarLaberintoDFS(lab, 0, 0, generador);
    tallarCaminoGarantizado(lab, generador);
    auto finGen = std::chrono::high_resolution_clock::now();

    vector<vector<bool>> visitado(filas, vector<bool>(columnas, false));
    vector<pair<int, int>> camino;

    auto inicioSol = std::chrono::high_resolution_clock::now();
    bool resuelto = resolverBacktracking(lab, 0, 0, visitado, camino);
    auto finSol = std::chrono::high_resolution_clock::now();

    auto tiempoGen = std::chrono::duration_cast<std::chrono::microseconds>(finGen - inicioGen).count();
    auto tiempoSol = std::chrono::duration_cast<std::chrono::microseconds>(finSol - inicioSol).count();

    cout << "Laberinto generado (" << filas << "x" << columnas << "):\n";
    cout << u8"Leyenda: 🧱 muro | ⬜ camino | 🚪 entrada | 🏁 salida | 🟨 solucion\n";
    imprimeLaberinto(lab);
    cout << '\n';

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
