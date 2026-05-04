#include "signature.h"

#include "keygen.h"
#include "base64.h"
#include "sha3.h"

bool signMessageSha3(const std::vector<unsigned char> &message,
                     const RSAKeyPair &sender,
                     std::string &signatureBase64,
                     std::string &error)
{
    const auto hashArray = sha3_256(message);
    std::vector<unsigned char> hashBytes(hashArray.begin(), hashArray.end());
    InfInt hashInt = bytesToBigInt(hashBytes);

    if (hashInt >= sender.n)
    {
        error = "Erro: hash >= modulo RSA do emissor.";
        return false;
    }

    InfInt signature = binpower(hashInt, sender.d, sender.n);
    const size_t kSignature = modulusByteLength(sender.n);
    std::vector<unsigned char> signatureBytes = bigIntToBytes(signature, kSignature);
    signatureBase64 = base64_encode(signatureBytes);
    return true;
}

bool verifyMessageSha3(const std::vector<unsigned char> &message,
                       const RSAKeyPair &sender,
                       const std::string &signatureBase64,
                       std::string &error)
{
    std::vector<unsigned char> signatureBytes = base64_decode(signatureBase64);
    InfInt signatureInt = bytesToBigInt(signatureBytes);
    InfInt hashRecuperado = binpower(signatureInt, sender.e, sender.n);

    const auto hashVerificacaoArray = sha3_256(message);
    std::vector<unsigned char> hashVerificacaoBytes(hashVerificacaoArray.begin(), hashVerificacaoArray.end());
    InfInt hashCalculado = bytesToBigInt(hashVerificacaoBytes);

    if (hashRecuperado != hashCalculado)
    {
        error = "[ERRO] Assinatura invalida: hash recuperado difere do hash calculado.";
        return false;
    }

    return true;
}
