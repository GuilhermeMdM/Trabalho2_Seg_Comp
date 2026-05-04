#include "sha3.h"

#include <array>
#include <cstdint>

namespace
{
    constexpr size_t kSha3DigestSize = 32;
    constexpr size_t kSha3RateBytes = 136; // SHA3-256

    inline uint64_t rotl64(uint64_t x, unsigned n)
    {
        return (x << n) | (x >> (64U - n));
    }

    void keccakF1600(std::array<uint64_t, 25> &state)
    {
        static const uint64_t roundConstants[24] = {
            0x0000000000000001ULL, 0x0000000000008082ULL,
            0x800000000000808aULL, 0x8000000080008000ULL,
            0x000000000000808bULL, 0x0000000080000001ULL,
            0x8000000080008081ULL, 0x8000000000008009ULL,
            0x000000000000008aULL, 0x0000000000000088ULL,
            0x0000000080008009ULL, 0x000000008000000aULL,
            0x000000008000808bULL, 0x800000000000008bULL,
            0x8000000000008089ULL, 0x8000000000008003ULL,
            0x8000000000008002ULL, 0x8000000000000080ULL,
            0x000000000000800aULL, 0x800000008000000aULL,
            0x8000000080008081ULL, 0x8000000000008080ULL,
            0x0000000080000001ULL, 0x8000000080008008ULL};
        static const unsigned rotationOffsets[25] = {
            0, 1, 62, 28, 27,
            36, 44, 6, 55, 20,
            3, 10, 43, 25, 39,
            41, 45, 15, 21, 8,
            18, 2, 61, 56, 14};

        for (int round = 0; round < 24; ++round)
        {
            uint64_t c[5] = {0};
            for (int x = 0; x < 5; ++x)
            {
                c[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^ state[x + 15] ^ state[x + 20];
            }
            uint64_t d[5] = {0};
            for (int x = 0; x < 5; ++x)
            {
                d[x] = c[(x + 4) % 5] ^ rotl64(c[(x + 1) % 5], 1);
            }
            for (int y = 0; y < 5; ++y)
            {
                for (int x = 0; x < 5; ++x)
                {
                    state[x + 5 * y] ^= d[x];
                }
            }

            std::array<uint64_t, 25> b{};
            for (int y = 0; y < 5; ++y)
            {
                for (int x = 0; x < 5; ++x)
                {
                    const int index = x + 5 * y;
                    const int newX = y;
                    const int newY = (2 * x + 3 * y) % 5;
                    b[newX + 5 * newY] = rotl64(state[index], rotationOffsets[index]);
                }
            }

            for (int y = 0; y < 5; ++y)
            {
                for (int x = 0; x < 5; ++x)
                {
                    state[x + 5 * y] =
                        b[x + 5 * y] ^
                        ((~b[((x + 1) % 5) + 5 * y]) & b[((x + 2) % 5) + 5 * y]);
                }
            }

            state[0] ^= roundConstants[round];
        }
    }

    uint64_t load64LE(const unsigned char *p)
    {
        uint64_t value = 0;
        for (int i = 0; i < 8; ++i)
        {
            value |= (static_cast<uint64_t>(p[i]) << (8 * i));
        }
        return value;
    }
}

std::array<unsigned char, 32> sha3_256(const std::vector<unsigned char> &input)
{
    std::array<uint64_t, 25> state{};
    size_t offset = 0;

    while (offset + kSha3RateBytes <= input.size())
    {
        for (size_t i = 0; i < (kSha3RateBytes / 8); ++i)
        {
            state[i] ^= load64LE(input.data() + offset + 8 * i);
        }
        keccakF1600(state);
        offset += kSha3RateBytes;
    }

    std::array<unsigned char, kSha3RateBytes> block{};
    const size_t remaining = input.size() - offset;
    for (size_t i = 0; i < remaining; ++i)
    {
        block[i] = input[offset + i];
    }
    block[remaining] ^= 0x06;
    block[kSha3RateBytes - 1] ^= 0x80;
    for (size_t i = 0; i < (kSha3RateBytes / 8); ++i)
    {
        state[i] ^= load64LE(block.data() + 8 * i);
    }
    keccakF1600(state);

    std::array<unsigned char, kSha3DigestSize> out{};
    for (size_t i = 0; i < kSha3DigestSize; ++i)
    {
        out[i] = static_cast<unsigned char>((state[i / 8] >> (8 * (i % 8))) & 0xFF);
    }
    return out;
}
