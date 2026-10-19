#include "chip8.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FONT_SIZE 5
#define FONT_DIGITS 16
#define FONT_BYTES FONT_SIZE * FONT_DIGITS

#define PROGRAM_BEGIN 0x200

static constexpr uint8_t FONT_BEGIN_OFFSET = 0;

static const uint8_t FONT[FONT_BYTES] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x60, 0x20, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0xA0, 0xA0, 0xF0, 0x20, 0x20, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x10, 0x10, 0x10, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xF0, 0x50, 0x70, 0x50, 0xF0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xF0, 0x50, 0x50, 0x50, 0xF0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

struct chip8 {
    uint8_t ram[CHIP8_RAM_SIZE];
    bool gfx[CHIP8_GFX_LENGTH];
    uint8_t V[16];
    uint16_t stack[16];
    bool keypad[16];
    uint16_t pc;
    uint16_t sp;
    uint16_t I;
    uint8_t dt;
    uint8_t st;

    int8_t waiting_key;
};

chip8_t *chip8_create(uint8_t const *rom, uint16_t const rom_size) {
    if (rom_size >= CHIP8_RAM_SIZE - PROGRAM_BEGIN) {
        return nullptr;
    }

    chip8_t *chip8 = calloc(1, sizeof(chip8_t));
    if (chip8 == NULL) {
        return nullptr;
    }

    chip8->pc = PROGRAM_BEGIN;
    chip8->waiting_key = -1;
    memcpy(chip8->ram, FONT, FONT_BYTES);

    for (size_t i = 0; i < rom_size; i++) {
        const uint32_t index = PROGRAM_BEGIN + i;
        chip8->ram[index] = rom[i];
    }

    return chip8;
}

static uint8_t chip8_read(chip8_t const *chip8, uint32_t const address) {
    return chip8->ram[address & 0xFFFF];
}

static void chip8_write(chip8_t * const chip8, uint32_t const address, uint8_t const value) {
    chip8->ram[address & 0xFFFF] = value;
}

static int8_t chip8_get_first_pressed_key(chip8_t const *chip8) {
    if (chip8 == NULL) {
        return -1;
    }
    for (int8_t i = 0; i < 16; i++) {
        if (chip8->keypad[i]) {
            return i;
        }
    }
    return -1;
}

