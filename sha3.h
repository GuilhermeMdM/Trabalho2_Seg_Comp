#ifndef SHA3_H
#define SHA3_H

#include <array>
#include <vector>

std::array<unsigned char, 32> sha3_256(const std::vector<unsigned char> &input);

#endif
