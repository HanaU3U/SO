# Diseño del Mini Kernel — SO

## Sistema de Gestión de Memoria

### Archivos

| Archivo | Responsabilidad |
|---|---|
| `src/Memoria/memoria.h` | Estructuras y declaraciones de toda la capa de memoria |
| `src/Memoria/memoria.cpp` | Alocación contigua, segmentación, compartición |
| `src/Memoria/paginacion.cpp` | Paginación, tabla de páginas, swapping |

---

### Constantes del sistema

| Constante | Valor | Descripción |
|---|---|---|
| `MEMORIA_TOTAL` | 1024 KB | RAM simulada |
| `TAMANO_PAGINA` | 4 KB | Tamaño de página/marco |
| `NUM_MARCOS` | 256 | Marcos físicos disponibles |
| `TAMANO_SWAP` | 2048 KB | Espacio de intercambio en disco |

---

### 1. Compartición de Memoria

Permite que varios procesos accedan a la **misma región física** sin duplicar datos.  
Se implementa con `RegionCompartida`, que mantiene un conteo de referencias (`refCount`).  
La región se libera automáticamente cuando `refCount` llega a 0.

```
Proceso A ──┐
             ├──▶ [base=512 KB, tam=32 KB]  ← misma dirección física
Proceso B ──┘
```

**API:**
```cpp
int  crearRegionCompartida(int pid, int tamanoKB);  // Crea y adjunta al creador
bool adjuntarRegion(int pid, int regionId);         // Otro proceso la mapea
void desadjuntarRegion(int pid, int regionId);      // Decrementa refCount
```

---

### 2. Alocación Contigua

El espacio de memoria se representa como una lista de `BloqueMemoria`.  
Se usa el algoritmo **First Fit**: se recorre la lista y se asigna el primer bloque libre con tamaño suficiente.  
Al liberar, los bloques adyacentes libres se fusionan (**coalescing**).

```
[ LIBRE 1024 KB ]
    ↓ asignarContiguo(PID=1, 100 KB)
[ USADO 100 KB | LIBRE 924 KB ]
    ↓ asignarContiguo(PID=2, 200 KB)
[ USADO 100 KB | USADO 200 KB | LIBRE 724 KB ]
    ↓ liberarContiguo(PID=1)
[ LIBRE 100 KB | USADO 200 KB | LIBRE 724 KB ]
    ↓ coalescing no aplica (no son adyacentes libres ambos lados del bloque 2)
```

**API:**
```cpp
int  asignarContiguo(int pid, int tamanoKB);  // Retorna dirección base
void liberarContiguo(int pid);
void mostrarMapaMemoria() const;
```

---

### 3. Segmentación

Cada proceso tiene **3 segmentos** dentro de su bloque contiguo:

| Segmento | Protección | Contenido |
|---|---|---|
| `codigo` | Solo lectura (RO) | Instrucciones del programa |
| `datos` | Lectura/Escritura | Variables globales y heap |
| `pila` | Lectura/Escritura | Stack de llamadas |

Al acceder a un segmento se valida que el `offset < límite`; de lo contrario se genera un **segfault** simulado.

```
Dirección lógica: (segmento="datos", offset=20)
    → base_datos + 20 = dirección física
```

**API:**
```cpp
bool crearSegmentos(int pid, int tamCodigo, int tamDatos, int tamPila);
bool accederSegmento(int pid, const std::string& segmento, int offset);
void liberarSegmentos(int pid);
void imprimirSegmentos(int pid) const;
```

---

### 4. Paginación

La memoria física se divide en **marcos** de tamaño fijo (4 KB).  
La memoria lógica de cada proceso se divide en **páginas** del mismo tamaño.  
El array `marcos_[256]` representa qué PID ocupa cada marco (-1 = libre).

```
Dirección lógica (32 bits simulada):
┌─────────────────────┬────────────────┐
│  Número de página   │     Offset     │
│      20 bits        │    12 bits     │
└─────────────────────┴────────────────┘
```

**API:**
```cpp
bool asignarPaginas(int pid, int numPaginas);
int  traducirDireccion(int pid, int dirLogica);  // Retorna dirección física
void liberarPaginas(int pid);
```

---

