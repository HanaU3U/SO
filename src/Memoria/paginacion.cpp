#include "memoria.h"
#include <iostream>
#include <iomanip>

// ═══════════════════════════════════════════════════════════════
//  MEMORIA VIRTUAL — Día 8: Paginación (mapeo lógico → físico)
//
//  Cada proceso ve un espacio de direcciones lógico contiguo.
//  La MMU traduce usando la tabla de páginas del proceso:
//
//  Dirección lógica (32 bits simulada):
//    [ número de página (20 bits) | offset (12 bits) ]
//  Con TAMANO_PAGINA = 4 KB = 2^12 → offset = dirLogica % 4096
//                                     página  = dirLogica / 4096
//
//  dirFísica = (marcoFísico << OFFSET_BITS) | offset
// ═══════════════════════════════════════════════════════════════

static constexpr int OFFSET_BITS = 12;   // log2(4096)

// ─────────────────────────────────────────────
//  Buscar N marcos físicos contiguos libres
//  (para simplicidad: marcos no necesitan ser contiguos)
// ─────────────────────────────────────────────
int GestorMemoria::buscarMarcosLibres(int cantidad) const {
    int encontrados = 0;
    for (int i = 0; i < NUM_MARCOS; ++i) {
        if (marcos_[i] == -1) {
            ++encontrados;
            if (encontrados == cantidad) return i - cantidad + 1;
        } else {
            encontrados = 0;
        }
    }
    return -1;
}

bool GestorMemoria::hayEspacioRAM(int numPaginas) const {
    int libres = 0;
    for (int m : marcos_) if (m == -1) ++libres;
    return libres >= numPaginas;
}

// ─────────────────────────────────────────────
//  Asignar páginas a un proceso
// ─────────────────────────────────────────────
bool GestorMemoria::asignarPaginas(int pid, int numPaginas) {
    if (!hayEspacioRAM(numPaginas)) {
        std::cout << "[PAGINACION] Sin marcos suficientes para PID " << pid
                  << " (" << numPaginas << " páginas). Intentando swap...\n";
        return false;
    }

    std::vector<PaginaEntry>& tabla = tablaPaginas_[pid];
    tabla.resize(static_cast<std::size_t>(numPaginas));

    int asignadas = 0;
    for (int m = 0; m < NUM_MARCOS && asignadas < numPaginas; ++m) {
        if (marcos_[m] == -1) {
            marcos_[m] = pid;
            tabla[static_cast<std::size_t>(asignadas)].marcoFisico  = m;
            tabla[static_cast<std::size_t>(asignadas)].presente      = true;
            tabla[static_cast<std::size_t>(asignadas)].modificada    = false;
            tabla[static_cast<std::size_t>(asignadas)].referenciada  = false;
            tabla[static_cast<std::size_t>(asignadas)].swapOffset    = -1;
            ++asignadas;
        }
    }

    std::cout << "[PAGINACION] PID " << pid << " → " << numPaginas
              << " páginas asignadas en marcos físicos.\n";
    return true;
}

// ─────────────────────────────────────────────
//  Traducción de dirección lógica → física
//  dirLogica = (numPagina << OFFSET_BITS) | offset
// ─────────────────────────────────────────────
int GestorMemoria::traducirDireccion(int pid, int dirLogica) {
    int numPagina = dirLogica >> OFFSET_BITS;
    int offset    = dirLogica & ((1 << OFFSET_BITS) - 1);

    auto it = tablaPaginas_.find(pid);
    if (it == tablaPaginas_.end()) {
        std::cout << "[PAGE FAULT] PID " << pid << " no tiene tabla de páginas.\n";
        return -1;
    }

    auto& tabla = it->second;
    if (numPagina < 0 || numPagina >= static_cast<int>(tabla.size())) {
        std::cout << "[PAGE FAULT] PID " << pid << " página " << numPagina << " fuera de rango.\n";
        return -1;
    }

    PaginaEntry& pte = tabla[static_cast<std::size_t>(numPagina)];

    if (!pte.presente) {
        std::cout << "[PAGE FAULT] PID " << pid << " página " << numPagina
                  << " no está en RAM → necesita swapIn.\n";
        return -1;
    }

    pte.referenciada = true;
    int dirFisica = (pte.marcoFisico << OFFSET_BITS) | offset;
    std::cout << "[PAGINACION] PID " << pid
              << " dirLogica=" << dirLogica
              << " → página=" << numPagina
              << " marco=" << pte.marcoFisico
              << " dirFísica=" << dirFisica << "\n";
    return dirFisica;
}

