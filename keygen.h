#ifndef KEYGEN_H
#define KEYGEN_H

#include <vector>
#include "lib/InfInt.h"

InfInt binpower(InfInt base, InfInt e, InfInt mod);
bool millerRabin(InfInt n, int iterations);
InfInt generateRandomCandidate(int bits);
InfInt generatePrime(int bits, int iterations);
InfInt gcd(InfInt a, InfInt b, InfInt& x, InfInt& y);
InfInt findModInv(InfInt a, InfInt m);
InfInt bytesToBigInt(const std::vector<unsigned char>& data);
std::vector<unsigned char> bigIntToBytes(InfInt num, size_t k);

#endif
