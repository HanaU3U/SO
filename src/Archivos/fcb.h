#ifndef FCB_H
#define FCB_H

#include <string>

struct FCB {
	std::string nombre;
	std::string tipo;
	int tamanoKB;
	int bloqueDatos;
	std::string fechaCreacion;
	std::string fechaModificacion;
	std::string permisos;
	bool abierto;
	int aperturasActivas;

	FCB();
	FCB(const std::string& nombreArchivo,
		const std::string& tipoArchivo,
		int tamano,
		int bloque,
		const std::string& permisosArchivo = "rw-");

	void actualizarTamano(int nuevoTamanoKB);
	void tocar();
	void abrir();
	void cerrar();
};

#endif // FCB_H
