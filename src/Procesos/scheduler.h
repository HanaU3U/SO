#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"
#include <queue>
#include <vector>
#include <iostream>

class Scheduler {
private:
    std::queue<PCB*> readyQueue;        // Cola de procesos listos
    std::vector<PCB*> allProcesses;     // Todos los procesos creados
    PCB* runningProcess;                // Proceso actualmente en ejecución
    int nextPID;                        // Próximo PID disponible
    int currentTime;                    // Tiempo actual del simulador
    
public:
    // Constructor y Destructor
    Scheduler();
    ~Scheduler();
    
    // Gestión de procesos
    PCB* createProcess(const std::string& name, int cpuTimeNeeded, int quantum = 5);
    void addToReadyQueue(PCB* process);
    
    // Planificación
    void scheduleNextProcess();
    PCB* getRunningProcess() const;
    
    // Ejecución con Round-Robin
    void executeTimeSlice(int timeSlice);
    void runScheduler(int totalSimulationTime);
    
    // Transiciones de estado
    void transitionNewToReady(PCB* process);
    void transitionReadyToRunning(PCB* process);
    void transitionRunningToReady(PCB* process);
    void transitionRunningToTerminated(PCB* process);
    
    // Utilidades
    void printReadyQueue() const;
    void printProcessStatus() const;
    void printStatistics() const;
    bool hasReadyProcesses() const;
};

#endif // SCHEDULER_H
