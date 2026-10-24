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

Antes de pasar a la ingeniería inversa con Ghidra, me gustaría explicar la debilidad de este método de encriptación. Si te fijas, el hash es 1116; hay infinitas combinaciones de sumas que dan ese valor, por lo que calcular la contraseña exacta es muy difícil, pero descifrar el hash no lo es. Por ejemplo, ttt suma en total 116 * 3 = 348.

En binario: 101011100 ----> Últimos 8 bits: 01011100, por lo que con una contraseña distinta se obtiene el mismo hash. Más adelante se explicarán en detalle las matemáticas del craqueo.
___

