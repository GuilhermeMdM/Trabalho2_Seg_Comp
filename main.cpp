#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>

#include "RSA.h"
#include "signature.h"
#include "base64.h"

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

    const int bitSize = 1024;
    const int primalityIterations = 3;

    RSAKeyPair sender = generateRsaKeyPair(bitSize, primalityIterations, "EMISSOR");
    RSAKeyPair receiver = generateRsaKeyPair(bitSize, primalityIterations, "RECEPTOR");

    std::string error;
    std::vector<unsigned char> cipherBytes;
    std::string cipherBase64;

    if (!rsaOaepEncrypt(message1, receiver, cipherBytes, cipherBase64, error))
    {
        std::cout << error << std::endl;
        return 1;
    }

    std::cout << "\n[CIFRA OAEP] Texto cifrado (Base64):\n" << cipherBase64 << std::endl;

    std::vector<unsigned char> message2;
    if (!rsaOaepDecrypt(cipherBytes, receiver, message2, error))
    {
        std::cout << error << std::endl;
        return 1;
    }

    std::cout << "\n[OAEP] Mensagem recuperada apos decifracao:" << std::endl;
    for (auto c : message2)
    {
        std::cout << c;
    }
    std::cout << std::endl;

    std::string signatureBase64;
    if (!signMessageSha3(message1, sender, signatureBase64, error))
    {
        std::cout << error << std::endl;
        return 1;
    }

    std::ofstream outSig("assinatura.sig");
    if (!outSig)
    {
        std::cout << "Erro ao criar o arquivo de assinatura." << std::endl;
        return 1;
    }
    outSig << signatureBase64;
    outSig.close();
    std::cout << "\n[OK] Assinatura salva no arquivo 'assinatura.sig' (Base64)." << std::endl;

    std::cout << "\n[...] Iniciando verificacao. Lendo 'assinatura.sig'..." << std::endl;
    std::ifstream inSig("assinatura.sig");
    if (!inSig)
    {
        std::cout << "Erro ao ler o arquivo de assinatura." << std::endl;
        return 1;
    }

    std::string assinaturaLidaBase64((std::istreambuf_iterator<char>(inSig)), std::istreambuf_iterator<char>());
    inSig.close();

    if (!verifyMessageSha3(message1, sender, assinaturaLidaBase64, error))
    {
        std::cout << error << std::endl;
        return 1;
    }

    std::cout << "[SUCESSO] Assinatura verificada com sucesso." << std::endl;
    return 0;
}
