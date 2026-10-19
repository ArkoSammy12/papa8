#ifndef PAPA8_CHIP8_HOST_H
#define PAPA8_CHIP8_HOST_H

#include <stdint.h>
#include "chip8.h"

typedef struct {
    int32_t error_num;
    char* msg;
} run_result_t;

run_result_t chip8_host_run_chip8(chip8_t *chip8);

#endif //PAPA8_CHIP8_HOST_H
