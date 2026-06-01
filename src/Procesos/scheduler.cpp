#include "scheduler.h"
#include <iostream>
#include <iomanip>
#include <cassert>

// Constructor
Scheduler::Scheduler() 
    : runningProcess(nullptr), nextPID(1000), currentTime(0) {
}

// Destructor
Scheduler::~Scheduler() {
    readyQueue = std::queue<PCB*>();   // vaciado sin desreferenciar
    for (PCB* p : allProcesses) delete p;
    allProcesses.clear();
}

/*VIEJOOOOO: Scheduler::~Scheduler() {
    for (PCB* process : allProcesses) {
        delete process;
    }
    allProcesses.clear();
}*/

// Crear un nuevo proceso
PCB* Scheduler::createProcess(const std::string& name, int cpuTimeNeeded, int quantum) {
    PCB* newProcess = new PCB(nextPID++, name, cpuTimeNeeded, quantum);
    allProcesses.push_back(newProcess);
    
    std::cout << "[CREAR] Proceso " << name << " creado (PID: " 
              << newProcess->pid << ", Tiempo requerido: " << cpuTimeNeeded << ")" << std::endl;
    
    return newProcess;
}

// Agregar proceso a la cola de listos
void Scheduler::addToReadyQueue(PCB* process) {
    assert(process->getState() == READY);
    readyQueue.push(process);
}

/*VIEJOOO: void Scheduler::addToReadyQueue(PCB* process) {
    if (process) {
        readyQueue.push(process);
    }
}*/

// Transición: Nuevo -> Listo
void Scheduler::transitionNewToReady(PCB* process) {
    if (process && process->getState() == NEW) {
        process->setState(READY);
        addToReadyQueue(process);
    }
}

// Transición: Listo -> Ejecución
void Scheduler::transitionReadyToRunning(PCB* process) {
    if (process && process->getState() == READY) {
        // Si hay un proceso en ejecución, primero devolverlo a Listo
        if (runningProcess != nullptr && runningProcess->getState() == RUNNING) {
            transitionRunningToReady(runningProcess);
        }
        
        process->setState(RUNNING);
        runningProcess = process;
    }
}

// Transición: Ejecución -> Listo
void Scheduler::transitionRunningToReady(PCB* process) {
    if (process && process->getState() == RUNNING) {
        process->setState(READY);
        addToReadyQueue(process);
        runningProcess = nullptr;
    }
}

// Transición: Ejecución -> Terminado

void Scheduler::transitionRunningToTerminated(PCB* process) {
    if (process && process->getState() == RUNNING) {
        process->setState(TERMINATED);
        std::cout << "[COMPLETADO] Proceso \"" << process->name
                  << "\" ha completado su ejecución." << std::endl;
        runningProcess = nullptr;
    }
}

/*VIEJOOO:void Scheduler::transitionRunningToTerminated(PCB* process) {
    if (process && process->getState() == RUNNING) {
        process->setState(TERMINATED);
        runningProcess = nullptr;
    }
}*/

// Seleccionar el siguiente proceso a ejecutar (Round-Robin)
void Scheduler::scheduleNextProcess() {
    if (!readyQueue.empty()) {
        PCB* nextProcess = readyQueue.front();
        readyQueue.pop();
        transitionReadyToRunning(nextProcess);
    } else {
        runningProcess = nullptr;
    }
}

// Obtener el proceso en ejecución
PCB* Scheduler::getRunningProcess() const {
    return runningProcess;
}

