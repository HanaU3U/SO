#ifndef PCB_H
#define PCB_H

#include <string>
#include <ctime>

// Estados posibles de un proceso
enum ProcessState {
    NEW,        // Nuevo
    READY,      // Listo
    RUNNING,    // En ejecución
    TERMINATED  // Terminado
};

// Estructura del Bloque de Control de Proceso (PCB)
struct PCB {
    int pid;                    // Process ID
    std::string name;           // Nombre del proceso
    ProcessState state;         // Estado actual del proceso
    int priority;               // Prioridad (0=highest, 10=lowest)
    int cpuTimeRemaining;       // Tiempo de CPU restante (en unidades de tiempo)
    int cpuTimeUsed;            // Tiempo de CPU utilizado
    time_t creationTime;        // Timestamp de creación
    int timeQuantum;            // Quantum de tiempo para Round-Robin
    
    // Constructor
    PCB(int id, const std::string& processName, int totalTime, int quantum = 5);
    
    // Métodos
    void setState(ProcessState newState);
    ProcessState getState() const;
    void updateCpuUsage(int timeSlice);
    bool isTimeFinished() const;
};

#endif // PCB_H
