#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"
#include "chip8_host.h"

int main(const int argc, char *argv[]) {
    if (argc < 2) {
        perror("No CHIP-8 ROM provided!");
        return EXIT_FAILURE;
    }

    char const *rom_path = argv[1];

    FILE* rom_file = nullptr;

    errno_t const error = fopen_s(&rom_file, rom_path, "rb");
    if (error) {
        printf("Failed to read CHIP-8 ROM file. Error code: %d\n", error);
        return error;
    }

    fseek(rom_file, 0, SEEK_END);

    long const rom_size = ftell(rom_file);
    if (rom_size < 0) {
        printf("Failed to determine CHIP-8 ROM file size!");
        fclose(rom_file);
        return EXIT_FAILURE;
    }

    fseek(rom_file, 0, SEEK_SET);

    uint8_t * const buffer = malloc(sizeof(uint8_t) * rom_size);
    if (buffer == NULL) {
        printf("Failed to allocate buffer for CHIP-8 rom file!");
        fclose(rom_file);
        return EXIT_FAILURE;
    }

    size_t const bytes_read = fread(buffer, rom_size, 1, rom_file);
    if (bytes_read != 1) {
        printf("Failed to properly read bytes from CHIP-8 ROM file!");
        free(buffer);
        fclose(rom_file);
        return EXIT_FAILURE;
    }

    chip8_t *chip8 = chip8_create(buffer, rom_size);
    run_result_t const run_result = chip8_host_run_chip8(chip8);

    if (run_result.error_num != 0) {
        printf("Error: %s\n", run_result.msg);
    }

    free(buffer);
    fclose(rom_file);

    return 0;
}
