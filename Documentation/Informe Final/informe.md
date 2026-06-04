
# Simulación de Kernel Didáctico - Documentación Técnica


>Sistemas Operativos - Grupo 020-82  
>Autores:
>> - Tomás Alejandro Delgado Ortíz - 20221020045
>>- Hana Sofía Pinilla Manrique -
>>- Ana Laura Morcote Chacón -

---

## Resumen 

El presente proyecto tiene como objetivo generar una **simulación de un kernel didáctico** que permita mostrar de manera clara y visual los componentes del mismo y la interoperabilidad entre ellos. La simulación incluye la implementación de algoritmos y procedimientos fundamentales para el manejo de **procesos, memoria, archivos y entradas/salidas**, permitiendo a los usuarios observar cómo interactúan estos elementos dentro de un kernel.

El alcance del proyecto abarca:
- **Gestión de procesos:** creación, ejecución y transición de estados de los procesos.
- **Gestión de memoria:** asignación, liberación, segmentación, paginación y swapping.
- **Gestión de archivos:** creación, apertura, cierre y listado de archivos y directorios.
- **Interrupciones y gestión de E/S:** simulación de dispositivos, colas FIFO y desbloqueo de procesos mediante interrupciones.

El proyecto tiene un **enfoque didáctico**, proporcionando un entorno que ilustra los fundamentos de la estructura y construcción de un kernel, facilitando la comprensión de cómo los distintos módulos interactúan y cómo se gestionan los recursos del sistema de manera coordinada.

---

## Introducción
### Contexto y Motivación
EL desarrollo de **Sistemas Operativos** como software es un area fundamental en el desarrollo de competencias dentro de la ingeniería de software. La comprensión de los componentes y los metodos de interoperabilidad de un Kernel brindan una concepción mas clara en cuanto a la busqueda de souluciones para optimización de recursos, solución de errores y previsión de la escalabilidad en la construcción de productos de software.

El proyecto surge con la motivación de ofrecer un **entorno didáctico y visual** que permita observar de manera práctica la estructura y el comportamiento de un kernel. La simulación propuesta busca **demostrar la interoperabilidad de los módulos**, los algoritmos implementados y los procedimientos que permiten la coordinación de recursos en el sistema, facilitando la comprensión de conceptos que en un kernel real serían complejos de analizar directamente.

### Objetivos

1. **Simular un kernel didáctico** que muestre la estructura y la interoperabilidad de sus módulos principales.  
2. **Implementar y demostrar la gestión de procesos**, incluyendo creación, ejecución y transición de estados implementando algoritmos de gestión como Round Robin.  
3. **Gestionar memoria**, incorporando algoritmos de asignación, liberación, paginación, segmentación y swapping.  
4. **Simular un sistema de archivos**, permitiendo la creación, apertura, cierre y listado de archivos y directorios.  
5. **Gestionar entradas y salidas (E/S) e interrupciones**, mostrando cómo los procesos interactúan con dispositivos y se desbloquean mediante eventos.  
6. **Ofrecer un enfoque practico simulado**, proporcionando un entorno visual para entender los fundamentos de un kernel.
### Requerimientos
#### Funcionales


#### No funcionales


---

## Diseño del Sistema
### Arquitectura General
[Diagrama de componentes y explicación]

### Diagrama de Clases
[Espacio para insertar el diagrama de clases Mermaid]

### Flujograma

---

## Estructuras de Datos
[Descripción breve de las clases]

---

## Algoritmos Implementados
### Gestión de Procesos


La gestión de procesos del mini-kernel utiliza un algoritmo de planificación **Round-Robin (RR)** para distribuir equitativamente el tiempo de CPU entre los procesos activos.

#### Round-Robin

Round-Robin es un algoritmo de planificación preventiva que asigna a cada proceso un intervalo fijo de ejecución denominado *quantum*. Una vez agotado dicho intervalo, el proceso es interrumpido y la CPU es asignada al siguiente proceso disponible.

##### Funcionamiento

