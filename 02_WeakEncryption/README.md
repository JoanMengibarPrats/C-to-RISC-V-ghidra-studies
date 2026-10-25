## Introducción

En esta carpeta se presenta una de las formas más simples e inseguras de cifrar un contenido. Es común encontrarte con este tipo de cifrado en firmwares antiguos de dispositivos IoT, por lo que conocer su lógica es esencial.

Se plantea un tipo de cifrado XOR en el código WeakEncryption.c. Este código espera una contraseña, la cual se convierte en un hash sumando cada uno de sus caracteres. La contraseña empleada para cifrar el secreto es "Chocolate1234", posteriormente convertida en el archivo Cifrado.

Se han usado los siguientes comandos:

Compilación:
```bash
riscv64-linux-gnu-gcc -march=rv64g -O2 -g WeakEncryption.c -o WeakEncryption.elf
```
Encriptación:
```bash
qemu-riscv64 -L /usr/riscv64-linux-gnu ./WeakEncryption.elf Chocolate1234 < Secreto > Cifrado
```
El código C realiza la operación con putchar, lo que hace que el programa sea aún más débil. Por ejemplo, el hash de Chocolate1234 es 1116 al sumar los valores de cada letra.

```
[Decimal] 1116 ---> [Binario] 10001011100
```

Si por ejemplo transformamos la letra A:
```
XOR:
        0 1 0 0 0 0 0 1   (Letra 'A' / 65)
^ 1 0 0 0 1 0 1 1 1 0 0   (Hash)
-----------------------
  1 0 0 0 1 0 1 1 1 0 1   
```
La función putchar redondea a 8 bits, eligiendo los últimos 8: 0 1 0 1 1 1 0 1 y convirtiéndolo en un corchete ].

Antes de pasar a la ingeniería inversa con Ghidra, me gustaría explicar la debilidad de este método de encriptación. Si te fijas, el hash es 1116; hay muchas combinaciones de sumas que dan ese valor, por lo que calcular la contraseña exacta es muy difícil, pero descifrar el hash no lo es. Por ejemplo, ttt suma en total 116 * 3 = 348.

En binario: 101011100 ----> Últimos 8 bits: 01011100, por lo que con una contraseña distinta se obtiene el mismo hash. Más adelante se explicarán en detalle las matemáticas del crackeo.

## Análisis en Ghidra

Si se abre el archivo .elf en ghidra y nos dirijimos a la función main nos encontramos con lo siguiente:

<img width="838" height="659" alt="imagen" src="https://github.com/user-attachments/assets/d1815a60-56b2-4a0c-9f90-cd84d6632f0a" />

Equivalencias aplicadas al código de Ghidra:

- uVar2 ---> hash: El acumulador donde se suman los valores de la contraseña.
- pbVar4 ---> password_ptr: El puntero que avanza por la cadena argv[1].
- bVar1 ---> current_char: El byte individual de la contraseña que se está evaluando.
- lVar3 ---> input_char: El byte leído del archivo original.
- FUN_ram_00100660 ---> getchar: Función estándar de lectura.
- FUN_ram_00100650 ---> putchar: Función estándar de escritura donde ocurre el truncamiento a 8 bits.

<img width="819" height="578" alt="imagen" src="https://github.com/user-attachments/assets/32882bd9-bb65-4e03-bf8d-d1ad89af9815" />

El pseudocódigo descompilado estructura la ejecución del binario en tres fases lógicas directamente legibles.

#### Inicialización
El programa asigna la contraseña proporcionada por el usuario (argv[1]) al puntero password_ptr e inicializa el acumulador hash en 0.

#### Generación del hash
Un bucle while recorre la contraseña byte a byte hasta alcanzar el carácter nulo final (0). En cada iteración, suma el valor ASCII del byte actual (current_char) al acumulador hash y avanza el puntero al siguiente carácter. El compilador ha incrustado aquí la lógica exacta de la función original get_bitmask para ahorrar tiempos de salto de memoria.

#### Lectura y cifrado XOR
Un bucle infinito captura bytes secuenciales de la entrada estándar (stdin) mediante getchar. Si la lectura devuelve -1 (EOF), el bucle se detiene y finaliza el programa. Cada byte capturado se somete a una operación XOR (^) directa contra la variable hash y se envía a la salida estándar. Al imprimir el resultado a través de putchar, el sistema trunca forzosamente el valor de 32 bits a los últimos 8 bits, consolidando de forma silenciosa la vulnerabilidad de colisiones múltiples.

### Function Graph

<img width="1003" height="1031" alt="Captura de pantalla 2026-10-25 203215" src="https://github.com/user-attachments/assets/74b5fc97-86b2-4a71-8ef1-8ac01c9e3b8b" />

El Function Graph revela la estructura de control a nivel de ensamblador RISC-V, confirmando visualmente la optimización que fusionó las funciones en el `main`.

**Bloque de Entrada y Preparación (`ram:00100670`)**
El programa comienza guardando registros en la pila y asignando el argumento `argv[1]` a la variable `password_ptr` (`ld password_ptr, 0x8(argv)`). Inicializa el acumulador a cero mediante `mv hash, zero` y carga el primer byte de la contraseña de forma no firmada con `lbu`. Si la contraseña está vacía, la instrucción condicional `beq a4, zero` salta el cálculo del hash y va directamente a la preparación del bucle de lectura.