static uint32_t chip8_cycle(chip8_t * const chip8) {
    if (chip8 == NULL) {
        return CHIP8_NULL_PTR;
    }

    #define INC_PC (chip8->pc += (2))

    uint8_t const first = chip8_read(chip8, chip8->pc);
    uint8_t const second = chip8_read(chip8, chip8->pc + 1);
    uint16_t const opcode = (first << 8) | second;

    INC_PC;

    #define GET_NNN ((opcode) & (0xFFF))
    #define GET_NN ((opcode) & (0xFF))
    #define GET_N ((opcode) & (0xF))
    #define GET_X (((opcode) & (0x0F00)) >> (8))
    #define GET_Y (((opcode) & (0x00F0)) >> (4))

    #define GET_VX (chip8->V[GET_X])
    #define GET_VY (chip8->V[GET_Y])

    bool dxyn_executed = false;

    switch (first >> 4) {
        case 0x0: {
            if (first == 0x00) {
                switch (second) {
                    case 0xE0: { // 00E0: cls
                        memset(chip8->gfx, 0x00, CHIP8_GFX_WIDTH * CHIP8_GFX_HEIGHT);
                        break;
                    }
                    case 0xEE: { // 00EE: return
                        chip8->sp = (chip8->sp - 1) & 0xF;
                        chip8->pc = chip8->stack[chip8->sp];
                        break;
                    }
                    default: {
                        return CHIP8_UNKNOWN_OPCODE;
                    }
                }
            } else {
                return CHIP8_UNKNOWN_OPCODE;
            }
            break;
        }
        case 0x1: { // 1NNN: jump NNN
            chip8->pc = GET_NNN;
            break;
        }
        case 0x2: { // 2NNN: :call NNN
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp = (chip8->sp + 1) & 0xF;
            chip8->pc = GET_NNN;
            break;
        }
        case 0x3: { // 3XNN: if vX != NN then
            if (GET_NN == GET_VX) {
                INC_PC;
            }
            break;
        }
        case 0x4: { // 4XNN: if vX == NN then
            if (GET_NN != GET_VX) {
                INC_PC;
            }
            break;
        }
        case 0x5: {
            if (GET_N == 0x0) { // 5XY0: if vX != vY then
                if (GET_VX == GET_VY) {
                    INC_PC;
                }
            } else {
                return CHIP8_UNKNOWN_OPCODE;
            }
            break;
        }
        case 0x6: { // 6XNN: vX := NN
            chip8->V[GET_X] = GET_NN;
            break;
        }
        case 0x7: { // 7XNN: vX += NN
            chip8->V[GET_X] += GET_NN;
            break;
        }
        case 0x8: {
            switch (GET_N) {
                case 0x0: { // 8XY0: vX := vY
                    chip8->V[GET_X] = GET_VY;
                    break;
                }
                case 0x1: { // 8XY1: vX |= vY
                    chip8->V[GET_X] |= GET_VY;
                    chip8->V[0xF] = 0;
                    break;
                }
                case 0x2: { // 8XY2: vX &= vY
                    chip8->V[GET_X] &= GET_VY;
                    chip8->V[0xF] = 0;
                    break;
                }
                case 0x3: { // 8XY2: vX ^= vY
                    chip8->V[GET_X] ^= GET_VY;
                    chip8->V[0xF] = 0;
                    break;
                }
                case 0x4: { // 8XY5: vX += vY
                    uint8_t const VX = GET_VX;
                    uint8_t const VY = GET_VY;
                    uint16_t const value = VX + VY;
                    chip8->V[GET_X] = value & 0xFF;
                    chip8->V[0xF] = value > 0xFF ? 1 : 0;
                    break;
                }
                case 0x5: { // 8XY5: vX -= vY
                    uint8_t const VX = GET_VX;
                    uint8_t const VY = GET_VY;
                    chip8->V[GET_X] = (VX - VY) & 0xFF;
                    chip8->V[0xF] = VX >= VY ? 1 : 0;
                    break;
                }
                case 0x6: { // 8XY6: vX >>= vY
                    uint8_t const VY = GET_VY;
                    chip8->V[GET_X] = VY >> 1;
                    chip8->V[0xF] = VY & 1;
                    break;
                }
                case 0x7: { // 8XY7: vX =- vY
                    uint8_t const VX = GET_VX;
                    uint8_t const VY = GET_VY;
                    chip8->V[GET_X] = (VY - VX) & 0xFF;
                    chip8->V[0xF] = VY >= VX ? 1 : 0;
                    break;
                }
                case 0xE: { // 8XYE: vX <<= vY
                    uint8_t const VY = GET_VY;
                    chip8->V[GET_X] = VY << 1;
                    chip8->V[0xF] = (VY & 0x80) != 0 ? 1 : 0;
                    break;
                }
                default: {
                    return CHIP8_UNKNOWN_OPCODE;
                }
            }
            break;
        }
        case 0x9: {
            if (GET_N == 0x0) { // 9XY0: if vX == vY then
                if (GET_VX != GET_VY) {
                    INC_PC;
                }
            } else {
                return CHIP8_UNKNOWN_OPCODE;
            }
            break;
        }
        case 0xA: { // ANNN: i := NNN
            chip8->I = GET_NNN;
            break;
        }
        case 0xB: { // BNNN: jump0 NNN
            chip8->pc = GET_NNN + chip8->V[0];
            break;
        }
        case 0xC: { // CXNN: vX := random NN
            chip8->V[GET_X] = rand() & GET_NN;
            break;
        }
        case 0xD: { // DXYN: sprite vX vY N
            uint8_t const sprite_x = GET_VX % CHIP8_GFX_WIDTH;
            uint8_t const sprite_y = GET_VY % CHIP8_GFX_HEIGHT;
            uint8_t const sprite_height = GET_N;
            chip8->V[0xF] = 0;
            for (size_t i = 0; i < sprite_height; i++) {
                uint8_t const slice_y = sprite_y + i;
                if (slice_y >= CHIP8_GFX_HEIGHT) {
                    break;
                }
                uint8_t const slice = chip8_read(chip8, chip8->I + i);
                for (uint8_t j = 0, slice_mask = 0x80; j < 8; j++, slice_mask >>= 1) {
                    uint8_t const slice_x = sprite_x + j;
                    if (slice_x >= CHIP8_GFX_WIDTH) {
                        break;
                    }
                    if (slice & slice_mask) {
                        uint16_t const index = (slice_y * CHIP8_GFX_WIDTH) + slice_x;
                        chip8->gfx[index] ^= 1;
                        if (!chip8->gfx[index]) {
                            chip8->V[0xF] = 1;
                        }
                    }
                }
            }
            dxyn_executed = true;
            break;
        }
        case 0xE: {
            switch (GET_NN) {
                case 0x9E: { // EX9E: if vX -key then
                    if (chip8->keypad[GET_VX & 0xF]) {
                        INC_PC;
                    }
                    break;
                }
                case 0xA1: { // EXA1: if vX key then
                    if (!chip8->keypad[GET_VX & 0xF]) {
                        INC_PC;
                    }
                    break;
                }
                default: {
                    return CHIP8_UNKNOWN_OPCODE;
                }
            }
            break;
        }
        case 0xF: {
            switch (GET_NN) {
                case 0x07: { // FX07 vX := delay
                    chip8->V[GET_X] = chip8->dt;
                    break;
                }
                case 0x0A: { // FX0A: vX := key
                    int8_t const first_pressed_key = chip8_get_first_pressed_key(chip8);
                    if (chip8->waiting_key >= 0) {
                        if (first_pressed_key < 0 || chip8->waiting_key != first_pressed_key) {
                            chip8->V[GET_X] = chip8->waiting_key & 0xF;
                            chip8->waiting_key = -1;
                        } else {
                            chip8->pc -= 2;
                        }
                    } else {
                        if (first_pressed_key >= 0) {
                            chip8->waiting_key = first_pressed_key;
                        }
                        chip8->pc -= 2;
                    }
                    break;
                }
                case 0x15: { // FX15: delay := vX
                    chip8->dt = GET_VX;
                    break;
                }
                case 0x18: { // FX18: buzzer := vX
                    chip8->st = GET_VX;
                    break;
                }
                case 0x1E: { // FX1E: i += vX
                    chip8->I += GET_VX;
                    break;
                }
                case 0x29: { // FX29: i := hex vX
                    chip8->I = FONT_BEGIN_OFFSET + FONT_SIZE * (GET_VX & 0xF);
                    break;
                }
                case 0x33: { // FXEE: bcd vX
                    uint16_t const I = chip8->I;
                    uint8_t const VX = GET_VX;
                    uint64_t const hundreds = ((uint64_t)VX * 0x51EB851FL) >> 37;
                    uint64_t const remainder = VX - hundreds * 100;
                    uint64_t const tens = (remainder * 0xCCCDL) >> 19;
                    uint64_t const ones = remainder - tens * 10;
                    chip8_write(chip8, I, hundreds);
                    chip8_write(chip8, I + 1, tens);
                    chip8_write(chip8, I + 2, ones);
                    break;
                }
                case 0x55: { // FX55: save vX
                    uint16_t const I = chip8->I;
                    uint8_t const X = GET_X;
                    for (int i = 0; i <= X; i++) {
                        chip8_write(chip8, I + i, chip8->V[i]);
                    }
                    chip8->I += X + 1;
                    break;
                }
                case 0x65: { // FX65: load vX
                    uint16_t const I = chip8->I;
                    uint8_t const X = GET_X;
                    for (int i = 0; i <= X; i++) {
                        chip8->V[i] = chip8_read(chip8, I + i);
                    }
                    chip8->I += X + 1;
                    break;
                }
                default: {
                    return CHIP8_UNKNOWN_OPCODE;
                }
            }
            break;
        }
        default: {
            return CHIP8_UNKNOWN_OPCODE;
        }
    }

    #undef GET_VX
    #undef GET_VY
    #undef GET_Y
    #undef GET_X
    #undef GET_N
    #undef GET_NN
    #undef GET_NNN
    #undef INC_PC

    return dxyn_executed ? CHIP8_DXYN : 0;
}