1. Seleccionar el primer proceso listo para ejecutarse.
2. Asignarle la CPU durante un quantum determinado.
3. Actualizar el tiempo de CPU consumido.
4. Verificar si el proceso finalizó.
5. Si terminó, abandonar la planificación.
6. Si aún requiere tiempo de CPU, devolverlo al final de la cola de listos.
7. Repetir el procedimiento con el siguiente proceso.

##### Pseudocódigo

```text
Mientras existan procesos listos:

    proceso ← siguiente proceso listo

    ejecutar(proceso, quantum)

    Si proceso terminó:
        finalizar proceso
    Sino:
        devolver proceso al final de la cola
```

##### Ventajas

- Distribución equitativa de la CPU.
- Evita que un único proceso monopolice el procesador.
- Reduce el riesgo de inanición.
- Fácil de implementar y mantener.

##### Limitaciones

- Un quantum demasiado pequeño incrementa el número de cambios de contexto.
- Un quantum demasiado grande aproxima el comportamiento a FCFS.

---

#### Gestión de Bloqueo y Desbloqueo

Además de la planificación Round-Robin, el sistema implementa el algoritmo clásico de suspensión por eventos de entrada/salida.

##### Bloqueo

Cuando un proceso solicita una operación de E/S:

1. Se suspende su ejecución.
2. Se libera la CPU.
3. El proceso deja de competir por tiempo de procesador.

##### Desbloqueo

Cuando la operación finaliza:

1. El proceso vuelve a estar disponible.
2. Se reincorpora a la planificación.
3. Podrá recibir CPU nuevamente mediante Round-Robin.

Este mecanismo permite aprovechar el procesador mientras otros procesos esperan recursos externos.

---

### Gestión de Memoria

El gestor de memoria implementa varios algoritmos clásicos utilizados en sistemas operativos para asignación, protección, traducción de direcciones y optimización del uso de la memoria principal.

---

#### First-Fit

First-Fit es un algoritmo de asignación contigua que selecciona el primer bloque libre capaz de satisfacer una solicitud de memoria.

##### Funcionamiento

1. Recorrer secuencialmente los bloques libres.
2. Seleccionar el primero cuyo tamaño sea suficiente.
3. Reservar la memoria solicitada.
4. Si existe espacio sobrante, dividir el bloque.

##### Pseudocódigo

```text
Para cada bloque libre:

    Si tamaño >= solicitado:
        asignar bloque
        terminar búsqueda
```

##### Ventajas

- Baja complejidad.
- Asignación rápida.

##### Desventajas

- Genera fragmentación externa con el tiempo.

---

#### Best-Fit

Best-Fit busca el bloque libre más pequeño que pueda contener la solicitud de memoria.

##### Funcionamiento

1. Examinar todos los bloques disponibles.
2. Encontrar el bloque que minimice el espacio desperdiciado.
3. Realizar la asignación.
4. Mantener el espacio sobrante como bloque libre.

##### Pseudocódigo

```text
mejor ← nulo

Para cada bloque libre:

    Si bloque puede almacenar la solicitud:
        Si bloque es menor que mejor:
            mejor ← bloque

Asignar mejor bloque
```

##### Ventajas

- Reduce el desperdicio inmediato de memoria.

##### Desventajas

- Mayor tiempo de búsqueda.
- Tiende a producir pequeños huecos difíciles de reutilizar.

---

#### Segmentación

La segmentación divide el espacio lógico de un proceso en regiones independientes y verifica que los accesos se mantengan dentro de sus límites.

##### Funcionamiento

1. Identificar el segmento solicitado.
2. Verificar que el desplazamiento pertenezca al rango válido.
3. Traducir la dirección lógica a física.
4. Si el acceso excede el límite permitido, generar una excepción.

##### Validación

```text
0 ≤ offset < límite
```

##### Beneficios

- Protección de memoria.
- Separación lógica entre código, datos y pila.
- Detección de accesos inválidos.

---

#### Paginación

La paginación implementa memoria virtual mediante la división del espacio lógico en páginas y de la memoria física en marcos.

