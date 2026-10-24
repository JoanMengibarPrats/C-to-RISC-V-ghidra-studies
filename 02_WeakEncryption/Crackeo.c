#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_cifrado>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) return 1;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    unsigned char *buffer = malloc(size);
    if (!buffer) { fclose(file); return 1; }

    if (fread(buffer, 1, size, file) != size) {
        free(buffer); 
        fclose(file); 
        return 1;
    }
    fclose(file);

    // Fuerza bruta pura: volcamos las 256 combinaciones posibles con su etiqueta de máscara
    for (int mask = 0; mask < 256; mask++) {
        printf("=== MASK:%d ===\n", mask);
        for (long i = 0; i < size; i++) {
            putchar(buffer[i] ^ mask);
        }
        printf("\n");
    }

    free(buffer);
    return 0;
}