### 5. Estructura de la Tabla de Páginas

Cada proceso tiene un vector de `PaginaEntry`. Cada entrada contiene:

| Campo | Tipo | Descripción |
|---|---|---|
| `marcoFisico` | `int` | Marco asignado en RAM (-1 si no está) |
| `presente` | `bool` | Está la página en RAM |
| `modificada` | `bool` | Dirty bit — fue escrita |
| `referenciada` | `bool` | Reference bit — fue accedida |
| `swapOffset` | `int` | Posición en disco swap (-1 si no aplica) |

```
Tabla de Páginas PID=3:
Pág  Marco  Presente  Modif.  Ref.  Swap
0    14     Sí        No      Sí    -
1    27     Sí        Sí      Sí    -
2    -      No        No      No    5
```

**API:**
```cpp
void imprimirTablaPaginas(int pid) const;
```

---

### 6. Intercambio (Swapping)

Cuando no hay marcos libres para un proceso, sus páginas se mueven a **disco swap** (`swapOut`).  
Cuando el proceso vuelve a ejecutarse, sus páginas se recuperan (`swapIn`).

**Flujo:**
```
swapOut(pid):
  Para cada página presente:
    1. Asignar slot en swap
    2. Liberar marco físico
    3. Marcar presente=false, swapOffset=slot

swapIn(pid):
  Para cada página en swap:
    1. Encontrar marco libre en RAM
    2. Leer desde disco (simulado)
    3. Marcar presente=true, liberar slot swap
```

**API:**
```cpp
bool swapOut(int pid);
bool swapIn(int pid);
bool hayEspacioRAM(int numPaginas) const;
```

---

### Diagrama de clases simplificado

```
GestorMemoria
├── bloques_          : vector<BloqueMemoria>       → Alocación contigua
├── tablaSegmentos_   : map<pid, vector<Segmento>>  → Segmentación
├── tablaPaginas_     : map<pid, vector<PaginaEntry>>→ Paginación
├── marcos_           : vector<int>[256]             → Marcos físicos
├── swapLibre_        : vector<bool>                 → Slots de swap
└── regionesCompartidas_: vector<RegionCompartida>  → Compartición
```

---

## Kernel — Integración de Procesos y Memoria

### Archivos

| Archivo | Responsabilidad |
|---|---|
| `src/Kernel/kernel.h` | Clase `Kernel`: declara la integración de `Scheduler` + `GestorMemoria` |
| `src/Kernel/kernel.cpp` | Implementación: ciclo de vida completo de procesos con memoria |
| `src/main.cpp` | Tests de integración que ejercen toda la funcionalidad |

---

### Responsabilidad del Kernel

El `Kernel` actúa como la capa de coordinación entre el **gestor de procesos** (`Scheduler`) y el **gestor de memoria** (`GestorMemoria`). Ninguna de las dos capas se conoce entre sí; el kernel las une.

```
          crearProceso()
               │
        ┌──────▼──────────────────────────┐
        │            Kernel               │
        │                                 │
        │  scheduler_.createProcess()     │
        │  memoria_.asignarPaginas()  ─── │──▶ GestorMemoria
        │  scheduler_.transitionNew…()    │
        └─────────────────────────────────┘
               │
           PCB* listo en cola READY
```

### Estructura `InfoMemoriaProceso`

Cada proceso activo tiene asociada una entrada que describe cómo está usando la memoria:

| Campo | Tipo | Descripción |
|---|---|---|
| `baseContigua` | `int` | Dirección base si usa segmentación (-1 si usa paginación) |
| `numPaginas` | `int` | Páginas asignadas (0 si usa segmentación) |
| `usaPaginacion` | `bool` | Modo de memoria del proceso |

---

### API del Kernel

**Ciclo de vida de procesos:**
```cpp
// Crea PCB + asigna memoria + pone en cola READY
PCB* crearProceso(nombre, cpuTime, memoriaKB, quantum=5, usarPaginacion=true);

// Libera PCB + devuelve memoria al sistema
void terminarProceso(int pid);
```

**Ejecución:**
```cpp
void ejecutar(int tiempoTotal);  // Lanza el scheduler Round-Robin
```

