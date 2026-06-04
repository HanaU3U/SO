#include "filesystem.h"
#include <algorithm>
#include <iostream>

SistemaArchivos::SistemaArchivos()
	: raiz_(std::unique_ptr<NodoDirectorio>(new NodoDirectorio("/"))) {}

std::vector<std::string> SistemaArchivos::dividirRuta(const std::string& ruta) {
	std::vector<std::string> partes;
	std::string token;
	for (char c : ruta) {
		if (c == '/') {
			if (!token.empty()) {
				partes.push_back(token);
				token.clear();
			}
			continue;
		}
		token.push_back(c);
	}
	if (!token.empty()) {
		partes.push_back(token);
	}
	return partes;
}

NodoDirectorio* SistemaArchivos::navegar(const std::string& ruta) {
	if (ruta.empty() || ruta == "/") {
		return raiz_.get();
	}

	NodoDirectorio* actual = raiz_.get();
	for (const auto& parte : dividirRuta(ruta)) {
		auto it = actual->hijos.find(parte);
		if (it == actual->hijos.end() || !it->second->esDirectorio) {
			return nullptr;
		}
		actual = it->second.get();
	}
	return actual;
}

const NodoDirectorio* SistemaArchivos::navegar(const std::string& ruta) const {
	return const_cast<SistemaArchivos*>(this)->navegar(ruta);
}

bool SistemaArchivos::crearDirectorio(const std::string& ruta) {
	if (ruta.empty() || ruta == "/") {
		return true;
	}

	NodoDirectorio* actual = raiz_.get();
	for (const auto& parte : dividirRuta(ruta)) {
		auto it = actual->hijos.find(parte);
		if (it == actual->hijos.end()) {
			actual->hijos[parte] = std::unique_ptr<NodoDirectorio>(new NodoDirectorio(parte));
			actual = actual->hijos[parte].get();
			continue;
		}

		if (!it->second->esDirectorio) {
			return false;
		}
		actual = it->second.get();
	}
	return true;
}

bool SistemaArchivos::crearArchivo(const std::string& ruta,
								   const std::string& tipo,
								   int tamanoKB,
								   int bloqueInicio,
								   const std::string& permisos) {
	if (ruta.empty() || ruta == "/") {
		return false;
	}

	auto partes = dividirRuta(ruta);
	if (partes.empty()) {
		return false;
	}

	const std::string nombreArchivo = partes.back();
	partes.pop_back();

	std::string rutaPadre = "/";
	for (const auto& parte : partes) {
		rutaPadre += parte + "/";
	}
	if (rutaPadre.size() > 1 && rutaPadre.back() == '/') {
		rutaPadre.pop_back();
	}

	NodoDirectorio* padre = navegar(rutaPadre);
	if (!padre || !padre->esDirectorio) {
		return false;
	}

	if (padre->hijos.find(nombreArchivo) != padre->hijos.end()) {
		return false;
	}

	FCB fcb(nombreArchivo, tipo, tamanoKB, bloqueInicio, permisos);
	padre->hijos[nombreArchivo] = std::unique_ptr<NodoDirectorio>(new NodoDirectorio(nombreArchivo, fcb));
	return true;
}

bool SistemaArchivos::abrirArchivo(int pid, const std::string& ruta) {
	auto partes = dividirRuta(ruta);
	if (partes.empty()) {
		return false;
	}

	const std::string nombre = partes.back();
	partes.pop_back();

	std::string rutaPadre = "/";
	for (const auto& parte : partes) {
		rutaPadre += parte + "/";
	}
	if (rutaPadre.size() > 1 && rutaPadre.back() == '/') {
		rutaPadre.pop_back();
	}

	NodoDirectorio* padre = navegar(rutaPadre);
	if (!padre) {
		return false;
	}

	auto it = padre->hijos.find(nombre);
	if (it == padre->hijos.end() || it->second->esDirectorio) {
		return false;
	}

	it->second->fcb.abrir();
	abiertosPorProceso_[pid].insert(ruta);
	return true;
}

bool SistemaArchivos::cerrarArchivo(int pid, const std::string& ruta) {
	auto itTabla = abiertosPorProceso_.find(pid);
	if (itTabla == abiertosPorProceso_.end() || itTabla->second.count(ruta) == 0) {
		return false;
	}

	auto partes = dividirRuta(ruta);
	if (partes.empty()) {
		return false;
	}

	const std::string nombre = partes.back();
	partes.pop_back();

	std::string rutaPadre = "/";
	for (const auto& parte : partes) {
		rutaPadre += parte + "/";
	}
	if (rutaPadre.size() > 1 && rutaPadre.back() == '/') {
		rutaPadre.pop_back();
	}

	NodoDirectorio* padre = navegar(rutaPadre);
	if (!padre) {
		return false;
	}

	auto it = padre->hijos.find(nombre);
	if (it == padre->hijos.end() || it->second->esDirectorio) {
		return false;
	}

	it->second->fcb.cerrar();
	itTabla->second.erase(ruta);
	if (itTabla->second.empty()) {
		abiertosPorProceso_.erase(itTabla);
	}
	return true;
}

void SistemaArchivos::cerrarTodosArchivosProceso(int pid) {
	auto itTabla = abiertosPorProceso_.find(pid);
	if (itTabla == abiertosPorProceso_.end()) {
		return;
	}

	std::vector<std::string> archivos(itTabla->second.begin(), itTabla->second.end());
	for (const auto& ruta : archivos) {
		cerrarArchivo(pid, ruta);
	}
}

bool SistemaArchivos::existe(const std::string& ruta) const {
	if (ruta.empty()) {
		return false;
	}
	if (ruta == "/") {
		return true;
	}

	auto partes = dividirRuta(ruta);
	if (partes.empty()) {
		return false;
	}

	const NodoDirectorio* actual = raiz_.get();
	for (const auto& parte : partes) {
		auto it = actual->hijos.find(parte);
		if (it == actual->hijos.end()) {
			return false;
		}
		actual = it->second.get();
	}
	return true;
}

void SistemaArchivos::listarDirectorio(const std::string& ruta) const {
	const NodoDirectorio* dir = navegar(ruta);
	if (!dir || !dir->esDirectorio) {
		std::cout << "[FS] Directorio no encontrado: " << ruta << "\n";
		return;
	}

	std::cout << "\n[FS] Contenido de " << ruta << ":\n";
	if (dir->hijos.empty()) {
		std::cout << "  (vacio)\n";
		return;
	}

	for (std::unordered_map<std::string, std::unique_ptr<NodoDirectorio>>::const_iterator it = dir->hijos.begin();
		 it != dir->hijos.end();
		 ++it) {
		const std::string& nombre = it->first;
		const std::unique_ptr<NodoDirectorio>& nodo = it->second;
		if (nodo->esDirectorio) {
			std::cout << "  [DIR]  " << nombre << "\n";
			continue;
		}

		const FCB& f = nodo->fcb;
		std::cout << "  [FILE] " << nombre
				  << " tipo=" << f.tipo
				  << " size=" << f.tamanoKB << "KB"
				  << " bloque=" << f.bloqueDatos
				  << " permisos=" << f.permisos
				  << " abierto=" << (f.abierto ? "si" : "no")
				  << "\n";
	}
}
