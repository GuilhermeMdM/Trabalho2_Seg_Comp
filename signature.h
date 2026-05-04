#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <string>
#include <vector>
#include "RSA.h"

bool signMessageSha3(const std::vector<unsigned char> &message,
                     const RSAKeyPair &sender,
                     std::string &signatureBase64,
                     std::string &error);

bool verifyMessageSha3(const std::vector<unsigned char> &message,
                       const RSAKeyPair &sender,
                       const std::string &signatureBase64,
                       std::string &error);

#endif
