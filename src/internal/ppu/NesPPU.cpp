//
// Created by Đức Bùi on 15/5/2026.
//

#include "NesPPU.h"

#include <cassert>

void NesPPU::write_to_controller(uint8_t value) {
    const auto before_nmi_status = controller_register.vblank_nmi_enabled();
    io_data_bus = value;
    controller_register.update(value);
    if (!before_nmi_status && controller_register.vblank_nmi_enabled() &&
        status_register.is_in_vblank()) {
        nmi_interrupt = 1;
    }
}

void NesPPU::increment_vram_address() {
    addr_register.increment(controller_register.vram_addr_increment());
}

uint8_t NesPPU::read_data() {
    const uint16_t addr = addr_register.get();
    increment_vram_address();

    uint8_t result = 0;
    if (addr < 0x3f00) {
        result = internal_data_buf;
        internal_data_buf = read_ppu_memory(addr);
    } else {
        result = read_ppu_memory(addr);
        internal_data_buf = read_ppu_memory(addr);
    }
    io_data_bus = result;
    return result;
}

// Horizontal:
//   [ A ] [ a ]
//   [ B ] [ b ]

// Vertical:
//   [ A ] [ B ]
//   [ a ] [ b ]
uint16_t NesPPU::mirror_vram_addr(uint16_t addr) const {
    auto mirror_vram = addr & 0x2fffu; // mirror down 0x3000-0x3eff
                                                    //to 0x2000 - 0x2eff;
    auto vram_index = mirror_vram - 0x2000; // to vram vector
    auto name_table = vram_index / 0x400;
    if (std::make_pair(mirroring, name_table) == std::make_pair(Vertical, 2)
        || std::make_pair(mirroring, name_table) == std::make_pair(Vertical,
            3)) {
        return vram_index - 0x800;
    }
    if (std::make_pair(mirroring, name_table) == std::make_pair(Horizontal, 2)) {
        return vram_index - 0x400;
    }

    if (std::make_pair(mirroring, name_table) == std::make_pair(Horizontal,
        1)) {
        return vram_index - 0x400;
    }

    if (std::make_pair(mirroring, name_table) == std::make_pair(Horizontal,
        3)) {
        return vram_index - 0x800;
    }
    return vram_index;
}

uint8_t NesPPU::read_status() {
    const uint8_t result = status_register.read(io_data_bus);
    first_register_write = true;
    addr_register.reset_latch();
    scroll_register.reset_latch();
    io_data_bus = result;
    return result;
}

uint8_t NesPPU::read_oam_data() {
    const uint8_t result = oam_data[oam_address_register.get()];
    io_data_bus = result;
    return result;
}

uint8_t NesPPU::read_open_bus() const {
    return io_data_bus;
}

uint8_t NesPPU::peek_register(uint16_t address) const {
    const uint16_t register_address = static_cast<uint16_t>(0x2000u + ((address - 0x2000u) % 8u));
    switch (register_address) {
        case 0x2002:
            return status_register.peek(io_data_bus);
        case 0x2004:
            return oam_data[oam_address_register.get()];
        case 0x2007:
            return read_ppu_memory(addr_register.get());
        case 0x2000:
        case 0x2001:
        case 0x2003:
        case 0x2005:
        case 0x2006:
        default:
            return io_data_bus;
    }
}

uint8_t NesPPU::peek_memory(uint16_t addr) const {
    return read_ppu_memory(addr);
}

void NesPPU::write_to_ppu_addr(uint8_t data) {
    io_data_bus = data;
    addr_register.update(data, first_register_write);
    first_register_write = !first_register_write;
}

void NesPPU::write_to_data(uint8_t data) {
    io_data_bus = data;
    write_ppu_memory(addr_register.get(), data);
    increment_vram_address();
}

void NesPPU::write_to_mask(uint8_t data) {
    io_data_bus = data;
    mask_register.update(data);
}

void NesPPU::write_to_oam_address(uint8_t data) {
    io_data_bus = data;
    oam_address_register.update(data);
}

void NesPPU::write_to_oam_data(uint8_t data) {
    io_data_bus = data;
    oam_data[oam_address_register.get()] = data;
    oam_address_register.increment();
}

void NesPPU::write_to_scroll(uint8_t data) {
    io_data_bus = data;
    scroll_register.update(data, first_register_write);
    first_register_write = !first_register_write;
}

void NesPPU::write_to_oam_dma(uint8_t page, std::span<const uint8_t, 256> data) {
    io_data_bus = page;
    oam_dma_register.update(page);
    for (uint8_t value : data) {
        oam_data[oam_address_register.get()] = value;
        oam_address_register.increment();
    }
}

bool NesPPU::is_sprite_0_hit(uint64_t cycle) const {
    const auto y = static_cast<size_t>(oam_data[0]);
    const auto x = static_cast<size_t>(oam_data[3]);
    return y == static_cast<size_t>(scanline)
        && x <= cycle
        && mask_register.show_sprite();
}

bool NesPPU::tick(uint64_t i) {
    cycles_ += i;

    while (cycles_ >= 341u) {
        if (is_sprite_0_hit(cycles_)) {
            status_register.set_sprite_zero_hit(true);
        }
        cycles_ = cycles_ - 341u;
        scanline += 1;

        if (scanline == 241) {
            status_register.set_vblank_started(true);
            status_register.set_sprite_zero_hit(false);
            if (controller_register.vblank_nmi_enabled()) {
                nmi_interrupt = 1;
            }

        }

        if (scanline >= 262) {
            scanline = 0;
            nmi_interrupt = std::nullopt;
            status_register.set_sprite_zero_hit(false);
            status_register.clear_vblank_started();
            return true;
        }
    }
    return false;

}

uint8_t NesPPU::read_ppu_memory(uint16_t addr) const {
    addr &= 0x3fffu;
    if (addr <= 0x1fff) {
        if (!chr_rom.empty()) {
            return chr_rom[addr % chr_rom.size()];
        }
        return chr_ram[addr % chr_ram.size()];
    }

    if (addr <= 0x3eff) {
        return vram[mirror_vram_addr(addr)];
    }

    return palette_table[palette_index(addr)];
}

void NesPPU::write_ppu_memory(uint16_t addr, uint8_t data) {
    addr &= 0x3fffu;
    if (addr <= 0x1fff) {
        if (chr_rom.empty()) {
            chr_ram[addr % chr_ram.size()] = data;
        }
        return;
    }
    if (addr <= 0x3eff) {
        vram[mirror_vram_addr(addr)] = data;
        return;
    }
    palette_table[palette_index(addr)] = data;
}

uint8_t NesPPU::palette_index(uint16_t addr) {
    auto index = static_cast<uint8_t>((addr - 0x3f00u) % 32u);
    if (index == 0x10 || index == 0x14 || index == 0x18 || index == 0x1c) {
        index -= 0x10;
    }
    return index;
}

std::optional<uint8_t> NesPPU::poll_nmi_interrupt() {
    auto val = nmi_interrupt;
    nmi_interrupt = std::nullopt;
    return val;
}