uint32_t chip8_execute_frame(chip8_t *chip8) {
    if (chip8 == NULL) {
        return CHIP8_NULL_PTR;
    }

    if (chip8->dt > 0) {
        chip8->dt--;
    }
    if (chip8->st > 0) {
        chip8->st--;
    }

    for (size_t i = 0; i < CHIP8_IPF; i++) {
        uint32_t const flags = chip8_cycle(chip8);
        if (flags & CHIP8_DXYN) {
            break;
        }
        if (flags & CHIP8_NULL_PTR || flags & CHIP8_UNKNOWN_OPCODE) {
            return flags;
        }
    }

    return 0;
}

uint32_t chip8_set_key(chip8_t * const chip8, const uint8_t index, const bool value) {
    if (chip8 == NULL) {
        return CHIP8_NULL_PTR;
    }
    chip8->keypad[index & 0xF] = value;
    return 0;
}

uint32_t chip8_dump_argb8(chip8_t const *chip8, uint32_t *frame_buffer, const size_t frame_buffer_length) {
    if (chip8 == NULL) {
        return CHIP8_NULL_PTR;
    }
    size_t const min = frame_buffer_length < CHIP8_GFX_LENGTH ? frame_buffer_length : CHIP8_GFX_LENGTH;
    for (size_t i = 0; i < min; i++) {
        frame_buffer[i] = chip8->gfx[i] ? 0xFFFFFFFF : 0x000000;
    }
    return 0;
}

bool chip8_play_buzzer(chip8_t const *chip8) {
    if (chip8 == NULL) {
        return false;
    }
    return chip8->st > 0;
}


