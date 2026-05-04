#include "RSA.h"

#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include "keygen.h"
#include "oaep.h"
#include "base64.h"

size_t modulusByteLength(InfInt n)
{
    size_t len = 0;
    while (n > 0)
    {
        n /= 256;
        len++;
    }
    return (len == 0) ? 1 : len;
}

RSAKeyPair generateRsaKeyPair(int bitSize, int primalityIterations, const std::string &ownerName)
{
    std::cout << "Gerando p e q (" << ownerName << ") em paralelo..." << std::endl;

    auto pFuture = std::async(std::launch::async, generatePrime, bitSize, primalityIterations);
    auto qFuture = std::async(std::launch::async, generatePrime, bitSize, primalityIterations);

    while (pFuture.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready ||
           qFuture.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
    {
        std::cout << "." << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    std::cout << " " << std::endl;

    InfInt p = pFuture.get();
    InfInt q = qFuture.get();
    while (q == p)
    {
        q = generatePrime(bitSize, primalityIterations);
    }
    std::cout << "[" << ownerName << "] p encontrado:" << std::endl;
    std::cout << p << std::endl;
    std::cout << "[" << ownerName << "] q encontrado:" << std::endl;
    std::cout << q << std::endl;

    InfInt n = p * q;
    InfInt z = (p - 1) * (q - 1);
    InfInt e = 65537;
    InfInt x, y;
    while (gcd(e, z, x, y) != 1)
    {
        e += 2;
    }

    InfInt d = findModInv(e, z);

    std::cout << "[" << ownerName << "] d: " << d << std::endl;

    return {n, e, d};
}

bool rsaOaepEncrypt(const std::vector<unsigned char> &message,
                    const RSAKeyPair &receiver,
                    std::vector<unsigned char> &cipherBytes,
                    std::string &cipherBase64,
                    std::string &error)
{
    const size_t k = modulusByteLength(receiver.n);
    const size_t maxLen = oaepMaxMessageLen(k);
    if (message.size() > maxLen)
    {
        error = "Mensagem maior que o limite OAEP para esse modulo.";
        return false;
    }

    std::vector<unsigned char> encodedMessage;
    if (!oaepEncode(message, k, encodedMessage))
    {
        error = "Falha no OAEP encode.";
        return false;
    }

    InfInt messageB1 = bytesToBigInt(encodedMessage);
    if (messageB1 >= receiver.n)
    {
        error = "Erro: bloco OAEP >= modulo RSA.";
        return false;
    }

    InfInt cypher = binpower(messageB1, receiver.e, receiver.n);
    cipherBytes = bigIntToBytes(cypher, k);
    cipherBase64 = base64_encode(cipherBytes);
    return true;
}

bool rsaOaepDecrypt(const std::vector<unsigned char> &cipherBytes,
                    const RSAKeyPair &receiver,
                    std::vector<unsigned char> &message,
                    std::string &error)
{
    const size_t k = modulusByteLength(receiver.n);
    if (cipherBytes.size() != k)
    {
        error = "Erro: tamanho do bloco cifrado difere do tamanho do modulo.";
        return false;
    }

    InfInt cypher = bytesToBigInt(cipherBytes);
    InfInt messageB2 = binpower(cypher, receiver.d, receiver.n);
    std::vector<unsigned char> decodedBlock = bigIntToBytes(messageB2, k);

    if (!oaepDecode(decodedBlock, message))
    {
        error = "[ERRO] Falha no OAEP decode.";
        return false;
    }

    return true;
}