##### Funcionamiento

1. Obtener el número de página.
2. Obtener el desplazamiento.
3. Consultar la tabla de páginas.
4. Localizar el marco correspondiente.
5. Construir la dirección física.

##### Traducción

```text
Dirección Física =
Marco × TamañoPágina + Offset
```

##### Beneficios

- Elimina la fragmentación externa.
- Permite memoria virtual.
- Facilita el intercambio de páginas.

---

#### Manejo de Page Faults

Cuando una página requerida no se encuentra en memoria principal se produce un fallo de página.

##### Funcionamiento

1. Detectar que la página no está presente.
2. Interrumpir temporalmente la ejecución.
3. Recuperar la página desde swap.
4. Actualizar la información de traducción.
5. Reanudar la ejecución.

Este mecanismo permite ejecutar procesos mayores que la memoria física disponible.

---

#### Swapping

El swapping permite mover temporalmente páginas entre RAM y almacenamiento secundario.

##### Swap-Out

Proceso mediante el cual una página es retirada de memoria principal.

```text
RAM → SWAP
```

##### Swap-In

Proceso mediante el cual una página almacenada en swap regresa a memoria principal.

```text
SWAP → RAM
```

##### Beneficios

- Incrementa el grado de multiprogramación.
- Permite utilizar memoria virtual.
- Libera espacio cuando la RAM es insuficiente.

---

#### Memoria Compartida

La memoria compartida permite que varios procesos utilicen simultáneamente una misma región física.

##### Funcionamiento

1. Crear una región compartida.
2. Permitir que otros procesos se adjunten.
3. Compartir una única copia física.
4. Liberar la región cuando deje de ser utilizada.

##### Beneficios

- Reduce duplicación de datos.
- Disminuye el consumo de memoria.
- Facilita la comunicación entre procesos.

---

#### Compactación

La compactación es un algoritmo destinado a reducir la fragmentación externa.

##### Funcionamiento

1. Identificar los bloques ocupados.
2. Reubicarlos hacia el inicio de la memoria.
3. Agrupar el espacio libre en un único bloque continuo.

##### Resultado

```text
Antes:
[P1][Libre][P2][Libre][P3]

Después:
[P1][P2][P3][Libre]
```

##### Beneficios

- Reduce la fragmentación externa.
- Facilita futuras asignaciones contiguas.
- Incrementa el tamaño del mayor bloque libre disponible.

---

### Sistema de Archivos

El sistema de archivos implementa los algoritmos básicos necesarios para la organización, localización y gestión de archivos dentro de una estructura jerárquica de directorios.

---

#### Resolución de Rutas

La localización de archivos y directorios se realiza mediante un algoritmo de recorrido jerárquico basado en rutas absolutas.

##### Funcionamiento

1. Dividir la ruta en sus componentes.
2. Iniciar la búsqueda desde el directorio raíz.
3. Recorrer cada nivel de la ruta.
4. Verificar la existencia del elemento solicitado.
5. Retornar el nodo encontrado o reportar error.

##### Pseudocódigo

```text
actual ← raíz

Para cada componente de la ruta:

    Si componente no existe:
        error

    actual ← componente

Retornar actual
```

##### Beneficios

- Permite organizar archivos en múltiples niveles.
- Facilita la localización de recursos.
- Simula el comportamiento de sistemas de archivos reales.

---

#### Creación Recursiva de Directorios

La creación de directorios utiliza un algoritmo incremental que construye automáticamente cada nivel inexistente de la ruta especificada.

##### Funcionamiento

1. Dividir la ruta solicitada.
2. Recorrer cada componente.
3. Verificar si existe.
4. Si no existe, crearlo.
5. Continuar hasta completar toda la ruta.

##### Ejemplo

```text
Ruta solicitada:

/documentos/proyectos/kernel

Resultado:

/
└── documentos
    └── proyectos
        └── kernel
```

##### Ventajas

- Simplifica la creación de estructuras complejas.
- Evita errores por directorios intermedios inexistentes.

---

#### Creación de Archivos

