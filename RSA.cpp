#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <future>
#include <thread>
#include <chrono>
#include "keygen.h"
#include "oaep.h"

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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Uso: " << argv[0] << " <arquivo_mensagem>" << std::endl;
        return 1;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in)
    {
        std::cout << "Falha ao abrir arquivo de mensagem: " << argv[1] << std::endl;
        return 1;
    }

    std::vector<unsigned char> message1(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    if (message1.empty())
    {
        std::cout << "Arquivo de mensagem vazio." << std::endl;
        return 1;
    }

    int bitSize = 1024;
    int primalityIterations = 3;
    std::cout << "Gerando p e q em paralelo..." << std::endl;

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

    std::cout << "p encontrado:" << std::endl;
    std::cout << p << std::endl;
    std::cout << "q encontrado:" << std::endl;
    std::cout << q << std::endl;

    InfInt n = p * q;

    // Totiente de Euler
    InfInt z = (p - 1) * (q - 1);

    // escolhendo e
    InfInt e = 65537;

    InfInt x, y;
    while (gcd(e, z, x, y) != 1)
    {
        e += 2;
    }
    std::cout << "Valor de e: " << e << std::endl;

    // encontrando d
    InfInt d = findModInv(e, z);
    std::cout << "Valor de d: " << d << std::endl;

    // criptografando com OAEP
    const size_t k = modulusByteLength(n);
    const size_t maxLen = oaepMaxMessageLen(k);
    if (message1.size() > maxLen)
    {
        std::cout << "Mensagem maior que o limite OAEP para esse modulo." << std::endl;
        return 1;
    }

    std::vector<unsigned char> encodedMessage;
    if (!oaepEncode(message1, k, encodedMessage))
    {
        std::cout << "Falha no OAEP encode." << std::endl;
        return 1;
    }

    InfInt messageB1 = bytesToBigInt(encodedMessage);
    if (messageB1 >= n)
    {
        std::cout << "Erro: bloco OAEP >= modulo RSA." << std::endl;
        return 1;
    }

    InfInt cypher = binpower(messageB1, e, n);

    std::cout << "Texto cifrado: " << cypher << std::endl;

    // descriptografando com OAEP
    InfInt messageB2 = binpower(cypher, d, n);
    std::vector<unsigned char> decodedBlock = bigIntToBytes(messageB2, k);
    std::vector<unsigned char> message2;
    if (!oaepDecode(decodedBlock, message2))
    {
        std::cout << "Falha no OAEP decode." << std::endl;
        return 1;
    }

    std::cout << "Texto recuperado:" << std::endl;

    for (auto c : message2)
    {
        std::cout << c;
    }
    std::cout << std::endl;

    return 0;
}
