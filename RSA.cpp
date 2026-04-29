#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include "lib/InfInt.h"



InfInt binpower(InfInt base, InfInt e, InfInt mod) {
    InfInt result = 1;
    base %= mod;
    while (e > 0) {
        if (e % 2 == 1)
            result = (result * base) % mod;
        base = (base * base) % mod;
        e /= 2;
    }
    return result;
}




bool millerRabin(InfInt n, int iterations) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    // Achar d tal que n-1 = 2^s * d
    InfInt d = n - 1;
    int s = 0;
    while (d % 2 == 0) {
        d /= 2;
        s++;
    }

    for (int i = 0; i < iterations; i++) {
        
        InfInt a = 2 + (i * 3); 
        InfInt x = binpower(a, d, n);

        if (x == 1 || x == n - 1) continue;

        bool composite = true;
        for (int r = 1; r < s; r++) {
            x = (x * x) % n;
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}


// Função para gerar um número aleatório de 1, 0 ou bits específicos
InfInt generateRandomCandidate(int bits) {
    std::random_device rd;  // Fonte de entropia do hardware
    std::mt19937 gen(rd()); // Gerador Mersenne Twister
    std::uniform_int_distribution<int> dist(0, 1);

    std::string binaryString = "1"; // Força o MSB a ser 1

    for (int i = 0; i < bits - 2; ++i) {
        binaryString += std::to_string(dist(gen));
    }

    binaryString += "1"; // Força o LSB a ser 1 (ímpar)

    // A InfInt não converte binário direto por padrão
    InfInt res = 0;
    InfInt base = 1;
    for (int i = binaryString.length() - 1; i >= 0; i--) {
        if (binaryString[i] == '1') {
            res += base;
        }
        base *= 2;
    }
    return res;
}


int main(){

    int bitSize = 1024;
    std::cout << "Gerando p " << std::flush;

    InfInt p = generateRandomCandidate(bitSize);

    // Loop até encontrar um primo
    while (!millerRabin(p, 3)) {
        p += 2; // Tenta o próximo número ímpar
        std::cout << "." << std::flush;
    }

    std::cout << "p encontrado:" << std::endl;
    std::cout << p << std::endl;

    std::cout << "Gerando q " << std::flush;
    InfInt q = generateRandomCandidate(bitSize);

    // Loop até encontrar um primo
    while (!millerRabin(q, 3) || p == q) {
        q += 2; 
        std::cout << "." << std::flush;
    }

    std::cout << "q encontrado:" << std::endl;
    std::cout << q << std::endl;

    InfInt n = p * q;
    InfInt z = (p - 1) * (q - 1);

    return 0;
}