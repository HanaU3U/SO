CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -g3 -O0 -std=c++17
INCLUDES := -I src
SOURCES := \
	src/main.cpp \
	src/Archivos/fcb.cpp \
	src/Archivos/filesystem.cpp \
	src/Entrada_Salida/dispositivo.cpp \
	src/Entrada_Salida/interrupciones.cpp \
	src/Kernel/kernel.cpp \
	src/Memoria/memoria.cpp \
	src/Memoria/paginacion.cpp \
	src/Procesos/pcb.cpp \
	src/Procesos/scheduler.cpp

ifeq ($(OS),Windows_NT)
TARGET := build/Debug/SO.exe
else
TARGET := build/Debug/SO
endif

.PHONY: all clean run

all: $(TARGET)

build/Debug:
	mkdir -p build/Debug

$(TARGET): build/Debug $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
