#include "gtest/gtest.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#define private public
#include "internal/cpu.hpp"
#undef private
#include "internal/flag.hpp"

namespace {
namespace fs = std::filesystem;

std::vector<uint8_t> read_binary_file(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

std::string sha256_hex(const std::vector<uint8_t>& data) {
    // Minimal embedded SHA-256 implementation for deterministic fixture checks.
    static constexpr std::array<uint32_t, 64> k = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
    };

    auto rotr = [](uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); };
    auto ch = [](uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); };
    auto maj = [](uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); };
    auto bsig0 = [&](uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); };
    auto bsig1 = [&](uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); };
    auto ssig0 = [&](uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); };
    auto ssig1 = [&](uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); };

    std::vector<uint8_t> padded = data;
    const uint64_t bit_len = static_cast<uint64_t>(padded.size()) * 8u;
    padded.push_back(0x80u);
    while ((padded.size() % 64u) != 56u) {
        padded.push_back(0u);
    }
    for (int i = 7; i >= 0; --i) {
        padded.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xffu));
    }

    uint32_t h0 = 0x6a09e667u;
    uint32_t h1 = 0xbb67ae85u;
    uint32_t h2 = 0x3c6ef372u;
    uint32_t h3 = 0xa54ff53au;
    uint32_t h4 = 0x510e527fu;
    uint32_t h5 = 0x9b05688cu;
    uint32_t h6 = 0x1f83d9abu;
    uint32_t h7 = 0x5be0cd19u;

    std::array<uint32_t, 64> w{};
    for (size_t chunk = 0; chunk < padded.size(); chunk += 64) {
        for (size_t i = 0; i < 16; ++i) {
            const size_t j = chunk + i * 4;
            w[i] = (static_cast<uint32_t>(padded[j]) << 24) |
                   (static_cast<uint32_t>(padded[j + 1]) << 16) |
                   (static_cast<uint32_t>(padded[j + 2]) << 8) |
                   static_cast<uint32_t>(padded[j + 3]);
        }
        for (size_t i = 16; i < 64; ++i) {
            w[i] = ssig1(w[i - 2]) + w[i - 7] + ssig0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;
        for (size_t i = 0; i < 64; ++i) {
            const uint32_t t1 = h + bsig1(e) + ch(e, f, g) + k[i] + w[i];
            const uint32_t t2 = bsig0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h0 += a; h1 += b; h2 += c; h3 += d;
        h4 += e; h5 += f; h6 += g; h7 += h;
    }

    std::array<uint32_t, 8> digest = {h0, h1, h2, h3, h4, h5, h6, h7};
    static constexpr char hex[] = "0123456789abcdef";
    std::string out;
    out.reserve(64);
    for (uint32_t v : digest) {
        for (int shift = 28; shift >= 0; shift -= 4) {
            out.push_back(hex[(v >> shift) & 0x0fu]);
        }
    }
    return out;
}

bool has_flag(const CPU& cpu, uint8_t flag) {
    return (cpu.status_.status & flag) != 0;
}
}  // namespace


TEST(CPUInstructions, LDAImmediateLoadsData) {
    CPU cpu;

    cpu.load_and_run({0xa9, 0x05, 0x00});

    EXPECT_EQ(cpu.register_a(), 5);
    EXPECT_EQ(cpu.status() & Flag::ZERO, 0);
    EXPECT_EQ(cpu.status() & Flag::NEGATIVE, 0);
}

TEST(CPUInstructions, TAXMovesAToX) {
    CPU cpu;
    cpu.set_register_a_for_test(10);

    cpu.load_and_run({0xaa, 0x00});

    EXPECT_EQ(cpu.register_x(), 10);
}

TEST(CPUInstructions, FiveOpsWorkTogether) {
    CPU cpu;

    cpu.load_and_run({0xa9, 0xc0, 0xaa, 0xe8, 0x00});

    EXPECT_EQ(cpu.register_x(), 0xc1);
}

TEST(CPUInstructions, INXOverflows) {
    CPU cpu;
    cpu.set_register_x_for_test(0xff);

    cpu.load_and_run({0xe8, 0xe8, 0x00});

    EXPECT_EQ(cpu.register_x(), 1);
}

