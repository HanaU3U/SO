# Mini Kernel

## Integrantes
- Ana Laura Morcote Chacón
- Tomás Alejandro Delgado
- Hana Sofía Pinilla Manrique

## Componentes
- Gestión de procesos
- Gestión de memoria
- Gestión de archivos
- Gestión de E/S

## Compilación

make

Si compilas manualmente con `g++`, incluye todos los módulos del proyecto:

g++ -g main.cpp Kernel/kernel.cpp Memoria/memoria.cpp Memoria/paginacion.cpp Procesos/pcb.cpp Procesos/scheduler.cpp Archivos/fcb.cpp Archivos/filesystem.cpp Entrada_Salida/dispositivo.cpp Entrada_Salida/interrupciones.cpp -I . -std=c++17 -o SO.exe

## Ejecución

./kernel