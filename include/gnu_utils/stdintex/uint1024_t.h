#ifndef UINT1024_T_H
#define UINT1024_T_H

#include "uint512_t.h"

struct uint1024_t {
    uint512_t high;
    uint512_t low;

    // --- CONSTRUCTORS ---

    uint1024_t() : high(), low() {}
    uint1024_t(uint64_t l) : high(), low(l) {}
    uint1024_t(uint128_t l) : high(), low(l) {}
    uint1024_t(uint256_t l) : high(), low(l) {}
    uint1024_t(uint512_t l) : high(), low(l) {}
    uint1024_t(uint512_t h, uint512_t l) : high(h), low(l) {}

    uint1024_t(const char* str) : high(), low() {
        if (!str) return;
        while (*str >= '0' && *str <= '9') {
            *this = (*this << 3) + (*this << 1) + uint1024_t((uint64_t)(*str - '0'));
            str++;
        }
    }

    // --- CORE MATH HELPERS ---

    int get_msb() const {
        if (high != uint512_t()) return 512 + high.get_msb();
        if (low  != uint512_t()) return low.get_msb();
        return -1;
    }

    // FIX: Passato per const reference per evitare Stack Smashing
    void div_mod(const uint1024_t& den, uint1024_t& q, uint1024_t& r) const {
        q = uint1024_t();
        r = uint1024_t();

        if (den == uint1024_t()) return;

        int msb_num = get_msb();
        int msb_den = den.get_msb();

        if (msb_num < 0)          return;
        if (msb_num < msb_den)  { r = *this; return; }

        int shift = msb_num - msb_den;
        uint1024_t shifted_den = den << shift;
        uint1024_t rem = *this;

        for (int i = shift; i >= 0; i--) {
            if (rem >= shifted_den) {
                rem = rem - shifted_den;
                if (i < 512) q.low  = q.low  | (uint512_t((uint64_t)1) << i);
                else         q.high = q.high | (uint512_t((uint64_t)1) << (i - 512));
            }
            shifted_den = shifted_den >> 1;
        }
        r = rem;
    }

    // NEW: Fast path per la printf
    uint64_t div_mod_64(uint64_t divisor, uint1024_t& quotient) const {
        if (divisor == 0) { quotient = uint1024_t(); return 0; } 
        
        uint1024_t q;
        uint128_t remainder = uint128_t();
        
        uint64_t chunks[16] = {
            low.low.low.parts.low,
            low.low.low.parts.high,
            low.low.high.parts.low,
            low.low.high.parts.high,
            low.high.low.parts.low,
            low.high.low.parts.high,
            low.high.high.parts.low,
            low.high.high.parts.high,
            high.low.low.parts.low,
            high.low.low.parts.high,
            high.low.high.parts.low,
            high.low.high.parts.high,
            high.high.low.parts.low,
            high.high.low.parts.high,
            high.high.high.parts.low,
            high.high.high.parts.high
        };

        for (int i = 15; i >= 0; i--) {
            remainder = (remainder << 64) | uint128_t(chunks[i]);
            uint128_t temp_q;
            uint64_t rem = remainder.div_mod_64(divisor, temp_q);
            
            q = (q << 64) | uint1024_t(temp_q.parts.low);
            remainder = rem;
        }
        quotient = q;
        return remainder.parts.low;
    }

    // --- ASSIGNMENT OPERATORS ---

    uint1024_t& operator=(uint64_t o)  { high = uint512_t(); low = o; return *this; }
    uint1024_t& operator=(uint512_t o) { high = uint512_t(); low = o; return *this; }

    // --- ARITHMETIC OPERATORS ---

    uint1024_t operator+(const uint1024_t& other) const {
        uint1024_t res;
        res.low = low + other.low;
        uint512_t carry = (res.low < low) ? uint512_t((uint64_t)1) : uint512_t();
        res.high = high + other.high + carry;
        return res;
    }

    uint1024_t operator-(const uint1024_t& other) const {
        uint1024_t res;
        res.low = low - other.low;
        uint512_t borrow = (low < other.low) ? uint512_t((uint64_t)1) : uint512_t();
        res.high = high - other.high - borrow;
        return res;
    }

    uint1024_t operator*(const uint1024_t& other) const {
        uint1024_t res;
        uint1024_t temp = *this;
        uint1024_t multiplier = other;
        uint512_t one((uint64_t)1);

        for (int i = 0; i < 1024; i++) {
            if (multiplier == uint1024_t()) break;
            if ((multiplier.low & one) == one) res = res + temp;
            temp = temp << 1;
            multiplier = multiplier >> 1;
        }
        return res;
    }

    uint1024_t operator/(const uint1024_t& other) const { uint1024_t q, r; div_mod(other, q, r); return q; }
    uint1024_t operator%(const uint1024_t& other) const { uint1024_t q, r; div_mod(other, q, r); return r; }

    // --- BITWISE OPERATORS ---

    uint1024_t operator&(const uint1024_t& other) const { return {high & other.high, low & other.low}; }
    uint1024_t operator|(const uint1024_t& other) const { return {high | other.high, low | other.low}; }
    uint1024_t operator^(const uint1024_t& other) const { return {high ^ other.high, low ^ other.low}; }

    uint1024_t operator<<(int shift) const {
        if (shift <= 0)    return *this;
        if (shift >= 1024) return uint1024_t();
        if (shift < 512)   return {(high << shift) | (low >> (512 - shift)), low << shift};
        return {low << (shift - 512), uint512_t()};
    }

    uint1024_t operator>>(int shift) const {
        if (shift <= 0)    return *this;
        if (shift >= 1024) return uint1024_t();
        if (shift < 512)   return {high >> shift, (low >> shift) | (high << (512 - shift))};
        return {uint512_t(), high >> (shift - 512)};
    }

    // --- COMPARISON OPERATORS ---

    bool operator==(const uint1024_t& other) const { return (high == other.high) && (low == other.low); }
    bool operator!=(const uint1024_t& other) const { return !(*this == other); }

    bool operator>(const uint1024_t& other) const {
        if (high != other.high) return high > other.high;
        return low > other.low;
    }

    bool operator<(const uint1024_t& other) const {
        if (high != other.high) return high < other.high;
        return low < other.low;
    }

    bool operator>=(const uint1024_t& other) const { return !(*this < other); }
    bool operator<=(const uint1024_t& other) const { return !(*this > other); }
};

inline uint1024_t operator"" _u1024(const char* str) { return uint1024_t(str); }

#endif