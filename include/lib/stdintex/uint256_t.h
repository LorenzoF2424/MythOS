#ifndef UINT256_T_H
#define UINT256_T_H

#include "uint128_t.h"

struct uint256_t {
    uint128_t high;
    uint128_t low;

    // --- CONSTRUCTORS ---

    uint256_t() : high(), low() {}
    uint256_t(uint64_t l) : high(), low(l) {}
    uint256_t(uint128_t l) : high(), low(l) {}
    uint256_t(uint128_t h, uint128_t l) : high(h), low(l) {}

    uint256_t(const char* str) : high(), low() {
        if (!str) return;
        while (*str >= '0' && *str <= '9') {
            *this = (*this << 3) + (*this << 1) + uint256_t((uint64_t)(*str - '0'));
            str++;
        }
    }

    // --- CORE MATH HELPERS ---

    int get_msb() const {
        if (high != uint128_t()) return 128 + high.get_msb();
        if (low  != uint128_t()) return low.get_msb();
        return -1;
    }

    // FIX: Passato per const reference per evitare Stack Smashing
    void div_mod(const uint256_t& den, uint256_t& q, uint256_t& r) const {
        q = uint256_t();
        r = uint256_t();

        if (den == uint256_t()) return;

        int msb_num = get_msb();
        int msb_den = den.get_msb();

        if (msb_num < 0)          return;
        if (msb_num < msb_den)  { r = *this; return; }

        int shift = msb_num - msb_den;
        uint256_t shifted_den = den << shift;
        uint256_t rem = *this;

        for (int i = shift; i >= 0; i--) {
            if (rem >= shifted_den) {
                rem = rem - shifted_den;
                if (i < 128) q.low  = q.low  | (uint128_t((uint64_t)1) << i);
                else         q.high = q.high | (uint128_t((uint64_t)1) << (i - 128));
            }
            shifted_den = shifted_den >> 1;
        }
        r = rem;
    }

    // NEW: Fast path per la printf
    uint64_t div_mod_64(uint64_t divisor, uint256_t& quotient) const {
        if (divisor == 0) { quotient = uint256_t(); return 0; } 
        
        uint256_t q;
        // FIX: Inizializzato con il costruttore di default per evitare ambiguità
        uint128_t remainder = uint128_t();
        
        uint64_t chunks[4] = {
            low.parts.low,
            low.parts.high,
            high.parts.low,
            high.parts.high
        };

        for (int i = 3; i >= 0; i--) {
            remainder = (remainder << 64) | uint128_t(chunks[i]);
            uint128_t temp_q;
            uint64_t rem = remainder.div_mod_64(divisor, temp_q);
            
            q = (q << 64) | uint256_t(temp_q.parts.low);
            remainder = rem;
        }
        quotient = q;
        return remainder.parts.low;
    }

    // --- ASSIGNMENT OPERATORS ---

    uint256_t& operator=(uint64_t o)  { high = uint128_t(); low = o; return *this; }
    uint256_t& operator=(uint128_t o) { high = uint128_t(); low = o; return *this; }

    // --- ARITHMETIC OPERATORS ---

    uint256_t operator+(const uint256_t& other) const {
        uint256_t res;
        res.low = low + other.low;
        uint128_t carry = (res.low < low) ? uint128_t((uint64_t)1) : uint128_t();
        res.high = high + other.high + carry;
        return res;
    }

    uint256_t operator-(const uint256_t& other) const {
        uint256_t res;
        res.low = low - other.low;
        uint128_t borrow = (low < other.low) ? uint128_t((uint64_t)1) : uint128_t();
        res.high = high - other.high - borrow;
        return res;
    }

    uint256_t operator*(const uint256_t& other) const {
        uint256_t res;
        uint256_t temp = *this;
        uint256_t multiplier = other;
        uint128_t one((uint64_t)1);

        for (int i = 0; i < 256; i++) {
            if (multiplier == uint256_t()) break;
            if ((multiplier.low & one) == one) res = res + temp;
            temp = temp << 1;
            multiplier = multiplier >> 1;
        }
        return res;
    }

    uint256_t operator/(const uint256_t& other) const { uint256_t q, r; div_mod(other, q, r); return q; }
    uint256_t operator%(const uint256_t& other) const { uint256_t q, r; div_mod(other, q, r); return r; }

    // --- BITWISE OPERATORS ---

    uint256_t operator&(const uint256_t& other) const { return {high & other.high, low & other.low}; }
    uint256_t operator|(const uint256_t& other) const { return {high | other.high, low | other.low}; }
    uint256_t operator^(const uint256_t& other) const { return {high ^ other.high, low ^ other.low}; }

    uint256_t operator<<(int shift) const {
        if (shift <= 0)   return *this;
        if (shift >= 256) return uint256_t();
        if (shift < 128)  return {(high << shift) | (low >> (128 - shift)), low << shift};
        return {low << (shift - 128), uint128_t()};
    }

    uint256_t operator>>(int shift) const {
        if (shift <= 0)   return *this;
        if (shift >= 256) return uint256_t();
        if (shift < 128)  return {high >> shift, (low >> shift) | (high << (128 - shift))};
        return {uint128_t(), high >> (shift - 128)};
    }

    // --- COMPARISON OPERATORS ---

    bool operator==(const uint256_t& other) const { return (high == other.high) && (low == other.low); }
    bool operator!=(const uint256_t& other) const { return !(*this == other); }

    bool operator>(const uint256_t& other) const {
        if (high != other.high) return high > other.high;
        return low > other.low;
    }

    bool operator<(const uint256_t& other) const {
        if (high != other.high) return high < other.high;
        return low < other.low;
    }

    bool operator>=(const uint256_t& other) const { return !(*this < other); }
    bool operator<=(const uint256_t& other) const { return !(*this > other); }
};

inline uint256_t operator"" _u256(const char* str) { return uint256_t(str); }

#endif