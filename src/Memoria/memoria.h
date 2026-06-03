#ifndef MEMORIA_H
#define MEMORIA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <climits>
#include <cstdint>

// ─────────────────────────────────────────────
//  Constantes del sistema de memoria
// ─────────────────────────────────────────────
static constexpr int MEMORIA_TOTAL     = 1024;   // KB de RAM simulada
static constexpr int TAMANO_PAGINA     = 4;       // KB por página/marco
static constexpr int NUM_MARCOS        = MEMORIA_TOTAL / TAMANO_PAGINA;  // 256 marcos
static constexpr int TAMANO_SWAP       = 2048;    // KB de espacio de swap

// ─────────────────────────────────────────────
//  Entrada de la Tabla de Páginas (PTE)
// ─────────────────────────────────────────────
struct PaginaEntry {
    int  marcoFisico;   // Marco físico asignado (-1 si no está en RAM)
    bool presente;      // Está en RAM?
    bool modificada;    // Dirty bit
    bool referenciada;  // Reference bit (para algoritmos de reemplazo)
    int  swapOffset;    // Posición en disco swap (-1 si no aplica)

    PaginaEntry()
        : marcoFisico(-1), presente(false),
          modificada(false), referenciada(false), swapOffset(-1) {}
};

// ─────────────────────────────────────────────
//  Segmento (para segmentación)
// ─────────────────────────────────────────────
struct Segmento {
    std::string nombre;   // "codigo", "datos", "pila"
    int base;             // Dirección base en memoria física
    int limite;           // Tamaño del segmento en KB
    bool soloLectura;     // Protección del segmento

    Segmento(const std::string& n, int b, int lim, bool ro = false)
        : nombre(n), base(b), limite(lim), soloLectura(ro) {}
};

// ─────────────────────────────────────────────
//  Bloque de memoria para alocación contigua
// ─────────────────────────────────────────────
struct BloqueMemoria {
    int  inicio;        // KB donde empieza el bloque
    int  tamano;        // KB del bloque
    bool libre;         // Está libre?
    int  pidDueno;      // PID del proceso dueño (-1 si libre)

    BloqueMemoria(int ini, int tam, bool lib = true, int pid = -1)
        : inicio(ini), tamano(tam), libre(lib), pidDueno(pid) {}
};

// ─────────────────────────────────────────────
//  Región de memoria compartida
// ─────────────────────────────────────────────
struct RegionCompartida {
    int              id;          // ID único de la región
    int              base;        // Dirección física base
    int              tamano;      // Tamaño en KB
    std::vector<int> procesos;    // PIDs que la comparten
    int              refCount;    // Conteo de referencias

    RegionCompartida(int id, int base, int tam)
        : id(id), base(base), tamano(tam), refCount(0) {}
};

// ─────────────────────────────────────────────
//  Algoritmos de asignación de memoria (Día 6)
// ─────────────────────────────────────────────
enum AlgoritmoAsignacion {
    FIRST_FIT,  // Primer bloque con espacio suficiente
    BEST_FIT    // Bloque más pequeño que alcance (minimiza desperdicio)
};

// ─────────────────────────────────────────────
//  Gestor de Memoria
// ─────────────────────────────────────────────
class GestorMemoria {
public:
    GestorMemoria();

    // --- Alocación contigua (Día 6: First-Fit / Best-Fit) ---
    int  asignarContiguo(int pid, int tamanoKB,
                         AlgoritmoAsignacion algo = FIRST_FIT);
    void liberarContiguo(int pid);
    void mostrarMapaMemoria() const;

    // --- Segmentación ---
    bool crearSegmentos(int pid, int tamCodigo, int tamDatos, int tamPila);
    bool accederSegmento(int pid, const std::string& segmento, int offset);
    void liberarSegmentos(int pid);

    // --- Paginación ---
    bool asignarPaginas(int pid, int numPaginas);
    int  traducirDireccion(int pid, int dirLogica);   // Retorna dirección física
    void liberarPaginas(int pid);

    // --- Compartición ---
    int  crearRegionCompartida(int pid, int tamanoKB);   // Retorna ID región
    bool adjuntarRegion(int pid, int regionId);
    void desadjuntarRegion(int pid, int regionId);

    // --- Swapping ---
    bool swapOut(int pid);    // Mueve páginas de pid a swap
    bool swapIn(int pid);     // Trae páginas de swap a RAM
    bool hayEspacioRAM(int numPaginas) const;

    // --- Fragmentación (Día 7) ---
    void mostrarFragmentacion() const;  // Muestra huecos y % frag. externa
    void compactar();                   // Elimina frag. externa moviendo bloques

    // --- Diagnóstico ---
    void imprimirTablaPaginas(int pid) const;
    void imprimirSegmentos(int pid) const;
    int  memoriaLibreKB() const;
    int  memoriaUsadaKB() const;

private:
    // Día 5: Array físico — índice = número de KB, valor = PID dueño (-1 = libre)
    std::array<int, MEMORIA_TOTAL> celdas_;

    // Alocación contigua (lista de bloques sobre el array físico)
    std::vector<BloqueMemoria> bloques_;

    // Segmentación: pid -> lista de segmentos
    std::unordered_map<int, std::vector<Segmento>> tablaSegmentos_;

    // Paginación: pid -> tabla de páginas
    std::unordered_map<int, std::vector<PaginaEntry>> tablaPaginas_;

    // Marcos físicos: índice = nro marco, valor = pid dueño (-1 si libre)
    std::vector<int> marcos_;

    // Swap: pid -> offsets en swap
    std::unordered_map<int, std::vector<int>> swapMap_;
    std::vector<bool> swapLibre_;   // Slots libres en swap

    // Compartición
    std::vector<RegionCompartida> regionesCompartidas_;
    int siguienteRegionId_;

    // Helpers internos
    int  buscarMarcosLibres(int cantidad) const;
    int  buscarBloqueLibre(int tamano) const;        // First-Fit (Día 6)
    int  buscarBloqueLibreBestFit(int tamano) const; // Best-Fit  (Día 6)
    int  asignarSlotSwap();
};

#endif // MEMORIA_H
