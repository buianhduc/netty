#include <cstdint>
#include <vector>
#include "SDL.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include "internal/cpu.hpp"
#include "internal/logger.hpp"
#include <memory>
#include <random>
#include <tuple>
#include <chrono>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <unordered_map>

#include "common/Color.h"
#include "common/Frame.h"
#include "internal/Joypad.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320

void put_surface_pixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    auto* row = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch;
    auto* pixel = row + x * surface->format->BytesPerPixel;

    switch (surface->format->BytesPerPixel) {
        case 1:
            *pixel = static_cast<uint8_t>(color);
            break;
        case 2:
            *reinterpret_cast<uint16_t*>(pixel) = static_cast<uint16_t>(color);
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                pixel[0] = static_cast<uint8_t>((color >> 16) & 0xff);
                pixel[1] = static_cast<uint8_t>((color >> 8) & 0xff);
                pixel[2] = static_cast<uint8_t>(color & 0xff);
            } else {
                pixel[0] = static_cast<uint8_t>(color & 0xff);
                pixel[1] = static_cast<uint8_t>((color >> 8) & 0xff);
                pixel[2] = static_cast<uint8_t>((color >> 16) & 0xff);
            }
            break;
        case 4:
            *reinterpret_cast<uint32_t*>(pixel) = color;
            break;
        default:
            break;
    }
}

void draw_frame_to_surface(const Frame& frame, SDL_Surface* surface) {
    if (surface == nullptr) {
        return;
    }

    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0) {
        return;
    }

    for (int dst_y = 0; dst_y < surface->h; ++dst_y) {
        const int src_y = std::clamp(dst_y * HEIGHT / surface->h, 0, HEIGHT - 1);
        for (int dst_x = 0; dst_x < surface->w; ++dst_x) {
            const int src_x = std::clamp(dst_x * WIDTH / surface->w, 0, WIDTH - 1);
            const int src_idx = (src_y * WIDTH + src_x) * 3;
            const auto color = SDL_MapRGB(
                surface->format,
                frame.data[src_idx],
                frame.data[src_idx + 1],
                frame.data[src_idx + 2]);
            put_surface_pixel(surface, dst_x, dst_y, color);
        }
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}


uint8_t random_u8() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution dist(0b00000000, 0b11111111);
    return static_cast<uint8_t>(dist(gen));
}
//
//
// bool read_screen_state(CPU *cpu, std::vector<uint8_t>& frame) {
//     auto frame_idx = 0;
//     auto update = false;
//     for (uint16_t i = 0x200u; i < 0x600u; i++) {
//         auto color_index = cpu->bus.read(i, true);
//         Color color = ColorHelper::color_rgb(color_index);
//         if (frame[frame_idx] != color.red || frame[frame_idx + 1] != color
//             .green || frame[frame_idx + 2] != color.blue ){
//            frame[frame_idx] = color.red;
//            frame[frame_idx + 1] = color.green;
//            frame[frame_idx + 2] = color.blue;
//            update = true;
//        }
//        frame_idx += 3;
//     }
//     return update;
// }

std::vector<uint8_t> read_binary_file(const char* path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }

    return {std::istreambuf_iterator(input), std::istreambuf_iterator<char>()};
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <program.bin|cartridge.nes> [trace.log|-]\n";
        return 1;
    }

    const auto game_code = read_binary_file(argv[1]);
    if (game_code.empty()) {
        std::cerr << "Failed to read binary program: " << argv[1] << "\n";
        return 1;
    }
    
    SDL_Surface* winSurface = nullptr;
	SDL_Window* window = nullptr;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Init failed"<<"\n";
        return 1;
    };
    window = SDL_CreateWindow("Snake game", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if ( !window ) {
	    std::cout << "Failed to create a window! Error: " << SDL_GetError() << "\n";
        return 1;
    }
    // Get the surface from the window
	winSurface = SDL_GetWindowSurface( window );

	// Make sure getting the surface succeeded
	if ( !winSurface ) {
		std::cout << "Error getting surface: " << SDL_GetError() << "\n";
		// End the program
		return 1;
	}

	// Update the window display
	SDL_UpdateWindowSurface( window );

    // Set up keymap
    std::unordered_map<SDL_Scancode, JoypadButton> key_map;
    key_map.emplace(SDL_SCANCODE_DOWN, JoypadButton::DOWN);
    key_map.emplace(SDL_SCANCODE_LEFT, JoypadButton::LEFT);
    key_map.emplace(SDL_SCANCODE_UP, JoypadButton::UP);
    key_map.emplace(SDL_SCANCODE_RIGHT, JoypadButton::RIGHT);
    key_map.emplace(SDL_SCANCODE_A, JoypadButton::BUTTON_A);
    key_map.emplace(SDL_SCANCODE_S, JoypadButton::BUTTON_B);
    key_map.emplace(SDL_SCANCODE_SPACE, JoypadButton::SELECT);
    key_map.emplace(SDL_SCANCODE_RETURN, JoypadButton::START);
    key_map.emplace(SDL_SCANCODE_KP_ENTER, JoypadButton::START);


    // Set up CPU
    auto rom = ROM(game_code);
    auto cpu = CPU(rom);

    cpu.reset();

    std::unique_ptr<Logger> trace_logger;
    if (argc == 3) {
        trace_logger = std::make_unique<Logger>(argv[2]);
        if (!trace_logger->is_open()) {
            std::cerr << "Failed to open trace log: " << argv[2] << "\n";
            return 1;
        }
        cpu.set_logger(trace_logger.get());
    }

    // Render screen state
    Frame frame = Frame();
    // try {
        cpu.interpret_with_callback([&frame, &key_map, winSurface,
                window](CPU* cpu){
            if (cpu->bus.take_frame_complete()) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        std::exit(0);
                    }
                    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                        const auto scancode = event.key.keysym.scancode;
                        if (scancode == SDL_SCANCODE_ESCAPE) {
                            std::exit(0);
                        }
                    }
                }
                SDL_PumpEvents();
                const auto* keyboard_state = SDL_GetKeyboardState(nullptr);
                std::array<bool, 8> button_pressed{};
                for (const auto& [scancode, button] : key_map) {
                    if (keyboard_state[scancode] != 0) {
                        for (uint8_t bit = 0; bit < button_pressed.size(); ++bit) {
                            if ((static_cast<uint8_t>(button) & (1u << bit)) != 0) {
                                button_pressed[bit] = true;
                            }
                        }
                    }
                }
                for (uint8_t bit = 0; bit < button_pressed.size(); ++bit) {
                    cpu->bus.joypad().set_button_pressed_status(
                        static_cast<JoypadButton>(1u << bit),
                        button_pressed[bit]);
                }
                cpu->bus.write(0xfe, random_u8(), true);
                render(cpu->bus.ppu(), frame);
                draw_frame_to_surface(frame, winSurface);
                SDL_UpdateWindowSurface(window);
            }

        // auto start = std::chrono::high_resolution_clock::now();
        // while (std::chrono::high_resolution_clock::now() - start < std::chrono::nanoseconds(1));

            // render screen state
        });
    // }

    
    // Destroy the window. This will also destroy the surface
    if (trace_logger) {
        trace_logger->flush();
    }
    SDL_DestroyWindow( window );
	
	
	// Quit SDL
	SDL_Quit();
}
