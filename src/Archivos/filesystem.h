#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "fcb.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct NodoDirectorio {
    bool esDirectorio;
    std::string nombre;
    FCB fcb;
    std::unordered_map<std::string, std::unique_ptr<NodoDirectorio>> hijos;

    explicit NodoDirectorio(const std::string& n)
        : esDirectorio(true), nombre(n), fcb() {}

    NodoDirectorio(const std::string& n, const FCB& archivo)
        : esDirectorio(false), nombre(n), fcb(archivo) {}
};

class SistemaArchivos {
public:
    SistemaArchivos();

    bool crearDirectorio(const std::string& ruta);
    bool crearArchivo(const std::string& ruta,
                      const std::string& tipo,
                      int tamanoKB,
                      int bloqueInicio,
                      const std::string& permisos = "rw-");

    bool abrirArchivo(int pid, const std::string& ruta);
    bool cerrarArchivo(int pid, const std::string& ruta);
    void cerrarTodosArchivosProceso(int pid);

    bool existe(const std::string& ruta) const;
    void listarDirectorio(const std::string& ruta = "/") const;

private:
    std::unique_ptr<NodoDirectorio> raiz_;
    std::unordered_map<int, std::unordered_set<std::string>> abiertosPorProceso_;

    static std::vector<std::string> dividirRuta(const std::string& ruta);
    NodoDirectorio* navegar(const std::string& ruta);
    const NodoDirectorio* navegar(const std::string& ruta) const;
};

#endif // FILESYSTEM_H
