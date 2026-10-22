Este laboratorio analiza un ofuscador de texto a nivel de bit. El objetivo principal es observar cómo el compilador cruzado GCC con el nivel de optimización `-O2` transforma el código C original, eliminando saltos de función innecesarios mediante técnicas de *inlining* y gestionando los punteros a archivos (`FILE *`).

## 1. Código Fuente en C

El programa lee caracteres de la entrada estándar hasta alcanzar el final del archivo (`EOF`), aplica una operación NOT a nivel de bit (`~`) para ofuscar el valor, y lo imprime en la salida estándar. Este código es un buen ejemplo para entender las funciones getchar y putchar y como en estas funciones se basa mucha de la implementación de la criptografía. Si se ejecuta ``` ./FileOfuscator.elf < secret > hidden ``` nos sacará en el archivo hidden texto ilegible ofuscado, si el comando se vuelve a realizar ``` ./FileOfuscator.elf < hidden > descifardo ``` se obtendría el texto inicial.

```c
#include <stdio.h>

int transform(int c) {
    return ~c;
}

int main(void) {
    int c;
    while ((c = getchar()) != EOF) {
        putchar(transform(c));
    }
    return 0;
}
```

En el código desensamblado, la función transform no existe como una entidad independiente. El compilador detectó que crear un marco de pila (Stack Frame) y ejecutar una instrucción de salto (jal) solo para una operación lógica era ineficiente. En su lugar, aplicó inlining: insertó la instrucción not directamente dentro del bucle de la función main.

<img width="1666" height="918" alt="image" src="https://github.com/user-attachments/assets/612239aa-f2fb-458f-89fc-2c84c617c539" />

```assembly
# Fragmento dentro de main
mv a5, c        # Mueve el carácter leído a a5
not c, c        # Aplica la operación bitwise NOT directamente
sext.w c, c     # Extiende el signo a 32 bits
```

Las macros getchar() y putchar() de C no se tradujeron como llamadas directas a las funciones homónimas en ensamblador. En su lugar, el compilador generó dos funciones intermedias:
- Lectura (fgetc): Una función que recibe el puntero global stdin estático y ejecuta internamente getc().
- Escritura (fputc): Una función que recibe el carácter ofuscado y el puntero stdout, ejecutando internamente putc().

El bucle while se reestructuró en ensamblador como un bucle infinito while(true) con una condición de ruptura interna. Tras invocar a la función de lectura, el programa evalúa el resultado comparándolo con -1 (el valor constante de EOF en C). Si se cumple, ejecuta un salto condicional (bne o similar dependiendo del bloque) para salir del bucle y preparar el return 0 de main.

