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

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320


uint8_t random_u8() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(1, 16);
    return static_cast<uint8_t>(dist(gen));
}

std::tuple<uint8_t, uint8_t, uint8_t> color_rgb(uint8_t color_index) {
    switch (color_index)
    {
    case 0:
        return {0, 0, 0};
    case 1:
        return {UINT8_MAX, UINT8_MAX, UINT8_MAX};
    case 2:
    case 9:
        return {128, 128, 128};
    case 3:
    case 10:
        return {UINT8_MAX, 0, 0};
    case 4:
    case 11:
        return {0, UINT8_MAX, 0};
    case 5:
    case 12:
        return {0, 0, UINT8_MAX};
    case 6:
    case 13:
        return {UINT8_MAX, 0, UINT8_MAX};
    case 7:
    case 14:
        return {UINT8_MAX, UINT8_MAX, 0};
    default:
        return {0, UINT8_MAX, UINT8_MAX};
    }
}

bool read_screen_state(CPU *cpu, std::vector<uint8_t>& frame) {
    auto frame_idx = 0;
    auto update = false;
    for (uint16_t i = 0x200u; i < 0x600u; i++) {
        auto color_index = cpu->bus.read(i);
        uint8_t r,g,b;
        std::tie(r,g,b) = color_rgb(color_index);
        if (frame[frame_idx] != r || frame[frame_idx + 1] != g || frame[frame_idx + 2] != b ){
           frame[frame_idx] = r;
           frame[frame_idx + 1] = g;
           frame[frame_idx + 2] = b;
           update = true;
       }
       frame_idx += 3;
    }
    return update;
}

std::vector<uint8_t> read_binary_file(const char* path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }

    return {std::istreambuf_iterator(input), std::istreambuf_iterator<char>()};
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <program.bin|cartridge.nes> [trace.log]\n";
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

    // Set up CPU
    auto cpu = CPU(ROM(game_code));
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
    auto screen_state = std::vector<uint8_t>(32*3*32, 0);
    cpu.interpret_with_callback([&screen_state, winSurface,
            window](CPU* cpu){
            SDL_PumpEvents();

            const Uint8* keyboard = SDL_GetKeyboardState(nullptr);
            if (keyboard[SDL_SCANCODE_W] || keyboard[SDL_SCANCODE_UP]) {
                cpu->bus.write(0xff, 0x77);
            } else if (keyboard[SDL_SCANCODE_D] || keyboard[SDL_SCANCODE_RIGHT]) {
                cpu->bus.write(0xff, 0x64);
            } else if (keyboard[SDL_SCANCODE_S] || keyboard[SDL_SCANCODE_DOWN]) {
                cpu->bus.write(0xff, 0x73);
            } else if (keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_LEFT]) {
                cpu->bus.write(0xff, 0x61);
            }

            // update mem[0xFE] with new Random Number
            cpu->bus.write(0xfe, random_u8());
            // read mem mapped screen state
            if (read_screen_state(cpu, screen_state)) {
                auto* pixels = static_cast<uint32_t*>(winSurface->pixels);
                const int pitch = winSurface->pitch / static_cast<int>(sizeof(uint32_t));
                constexpr int scale = SCREEN_WIDTH / 32;
                for (int y = 0; y < 32; ++y) {
                    for (int x = 0; x < 32; ++x) {
                        const int src_idx = (y * 32 + x) * 3;
                        const uint32_t color = SDL_MapRGB(
                            winSurface->format,
                            screen_state[src_idx],
                            screen_state[src_idx + 1],
                            screen_state[src_idx + 2]
                        );
                        const int dst_y = y * scale;
                        const int dst_x = x * scale;
                        for (int yy = 0; yy < scale; ++yy) {
                            uint32_t* row = pixels + (dst_y + yy) * pitch + dst_x;
                            for (int xx = 0; xx < scale; ++xx) {
                                row[xx] = color;
                            }
                        }
                    }
                }
                SDL_UpdateWindowSurface(window);
            }
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::high_resolution_clock::now() - start < std::chrono::nanoseconds(1));

            // render screen state
        });

    
    // Destroy the window. This will also destroy the surface
    SDL_DestroyWindow( window );
	
	
	// Quit SDL
	SDL_Quit();
}
