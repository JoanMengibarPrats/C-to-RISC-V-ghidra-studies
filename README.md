+-------------------------------------------------------------------+
                                                                 
C -> RISC-V : LABORATORIO DE INGENIERÍA INVERSA Y ANÁLISIS         
                                                                  
+-------------------------------------------------------------------+

Este repositorio constituye una bitácora técnica orientada al estudio de la arquitectura RISC-V y la ingeniería inversa de binarios. Mi propósito es documentar cómo los compiladores estructuran la memoria, optimizan rutinas y asignan registros a nivel de instrucción, utilizando código en C como punto de partida y Ghidra como herramienta principal de análisis estático.

## Estructura del Repositorio

El proyecto se segmenta en módulos o pruebas de concepto independientes. En cada directorio se incluyen los siguientes elementos:

*   **Código fuente (`.c`):** La implementación original de la lógica a analizar.
*   **Binario compilado (`.elf`):** El ejecutable resultante tras el proceso de compilación cruzada.
*   **Documentación de análisis (`README.md`):** Desglose técnico que correlaciona el flujo de control en C con su respectiva representación en ensamblador RISC-V extraída de Ghidra.
*   El tamaño de los registros es de rv64.

## Entorno de Trabajo

El entorno de desarrollo y análisis opera sobre el Subsistema de Windows para Linux (WSL) utilizando una distribución Ubuntu. Esta arquitectura me otorga la cadena de herramientas de GNU para RISC-V de forma nativa.

### Cadena de Compilación

Para la generación de los binarios objeto de análisis, se emplea un enfoque estandarizado mediante el siguiente comando:

```bash
riscv64-linux-gnu-gcc -march=rv64g -O2 -g <nombre>.c -o <nombre>.elf
```
Cada parámetro ha sido definido con un propósito analítico específico:

- riscv64-linux-gnu-gcc: Invocación del compilador cruzado. Permite generar código máquina destinado a la arquitectura RISC-V (64 bits) desde un entorno host x86_64, incluyendo el soporte de librerías del entorno Linux.

- -march=rv64g: Define la arquitectura objetivo (rv64 para 64 bits, y g para las extensiones generales: matemáticas, multiplicaciones, atómicas y de coma flotante). Nota: Se omite deliberadamente la extensión c (Compressed). Al forzar que todas las instrucciones ocupen exactamente 32 bits (4 bytes), el mapa de memoria generado es simétrico, lo que reduce la fricción visual y facilita la lectura del código desensamblado en Ghidra.

- -O2: Aplica un nivel de optimización de código estructural. Este parámetro obliga al compilador a tomar decisiones de eficiencia (como el desenrollado de bucles o la prioridad en la asignación de registros de CPU sobre la memoria RAM), permitiendo estudiar transformaciones de código de grado producción.

- -g: Integra símbolos de depuración en el binario. Esto incrusta los identificadores originales del código fuente dentro del archivo ELF, sirviendo como punto de anclaje para que Ghidra pueda mapear las rutinas en ensamblador con la lógica escrita en C.

- -o <nombre>.elf: Genera el binario final bajo el formato estándar de sistemas Unix (Executable and Linkable Format).

## Ejecución y Emulación

Dado que el entorno de pruebas no se ejecuta sobre silicio RISC-V nativo, la validación del comportamiento de los binarios se realiza mediante emulación en espacio de usuario empleando QEMU:
```bash
qemu-riscv64 -L /usr/riscv64-linux-gnu ./<nombre>.elf
```