**Memoria compartida:**
```cpp
int  crearMemoriaCompartida(int pid, int tamanoKB);  // Crea región y adjunta al creador
bool adjuntarMemoriaCompartida(int pid, int regionId);
void desadjuntarMemoriaCompartida(int pid, int regionId);
```

**Swapping:**
```cpp
bool swapOut(int pid);  // Mueve páginas a disco, libera marcos
bool swapIn(int pid);   // Recupera páginas del disco a RAM
```

**Diagnóstico:**
```cpp
void imprimirEstado() const;              // Estado de todos los procesos
void imprimirMemoria() const;             // Mapa de memoria + uso
void imprimirTablaPaginas(int pid) const;
void imprimirSegmentos(int pid) const;
```

---

### Flujo de `crearProceso()`

```
1. scheduler_.createProcess(nombre, cpuTime, quantum)
      → PCB creado en estado NEW

2a. Si usarPaginacion=true:
      paginas = ceil(memoriaKB / TAMANO_PAGINA)
      memoria_.asignarPaginas(pid, paginas)
        → Si falla: intenta swapOut de otro proceso y reintenta

2b. Si usarPaginacion=false:
      tamCodigo = memoriaKB/3
      tamDatos  = memoriaKB/3
      tamPila   = resto
      memoria_.crearSegmentos(pid, tamCodigo, tamDatos, tamPila)

3. scheduler_.transitionNewToReady(proceso)
      → PCB pasa a READY y entra en la cola del scheduler
```

---

### Diagrama de integración completo

```
┌─────────────────────────────────────────────┐
│                   Kernel                    │
│                                             │
│  ┌─────────────┐      ┌──────────────────┐  │
│  │  Scheduler  │      │  GestorMemoria   │  │
│  │             │      │                  │  │
│  │ readyQueue  │      │ bloques_         │  │
│  │ allProcs    │      │ tablaPaginas_    │  │
│  │ Round-Robin │      │ tablaSegmentos_  │  │
│  └──────┬──────┘      │ marcos_[256]     │  │
│         │             │ swapLibre_       │  │
│         │             └──────────────────┘  │
│         └── infoMemoria_[pid] ──────────────┤
└─────────────────────────────────────────────┘
```

---

## Tests de integración (`main.cpp`)

Los 4 tests se ejecutan secuencialmente al correr el binario. Cada uno instancia un `Kernel` independiente.

### Test 1 — Procesos con Paginación + Round-Robin

Crea 3 procesos con páginas asignadas y los corre con Round-Robin.  
Verifica que las tablas de páginas se crean correctamente y que el scheduler los termina.

```
Proceso-A: cpuTime=15, mem=20 KB → 5 páginas, quantum=5
Proceso-B: cpuTime=10, mem=12 KB → 3 páginas, quantum=3
Proceso-C: cpuTime= 8, mem=16 KB → 4 páginas, quantum=4
Simulación: 40 unidades de tiempo
```

### Test 2 — Segmentación

Crea un proceso con alocación contigua dividida en segmentos código/datos/pila.  
Verifica el mapa de memoria y la liberación por coalescing.

```
Proceso-Seg: cpuTime=6, mem=30 KB
  → codigo: 10 KB (RO)
  → datos:  10 KB (RW)
  → pila:   10 KB (RW)
```

### Test 3 — Memoria Compartida

Un proceso Editor crea una región compartida de 16 KB.  
Dos Visores se adjuntan a la misma dirección física (sin copiar datos).  
Al desadjuntarse todos, la región se libera automáticamente.

```
Editor   ──crea──▶ región #1 [base=X, 16 KB]
Visor-1  ──adjunta▶ región #1  (refCount=2)
Visor-2  ──adjunta▶ región #1  (refCount=3)
Visor-1  ──desadjunta          (refCount=2)
Visor-2  ──desadjunta          (refCount=1)
Editor   ──desadjunta          (refCount=0 → liberada)
```

### Test 4 — Swapping

Demuestra el ciclo completo swap out → swap in.

```
swapOut(Proceso-X):
  marcos liberados → páginas marcadas presente=false, swapOffset=slot

swapIn(Proceso-X):
  páginas traídas de swap → marcos reasignados, presente=true
```