// Ejecutar un slice de tiempo con Round-Robin
void Scheduler::executeTimeSlice(int timeSlice) {
    if (runningProcess == nullptr) {
        scheduleNextProcess();
    }
    
    if (runningProcess != nullptr) {
        std::cout << "\n[EJECUCIÓN T=" << currentTime << "] Proceso \"" 
                  << runningProcess->name << "\" ejecutándose..." << std::endl;
        
        // Ejecutar el proceso por el time slice
        runningProcess->updateCpuUsage(timeSlice);
        
        std::cout << "[INFO] Tiempo usado: " << runningProcess->cpuTimeUsed 
                  << " / Tiempo restante: " << runningProcess->cpuTimeRemaining << std::endl;
        
        // Verificar si el proceso terminó

        if (runningProcess->isTimeFinished()) {
            transitionRunningToTerminated(runningProcess);
        } else {
            transitionRunningToReady(runningProcess);
        }
        
        /*VIEJOOO: if (runningProcess->isTimeFinished()) {
            transitionRunningToTerminated(runningProcess);
            std::cout << "[COMPLETADO] Proceso \"" << runningProcess->name 
                      << "\" ha completado su ejecución." << std::endl;
        } else {
            // Si el quantum no está agotado pero el proceso terminó, no lo devolvemos a cola
            if (runningProcess->cpuTimeRemaining > 0) {
                transitionRunningToReady(runningProcess);
            }
        }*/
    }
    
    currentTime += timeSlice;
}

// Ejecutar el simulador de planificación
void Scheduler::runScheduler(int totalSimulationTime) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "INICIANDO SIMULADOR DE PLANIFICACIÓN (Round-Robin)" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    while (currentTime < totalSimulationTime && (!readyQueue.empty() || runningProcess != nullptr)) {
        //VIEJOOO: executeTimeSlice(5);  // Quantum de 5 unidades de tiempo
        int quantum = (runningProcess != nullptr) ? runningProcess->timeQuantum : 5;
        executeTimeSlice(quantum);
        
        if (!readyQueue.empty()) {
            std::cout << "[COLA] Procesos listos: " << readyQueue.size() << std::endl;
        }
    }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "SIMULACIÓN COMPLETADA" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

// Imprimir la cola de listos
void Scheduler::printReadyQueue() const {
    std::cout << "\n[COLA DE LISTOS]" << std::endl;
    if (readyQueue.empty()) {
        std::cout << "  (vacía)" << std::endl;
    } else {
        std::queue<PCB*> tempQueue = readyQueue;
        int pos = 1;
        while (!tempQueue.empty()) {
            PCB* p = tempQueue.front();
            tempQueue.pop();
            std::cout << "  " << pos++ << ". " << p->name << " (PID: " 
                      << p->pid << ", Restante: " << p->cpuTimeRemaining << ")" << std::endl;
        }
    }
}

// Imprimir estado de todos los procesos
void Scheduler::printProcessStatus() const {
    std::string stateNames[] = {"NEW", "READY", "RUNNING", "TERMINATED"};
    
    std::cout << "\n[ESTADO DE PROCESOS]" << std::endl;
    std::cout << std::left << std::setw(15) << "Nombre" 
              << std::setw(10) << "PID"
              << std::setw(12) << "Estado"
              << std::setw(10) << "Usado"
              << std::setw(10) << "Restante" << std::endl;
    std::cout << std::string(57, '-') << std::endl;
    
    for (const PCB* p : allProcesses) {
        std::cout << std::left << std::setw(15) << p->name
                  << std::setw(10) << p->pid
                  << std::setw(12) << stateNames[p->state]
                  << std::setw(10) << p->cpuTimeUsed
                  << std::setw(10) << p->cpuTimeRemaining << std::endl;
    }
}

// Imprimir estadísticas finales
void Scheduler::printStatistics() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "ESTADÍSTICAS FINALES" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    int totalTimeUsed = 0;
    int completedProcesses = 0;
    
    for (const PCB* p : allProcesses) {
        totalTimeUsed += p->cpuTimeUsed;
        if (p->state == TERMINATED) {
            completedProcesses++;
        }
    }
    
    std::cout << "Total de procesos: " << allProcesses.size() << std::endl;
    std::cout << "Procesos completados: " << completedProcesses << std::endl;
    std::cout << "Tiempo total de CPU utilizado: " << totalTimeUsed << std::endl;
    std::cout << "Tiempo de simulación: " << currentTime << std::endl;
    
    printProcessStatus();
}

// Verificar si hay procesos listos
bool Scheduler::hasReadyProcesses() const {
    return !readyQueue.empty();
}
