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

    int exit_code = 0;
    FILE* rom_file = nullptr;
    uint8_t *buffer = nullptr;
    chip8_t *chip8 = nullptr;

    errno_t const open_error = fopen_s(&rom_file, rom_path, "rb");
    if (open_error) {
        printf("Failed to open CHIP-8 ROM file. Error code: %d\n", open_error);
        exit_code = open_error;
        goto exit;
    }

    fseek(rom_file, 0, SEEK_END);

    long const rom_size = ftell(rom_file);
    if (rom_size < 0) {
        printf("Failed to determine CHIP-8 ROM file size!\n");
        exit_code = EXIT_FAILURE;
        goto exit;
    }

    fseek(rom_file, 0, SEEK_SET);

    buffer = malloc(sizeof(uint8_t) * rom_size);
    if (buffer == NULL) {
        printf("Failed to allocate buffer for CHIP-8 rom file!\n");
        exit_code = EXIT_FAILURE;
        goto exit;
    }

    size_t const bytes_read = fread(buffer, rom_size, 1, rom_file);
    if (bytes_read != 1) {
        printf("Failed to properly read bytes from CHIP-8 ROM file!\n");
        exit_code = EXIT_FAILURE;
        goto exit;
    }

    chip8 = chip8_create(buffer, rom_size);
    if (chip8 == NULL) {
        printf("Failed to create CHIP-8 object!\n");
        exit_code = EXIT_FAILURE;
        goto exit;
    }

    run_result_t const run_result = chip8_host_run_chip8(chip8);
    if (run_result.error_num) {
        printf("Error initializing or running CHIP-8!\n");
        if (run_result.msg) {
            printf("Error: %s\n", run_result.msg);
        }
    }

    exit:
    if (buffer) {
        free(buffer);
    }
    if (rom_file) {
        fclose(rom_file);
    }
    if (chip8) {
        free(chip8);
    }

    return exit_code;
}
