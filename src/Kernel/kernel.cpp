#include "kernel.h"
#include <iostream>
#include <iomanip>

// ═══════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════
Kernel::Kernel() {
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║        Mini Kernel iniciado          ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
}

// ═══════════════════════════════════════════════════════════════
//  Creación de proceso: crea PCB + asigna memoria
// ═══════════════════════════════════════════════════════════════
PCB* Kernel::crearProceso(const std::string& nombre,
                           int cpuTime,
                           int memoriaKB,
                           int quantum,
                           bool usarPaginacion) {
    std::cout << "\n--- KERNEL: creando proceso '" << nombre << "' ---\n";

    // 1. Crear PCB en el scheduler
    PCB* proceso = scheduler_.createProcess(nombre, cpuTime, quantum);
    int pid = proceso->pid;

    // 2. Asignar memoria
    InfoMemoriaProceso info{};
    info.usaPaginacion = usarPaginacion;

    if (usarPaginacion) {
        // Calcular páginas necesarias (redondeo hacia arriba)
        int paginas = (memoriaKB + TAMANO_PAGINA - 1) / TAMANO_PAGINA;
        info.numPaginas  = paginas;
        info.baseContigua = -1;

        bool ok = memoria_.asignarPaginas(pid, paginas);
        if (!ok) {
            // Intentar liberar memoria via swapOut de otro proceso
            std::cout << "[KERNEL] RAM insuficiente, intentando swapOut...\n";
            // Busca el primer proceso en swap-out candidates (no running)
            for (auto& [otherPid, otherInfo] : infoMemoria_) {
                if (otherInfo.usaPaginacion && otherPid != pid) {
                    if (memoria_.swapOut(otherPid)) {
                        ok = memoria_.asignarPaginas(pid, paginas);
                        break;
                    }
                }
            }
            if (!ok) {
                std::cout << "[KERNEL] ERROR: No se pudo asignar memoria para '" << nombre << "'\n";
                return proceso;  // proceso creado pero sin memoria
            }
        }
    } else {
        // Contiguo + segmentos
        int tamCodigo = memoriaKB / 3;
        int tamDatos  = memoriaKB / 3;
        int tamPila   = memoriaKB - tamCodigo - tamDatos;

        bool ok = memoria_.crearSegmentos(pid, tamCodigo, tamDatos, tamPila);
        info.baseContigua = ok ? 0 : -1;
        info.numPaginas   = 0;

        if (!ok) {
            std::cout << "[KERNEL] ERROR: No se pudo asignar segmentos para '" << nombre << "'\n";
        }
    }

    infoMemoria_[pid] = info;

    // 3. Pasar proceso a READY
    scheduler_.transitionNewToReady(proceso);

    std::cout << "[KERNEL] Proceso '" << nombre << "' (PID=" << pid
              << ") listo con " << memoriaKB << " KB asignados.\n";
    return proceso;
}

// ═══════════════════════════════════════════════════════════════
//  Terminar proceso: libera PCB y memoria
// ═══════════════════════════════════════════════════════════════
void Kernel::liberarMemoriaProceso(int pid) {
    auto it = infoMemoria_.find(pid);
    if (it == infoMemoria_.end()) return;

    const InfoMemoriaProceso& info = it->second;
    if (info.usaPaginacion) {
        memoria_.liberarPaginas(pid);
    } else {
        memoria_.liberarSegmentos(pid);
    }
    infoMemoria_.erase(it);
}

void Kernel::terminarProceso(int pid) {
    std::cout << "\n[KERNEL] Terminando proceso PID=" << pid << "\n";
    liberarMemoriaProceso(pid);
    // El scheduler limpia el PCB en su destructor
}

// ═══════════════════════════════════════════════════════════════
//  Memoria compartida
// ═══════════════════════════════════════════════════════════════
int Kernel::crearMemoriaCompartida(int pid, int tamanoKB) {
    return memoria_.crearRegionCompartida(pid, tamanoKB);
}

bool Kernel::adjuntarMemoriaCompartida(int pid, int regionId) {
    return memoria_.adjuntarRegion(pid, regionId);
}

void Kernel::desadjuntarMemoriaCompartida(int pid, int regionId) {
    memoria_.desadjuntarRegion(pid, regionId);
}

// ═══════════════════════════════════════════════════════════════
//  Swapping
// ═══════════════════════════════════════════════════════════════
bool Kernel::swapOut(int pid) {
    return memoria_.swapOut(pid);
}

bool Kernel::swapIn(int pid) {
    return memoria_.swapIn(pid);
}

// ═══════════════════════════════════════════════════════════════
//  Ejecutar scheduler
// ═══════════════════════════════════════════════════════════════
void Kernel::ejecutar(int tiempoTotal) {
    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║  Iniciando ejecución del scheduler   ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
    scheduler_.runScheduler(tiempoTotal);
    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║       Ejecución finalizada           ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
}

// ═══════════════════════════════════════════════════════════════
//  Diagnóstico
// ═══════════════════════════════════════════════════════════════
void Kernel::imprimirEstado() const {
    scheduler_.printProcessStatus();
}

void Kernel::imprimirMemoria() const {
    memoria_.mostrarMapaMemoria();
    std::cout << "[MEMORIA] Libre: " << memoria_.memoriaLibreKB()
              << " KB  |  Usada: " << memoria_.memoriaUsadaKB() << " KB\n";
}

void Kernel::imprimirTablaPaginas(int pid) const {
    memoria_.imprimirTablaPaginas(pid);
}

void Kernel::imprimirSegmentos(int pid) const {
    memoria_.imprimirSegmentos(pid);
}

void Kernel::mostrarFragmentacion() const {
    memoria_.mostrarFragmentacion();
}

void Kernel::compactar() {
    memoria_.compactar();
}
