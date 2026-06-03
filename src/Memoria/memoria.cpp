#include "memoria.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <climits>

// ═══════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════
GestorMemoria::GestorMemoria()
    : marcos_(NUM_MARCOS, -1),
      swapLibre_(TAMANO_SWAP / TAMANO_PAGINA, true),
      siguienteRegionId_(1)
{
    // Día 5: inicializar array físico de memoria (toda la RAM libre al inicio)
    celdas_.fill(-1);
    // Un único bloque contiguo libre al inicio
    bloques_.emplace_back(0, MEMORIA_TOTAL, true, -1);
}

// ═══════════════════════════════════════════════════════════════
//  ALOCACIÓN CONTIGUA — Día 6: First-Fit y Best-Fit
// ═══════════════════════════════════════════════════════════════
// First-Fit: primer bloque libre con tamaño suficiente
int GestorMemoria::buscarBloqueLibre(int tamano) const {
    for (int i = 0; i < static_cast<int>(bloques_.size()); ++i) {
        if (bloques_[i].libre && bloques_[i].tamano >= tamano)
            return i;
    }
    return -1;
}

// Best-Fit: bloque libre más pequeño que alcance (minimiza fragmentación interna)
int GestorMemoria::buscarBloqueLibreBestFit(int tamano) const {
    int mejorIdx = -1;
    int mejorTam = INT_MAX;
    for (int i = 0; i < static_cast<int>(bloques_.size()); ++i) {
        if (bloques_[i].libre && bloques_[i].tamano >= tamano) {
            if (bloques_[i].tamano < mejorTam) {
                mejorTam = bloques_[i].tamano;
                mejorIdx = i;
            }
        }
    }
    return mejorIdx;
}

int GestorMemoria::asignarContiguo(int pid, int tamanoKB, AlgoritmoAsignacion algo) {
    int idx = (algo == BEST_FIT) ? buscarBloqueLibreBestFit(tamanoKB)
                                 : buscarBloqueLibre(tamanoKB);
    if (idx == -1) {
        std::cout << "[MEMORIA] No hay bloque libre de " << tamanoKB
                  << " KB para PID " << pid << "\n";
        return -1;
    }

    BloqueMemoria& bloque = bloques_[static_cast<std::size_t>(idx)];
    int base = bloque.inicio;

    if (bloque.tamano > tamanoKB) {
        // Fragmento sobrante → queda como nuevo bloque libre
        bloques_.emplace(bloques_.begin() + idx + 1,
                         bloque.inicio + tamanoKB,
                         bloque.tamano - tamanoKB,
                         true, -1);
    }

    bloques_[static_cast<std::size_t>(idx)].tamano   = tamanoKB;
    bloques_[static_cast<std::size_t>(idx)].libre     = false;
    bloques_[static_cast<std::size_t>(idx)].pidDueno  = pid;

    // Día 5: marcar celdas físicas con el PID propietario
    for (int c = base; c < base + tamanoKB && c < MEMORIA_TOTAL; ++c)
        celdas_[c] = pid;

    std::cout << "[MEMORIA] PID " << pid
              << " → bloque en " << base << " KB, tamaño " << tamanoKB << " KB"
              << " [" << (algo == BEST_FIT ? "BEST-FIT" : "FIRST-FIT") << "]\n";
    return base;
}

void GestorMemoria::liberarContiguo(int pid) {
    for (auto& b : bloques_) {
        if (!b.libre && b.pidDueno == pid) {
            // Día 5: liberar celdas físicas
            for (int c = b.inicio; c < b.inicio + b.tamano && c < MEMORIA_TOTAL; ++c)
                celdas_[c] = -1;
            b.libre    = true;
            b.pidDueno = -1;
        }
    }
    // Día 7: fusión (coalescing) de bloques adyacentes libres
    bool fusiono = true;
    while (fusiono) {
        fusiono = false;
        for (std::size_t i = 0; i + 1 < bloques_.size(); ++i) {
            if (bloques_[i].libre && bloques_[i + 1].libre) {
                bloques_[i].tamano += bloques_[i + 1].tamano;
                bloques_.erase(bloques_.begin() + static_cast<int>(i) + 1);
                fusiono = true;
                break;
            }
        }
    }
    std::cout << "[MEMORIA] Memoria contigua de PID " << pid << " liberada.\n";
}

