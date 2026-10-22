
Este primer laboratorio analiza la estructura más básica de un programa en C compilado para la arquitectura RISC-V (64 bits, sin compresión). El objetivo es comprender cómo interactúa una función principal (`main`) con la memoria (Stack) y los registros al invocar a una función externa.

## 1. Código Fuente en C

El código original es un programa elemental que imprime una cadena de texto y finaliza:

```c
#include <stdio.h>

int main(void) {
    puts("Hello World!");
    return 0;
}
```
<img width="1560" height="793" alt="Captura de pantalla 2026-10-22 163513" src="https://github.com/user-attachments/assets/7e17b33e-5b94-4c4a-87fc-3f07286a210e" />

Al analizar el binario ELF resultante (HelloWorld.elf) en Ghidra con la arquitectura rv64g, el descompilador de C identifica correctamente la llamada a una función externa y el retorno. En la vista de ensamblador (Listing), se observan las siguientes fases estructurales:

```assembly
addi sp, sp, -0x10
```
La función main comienza expandiendo la pila (Stack) en 16 bytes (-0x10) modificando el Stack Pointer (sp). Esto prepara el espacio necesario para salvaguardar registros locales.

```assembly
auipc a0, 0x0
addi a0 => s_Hello_World!_ram_001006c0, a0, 0x10c
```

Para cumplir con la convención de llamadas (ABI) de RISC-V, el primer argumento de una función debe pasarse a través del registro a0. El compilador utiliza la pareja de instrucciones auipc y addi para calcular la dirección relativa en memoria de la cadena estática "Hello World!" e introduce dicha dirección en a0.

```assembly
sd ra, 0x8(sp)
jal ra, puts (antes FUN_ram_001005a0)
```

Dado que main invoca a otra función (puts, identificada como FUN_ram_001005a0 y renombrada a puts posteriormente), deja de ser una función "hoja" (leaf function). Por tanto, almacena obligatoriamente su propia dirección de retorno (ra) en la pila mediante sd (Store Doubleword) antes de ejecutar la instrucción de salto y enlace (jal).

Al inspeccionar la dirección de memoria a la que salta `main` (`001005a0`), se observa que el desensamblador no muestra el código real de la función `puts`. En su lugar, revela un pequeño fragmento de código intermedio conocido como *PLT stub* (Procedure Linkage Table). 

<img width="1557" height="769" alt="image" src="https://github.com/user-attachments/assets/d8a37687-c739-4a57-a234-fed1561c55ff" />

Dado que `puts` pertenece a la librería estándar de C (`libc`) y se enlaza dinámicamente, el ejecutable utiliza este trampolín para localizar la función externa durante el tiempo de ejecución.


```assembly
auipc t3, 0x2
ld t3, -0x5d0(t3=>-><EXTERNAL>::puts)
jalr t1, t3=><EXTERNAL>::puts, 0x0
nop
```
El código utiliza la combinación de instrucciones auipc y ld para buscar la dirección real de puts en la Global Offset Table (GOT) y cargarla en t3. Utiliza un registro temporal (clasificado como Caller-Saved) para realizar este cálculo sin alterar el registro a0, protegiendo así el puntero a la cadena "Hello World!" que pasamos como argumento.

El salto final hacia el código de la librería se ejecuta guardando la dirección de retorno local en el registro temporal t1, en lugar de utilizar el registro estándar ra. Esta decisión arquitectónica garantiza que el registro ra original (que apunta de vuelta a main) no se sobrescriba. Como resultado, cuando la función externa puts finalice, retornará directamente a main, omitiendo este trampolín de bajada.

La instrucción vacía al final asegura que el bloque mantenga una alineación simétrica en memoria de 16 bytes.

Volviendo a la imagen anterior:

<img width="1560" height="793" alt="Captura de pantalla 2026-10-22 163513" src="https://github.com/user-attachments/assets/7e17b33e-5b94-4c4a-87fc-3f07286a210e" />

```assembly
ld ra, 0x8(sp)
mv a0, zero
addi sp, sp, 0x10
ret
```
Tras volver de la función de impresión, main prepara su salida:
- Restaura su dirección de retorno original leyendo la pila con ld (Load Doubleword).   
- Asigna el valor 0 al registro a0 (moviendo el valor del registro físico zero) para cumplir con la instrucción return 0; en C.
- Contrae la pila devolviendo los 16 bytes prestados (addi sp, sp, 0x10).
- Ejecuta ret para devolver el control al sistema operativo.   