// ─────────────────────────────────────────────
//  Liberar páginas de un proceso
// ─────────────────────────────────────────────
void GestorMemoria::liberarPaginas(int pid) {
    auto it = tablaPaginas_.find(pid);
    if (it == tablaPaginas_.end()) return;

    for (auto& pte : it->second) {
        if (pte.presente && pte.marcoFisico >= 0) {
            marcos_[static_cast<std::size_t>(pte.marcoFisico)] = -1;
        }
        // Liberar slot swap si lo tenía
        if (pte.swapOffset >= 0) {
            swapLibre_[static_cast<std::size_t>(pte.swapOffset)] = true;
        }
    }
    tablaPaginas_.erase(it);
    std::cout << "[PAGINACION] Páginas de PID " << pid << " liberadas.\n";
}

// ─────────────────────────────────────────────
//  Imprimir tabla de páginas
// ─────────────────────────────────────────────
void GestorMemoria::imprimirTablaPaginas(int pid) const {
    auto it = tablaPaginas_.find(pid);
    if (it == tablaPaginas_.end()) {
        std::cout << "PID " << pid << " no tiene tabla de páginas.\n";
        return;
    }
    std::cout << "\n=== TABLA DE PÁGINAS PID " << pid << " ===\n";
    std::cout << std::left
              << std::setw(8)  << "Pág"
              << std::setw(8)  << "Marco"
              << std::setw(10) << "Presente"
              << std::setw(10) << "Modif."
              << std::setw(10) << "Ref."
              << "Swap\n";
    std::cout << std::string(50, '-') << "\n";
    for (std::size_t i = 0; i < it->second.size(); ++i) {
        const auto& pte = it->second[i];
        std::cout << std::setw(8)  << i
                  << std::setw(8)  << (pte.presente ? std::to_string(pte.marcoFisico) : "-")
                  << std::setw(10) << (pte.presente  ? "Sí" : "No")
                  << std::setw(10) << (pte.modificada ? "Sí" : "No")
                  << std::setw(10) << (pte.referenciada ? "Sí" : "No")
                  << (pte.swapOffset >= 0 ? std::to_string(pte.swapOffset) : "-") << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════
//  SWAPPING — mueve páginas de un proceso entre RAM y disco
// ═══════════════════════════════════════════════════════════════
int GestorMemoria::asignarSlotSwap() {
    for (std::size_t i = 0; i < swapLibre_.size(); ++i) {
        if (swapLibre_[i]) {
            swapLibre_[i] = false;
            return static_cast<int>(i);
        }
    }
    return -1;   // Swap lleno
}

bool GestorMemoria::swapOut(int pid) {
    auto it = tablaPaginas_.find(pid);
    if (it == tablaPaginas_.end()) return false;

    bool alguna = false;
    for (auto& pte : it->second) {
        if (!pte.presente) continue;

        int slot = asignarSlotSwap();
        if (slot == -1) {
            std::cout << "[SWAP] Espacio de swap agotado.\n";
            return false;
        }

        // Simula escritura a disco: libera el marco físico
        marcos_[static_cast<std::size_t>(pte.marcoFisico)] = -1;
        pte.swapOffset   = slot;
        pte.presente     = false;
        pte.modificada   = false;
        pte.referenciada = false;
        alguna = true;
    }

    if (alguna)
        std::cout << "[SWAP] PID " << pid << " → páginas movidas a swap.\n";
    return alguna;
}

bool GestorMemoria::swapIn(int pid) {
    auto it = tablaPaginas_.find(pid);
    if (it == tablaPaginas_.end()) return false;

    int paginas = 0;
    for (const auto& pte : it->second)
        if (!pte.presente) ++paginas;

    if (!hayEspacioRAM(paginas)) {
        std::cout << "[SWAP] No hay marcos libres para swapIn de PID " << pid << ".\n";
        return false;
    }

    for (auto& pte : it->second) {
        if (pte.presente) continue;

        // Buscar marco libre
        for (int m = 0; m < NUM_MARCOS; ++m) {
            if (marcos_[m] == -1) {
                // Simula lectura desde disco
                marcos_[m]       = pid;
                swapLibre_[static_cast<std::size_t>(pte.swapOffset)] = true;
                pte.marcoFisico  = m;
                pte.presente     = true;
                pte.swapOffset   = -1;
                break;
            }
        }
    }

    std::cout << "[SWAP] PID " << pid << " → páginas traídas de swap a RAM.\n";
    return true;
}
