#include "oaep.h"

#include <array>
#include <cstdint>
#include <random>
#include <algorithm>

namespace {

constexpr size_t kSha256DigestSize = 32;

inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

std::array<unsigned char, kSha256DigestSize> sha256(const std::vector<unsigned char>& input) {
    static const uint32_t sha256RoundConstants[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    uint32_t hashState[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };

    std::vector<unsigned char> msg = input;
    const uint64_t bitLen = static_cast<uint64_t>(msg.size()) * 8ULL;
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) msg.push_back(0x00);
    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<unsigned char>((bitLen >> (i * 8)) & 0xFF));
    }

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t messageSchedule[64] = {0};
        for (int i = 0; i < 16; ++i) {
            const size_t j = chunk + (i * 4);
            messageSchedule[i] = (static_cast<uint32_t>(msg[j]) << 24) |
                                 (static_cast<uint32_t>(msg[j + 1]) << 16) |
                                 (static_cast<uint32_t>(msg[j + 2]) << 8) |
                                 static_cast<uint32_t>(msg[j + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            const uint32_t sigma0 = rotr(messageSchedule[i - 15], 7) ^ rotr(messageSchedule[i - 15], 18) ^ (messageSchedule[i - 15] >> 3);
            const uint32_t sigma1 = rotr(messageSchedule[i - 2], 17) ^ rotr(messageSchedule[i - 2], 19) ^ (messageSchedule[i - 2] >> 10);
            messageSchedule[i] = messageSchedule[i - 16] + sigma0 + messageSchedule[i - 7] + sigma1;
        }

        uint32_t stateA = hashState[0], stateB = hashState[1], stateC = hashState[2], stateD = hashState[3];
        uint32_t stateE = hashState[4], stateF = hashState[5], stateG = hashState[6], stateH = hashState[7];

        for (int i = 0; i < 64; ++i) {
            const uint32_t bigSigma1 = rotr(stateE, 6) ^ rotr(stateE, 11) ^ rotr(stateE, 25);
            const uint32_t choose = (stateE & stateF) ^ ((~stateE) & stateG);
            const uint32_t temp1 = stateH + bigSigma1 + choose + sha256RoundConstants[i] + messageSchedule[i];
            const uint32_t bigSigma0 = rotr(stateA, 2) ^ rotr(stateA, 13) ^ rotr(stateA, 22);
            const uint32_t majority = (stateA & stateB) ^ (stateA & stateC) ^ (stateB & stateC);
            const uint32_t temp2 = bigSigma0 + majority;

            stateH = stateG;
            stateG = stateF;
            stateF = stateE;
            stateE = stateD + temp1;
            stateD = stateC;
            stateC = stateB;
            stateB = stateA;
            stateA = temp1 + temp2;
        }

        hashState[0] += stateA; hashState[1] += stateB; hashState[2] += stateC; hashState[3] += stateD;
        hashState[4] += stateE; hashState[5] += stateF; hashState[6] += stateG; hashState[7] += stateH;
    }

    std::array<unsigned char, kSha256DigestSize> out{};
    for (int i = 0; i < 8; ++i) {
        out[i * 4] = static_cast<unsigned char>((hashState[i] >> 24) & 0xFF);
        out[i * 4 + 1] = static_cast<unsigned char>((hashState[i] >> 16) & 0xFF);
        out[i * 4 + 2] = static_cast<unsigned char>((hashState[i] >> 8) & 0xFF);
        out[i * 4 + 3] = static_cast<unsigned char>(hashState[i] & 0xFF);
    }
    return out;
}

std::vector<unsigned char> mgf1(const std::vector<unsigned char>& seed, size_t maskLen) {
    std::vector<unsigned char> mask;
    mask.reserve(maskLen);

    uint32_t counter = 0;
    while (mask.size() < maskLen) {
        std::vector<unsigned char> input(seed);
        input.push_back(static_cast<unsigned char>((counter >> 24) & 0xFF));
        input.push_back(static_cast<unsigned char>((counter >> 16) & 0xFF));
        input.push_back(static_cast<unsigned char>((counter >> 8) & 0xFF));
        input.push_back(static_cast<unsigned char>(counter & 0xFF));

        auto h = sha256(input);
        for (size_t i = 0; i < h.size() && mask.size() < maskLen; ++i) {
            mask.push_back(h[i]);
        }
        ++counter;
    }

    return mask;
}

