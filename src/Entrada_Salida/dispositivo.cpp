#include "dispositivo.h"
#include <iostream>

DispositivoES::DispositivoES()
	: nombre(), tipo(), estado(EstadoDispositivo::LIBRE),
	  tieneSolicitudActiva(false), solicitudActiva{} {}

DispositivoES::DispositivoES(const std::string& nombreDispositivo, const std::string& tipoDispositivo)
	: nombre(nombreDispositivo), tipo(tipoDispositivo), estado(EstadoDispositivo::LIBRE),
	  tieneSolicitudActiva(false), solicitudActiva{} {}

bool GestorES::registrarDispositivo(const std::string& nombre, const std::string& tipo) {
	if (dispositivos_.count(nombre) > 0) {
		return false;
	}
	dispositivos_.emplace(nombre, DispositivoES(nombre, tipo));
	return true;
}

bool GestorES::solicitarIO(int pid,
						  const std::string& nombreDispositivo,
						  const std::string& operacion,
						  int duracionTicks) {
	auto it = dispositivos_.find(nombreDispositivo);
	if (it == dispositivos_.end() || duracionTicks <= 0) {
		return false;
	}

	SolicitudES solicitud{pid, nombreDispositivo, operacion, duracionTicks, duracionTicks};
	it->second.colaSolicitudes.push(solicitud);
	return true;
}

void GestorES::procesarTick() {
	for (std::unordered_map<std::string, DispositivoES>::iterator it = dispositivos_.begin();
		 it != dispositivos_.end();
		 ++it) {
		DispositivoES& dispositivo = it->second;
		if (!dispositivo.tieneSolicitudActiva && !dispositivo.colaSolicitudes.empty()) {
			dispositivo.solicitudActiva = dispositivo.colaSolicitudes.front();
			dispositivo.colaSolicitudes.pop();
			dispositivo.tieneSolicitudActiva = true;
			dispositivo.estado = EstadoDispositivo::OCUPADO;
		}

		if (!dispositivo.tieneSolicitudActiva) {
			continue;
		}

		--dispositivo.solicitudActiva.ticksRestantes;
		if (dispositivo.solicitudActiva.ticksRestantes > 0) {
			continue;
		}

		EventoInterrupcion evento;
		evento.pid = dispositivo.solicitudActiva.pid;
		evento.nombreDispositivo = dispositivo.nombre;
		evento.detalle = "Operacion completada: " + dispositivo.solicitudActiva.operacion;
		interrupcionesPendientes_.push(evento);

		dispositivo.tieneSolicitudActiva = false;
		dispositivo.estado = EstadoDispositivo::LIBRE;
	}
}

std::vector<EventoInterrupcion> GestorES::consumirInterrupciones() {
	std::vector<EventoInterrupcion> eventos;
	while (!interrupcionesPendientes_.empty()) {
		eventos.push_back(interrupcionesPendientes_.front());
		interrupcionesPendientes_.pop();
	}
	return eventos;
}

bool GestorES::haySolicitudesPendientes() const {
	if (!interrupcionesPendientes_.empty()) {
		return true;
	}

	for (std::unordered_map<std::string, DispositivoES>::const_iterator it = dispositivos_.begin();
		 it != dispositivos_.end();
		 ++it) {
		const DispositivoES& dispositivo = it->second;
		if (dispositivo.tieneSolicitudActiva || !dispositivo.colaSolicitudes.empty()) {
			return true;
		}
	}
	return false;
}

bool GestorES::existeDispositivo(const std::string& nombre) const {
	return dispositivos_.count(nombre) > 0;
}

void GestorES::imprimirTablaDispositivos() const {
	std::cout << "\n[TABLA E/S]\n";
	if (dispositivos_.empty()) {
		std::cout << "  (sin dispositivos)\n";
		return;
	}

	for (std::unordered_map<std::string, DispositivoES>::const_iterator it = dispositivos_.begin();
		 it != dispositivos_.end();
		 ++it) {
		const DispositivoES& disp = it->second;
		std::cout << "  " << disp.nombre
				  << " tipo=" << disp.tipo
				  << " estado=" << (disp.estado == EstadoDispositivo::LIBRE ? "LIBRE" : "OCUPADO")
				  << " cola=" << disp.colaSolicitudes.size()
				  << "\n";
	}
}
