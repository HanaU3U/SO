#include "Kernel/kernel.h"
#include <iostream>
#include <windows.h>

// ─────────────────────────────────────────────────────────────
//  Helpers de sección
// ─────────────────────────────────────────────────────────────
static void seccion(const std::string& titulo) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  TEST: " << titulo << "\n";
    std::cout << std::string(60, '=') << "\n";
}

// ─────────────────────────────────────────────────────────────
//  TEST 1: Creación de procesos con paginación y ejecución RR
// ─────────────────────────────────────────────────────────────
static void test_procesos_paginacion() {
    seccion("Procesos con paginacion + Round-Robin");

    Kernel kernel;

    // Crear procesos: (nombre, cpuTime, memoriaKB, quantum, usarPaginacion)
    PCB* p1 = kernel.crearProceso("Proceso-A", 15, 20, 5, true);
    PCB* p2 = kernel.crearProceso("Proceso-B", 10, 12, 3, true);
    PCB* p3 = kernel.crearProceso("Proceso-C",  8, 16, 4, true);

    std::cout << "\n--- Tabla de paginas tras creacion ---\n";
    kernel.imprimirTablaPaginas(p1->pid);
    kernel.imprimirTablaPaginas(p2->pid);

    std::cout << "\n--- Estado de procesos ---\n";
    kernel.imprimirEstado();

    std::cout << "\n--- Memoria antes de ejecutar ---\n";
    kernel.imprimirMemoria();

    // Ejecutar el scheduler 40 unidades de tiempo
    kernel.ejecutar(40);

    std::cout << "\n--- Estado final de procesos ---\n";
    kernel.imprimirEstado();

    std::cout << "\n--- Memoria despues de ejecutar ---\n";
    kernel.imprimirMemoria();

    // Liberar manualmente el proceso C (terminó por scheduler pero demo de liberación)
    kernel.terminarProceso(p3->pid);
    kernel.imprimirMemoria();
}

// ─────────────────────────────────────────────────────────────
//  TEST 2: Segmentación (alocación contigua + segmentos)
// ─────────────────────────────────────────────────────────────
static void test_segmentacion() {
    seccion("Segmentacion");

    Kernel kernel;

    // Crear proceso con segmentos (usarPaginacion=false)
    PCB* p = kernel.crearProceso("Proceso-Seg", 6, 30, 5, false);

    std::cout << "\n--- Segmentos del proceso ---\n";
    kernel.imprimirSegmentos(p->pid);

    std::cout << "\n--- Mapa de memoria contigua ---\n";
    kernel.imprimirMemoria();

    kernel.ejecutar(10);
    kernel.terminarProceso(p->pid);

    std::cout << "\n--- Memoria tras liberar proceso ---\n";
    kernel.imprimirMemoria();
}

// ─────────────────────────────────────────────────────────────
//  TEST 3: Fragmentación, Best-Fit y Compactación (Días 6-7)
// ─────────────────────────────────────────────────────────────
static void test_fragmentacion_y_bestfit() {
    seccion("Fragmentacion, Best-Fit y Compactacion (Dias 6-7)");

    // Usar GestorMemoria directamente para controlar el algoritmo
    GestorMemoria mem;

    std::cout << "\n--- [FIRST-FIT] Asignando 3 procesos ---\n";
    mem.asignarContiguo(100, 200, FIRST_FIT);  // PID 100, 200 KB
    mem.asignarContiguo(101,  50, FIRST_FIT);  // PID 101,  50 KB
    mem.asignarContiguo(102, 300, FIRST_FIT);  // PID 102, 300 KB
    mem.mostrarMapaMemoria();
    mem.mostrarFragmentacion();

    std::cout << "\n--- Liberando PID 100 y PID 102 (crea huecos) ---\n";
    mem.liberarContiguo(100);
    mem.liberarContiguo(102);
    mem.mostrarMapaMemoria();
    mem.mostrarFragmentacion();

    std::cout << "\n--- [BEST-FIT] Asignando 80 KB (elegiria el hueco mas ajustado) ---\n";
    mem.asignarContiguo(103, 80, BEST_FIT);
    mem.mostrarMapaMemoria();
    mem.mostrarFragmentacion();

    std::cout << "\n--- Compactacion: elimina fragmentacion externa ---\n";
    mem.compactar();
    mem.mostrarMapaMemoria();
    mem.mostrarFragmentacion();
}

// ─────────────────────────────────────────────────────────────
//  TEST 3: Memoria compartida entre procesos
// ─────────────────────────────────────────────────────────────
static void test_memoria_compartida() {
    seccion("Memoria Compartida");

    Kernel kernel;

    PCB* p1 = kernel.crearProceso("Editor",   5, 8, 5, true);
    PCB* p2 = kernel.crearProceso("Visor-1",  4, 4, 5, true);
    PCB* p3 = kernel.crearProceso("Visor-2",  4, 4, 5, true);

    // El Editor crea una región compartida (ej. buffer de 16 KB)
    std::cout << "\n--- Creando region compartida ---\n";
    int regionId = kernel.crearMemoriaCompartida(p1->pid, 16);

    // Visor-1 y Visor-2 se adjuntan a la misma región física
    kernel.adjuntarMemoriaCompartida(p2->pid, regionId);
    kernel.adjuntarMemoriaCompartida(p3->pid, regionId);

    std::cout << "\n--- Memoria con region compartida ---\n";
    kernel.imprimirMemoria();

    // Visor-1 se desconecta
    kernel.desadjuntarMemoriaCompartida(p2->pid, regionId);

    // Visor-2 y Editor se desconectan → región se libera
    kernel.desadjuntarMemoriaCompartida(p3->pid, regionId);
    kernel.desadjuntarMemoriaCompartida(p1->pid, regionId);

    std::cout << "\n--- Memoria tras liberar region compartida ---\n";
    kernel.imprimirMemoria();

    kernel.ejecutar(15);
}

// ─────────────────────────────────────────────────────────────
//  TEST 4: Swapping
// ─────────────────────────────────────────────────────────────
static void test_swapping() {
    seccion("Swapping");

    Kernel kernel;

    PCB* p1 = kernel.crearProceso("Proceso-X", 10, 20, 5, true);
    kernel.crearProceso("Proceso-Y", 10, 16, 5, true);

    std::cout << "\n--- Tabla de paginas antes del swap ---\n";
    kernel.imprimirTablaPaginas(p1->pid);

    // Mover proceso-X a disco (swap out)
    std::cout << "\n--- SwapOut Proceso-X ---\n";
    kernel.swapOut(p1->pid);
    kernel.imprimirTablaPaginas(p1->pid);
    kernel.imprimirMemoria();

    // Traer de vuelta a RAM (swap in)
    std::cout << "\n--- SwapIn Proceso-X ---\n";
    kernel.swapIn(p1->pid);
    kernel.imprimirTablaPaginas(p1->pid);
    kernel.imprimirMemoria();

    kernel.ejecutar(25);
}

// ─────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    test_procesos_paginacion();
    test_segmentacion();
    test_fragmentacion_y_bestfit();
    test_memoria_compartida();
    test_swapping();

    std::cout << "\n[OK] Todos los tests completados.\n";
    return 0;
}