std::vector<unsigned char> strToBytes(const std::string& s) {
    return std::vector<unsigned char>(s.begin(), s.end());
}

}

size_t oaepMaxMessageLen(size_t k, size_t hLen) {
    if (k < (2 * hLen + 2)) return 0;
    return k - (2 * hLen) - 2;
}

bool oaepEncode(const std::vector<unsigned char>& message,
                size_t k,
                std::vector<unsigned char>& encoded,
                const std::string& label) {
    const size_t hLen = kSha256DigestSize;
    if (message.size() > oaepMaxMessageLen(k, hLen)) return false;

    const std::vector<unsigned char> lBytes = strToBytes(label);
    const auto lHashArr = sha256(lBytes);

    const size_t psLen = k - message.size() - (2 * hLen) - 2;
    std::vector<unsigned char> db;
    db.reserve(k - hLen - 1);
    db.insert(db.end(), lHashArr.begin(), lHashArr.end());
    db.insert(db.end(), psLen, 0x00);
    db.push_back(0x01);
    db.insert(db.end(), message.begin(), message.end());

    std::vector<unsigned char> seed(hLen);
    std::random_device rd;
    for (size_t i = 0; i < hLen; ++i) {
        seed[i] = static_cast<unsigned char>(rd() & 0xFF);
    }

    const std::vector<unsigned char> dbMask = mgf1(seed, k - hLen - 1);
    std::vector<unsigned char> maskedDB(db.size());
    for (size_t i = 0; i < db.size(); ++i) {
        maskedDB[i] = db[i] ^ dbMask[i];
    }

    const std::vector<unsigned char> seedMask = mgf1(maskedDB, hLen);
    std::vector<unsigned char> maskedSeed(hLen);
    for (size_t i = 0; i < hLen; ++i) {
        maskedSeed[i] = seed[i] ^ seedMask[i];
    }

    encoded.clear();
    encoded.reserve(k);
    encoded.push_back(0x00);
    encoded.insert(encoded.end(), maskedSeed.begin(), maskedSeed.end());
    encoded.insert(encoded.end(), maskedDB.begin(), maskedDB.end());

    return encoded.size() == k;
}

bool oaepDecode(const std::vector<unsigned char>& encoded,
                std::vector<unsigned char>& message,
                const std::string& label) {
    const size_t hLen = kSha256DigestSize;
    const size_t k = encoded.size();
    if (k < (2 * hLen + 2)) return false;
    if (encoded[0] != 0x00) return false;

    std::vector<unsigned char> maskedSeed(encoded.begin() + 1, encoded.begin() + 1 + hLen);
    std::vector<unsigned char> maskedDB(encoded.begin() + 1 + hLen, encoded.end());

    const std::vector<unsigned char> seedMask = mgf1(maskedDB, hLen);
    std::vector<unsigned char> seed(hLen);
    for (size_t i = 0; i < hLen; ++i) {
        seed[i] = maskedSeed[i] ^ seedMask[i];
    }

    const std::vector<unsigned char> dbMask = mgf1(seed, k - hLen - 1);
    std::vector<unsigned char> db(maskedDB.size());
    for (size_t i = 0; i < db.size(); ++i) {
        db[i] = maskedDB[i] ^ dbMask[i];
    }

    const auto lHashArr = sha256(strToBytes(label));
    if (!std::equal(lHashArr.begin(), lHashArr.end(), db.begin())) return false;

    size_t idx = hLen;
    while (idx < db.size() && db[idx] == 0x00) {
        ++idx;
    }
    if (idx >= db.size() || db[idx] != 0x01) return false;
    ++idx;

    message.assign(db.begin() + idx, db.end());
    return true;
}