TEST(CPUInstructions, LDAFromMemory) {
    CPU cpu;
    cpu.mem_write(0x10, 0x55);

    cpu.load_and_run({0xa5, 0x10, 0x00});

    EXPECT_EQ(cpu.register_a(), 0x55);
}

TEST(CPUInstructions, LDAIndirectXUsesIndexedZeroPagePointer) {
    CPU cpu;
    cpu.register_x_ = 0x02;
    cpu.bus.write(0x82, 0x00);
    cpu.bus.write(0x83, 0x03);
    cpu.bus.write(0x0300, 0x5b);

    cpu.load_and_run({0xa1, 0x80, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x5b);
}

TEST(CPUInstructions, LDAIndirectXPointerWrapsInZeroPage) {
    CPU cpu;
    cpu.register_x_ = 0x04;
    cpu.bus.write(0x03, 0x34);
    cpu.bus.write(0x04, 0x12);
    cpu.bus.write(0x1234, 0x5c);

    cpu.load_and_run({0xa1, 0xff, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x5c);
}

TEST(CPUInstructions, ADCAddsMemoryAndCarryToAccumulator) {
    CPU cpu;
    cpu.register_a_ = 1;
    cpu.status_.set(Flag::CARRY);

    cpu.load_and_run({0x69, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 4);
    EXPECT_FALSE(has_flag(cpu, Flag::CARRY));
    EXPECT_FALSE(has_flag(cpu, Flag::ZERO));
    EXPECT_FALSE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, ANDBitwiseAndsAccumulatorWithMemory) {
    CPU cpu;
    cpu.register_a_ = 0b1100'1010;

    cpu.load_and_run({0x29, 0b1010'1100, 0x00});

    EXPECT_EQ(cpu.register_a_, 0b1000'1000);
    EXPECT_TRUE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, ASLShiftsAccumulatorLeftAndSetsCarry) {
    CPU cpu;
    cpu.register_a_ = 0x81;

    cpu.load_and_run({0x0a, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
    EXPECT_FALSE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, BCCBranchesWhenCarryClear) {
    CPU cpu;

    cpu.load_and_run({0x90, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, BCSBranchesWhenCarrySet) {
    CPU cpu;
    cpu.status_.set(Flag::CARRY);

    cpu.load_and_run({0xb0, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, BEQBranchesWhenZeroSet) {
    CPU cpu;
    cpu.status_.set(Flag::ZERO);

    cpu.load_and_run({0xf0, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, BITSetsZeroOverflowAndNegativeFromMemory) {
    CPU cpu;
    cpu.register_a_ = 0x40;
    cpu.bus.write(0x10, 0xc0);

    cpu.load_and_run({0x24, 0x10, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::ZERO));
    EXPECT_TRUE(has_flag(cpu, Flag::OVERFLOWED));
    EXPECT_TRUE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, BITClearsOverflowAndNegativeFromMemory) {
    CPU cpu;
    cpu.register_a_ = 0x00;
    cpu.status_.set(Flag::OVERFLOWED).set(Flag::NEGATIVE);
    cpu.bus.write(0x10, 0x00);

    cpu.load_and_run({0x24, 0x10, 0x00});

    EXPECT_TRUE(has_flag(cpu, Flag::ZERO));
    EXPECT_FALSE(has_flag(cpu, Flag::OVERFLOWED));
    EXPECT_FALSE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, BMIBranchesWhenNegativeSet) {
    CPU cpu;
    cpu.status_.set(Flag::NEGATIVE);

    cpu.load_and_run({0x30, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, BNEBranchesWhenZeroClear) {
    CPU cpu;

    cpu.load_and_run({0xd0, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, BPLBranchesWhenNegativeClear) {
    CPU cpu;

    cpu.load_and_run({0x10, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, BRKSetsBreakFlagAndStops) {
    CPU cpu;

    cpu.load_and_run({0x00, 0xa9, 0x01});

    EXPECT_TRUE(has_flag(cpu, Flag::BREAK));
    EXPECT_EQ(cpu.register_a_, 0);
}

TEST(CPUInstructions, BVCBranchesWhenOverflowClear) {
    CPU cpu;

    cpu.load_and_run({0x50, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, BVSBranchesWhenOverflowSet) {
    CPU cpu;
    cpu.status_.set(Flag::OVERFLOWED);

    cpu.load_and_run({0x70, 0x02, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, CLCClearsCarry) {
    CPU cpu;
    cpu.status_.set(Flag::CARRY);

    cpu.load_and_run({0x18, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::CARRY));
}

TEST(CPUInstructions, CLDClearsDecimal) {
    CPU cpu;
    cpu.status_.set(Flag::DECIMAL);

    cpu.load_and_run({0xd8, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::DECIMAL));
}

TEST(CPUInstructions, CLIClearsInterruptDisable) {
    CPU cpu;
    cpu.status_.set(Flag::INTERRUPT);

    cpu.load_and_run({0x58, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::INTERRUPT));
}

TEST(CPUInstructions, CLVClearsOverflow) {
    CPU cpu;
    cpu.status_.set(Flag::OVERFLOWED);

    cpu.load_and_run({0xb8, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::OVERFLOWED));
}

TEST(CPUInstructions, CMPComparesAccumulatorWithMemory) {
    CPU cpu;
    cpu.register_a_ = 0x10;

    cpu.load_and_run({0xc9, 0x10, 0x00});

    EXPECT_TRUE(has_flag(cpu, Flag::ZERO));
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
    EXPECT_FALSE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, CPXComparesXWithMemory) {
    CPU cpu;
    cpu.register_x_ = 0x10;

    cpu.load_and_run({0xe0, 0x20, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::ZERO));
    EXPECT_FALSE(has_flag(cpu, Flag::CARRY));
    EXPECT_TRUE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, CPYComparesYWithMemory) {
    CPU cpu;
    cpu.register_y_ = 0x20;

    cpu.load_and_run({0xc0, 0x10, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::ZERO));
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
    EXPECT_FALSE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, CPYSetsNegativeFromSubtractionResult) {
    CPU cpu;
    cpu.register_y_ = 0x40;

    cpu.load_and_run({0xc0, 0x41, 0x00});

    EXPECT_FALSE(has_flag(cpu, Flag::ZERO));
    EXPECT_FALSE(has_flag(cpu, Flag::CARRY));
    EXPECT_TRUE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, DECDecrementsMemory) {
    CPU cpu;
    cpu.bus.write(0x10, 0x02);

    cpu.load_and_run({0xc6, 0x10, 0x00});

    EXPECT_EQ(cpu.bus.read(0x10), 0x01);
}

TEST(CPUInstructions, DEXDecrementsX) {
    CPU cpu;
    cpu.register_x_ = 0x02;

    cpu.load_and_run({0xca, 0x00});

    EXPECT_EQ(cpu.register_x_, 0x01);
}

TEST(CPUInstructions, DEYDecrementsY) {
    CPU cpu;
    cpu.register_y_ = 0x02;

    cpu.load_and_run({0x88, 0x00});

    EXPECT_EQ(cpu.register_y_, 0x01);
}

TEST(CPUInstructions, EORExclusiveOrsAccumulatorWithMemory) {
    CPU cpu;
    cpu.register_a_ = 0b1010'1010;

    cpu.load_and_run({0x49, 0b1111'0000, 0x00});

    EXPECT_EQ(cpu.register_a_, 0b0101'1010);
}

TEST(CPUInstructions, INCIncrementsMemory) {
    CPU cpu;
    cpu.bus.write(0x10, 0x01);

    cpu.load_and_run({0xe6, 0x10, 0x00});

    EXPECT_EQ(cpu.bus.read(0x10), 0x02);
}

TEST(CPUInstructions, INYIncrementsY) {
    CPU cpu;
    cpu.register_y_ = 0x01;

    cpu.load_and_run({0xc8, 0x00});

    EXPECT_EQ(cpu.register_y_, 0x02);
}

TEST(CPUInstructions, JMPJumpsToAbsoluteAddress) {
    CPU cpu;

    cpu.load_and_run({0x4c, 0x05, 0x06, 0xa9, 0x01, 0xa9, 0x02, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x02);
}

TEST(CPUInstructions, JSRCallsSubroutineAndRTSReturns) {
    CPU cpu;

    cpu.load_and_run({0x20, 0x06, 0x06, 0xa9, 0x01, 0x00, 0xa9, 0x02, 0x60});

    EXPECT_EQ(cpu.register_a_, 0x01);
}

TEST(CPUInstructions, LDXLoadsX) {
    CPU cpu;

    cpu.load_and_run({0xa2, 0x44, 0x00});

    EXPECT_EQ(cpu.register_x_, 0x44);
}

TEST(CPUInstructions, LDYLoadsY) {
    CPU cpu;

    cpu.load_and_run({0xa0, 0x44, 0x00});

    EXPECT_EQ(cpu.register_y_, 0x44);
}

TEST(CPUInstructions, ZeroPageXAddressingWraps) {
    CPU cpu;
    cpu.bus.write(0x89, 0xbb);

    cpu.load_and_run({0xa2, 0x8a, 0xb4, 0xff, 0x00});

    EXPECT_EQ(cpu.register_y_, 0xbb);
}

TEST(CPUInstructions, ZeroPageYAddressingWraps) {
    CPU cpu;
    cpu.bus.write(0x89, 0xbb);

    cpu.load_and_run({0xa0, 0x8a, 0xb6, 0xff, 0x00});

    EXPECT_EQ(cpu.register_x_, 0xbb);
}

TEST(CPUInstructions, LSRShiftsAccumulatorRightAndSetsCarry) {
    CPU cpu;
    cpu.register_a_ = 0x03;

    cpu.load_and_run({0x4a, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x01);
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
    EXPECT_FALSE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, LSRDirectionDispatchUsesOriginalLowBitForCarry) {
    CPU cpu;
    cpu.register_a_ = 0x01;

    cpu.load_and_run({0x4a, 0xb0, 0x02, 0xa9, 0xff, 0xa9, 0x42, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x42);
}

TEST(CPUInstructions, NOPDoesNothing) {
    CPU cpu;

    cpu.load_and_run({0xea, 0xa9, 0x07, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x07);
}

TEST(CPUInstructions, ORABitwiseOrsAccumulatorWithMemory) {
    CPU cpu;
    cpu.register_a_ = 0b0101'0000;

    cpu.load_and_run({0x09, 0b0000'1111, 0x00});

    EXPECT_EQ(cpu.register_a_, 0b0101'1111);
}

TEST(CPUInstructions, PHAPushesAccumulator) {
    CPU cpu;
    cpu.register_a_ = 0x42;

    cpu.load_and_run({0x48, 0x00});

    EXPECT_EQ(cpu.bus.read(STACK + cpu.stack_pointer_+1), 0x42);
}

TEST(CPUInstructions, PHPPushesProcessorStatus) {
    CPU cpu;
    cpu.status_.set(Flag::CARRY);

    cpu.load_and_run({0x08, 0x00});

    EXPECT_NE(cpu.bus.read(STACK + cpu.stack_pointer_+1) & Flag::CARRY, 0);
}

TEST(CPUInstructions, PLAPullsAccumulator) {
    CPU cpu;
    cpu.stack_pointer_ = 0xfc;
    cpu.bus.write(0x01fd, 0x42);

    cpu.load_and_run({0x68, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x42);
}

TEST(CPUInstructions, PLPPullsProcessorStatus) {
    CPU cpu;
    cpu.stack_pointer_ = 0xfc;
    cpu.bus.write(0x01fd, Flag::CARRY);

    cpu.load_and_run({0x28, 0x00});

    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
}

TEST(CPUInstructions, ROLRotatesAccumulatorLeftThroughCarry) {
    CPU cpu;
    cpu.register_a_ = 0x80;
    cpu.status_.set(Flag::CARRY);

    cpu.load_and_run({0x2a, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x01);
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
}

TEST(CPUInstructions, RORRotatesAccumulatorRightThroughCarry) {
    CPU cpu;
    cpu.register_a_ = 0x01;
    cpu.status_.set(Flag::CARRY);

    cpu.load_and_run({0x6a, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x80);
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
    EXPECT_TRUE(has_flag(cpu, Flag::NEGATIVE));
}

TEST(CPUInstructions, RTIRestoresStatusAndProgramCounterFromStack) {
    CPU cpu;

    cpu.stack_push_u16(0x0602);
    cpu.stack_push(Flag::CARRY);

    cpu.load_and_run({0x40, 0x00, 0xa9, 0x44, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x44);
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
}

TEST(CPUInstructions, RTSReturnsFromSubroutine) {
    CPU cpu;

    cpu.load_and_run({0x20, 0x06, 0x06, 0xa9, 0x01, 0x00, 0xa9, 0x02, 0x60});

    EXPECT_EQ(cpu.register_a_, 0x01);
}

TEST(CPUInstructions, SBCSubtractsMemoryAndBorrowFromAccumulator) {
    CPU cpu;
    cpu.register_a_ = 5;
    cpu.status_.set(Flag::CARRY);

    cpu.load_and_run({0xe9, 0x03, 0x00});

    EXPECT_EQ(cpu.register_a_, 2);
    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
}

TEST(CPUInstructions, SECSetCarry) {
    CPU cpu;

    cpu.load_and_run({0x38, 0x00});

    EXPECT_TRUE(has_flag(cpu, Flag::CARRY));
}

TEST(CPUInstructions, SEDSetsDecimal) {
    CPU cpu;

    cpu.load_and_run({0xf8, 0x00});

    EXPECT_TRUE(has_flag(cpu, Flag::DECIMAL));
}

TEST(CPUInstructions, SEISetsInterruptDisable) {
    CPU cpu;

    cpu.load_and_run({0x78, 0x00});

    EXPECT_TRUE(has_flag(cpu, Flag::INTERRUPT));
}

TEST(CPUInstructions, STAStoresAccumulator) {
    CPU cpu;
    cpu.register_a_ = 0x42;

    cpu.load_and_run({0x85, 0x10, 0x00});

    EXPECT_EQ(cpu.bus.read(0x10), 0x42);
}

TEST(CPUInstructions, STAIndirectXUsesZeroPagePointerOffsetByX) {
    CPU cpu;
    cpu.register_a_ = 0x42;
    cpu.register_x_ = 0x04;
    cpu.bus.write(0x14, 0x00);
    cpu.bus.write(0x15, 0x02);

    cpu.load_and_run({0x81, 0x10, 0x00});

    EXPECT_EQ(cpu.bus.read(0x0200), 0x42);
}

TEST(CPUInstructions, STAIndirectYUsesZeroPagePointerPlusY) {
    CPU cpu;
    cpu.register_a_ = 0x42;
    cpu.register_y_ = 0x04;
    cpu.bus.write(0x10, 0x00);
    cpu.bus.write(0x11, 0x02);

    cpu.load_and_run({0x91, 0x10, 0x00});

    EXPECT_EQ(cpu.bus.read(0x0204), 0x42);
}

TEST(CPUInstructions, STXStoresX) {
    CPU cpu;
    cpu.register_x_ = 0x42;

    cpu.load_and_run({0x86, 0x10, 0x00});

    EXPECT_EQ(cpu.bus.read(0x10), 0x42);
}

TEST(CPUInstructions, STYStoresY) {
    CPU cpu;
    cpu.register_y_ = 0x42;

    cpu.load_and_run({0x84, 0x10, 0x00});

    EXPECT_EQ(cpu.bus.read(0x10), 0x42);
}

TEST(CPUInstructions, TAYMovesAToY) {
    CPU cpu;
    cpu.register_a_ = 0x42;

    cpu.load_and_run({0xa8, 0x00});

    EXPECT_EQ(cpu.register_y_, 0x42);
}

TEST(CPUInstructions, TSXMovesStackPointerToX) {
    CPU cpu;
    cpu.stack_pointer_ = 0x42;

    cpu.load_and_run({0xba, 0x00});

    EXPECT_EQ(cpu.register_x_, 0x42);
}

TEST(CPUInstructions, TXAMovesXToA) {
    CPU cpu;
    cpu.register_x_ = 0x42;

    cpu.load_and_run({0x8a, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x42);
}

TEST(CPUInstructions, TXSMovesXToStackPointer) {
    CPU cpu;
    cpu.register_x_ = 0x42;

    cpu.load_and_run({0x9a, 0x00});

    EXPECT_EQ(cpu.stack_pointer_, 0x42);
}

TEST(CPUInstructions, TYAMovesYToA) {
    CPU cpu;
    cpu.register_y_ = 0x42;

    cpu.load_and_run({0x98, 0x00});

    EXPECT_EQ(cpu.register_a_, 0x42);
}
