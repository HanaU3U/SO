#ifndef KERNEL_H
#define KERNEL_H

#include "../Procesos/scheduler.h"
#include "../Memoria/memoria.h"
#include "../Archivos/filesystem.h"
#include "../Entrada_Salida/dispositivo.h"
#include "../Entrada_Salida/interrupciones.h"
#include <string>
#include <unordered_map>

// ─────────────────────────────────────────────────────────────
//  Información de memoria asociada a cada proceso
// ─────────────────────────────────────────────────────────────
struct InfoMemoriaProceso {
    int  baseContigua;      // Dirección base asignada (-1 si usa paginación)
    int  numPaginas;        // Páginas asignadas (0 si usa contiguo)
    bool usaPaginacion;     // true = paginación, false = contiguo + segmentos
};

// ─────────────────────────────────────────────────────────────
//  Kernel — integra Scheduler y GestorMemoria
// ─────────────────────────────────────────────────────────────
class Kernel {
public:
    Kernel();

    // Ciclo de vida de procesos (crea PCB + asigna memoria)
    PCB* crearProceso(const std::string& nombre,
                      int cpuTime,
                      int memoriaKB,
                      int quantum = 5,
                      bool usarPaginacion = true);

    // Termina el proceso y libera su memoria
    void terminarProceso(int pid);

    // Memoria compartida entre procesos
    int  crearMemoriaCompartida(int pid, int tamanoKB);
    bool adjuntarMemoriaCompartida(int pid, int regionId);
    void desadjuntarMemoriaCompartida(int pid, int regionId);

    // Swapping manual
    bool swapOut(int pid);
    bool swapIn(int pid);

    // Ejecución del scheduler
    void ejecutar(int tiempoTotal);

    // Sistema de archivos
    bool crearDirectorio(const std::string& ruta);
    bool crearArchivo(const std::string& ruta,
                      const std::string& tipo,
                      int tamanoKB,
                      const std::string& permisos = "rw-");
    bool abrirArchivo(int pid, const std::string& ruta);
    bool cerrarArchivo(int pid, const std::string& ruta);
    void listarDirectorio(const std::string& ruta = "/") const;

    // Entrada/Salida e interrupciones
    bool registrarDispositivo(const std::string& nombre, const std::string& tipo);
    bool solicitarIO(int pid,
                     const std::string& nombreDispositivo,
                     const std::string& operacion,
                     int duracionTicks);
    void imprimirDispositivos() const;

    // Diagnóstico
    void imprimirEstado() const;
    void imprimirMemoria() const;
    void imprimirTablaPaginas(int pid) const;
    void imprimirSegmentos(int pid) const;
    void mostrarFragmentacion() const;  // Día 7
    void compactar();                   // Día 7

private:
    Scheduler    scheduler_;
    GestorMemoria memoria_;
    SistemaArchivos fs_;
    GestorES gestorES_;
    ControladorInterrupciones controladorInterrupciones_;
    int siguienteBloqueDatos_;

    // pid → info de memoria
    std::unordered_map<int, InfoMemoriaProceso> infoMemoria_;

    void liberarMemoriaProceso(int pid);
};

#endif // KERNEL_H
