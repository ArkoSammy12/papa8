#ifndef PAPA8_CHIP8_H
#define PAPA8_CHIP8_H

#include <stdint.h>

typedef struct chip8 chip8_t;

#define CHIP8_RAM_SIZE 4096
#define CHIP8_GFX_WIDTH 64
#define CHIP8_GFX_HEIGHT 32
#define CHIP8_GFX_LENGTH ((CHIP8_GFX_WIDTH) * (CHIP8_GFX_HEIGHT))

#define CHIP8_IPF 15

#define CHIP8_NULL_PTR (1)
#define CHIP8_UNKNOWN_OPCODE (1 << 1)
#define CHIP8_DXYN (1 << 2)

uint8_t constexpr CHIP8_FPS = 60;

chip8_t *chip8_create(const uint8_t *rom, uint16_t rom_size);

uint32_t chip8_execute_frame(chip8_t *chip8);

uint32_t chip8_set_key(chip8_t *chip8, uint8_t index, bool value);

uint32_t chip8_dump_argb8(chip8_t const *chip8, uint32_t *frame_buffer, size_t frame_buffer_length);

bool chip8_play_buzzer(chip8_t const *chip8);

#endif //PAPA8_CHIP8_H
