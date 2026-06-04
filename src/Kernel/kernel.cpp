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
    siguienteBloqueDatos_ = 1;
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
            for (std::unordered_map<int, InfoMemoriaProceso>::iterator itInfo = infoMemoria_.begin();
                 itInfo != infoMemoria_.end();
                 ++itInfo) {
                int otherPid = itInfo->first;
                InfoMemoriaProceso& otherInfo = itInfo->second;
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
    fs_.cerrarTodosArchivosProceso(pid);
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
    std::cout << "║  Iniciando ejecución integrada       ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    for (int tick = 0; tick < tiempoTotal; ++tick) {
        controladorInterrupciones_.despachar(gestorES_, scheduler_);

        if (scheduler_.hasReadyProcesses() || scheduler_.getRunningProcess() != nullptr) {
            scheduler_.executeTimeSlice(1);
        }

        gestorES_.procesarTick();
        controladorInterrupciones_.despachar(gestorES_, scheduler_);

        if (!scheduler_.hasReadyProcesses() &&
            scheduler_.getRunningProcess() == nullptr &&
            !gestorES_.haySolicitudesPendientes() &&
            !scheduler_.hasBlockedProcesses()) {
            break;
        }
    }

    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║       Ejecución finalizada           ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
}

bool Kernel::crearDirectorio(const std::string& ruta) {
    const bool ok = fs_.crearDirectorio(ruta);
    if (!ok) {
        std::cout << "[KERNEL][FS] No se pudo crear directorio: " << ruta << "\n";
    }
    return ok;
}

bool Kernel::crearArchivo(const std::string& ruta,
                          const std::string& tipo,
                          int tamanoKB,
                          const std::string& permisos) {
    const bool ok = fs_.crearArchivo(ruta, tipo, tamanoKB, siguienteBloqueDatos_++, permisos);
    if (!ok) {
        std::cout << "[KERNEL][FS] No se pudo crear archivo: " << ruta << "\n";
    }
    return ok;
}

bool Kernel::abrirArchivo(int pid, const std::string& ruta) {
    PCB* proceso = scheduler_.getProcessByPid(pid);
    if (!proceso || proceso->getState() == TERMINATED) {
        std::cout << "[KERNEL][FS] PID invalido para abrir archivo: " << pid << "\n";
        return false;
    }

    const bool ok = fs_.abrirArchivo(pid, ruta);
    if (!ok) {
        std::cout << "[KERNEL][FS] No se pudo abrir archivo: " << ruta << "\n";
    }
    return ok;
}

bool Kernel::cerrarArchivo(int pid, const std::string& ruta) {
    const bool ok = fs_.cerrarArchivo(pid, ruta);
    if (!ok) {
        std::cout << "[KERNEL][FS] No se pudo cerrar archivo: " << ruta << "\n";
    }
    return ok;
}

void Kernel::listarDirectorio(const std::string& ruta) const {
    fs_.listarDirectorio(ruta);
}

bool Kernel::registrarDispositivo(const std::string& nombre, const std::string& tipo) {
    const bool ok = gestorES_.registrarDispositivo(nombre, tipo);
    if (!ok) {
        std::cout << "[KERNEL][E/S] No se pudo registrar dispositivo: " << nombre << "\n";
    }
    return ok;
}

bool Kernel::solicitarIO(int pid,
                         const std::string& nombreDispositivo,
                         const std::string& operacion,
                         int duracionTicks) {
    PCB* proceso = scheduler_.getProcessByPid(pid);
    if (!proceso || proceso->getState() == TERMINATED) {
        std::cout << "[KERNEL][E/S] PID invalido para solicitud de E/S: " << pid << "\n";
        return false;
    }

    if (!gestorES_.existeDispositivo(nombreDispositivo)) {
        std::cout << "[KERNEL][E/S] Dispositivo no encontrado: " << nombreDispositivo << "\n";
        return false;
    }

    const bool bloqueado = scheduler_.blockProcess(pid);
    if (!bloqueado) {
        std::cout << "[KERNEL][E/S] PID " << pid << " no pudo pasar a BLOQUEADO\n";
        return false;
    }

    const bool encolada = gestorES_.solicitarIO(pid, nombreDispositivo, operacion, duracionTicks);
    if (!encolada) {
        scheduler_.unblockProcess(pid);
        std::cout << "[KERNEL][E/S] Error encolando solicitud de E/S\n";
        return false;
    }

    std::cout << "[KERNEL][E/S] PID " << pid
              << " bloqueado por solicitud en " << nombreDispositivo
              << " (" << operacion << ")\n";
    return true;
}

void Kernel::imprimirDispositivos() const {
    gestorES_.imprimirTablaDispositivos();
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
