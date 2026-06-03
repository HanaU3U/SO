#include "interrupciones.h"
#include <iostream>

void ControladorInterrupciones::despachar(GestorES& gestorES, Scheduler& scheduler) {
	const std::vector<EventoInterrupcion> eventos = gestorES.consumirInterrupciones();
	for (const auto& evento : eventos) {
		const bool desbloqueado = scheduler.unblockProcess(evento.pid);
		std::cout << "[INTERRUPCION] PID=" << evento.pid
				  << " dispositivo=" << evento.nombreDispositivo
				  << " detalle=\"" << evento.detalle << "\""
				  << " -> " << (desbloqueado ? "READY" : "sin-cambio")
				  << "\n";
	}
}