void GestorMemoria::mostrarMapaMemoria() const {
    std::cout << "\n=== MAPA DE MEMORIA CONTIGUA (Dia 5) ===\n";
    std::cout << std::left << std::setw(12) << "Inicio(KB)"
              << std::setw(12) << "Fin(KB)"
              << std::setw(12) << "Tama\u00f1o(KB)"
              << std::setw(8)  << "Estado"
              << "PID\n";
    std::cout << std::string(50, '-') << "\n";
    for (const auto& b : bloques_) {
        std::cout << std::left
                  << std::setw(12) << b.inicio
                  << std::setw(12) << (b.inicio + b.tamano)
                  << std::setw(12) << b.tamano
                  << std::setw(8)  << (b.libre ? "LIBRE" : "USADO")
                  << (b.libre ? "-" : std::to_string(b.pidDueno)) << "\n";
    }
    // Visualización compacta del array físico (cada carácter = 32 KB)
    std::cout << "\nArray físico [" << MEMORIA_TOTAL << " KB] (cada pos = 32 KB):\n[";
    for (int c = 0; c < MEMORIA_TOTAL; c += 32) {
        int libres = 0;
        for (int k = c; k < c + 32 && k < MEMORIA_TOTAL; ++k)
            if (celdas_[k] == -1) ++libres;
        std::cout << (libres > 16 ? '.' : '#');
    }
    std::cout << "]  (. = libre, # = usado)\n";
    std::cout << "================================\n";
}

// ═══════════════════════════════════════════════════════════════
//  SEGMENTACIÓN
//  Cada proceso tiene segmentos: código (RO), datos, pila
// ═══════════════════════════════════════════════════════════════
bool GestorMemoria::crearSegmentos(int pid, int tamCodigo, int tamDatos, int tamPila) {
    int total = tamCodigo + tamDatos + tamPila;
    int base  = asignarContiguo(pid, total);
    if (base == -1) return false;

    std::vector<Segmento> segs;
    segs.emplace_back("codigo", base,                      tamCodigo, true);
    segs.emplace_back("datos",  base + tamCodigo,          tamDatos,  false);
    segs.emplace_back("pila",   base + tamCodigo + tamDatos, tamPila, false);

    tablaSegmentos_[pid] = segs;

    std::cout << "[SEGMENTACION] PID " << pid
              << " → codigo@" << base
              << " datos@"    << (base + tamCodigo)
              << " pila@"     << (base + tamCodigo + tamDatos) << "\n";
    return true;
}

bool GestorMemoria::accederSegmento(int pid, const std::string& segmento, int offset) {
    auto it = tablaSegmentos_.find(pid);
    if (it == tablaSegmentos_.end()) {
        std::cout << "[SEGFAULT] PID " << pid << " no tiene segmentos.\n";
        return false;
    }
    for (const auto& seg : it->second) {
        if (seg.nombre == segmento) {
            if (offset < 0 || offset >= seg.limite) {
                std::cout << "[SEGFAULT] PID " << pid << " offset " << offset
                          << " fuera de segmento '" << segmento << "' (límite=" << seg.limite << ")\n";
                return false;
            }
            std::cout << "[SEGMENTACION] PID " << pid << " accede '" << segmento
                      << "' offset=" << offset
                      << " → dirFísica=" << (seg.base + offset) << "\n";
            return true;
        }
    }
    std::cout << "[SEGFAULT] Segmento '" << segmento << "' no existe para PID " << pid << "\n";
    return false;
}

void GestorMemoria::liberarSegmentos(int pid) {
    tablaSegmentos_.erase(pid);
    liberarContiguo(pid);
}