**Bucle de Generación del Hash (`LAB_ram_00100698`)**
Este bloque encapsula la antigua función `get_bitmask`. Acumula el valor ASCII del carácter en el hash usando `addw hash, a4, hash` y avanza el puntero de la cadena. A continuación, carga el siguiente byte (`lbu a4, 0x0(password_ptr)`) y evalúa si se ha llegado al final de la cadena; si no es el carácter nulo, el salto `bne a4, zero` reinicia el ciclo.

**Configuración de I/O (`LAB_ram_001006a8`)**
Este nodo de transición prepara los punteros de memoria para `stdin` y `stdout`. Un detalle crucial para el flujo es la instrucción `li s1, -0x1`, que carga el valor `-1` (representando `EOF`) en el registro `s1` para usarlo más adelante como condición de parada del programa. Tras esto, realiza un salto incondicional (`j`) al núcleo del cifrado.

**Bucle de Lectura y Cifrado (`LAB_ram_001006c8`)**
Aquí reside la vulnerabilidad central. El programa invoca la función de lectura con `jal ra, getchar`. Acto seguido, ejecuta la operación criptográfica con la instrucción `xor input_char, input_char, hash`, aplicando la máscara al byte capturado. Finalmente, comprueba si el valor original capturado coincide con el `-1` almacenado previamente en `s1` (`bne ... s1`); si no es así, continuará la ejecución hacia el bloque de impresión y repetirá el ciclo.

<img width="1005" height="1055" alt="Captura de pantalla 2026-10-25 203239" src="https://github.com/user-attachments/assets/db00f0a0-59cd-4233-be83-5f48f2034ac4" />

Esta captura completa la visión del flujo de ejecución del programa, mostrando el final del bucle `while` y el epílogo de la función.

**Bloque de Escritura (`LAB_ram_001006c0`)**
Si la condición de EOF no se cumple en el bloque anterior (es decir, el byte capturado es válido y no es `-1`), la flecha verde etiquetada como "While Loop" dirige la ejecución a este pequeño bloque. Aquí se carga el puntero de salida estándar (`ld argv, 0x0(s3=>stdout)`) y se invoca la función de escritura mediante `jal ra, putchar`. Es en esta llamada exacta donde el sistema asume un `unsigned char` y recorta silenciosamente los 24 bits superiores del resultado del XOR. Tras imprimir el carácter cifrado, el flujo sigue la flecha roja hacia arriba para volver al bloque de lectura (`LAB_ram_001006c8`), cerrando el ciclo.

**Bloque de Salida / Epílogo (`ram:001006e0`)**
Cuando `getchar` devuelve `-1` (fin de archivo), la instrucción `bne` del bloque anterior ya no salta, por lo que la ejecución cae por la flecha roja inferior hacia este último nodo. Este bloque se encarga de la limpieza de la función antes de terminar:

1. Restaura los valores originales de todos los registros que se guardaron al inicio (`ra`, `hash`, `s1`, `s2`, `s3`) extrayéndolos de la pila con instrucciones `ld`.


2. Ejecuta `mv input_char, zero`, lo que pone un `0` en el registro de retorno, equivaliendo directamente al `return 0;` del código C.


3. Restaura el tamaño de la pila (`addi sp, sp, 0x30`) y ejecuta `ret` para devolver el control al sistema operativo.

## Crackeo

El cifrado basa su teórica seguridad en sumar los valores ASCII de los caracteres de una contraseña para generar una clave. Aunque contraseñas largas generen un número entero elevado (como 1116 para "Chocolate1234"), la escritura del archivo mediante la función putchar recorta de forma silenciosa el valor a 8 bits.

Matemáticamente, este desbordamiento equivale a realizar la operación suma_ascii % 256. Esto reduce drásticamente el espacio de búsqueda: sin importar la longitud o complejidad de la contraseña original, solo existen 256 claves criptográficas posibles (del 0 al 255). Además, provoca colisiones constantes; cualquier contraseña alternativa cuya suma ASCII comparta el mismo residuo módulo 256 (como "ttt", que suma 348, y 348 % 256 = 92) funcionará como clave maestra válida.

Para romper el cifrado, el siguiente script (Crack.c) aprovecha este reducido espacio de estados. En lugar de realizar ataques de diccionario para adivinar la contraseña original o usar heurísticas con sesgos, aplica fuerza bruta pura iterando sobre las 256 máscaras matemáticas posibles y aplicando la operación inversa (XOR es simétrico) a cada byte del archivo.

Una vez compilado el código se puede ejecutar el siguiente comando para obtener un fichero .txt con 256 posibilidades de texto descifrado:

``` bash
qemu-riscv64 -L /usr/riscv64-linux-gnu ./Crack.elf Cifrado > posibilidades.txt
```
En este posibilidades.txt se encuentra nuestro "Secreto", se podría programar algo para sacar la clave directamente. Además, como tenemos el texto cifrado y el descifrado, podemos calcular el valor del hash y de este valor hacer una base de datos con todas las posibles contraseñas.
