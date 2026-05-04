#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>

// Transforma bytes em string legível
std::string base64_encode(const std::vector<unsigned char>& data);

// Transforma string legível de volta em bytes
std::vector<unsigned char> base64_decode(const std::string& encoded_string);

#endif // BASE64_H