#include "keygen.h"

#include <random>
#include <tuple>
#include <chrono>
#include <thread>

namespace
{
    const int smallPrimes[] = {
        3, 5, 7, 11, 13, 17, 19, 23, 29, 31,
        37, 41, 43, 47, 53, 59, 61, 67, 71, 73,
        79, 83, 89, 97, 101, 103, 107, 109, 113, 127,
        131, 137, 139, 149, 151, 157, 163, 167, 173, 179,
        181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
        239, 241, 251, 257, 263, 269, 271, 277, 281, 283,
        293, 307, 311, 313, 317, 331, 337, 347, 349, 353,
        359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
        421, 431, 433, 439, 443, 449, 457, 461, 463, 467,
        479, 487, 491, 499, 503, 509, 521, 523, 541, 547,
        557, 563, 569, 571, 577, 587, 593, 599, 601, 607,
        613, 617, 619, 631, 641, 643, 647, 653, 659, 661,
        673, 677, 683, 691, 701, 709, 719, 727, 733, 739,
        743, 751, 757, 761, 769, 773, 787, 797, 809, 811,
        821, 823, 827, 829, 839, 853, 857, 859, 863, 877,
        881, 883, 887, 907, 911, 919, 929, 937, 941, 947,
        953, 967, 971, 977, 983, 991, 997};
}

InfInt binpower(InfInt base, InfInt e, InfInt mod)
{
    InfInt result = 1;
    base %= mod;
    while (e > 0)
    {
        if (e % 2 == 1)
            result = (result * base) % mod;
        base = (base * base) % mod;
        e /= 2;
    }
    return result;
}

bool millerRabin(InfInt n, int iterations)
{
    if (n < 2)
        return false;
    if (n == 2 || n == 3)
        return true;
    if (n % 2 == 0)
        return false;
    for (int p : smallPrimes)
    {
        if (n == p)
            return true;
        if (n % p == 0)
            return false;
    }

    // Achar d tal que n-1 = 2^s * d
    InfInt d = n - 1;
    int s = 0;
    while (d % 2 == 0)
    {
        d /= 2;
        s++;
    }

    for (int i = 0; i < iterations; i++)
    {

        InfInt a = 2 + (i * 3);
        InfInt x = binpower(a, d, n);

        if (x == 1 || x == n - 1)
            continue;

        bool composite = true;
        for (int r = 1; r < s; r++)
        {
            x = (x * x) % n;
            if (x == n - 1)
            {
                composite = false;
                break;
            }
        }
        if (composite)
            return false;
    }
    return true;
}

InfInt generateRandomCandidate(int bits)
{
    std::random_device rd;
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
    const unsigned seed = static_cast<unsigned>(rd() ^ now ^ tid);
    thread_local std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, 1);

    InfInt res = 1; // bit mais significativo eh 1

    for (int i = 0; i < bits - 1; i++)
    {
        res *= 2; // bit shift
        if (i < bits - 2)
        { // nao altera o ultimo bit aqui
            res += dist(gen);
        }
    }

    res += 1; // para ser impar

    return res;
}

InfInt generatePrime(int bits, int iterations)
{
    InfInt candidate = generateRandomCandidate(bits);
    while (!millerRabin(candidate, iterations))
    {
        candidate += 2;
    }
    return candidate;
}

// Algoritmo de Euclides Estendido
InfInt gcd(InfInt a, InfInt b, InfInt &x, InfInt &y)
{
    x = 1;
    y = 0;
    InfInt x1 = 0, y1 = 1, a1 = a, b1 = b;
    while (b1 != 0)
    {
        InfInt q = a1 / b1;
        std::tie(x, x1) = std::make_tuple(x1, x - q * x1);
        std::tie(y, y1) = std::make_tuple(y1, y - q * y1);
        std::tie(a1, b1) = std::make_tuple(b1, a1 - q * b1);
    }
    return a1;
}

InfInt findModInv(InfInt a, InfInt m)
{
    InfInt x, y;
    InfInt gcm = gcd(a, m, x, y);
    if (gcm != 1)
    {
        return 0;
    }
    else
    {
        x = (x % m + m) % m;
        return x;
    }
}

InfInt bytesToBigInt(const std::vector<unsigned char> &data)
{
    InfInt result = 0;
    for (unsigned char b : data)
    {
        result *= 256;
        result += b;
    }
    return result;
}

std::vector<unsigned char> bigIntToBytes(InfInt num, size_t k)
{
    std::vector<unsigned char> data(k);

    for (int i = k - 1; i >= 0; i--)
    {
        data[i] = (num % 256).toInt();
        num /= 256;
    }

    return data;
}
