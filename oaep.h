#ifndef OAEP_H
#define OAEP_H

#include <string>
#include <vector>

size_t oaepMaxMessageLen(size_t k, size_t hLen = 32);
bool oaepEncode(const std::vector<unsigned char> &message,
                size_t k,
                std::vector<unsigned char> &encoded,
                const std::string &label = "");
bool oaepDecode(const std::vector<unsigned char> &encoded,
                std::vector<unsigned char> &message,
                const std::string &label = "");

#endif
