#include <SDL3/SDL.h>
#include "chip8_host.h"

#include <stdio.h>

static void poll_chip8_keypad(chip8_t *chip8, const SDL_Keycode key, const bool value) {
    if (chip8 == NULL) {
        return;
    }
    switch (key) {
        case SDLK_1: {
            chip8_set_key(chip8, 0x1, value);
            break;
        }
        case SDLK_2: {
            chip8_set_key(chip8, 0x2, value);
            break;
        }
        case SDLK_3: {
            chip8_set_key(chip8, 0x3, value);
        }
        case SDLK_4: {
            chip8_set_key(chip8, 0xC, value);
            break;
        }
        case SDLK_Q: {
            chip8_set_key(chip8, 0x4, value);
            break;
        }
        case SDLK_W: {
            chip8_set_key(chip8, 0x5, value);
            break;
        }
        case SDLK_E: {
            chip8_set_key(chip8, 0x6, value);
            break;
        }
        case SDLK_R: {
            chip8_set_key(chip8, 0xD, value);
            break;
        }
        case SDLK_A: {
            chip8_set_key(chip8, 0x7, value);
            break;
        }
        case SDLK_S: {
            chip8_set_key(chip8, 0x8, value);
            break;
        }
        case SDLK_D: {
            chip8_set_key(chip8, 0x9, value);
            break;
        }
        case SDLK_F: {
            chip8_set_key(chip8, 0xE, value);
            break;
        }
        case SDLK_Z: {
            chip8_set_key(chip8, 0xA, value);
            break;
        }
        case SDLK_X: {
            chip8_set_key(chip8, 0x0, value);
            break;
        }
        case SDLK_C: {
            chip8_set_key(chip8, 0xB, value);
            break;
        }
        case SDLK_V: {
            chip8_set_key(chip8, 0xF, value);
            break;
        }
        default: {}
    }
}

run_result_t chip8_host_run_chip8(chip8_t *chip8) {
    if (chip8 == NULL) {
        return (run_result_t) {
            .error_num = CHIP8_NULL_PTR,
            .msg = "Failed to create CHIP-8 object!"
        };
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    constexpr int scale = 25;
    SDL_Window *window = SDL_CreateWindow("papa8", CHIP8_GFX_WIDTH * scale, CHIP8_GFX_HEIGHT * scale, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_FOCUS);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    SDL_Texture *texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            CHIP8_GFX_WIDTH,
            CHIP8_GFX_HEIGHT
    );
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderLogicalPresentation(renderer, CHIP8_GFX_WIDTH, CHIP8_GFX_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    run_result_t run_result = {0};
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    running = false;
                    break;
                }
                case SDL_EVENT_KEY_DOWN: {
                    poll_chip8_keypad(chip8, event.key.key, true);
                    break;
                }
                case SDL_EVENT_KEY_UP: {
                    poll_chip8_keypad(chip8, event.key.key, false);
                    break;
                }
                default: {}
            }
        }

        uint32_t const result = chip8_execute_frame(chip8);
        if (result & CHIP8_UNKNOWN_OPCODE) {
            running = false;
            run_result.error_num = CHIP8_UNKNOWN_OPCODE;
            run_result.msg = "Unrecognized opcode!";
            break;
        }

        uint32_t frame_buffer[CHIP8_GFX_LENGTH] = {0};
        chip8_dump_argb8(chip8, frame_buffer, CHIP8_GFX_LENGTH);
        constexpr int pitch = CHIP8_GFX_WIDTH * sizeof(uint32_t);
        SDL_UpdateTexture(texture, nullptr, frame_buffer, pitch);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    exit:
    if (texture) {
        SDL_DestroyTexture(texture);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    return run_result;
}