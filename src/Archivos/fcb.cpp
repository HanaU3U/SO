#include "fcb.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

static std::string ahoraISO8601() {
	const auto now = std::chrono::system_clock::now();
	const std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
	std::tm localTime{};
	if (std::tm* timeInfo = std::localtime(&timeNow)) {
		localTime = *timeInfo;
	}

	std::ostringstream out;
	out << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
	return out.str();
}

FCB::FCB()
	: nombre(), tipo(), tamanoKB(0), bloqueDatos(-1),
	  fechaCreacion(ahoraISO8601()), fechaModificacion(fechaCreacion),
	  permisos("rw-"), abierto(false), aperturasActivas(0) {}

FCB::FCB(const std::string& nombreArchivo,
		 const std::string& tipoArchivo,
		 int tamano,
		 int bloque,
		 const std::string& permisosArchivo)
	: nombre(nombreArchivo), tipo(tipoArchivo), tamanoKB(tamano), bloqueDatos(bloque),
	  fechaCreacion(ahoraISO8601()), fechaModificacion(fechaCreacion),
	  permisos(permisosArchivo), abierto(false), aperturasActivas(0) {}

void FCB::actualizarTamano(int nuevoTamanoKB) {
	tamanoKB = nuevoTamanoKB;
	tocar();
}

void FCB::tocar() {
	fechaModificacion = ahoraISO8601();
}

void FCB::abrir() {
	++aperturasActivas;
	abierto = true;
	tocar();
}

void FCB::cerrar() {
	if (aperturasActivas > 0) {
		--aperturasActivas;
	}
	abierto = (aperturasActivas > 0);
	tocar();
}
