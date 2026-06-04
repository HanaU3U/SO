#ifndef INTERRUPCIONES_H
#define INTERRUPCIONES_H

#include "dispositivo.h"
#include "../Procesos/scheduler.h"

class ControladorInterrupciones {
public:
    void despachar(GestorES& gestorES, Scheduler& scheduler);
};

#endif // INTERRUPCIONES_H
