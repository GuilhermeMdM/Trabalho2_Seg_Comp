#ifndef RSA_H
#define RSA_H

#include <string>
#include <vector>
#include "lib/InfInt.h"

struct RSAKeyPair
{
    InfInt n;
    InfInt e;
    InfInt d;
};

size_t modulusByteLength(InfInt n);
RSAKeyPair generateRsaKeyPair(int bitSize, int primalityIterations, const std::string &ownerName);
bool rsaOaepEncrypt(const std::vector<unsigned char> &message,
                    const RSAKeyPair &receiver,
                    std::vector<unsigned char> &cipherBytes,
                    std::string &cipherBase64,
                    std::string &error);
bool rsaOaepDecrypt(const std::vector<unsigned char> &cipherBytes,
                    const RSAKeyPair &receiver,
                    std::vector<unsigned char> &message,
                    std::string &error);

#endif
