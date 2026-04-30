#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <tuple>
#include <chrono>
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
    if (n % 2 == 0 ||
        n % 3 == 0||
        n % 5 == 0||
        n % 7 == 0||
        n % 11 == 0||
        n % 13 == 0||
        n % 17 == 0||
        n % 19 == 0||
        n % 23 == 0||
        n % 29 == 0||
        n % 31 == 0||
        n % 37 == 0||
        n % 41 == 0||
        n % 53 == 0) return false;

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



InfInt generateRandomCandidate(int bits) {
    //static std::random_device rd;  // Fonte de entropia do hardware
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count(); //seed do relogio do sistema
    static std::mt19937 gen(seed); // Gerador Mersenne Twister
    std::uniform_int_distribution<int> dist(0, 1);


    InfInt res = 1; //bit mais significativo eh 1

    for(int i = 0; i < bits - 1; i++){
        res *= 2; //bit shift
        if(i < bits-2){ //nao altera o ultimo bit aqui
            res += dist(gen);
        }
    }

    res += 1; //para ser impar

    return res;
}

//Algoritmo de Euclides Extendido
InfInt gcd(InfInt a, InfInt b, InfInt& x, InfInt& y){
    x = 1;
    y = 0;
    InfInt x1 = 0, y1 = 1, a1 = a, b1 = b;
    while (b1 != 0)
    {
        InfInt q = a1 / b1;
        std::tie(x,x1) = std::make_tuple(x1, x - q * x1);
        std::tie(y,y1) = std::make_tuple(y1, y - q * y1);
        std::tie(a1,b1) = std::make_tuple(b1, a1 - q * b1);
    }
    
}

InfInt findModInv(InfInt a,InfInt m){
    InfInt x,y;
    InfInt gcm = gcd(a, m, x, y);
    if(gcm != 1){
        return 0;
    }
    else{
        x = (x % m + m) % m;
        return x;
    }
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

    std::cout << " " << std::endl;
    std::cout << "p encontrado:" << std::endl;
    std::cout << p << std::endl;

    std::cout << "Gerando q " << std::flush;
    InfInt q = generateRandomCandidate(bitSize);

    // Loop até encontrar um primo
    while (!millerRabin(q, 3) || (q == p)) {
        q += 2; 
        std::cout << "." << std::flush;
    }

    std::cout << " " << std::endl;
    std::cout << "q encontrado:" << std::endl;
    std::cout << q << std::endl;

    InfInt n = p * q;

    // Totiente de Euler
    InfInt z = (p - 1) * (q - 1);

    return 0;
}