void GestorMemoria::imprimirSegmentos(int pid) const {
    auto it = tablaSegmentos_.find(pid);
    if (it == tablaSegmentos_.end()) { std::cout << "PID " << pid << " sin segmentos.\n"; return; }
    std::cout << "\n=== TABLA DE SEGMENTOS PID " << pid << " ===\n";
    std::cout << std::left << std::setw(10) << "Nombre"
              << std::setw(10) << "Base"
              << std::setw(10) << "Límite"
              << "Prot.\n";
    std::cout << std::string(35, '-') << "\n";
    for (const auto& s : it->second) {
        std::cout << std::setw(10) << s.nombre
                  << std::setw(10) << s.base
                  << std::setw(10) << s.limite
                  << (s.soloLectura ? "RO" : "RW") << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════
//  COMPARTICIÓN DE MEMORIA
//  Múltiples procesos mapean la misma región física (misma copia)
// ═══════════════════════════════════════════════════════════════
int GestorMemoria::crearRegionCompartida(int pid, int tamanoKB) {
    int base = asignarContiguo(-siguienteRegionId_, tamanoKB);
    if (base == -1) return -1;

    RegionCompartida region(siguienteRegionId_++, base, tamanoKB);
    region.procesos.push_back(pid);
    region.refCount = 1;
    regionesCompartidas_.push_back(region);

    std::cout << "[COMPARTIDA] PID " << pid << " creó región ID="
              << region.id << " en base=" << base << " KB\n";
    return region.id;
}

bool GestorMemoria::adjuntarRegion(int pid, int regionId) {
    for (auto& r : regionesCompartidas_) {
        if (r.id == regionId) {
            // El proceso mapea la misma dirección física — no se copia memoria
            r.procesos.push_back(pid);
            ++r.refCount;
            std::cout << "[COMPARTIDA] PID " << pid << " adjuntado a región "
                      << regionId << " (base=" << r.base << " KB, refCount=" << r.refCount << ")\n";
            return true;
        }
    }
    std::cout << "[COMPARTIDA] Región " << regionId << " no existe.\n";
    return false;
}

void GestorMemoria::desadjuntarRegion(int pid, int regionId) {
    for (auto& r : regionesCompartidas_) {
        if (r.id == regionId) {
            r.procesos.erase(std::remove(r.procesos.begin(), r.procesos.end(), pid), r.procesos.end());
            --r.refCount;
            std::cout << "[COMPARTIDA] PID " << pid << " desadjuntado de región "
                      << regionId << " (refCount=" << r.refCount << ")\n";
            if (r.refCount == 0) {
                // Nadie la usa, liberar
                liberarContiguo(-regionId);
                std::cout << "[COMPARTIDA] Región " << regionId << " eliminada (sin referencias).\n";
            }
            return;
        }
    }
}

// ═══════════════════════════════════════════════════════════════
//  Helpers de diagnóstico
// ═══════════════════════════════════════════════════════════════
int GestorMemoria::memoriaLibreKB() const {
    int libre = 0;
    for (const auto& b : bloques_)
        if (b.libre) libre += b.tamano;
    return libre;
}

int GestorMemoria::memoriaUsadaKB() const {
    return MEMORIA_TOTAL - memoriaLibreKB();
}

// ═══════════════════════════════════════════════════════════════
//  FRAGMENTACIÓN — Día 7
// ═══════════════════════════════════════════════════════════════
void GestorMemoria::mostrarFragmentacion() const {
    int huecos     = 0;
    int totalLibre = 0;
    int mayorHueco = 0;
    for (const auto& b : bloques_) {
        if (b.libre) {
            ++huecos;
            totalLibre += b.tamano;
            if (b.tamano > mayorHueco) mayorHueco = b.tamano;
        }
    }
    // Fragmentación externa: % de mem. libre que NO está en el mayor hueco
    double fragExt = (totalLibre > 0)
        ? (1.0 - static_cast<double>(mayorHueco) / totalLibre) * 100.0
        : 0.0;

    std::cout << "\n=== FRAGMENTACION (Dia 7) ===\n";
    std::cout << "  Memoria total   : " << MEMORIA_TOTAL    << " KB\n";
    std::cout << "  Memoria usada   : " << memoriaUsadaKB() << " KB\n";
    std::cout << "  Memoria libre   : " << totalLibre       << " KB\n";
    std::cout << "  Huecos libres   : " << huecos           << "\n";
    std::cout << "  Mayor hueco     : " << mayorHueco       << " KB\n";
    std::cout << "  Frag. externa   : " << std::fixed << std::setprecision(1)
              << fragExt << "%\n";
    std::cout << "=============================\n";
}

// Compactación: mueve todos los bloques usados al inicio, dejando un único hueco libre al final
void GestorMemoria::compactar() {
    std::cout << "[MEMORIA] Iniciando compactacion...\n";
    std::vector<BloqueMemoria> nuevos;
    int cursor = 0;

    for (auto& b : bloques_) {
        if (!b.libre) {
            // Limpiar posición anterior en celdas_
            for (int c = b.inicio; c < b.inicio + b.tamano && c < MEMORIA_TOTAL; ++c)
                celdas_[c] = -1;
            // Marcar nueva posición
            for (int c = cursor; c < cursor + b.tamano && c < MEMORIA_TOTAL; ++c)
                celdas_[c] = b.pidDueno;
            nuevos.emplace_back(cursor, b.tamano, false, b.pidDueno);
            cursor += b.tamano;
        }
    }
    // Único bloque libre contiguo al final
    if (cursor < MEMORIA_TOTAL) {
        nuevos.emplace_back(cursor, MEMORIA_TOTAL - cursor, true, -1);
        for (int c = cursor; c < MEMORIA_TOTAL; ++c)
            celdas_[c] = -1;
    }
    bloques_ = std::move(nuevos);
    std::cout << "[MEMORIA] Compactacion completa. "
              << (MEMORIA_TOTAL - cursor) << " KB libres contiguos.\n";
}
