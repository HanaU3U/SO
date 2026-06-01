#include "pcb.h"
#include <iostream>
#include <ctime>

// Constructor del PCB
PCB::PCB(int id, const std::string& processName, int totalTime, int quantum)
    : pid(id), 
      name(processName), 
      state(NEW), 
      priority(5),
      cpuTimeRemaining(totalTime),
      cpuTimeUsed(0),
      creationTime(time(nullptr)),
      timeQuantum(quantum) {
}

// Cambiar el estado del proceso
void PCB::setState(ProcessState newState) {
    ProcessState oldState = state;
    state = newState;
    
    // Log de cambio de estado
    // VIEJOOOO: std::string stateNames[] = {"NEW", "READY", "RUNNING", "TERMINATED"};
    static const std::string stateNames[] = {"NEW", "READY", "RUNNING", "TERMINATED"};
    std::cout << "[TRANSICIÓN] Proceso " << name << " (PID: " << pid 
              << "): " << stateNames[oldState] << " -> " 
              << stateNames[newState] << std::endl;
}

// Obtener el estado actual del proceso
ProcessState PCB::getState() const {
    return state;
}

// Actualizar el tiempo de CPU utilizado
void PCB::updateCpuUsage(int timeSlice) {
    int executed = (timeSlice > cpuTimeRemaining) ? cpuTimeRemaining : timeSlice;
    cpuTimeUsed += executed;
    cpuTimeRemaining -= executed;
}

// Verificar si el proceso ha terminado su tiempo
bool PCB::isTimeFinished() const {
    return cpuTimeRemaining <= 0;
}
