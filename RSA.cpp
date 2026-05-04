#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <future>
#include <thread>
#include <chrono>
#include <fstream>
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

    // ==========================================
    // ASSINATURA (SALVANDO NO ARQUIVO)
    // ==========================================
    std::vector<unsigned char> cypherBytes = bigIntToBytes(cypher, k);
    std::string cypherBase64 = base64_encode(cypherBytes);
    
    std::ofstream outSig("assinatura.sig");
    if (outSig) {
        outSig << cypherBase64;
        outSig.close();
        std::cout << "\n[OK] Assinatura salva no arquivo 'assinatura.sig' (Base64)." << std::endl;
    } else {
        std::cout << "Erro ao criar o arquivo de assinatura." << std::endl;
        return 1;
    }

    // ==========================================
    // VERIFICAÇÃO (LENDO DO ARQUIVO)
    // ==========================================
    std::cout << "\n[...] Iniciando verificacao. Lendo 'assinatura.sig'..." << std::endl;
    
    std::ifstream inSig("assinatura.sig");
    if (!inSig) {
        std::cout << "Erro ao ler o arquivo de assinatura." << std::endl;
        return 1;
    }
    // Lê todo o conteúdo do arquivo txt/sig para uma string
    std::string assinaturaLidaBase64((std::istreambuf_iterator<char>(inSig)), std::istreambuf_iterator<char>());
    inSig.close();

    // agora usa a string que veio do arquivo
    std::vector<unsigned char> decodedBytes = base64_decode(assinaturaLidaBase64);
    InfInt cypherRecuperado = bytesToBigInt(decodedBytes);

    // Decifra a assinatura recuperada
    InfInt messageB2 = binpower(cypherRecuperado, d, n);

    // descriptografando o OAEP
    std::vector<unsigned char> decodedBlock = bigIntToBytes(messageB2, k);
    std::vector<unsigned char> message2;
    
    if (!oaepDecode(decodedBlock, message2)) {
        std::cout << "[ERRO] Falha na verificacao. OAEP decode falhou." << std::endl;
        return 1;
    }

    std::cout << "\n[SUCESSO] Assinatura verificada! Texto recuperado do arquivo:" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    
    for (auto c : message2) {
        std::cout << c;
    }
    std::cout << "---------------------------------------------------" << std::endl;

    return 0;
}
