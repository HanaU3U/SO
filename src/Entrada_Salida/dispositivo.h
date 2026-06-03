#ifndef DISPOSITIVO_H
#define DISPOSITIVO_H

#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

enum class EstadoDispositivo {
	LIBRE,
	OCUPADO
};

struct SolicitudES {
	int pid;
	std::string nombreDispositivo;
	std::string operacion;
	int duracionTicks;
	int ticksRestantes;
};

struct EventoInterrupcion {
	int pid;
	std::string nombreDispositivo;
	std::string detalle;
};

struct DispositivoES {
	std::string nombre;
	std::string tipo;
	EstadoDispositivo estado;
	std::queue<SolicitudES> colaSolicitudes;
	bool tieneSolicitudActiva;
	SolicitudES solicitudActiva;

	DispositivoES();
	DispositivoES(const std::string& nombreDispositivo, const std::string& tipoDispositivo);
};

class GestorES {
public:
	bool registrarDispositivo(const std::string& nombre, const std::string& tipo);
	bool solicitarIO(int pid,
					 const std::string& nombreDispositivo,
					 const std::string& operacion,
					 int duracionTicks);

	void procesarTick();
	std::vector<EventoInterrupcion> consumirInterrupciones();

	bool haySolicitudesPendientes() const;
	bool existeDispositivo(const std::string& nombre) const;
	void imprimirTablaDispositivos() const;

private:
	std::unordered_map<std::string, DispositivoES> dispositivos_;
	std::queue<EventoInterrupcion> interrupcionesPendientes_;
};

#endif // DISPOSITIVO_H