La creación de archivos verifica la validez de la ruta antes de registrar un nuevo archivo en el sistema.

##### Funcionamiento

1. Extraer el nombre del archivo.
2. Localizar el directorio padre.
3. Verificar que no exista otro archivo con el mismo nombre.
4. Registrar el archivo.
5. Asociar su información de control.

##### Pseudocódigo

```text
Si archivo existe:
    rechazar creación

Si directorio padre no existe:
    rechazar creación

Crear archivo
Registrar metadatos
```

##### Beneficios

- Evita duplicidad de nombres.
- Garantiza consistencia en la estructura del sistema.

---

#### Gestión de Apertura de Archivos

El sistema implementa un algoritmo de control de acceso basado en conteo de aperturas.

##### Funcionamiento

1. Localizar el archivo solicitado.
2. Incrementar el contador de aperturas.
3. Marcar el archivo como abierto.
4. Registrar qué proceso realizó la apertura.

##### Pseudocódigo

```text
abrir archivo

contadorAperturas++

archivo.estado = abierto
```

##### Beneficios

- Permite múltiples accesos simultáneos.
- Mantiene seguimiento de archivos activos.

---

#### Gestión de Cierre de Archivos

Cuando un proceso finaliza el uso de un archivo se ejecuta el algoritmo de cierre.

##### Funcionamiento

1. Verificar que el proceso tenga abierto el archivo.
2. Reducir el contador de aperturas.
3. Actualizar el estado del archivo.
4. Eliminar el registro de apertura del proceso.

##### Pseudocódigo

```text
contadorAperturas--

Si contadorAperturas = 0:
    archivo.estado = cerrado
```

##### Beneficios

- Evita inconsistencias en el acceso concurrente.
- Garantiza una liberación controlada de recursos.

---

#### Liberación Automática de Archivos

Para prevenir fugas de recursos, el sistema implementa un algoritmo de cierre masivo asociado al ciclo de vida de los procesos.

##### Funcionamiento

1. Identificar todos los archivos abiertos por un proceso.
2. Recorrer la lista de aperturas.
3. Ejecutar el cierre individual de cada archivo.
4. Liberar los registros asociados.

##### Beneficios

- Evita archivos huérfanos.
- Mantiene la consistencia del sistema.
- Simula el comportamiento de los sistemas operativos modernos al finalizar procesos.

---

#### Actualización de Metadatos

Cada modificación relevante sobre un archivo actualiza automáticamente su información temporal.

##### Eventos que generan actualización

- Creación.
- Apertura.
- Cierre.
- Modificación de tamaño.

##### Objetivo

Mantener sincronizada la información de estado del archivo para facilitar su administración y monitoreo.

---

#### Resumen de Algoritmos Implementados

| Categoría | Algoritmo |
|------------|------------|
| Localización | Resolución jerárquica de rutas |
| Directorios | Creación recursiva de directorios |
| Archivos | Creación con validación de existencia |
| Acceso | Apertura con conteo de referencias |
| Liberación | Cierre controlado de archivos |
| Gestión de Recursos | Cierre automático por proceso |
| Metadatos | Actualización automática de timestamps |

---

### Gestión de E/S e Interrupciones


La gestión de entrada/salida del mini-kernel simula el funcionamiento de dispositivos físicos mediante solicitudes asíncronas, colas de espera e interrupciones de finalización. Este mecanismo permite que los procesos continúen compartiendo la CPU mientras esperan operaciones de E/S.

---

#### Planificación FIFO de Solicitudes de E/S

Cada dispositivo procesa las solicitudes siguiendo el algoritmo **First-In First-Out (FIFO)**.

La primera solicitud que ingresa a la cola es la primera en ser atendida.

##### Funcionamiento

1. Un proceso solicita una operación de E/S.
2. La solicitud se agrega al final de la cola del dispositivo.
3. Si el dispositivo está libre, comienza inmediatamente su ejecución.
4. Las solicitudes restantes esperan su turno.

##### Pseudocódigo

