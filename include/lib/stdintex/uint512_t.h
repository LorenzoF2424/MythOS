#ifndef UINT512_T_H
#define UINT512_T_H

#include "uint256_t.h"

struct uint512_t {
    uint256_t high;
    uint256_t low;

    // --- CONSTRUCTORS ---

    uint512_t() : high(), low() {}
    uint512_t(uint64_t l) : high(), low(l) {}
    uint512_t(uint128_t l) : high(), low(l) {}
    uint512_t(uint256_t l) : high(), low(l) {}
    uint512_t(uint256_t h, uint256_t l) : high(h), low(l) {}

    uint512_t(const char* str) : high(), low() {
        if (!str) return;
        while (*str >= '0' && *str <= '9') {
            *this = (*this << 3) + (*this << 1) + uint512_t((uint64_t)(*str - '0'));
            str++;
        }
    }

    // --- CORE MATH HELPERS ---

    int get_msb() const {
        if (high != uint256_t()) return 256 + high.get_msb();
        if (low  != uint256_t()) return low.get_msb();
        return -1;
    }

    // FIX: Passato per const reference per evitare Stack Smashing
    void div_mod(const uint512_t& den, uint512_t& q, uint512_t& r) const {
        q = uint512_t();
        r = uint512_t();

        if (den == uint512_t()) return;

        int msb_num = get_msb();
        int msb_den = den.get_msb();

        if (msb_num < 0)          return;
        if (msb_num < msb_den)  { r = *this; return; }

        int shift = msb_num - msb_den;
        uint512_t shifted_den = den << shift;
        uint512_t rem = *this;

        for (int i = shift; i >= 0; i--) {
            if (rem >= shifted_den) {
                rem = rem - shifted_den;
                if (i < 256) q.low  = q.low  | (uint256_t((uint64_t)1) << i);
                else         q.high = q.high | (uint256_t((uint64_t)1) << (i - 256));
            }
            shifted_den = shifted_den >> 1;
        }
        r = rem;
    }

    // NEW: Fast path per la printf
    uint64_t div_mod_64(uint64_t divisor, uint512_t& quotient) const {
        if (divisor == 0) { quotient = uint512_t(); return 0; } 
        
        uint512_t q;
        // FIX: Inizializzato con il costruttore di default per evitare ambiguità
        uint128_t remainder = uint128_t();
        
        uint64_t chunks[8] = {
            low.low.parts.low,
            low.low.parts.high,
            low.high.parts.low,
            low.high.parts.high,
            high.low.parts.low,
            high.low.parts.high,
            high.high.parts.low,
            high.high.parts.high
        };

        for (int i = 7; i >= 0; i--) {
            remainder = (remainder << 64) | uint128_t(chunks[i]);
            uint128_t temp_q;
            uint64_t rem = remainder.div_mod_64(divisor, temp_q);
            
            q = (q << 64) | uint512_t(temp_q.parts.low);
            remainder = rem;
        }
        quotient = q;
        return remainder.parts.low;
    }

    // --- ASSIGNMENT OPERATORS ---

    uint512_t& operator=(uint64_t o)  { high = uint256_t(); low = o; return *this; }
    uint512_t& operator=(uint128_t o) { high = uint256_t(); low = o; return *this; }
    uint512_t& operator=(uint256_t o) { high = uint256_t(); low = o; return *this; }

    // --- ARITHMETIC OPERATORS ---

    uint512_t operator+(const uint512_t& other) const {
        uint512_t res;
        res.low = low + other.low;
        uint256_t carry = (res.low < low) ? uint256_t((uint64_t)1) : uint256_t();
        res.high = high + other.high + carry;
        return res;
    }

    uint512_t operator-(const uint512_t& other) const {
        uint512_t res;
        res.low = low - other.low;
        uint256_t borrow = (low < other.low) ? uint256_t((uint64_t)1) : uint256_t();
        res.high = high - other.high - borrow;
        return res;
    }

    uint512_t operator*(const uint512_t& other) const {
        uint512_t res;
        uint512_t temp = *this;
        uint512_t multiplier = other;
        uint256_t one((uint64_t)1);

        for (int i = 0; i < 512; i++) {
            if (multiplier == uint512_t()) break;
            if ((multiplier.low & one) == one) res = res + temp;
            temp = temp << 1;
            multiplier = multiplier >> 1;
        }
        return res;
    }

    uint512_t operator/(const uint512_t& other) const { uint512_t q, r; div_mod(other, q, r); return q; }
    uint512_t operator%(const uint512_t& other) const { uint512_t q, r; div_mod(other, q, r); return r; }

    // --- BITWISE OPERATORS ---

    uint512_t operator&(const uint512_t& other) const { return {high & other.high, low & other.low}; }
    uint512_t operator|(const uint512_t& other) const { return {high | other.high, low | other.low}; }
    uint512_t operator^(const uint512_t& other) const { return {high ^ other.high, low ^ other.low}; }

    uint512_t operator<<(int shift) const {
        if (shift <= 0)   return *this;
        if (shift >= 512) return uint512_t();
        if (shift < 256)  return {(high << shift) | (low >> (256 - shift)), low << shift};
        return {low << (shift - 256), uint256_t()};
    }

    uint512_t operator>>(int shift) const {
        if (shift <= 0)   return *this;
        if (shift >= 512) return uint512_t();
        if (shift < 256)  return {high >> shift, (low >> shift) | (high << (256 - shift))};
        return {uint256_t(), high >> (shift - 256)};
    }

    // --- COMPARISON OPERATORS ---

    bool operator==(const uint512_t& other) const { return (high == other.high) && (low == other.low); }
    bool operator!=(const uint512_t& other) const { return !(*this == other); }

    bool operator>(const uint512_t& other) const {
        if (high != other.high) return high > other.high;
        return low > other.low;
    }

    bool operator<(const uint512_t& other) const {
        if (high != other.high) return high < other.high;
        return low < other.low;
    }

    bool operator>=(const uint512_t& other) const { return !(*this < other); }
    bool operator<=(const uint512_t& other) const { return !(*this > other); }
};

inline uint512_t operator"" _u512(const char* str) { return uint512_t(str); }

#endif