```text
solicitar E/S

encolar solicitud

Si dispositivo libre:
    iniciar atención
```

##### Beneficios

- Implementación sencilla.
- Garantiza orden de llegada.
- Evita reordenamientos inesperados.

---

#### Procesamiento por Ticks

La ejecución de las operaciones de E/S se simula mediante un algoritmo basado en unidades discretas de tiempo (*ticks*).

Cada operación posee una duración determinada y consume un tick en cada ciclo de simulación.

##### Funcionamiento

1. Seleccionar la solicitud activa.
2. Reducir su tiempo restante.
3. Verificar si la operación terminó.
4. Si finaliza, liberar el dispositivo.
5. Atender la siguiente solicitud pendiente.

##### Pseudocódigo

```text
Para cada dispositivo:

    Si existe solicitud activa:

        tiempoRestante--

        Si tiempoRestante == 0:
            finalizar operación
```

##### Beneficios

- Simula el comportamiento temporal de dispositivos reales.
- Permite modelar operaciones de distinta duración.
- Facilita la integración con el scheduler.

---

#### Asignación de Dispositivos

Cada dispositivo puede ejecutar únicamente una operación de E/S a la vez.

##### Funcionamiento

1. Verificar si existe una operación activa.
2. Si el dispositivo está libre:
   - Tomar la siguiente solicitud de la cola.
   - Marcar el dispositivo como ocupado.
3. Ejecutar la operación hasta su finalización.

##### Resultado

```text
Dispositivo LIBRE
        ↓
Asignar solicitud
        ↓
Dispositivo OCUPADO
        ↓
Finalizar operación
        ↓
Dispositivo LIBRE
```

##### Beneficios

- Simula exclusión mutua sobre dispositivos físicos.
- Evita conflictos de acceso simultáneo.

---

#### Generación de Interrupciones

Cuando una operación de E/S finaliza, el sistema genera automáticamente una interrupción.

##### Funcionamiento

1. Detectar que la operación terminó.
2. Crear un evento de interrupción.
3. Registrar el proceso asociado.
4. Almacenar la interrupción para su posterior atención.

##### Pseudocódigo

```text
Si operación completada:

    crear interrupción

    registrar PID

    encolar interrupción
```

##### Beneficios

- Evita que los procesos consulten continuamente el estado del dispositivo.
- Simula el comportamiento de hardware real.

---

#### Atención de Interrupciones

Las interrupciones pendientes son procesadas por el controlador de interrupciones.

##### Funcionamiento

1. Obtener todas las interrupciones pendientes.
2. Procesarlas en orden de llegada.
3. Identificar el proceso asociado.
4. Notificar la finalización de la operación.
5. Solicitar el desbloqueo del proceso.

##### Pseudocódigo

```text
Mientras existan interrupciones:

    interrupción ← siguiente evento

    notificar scheduler

    desbloquear proceso
```

##### Beneficios

- Centraliza el manejo de eventos.
- Mantiene desacoplados los dispositivos y el scheduler.
- Facilita la coordinación entre subsistemas.

---

#### Desbloqueo de Procesos por Interrupción

Cuando una operación de E/S finaliza, el proceso que la solicitó puede volver a competir por la CPU.

##### Funcionamiento

1. Recibir la interrupción.
2. Identificar el proceso bloqueado.
3. Cambiar su estado a READY.
4. Reincorporarlo al algoritmo de planificación.

##### Flujo

```text
RUNNING
    ↓
Solicita E/S
    ↓
BLOCKED
    ↓
Fin de E/S
    ↓
INTERRUPCIÓN
    ↓
READY
```

##### Beneficios

- Permite superponer cómputo y operaciones de E/S.
- Incrementa el aprovechamiento del procesador.
- Reproduce el comportamiento de sistemas operativos reales.


---

## Resultados
[Dejemos aqui un ejemplo de los logs de ejecución y lo explicamos]

---

## Conclusiones
[Acá iria anexo también el apartado de las desiciones técnicas importantes en un párrafo]

---

## Anexos

[Propongo dejar aqui los diagramas]
