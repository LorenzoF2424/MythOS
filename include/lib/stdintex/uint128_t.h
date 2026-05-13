#ifndef UINT128_T_H
#define UINT128_T_H

#include <stdint.h>

union uint128_t {
    // Native compiler extension for extremely fast 128-bit operations
    unsigned __int128 full;

    // Manual structure to access high and low parts safely
    struct {
        uint64_t low;
        uint64_t high;
    } parts;

    // --- CONSTRUCTORS ---

    uint128_t() : full(0) {}
    uint128_t(uint64_t l) : full(l) {}
    uint128_t(uint64_t h, uint64_t l) { parts.low = l; parts.high = h; }

    uint128_t(const char* str) : full(0) {
        if (!str) return; // Early Return: Safety check

        while (*str >= '0' && *str <= '9') {
            // Avoid __multi3 linker errors by using shifts
            full = (full << 3) + (full << 1) + (*str - '0');
            str++;
        }
    }

    // --- CORE MATH HELPERS ---
    
    int get_msb() const {
        if (parts.high != 0) {
            for (int i = 63; i >= 0; i--) {
                if ((parts.high >> i) & 1) return i + 64;
            }
        }
        if (parts.low != 0) {
            for (int i = 63; i >= 0; i--) {
                if ((parts.low >> i) & 1) return i;
            }
        }
        return -1; // Value is 0
    }

    // Shift-and-subtract algorithm optimized with MSB to avoid __udivti3
    void div_mod(const uint128_t& den, uint128_t& q, uint128_t& r) const {
        q = uint128_t(); 
        r = uint128_t();
        
        if (den.full == 0) return; // Early Return: Div by zero

        int msb_num = get_msb();
        int msb_den = den.get_msb();

        if (msb_num < 0)         return; // Early Return: Numerator is 0
        if (msb_num < msb_den) { r = *this; return; }

        int shift = msb_num - msb_den;
        uint128_t shifted_den = den << shift;
        uint128_t rem = *this;

        for (int i = shift; i >= 0; i--) {
            if (rem >= shifted_den) {
                rem = rem - shifted_den;
                if (i < 64) q.parts.low |= (1ULL << i);
                else        q.parts.high |= (1ULL << (i - 64));
            }
            shifted_den = shifted_den >> 1;
        }
        r = rem;
    }

    // Fast path for printf generic template compatibility
    uint64_t div_mod_64(uint64_t divisor, uint128_t& q) const {
        if (divisor == 0) { q = uint128_t(); return 0; } // Early Return
        
        uint128_t r_full = uint128_t();
        // Since we have MSB optimization, software division is extremely fast for 128-bit
        div_mod(uint128_t(divisor), q, r_full);
        return r_full.parts.low;
    }

    // --- ASSIGNMENT OPERATORS ---
    
    uint128_t& operator=(uint64_t other) { full = other; return *this; }
    uint128_t& operator=(const uint128_t& other) { full = other.full; return *this; }

    // --- ARITHMETIC OPERATORS ---

    uint128_t operator+(const uint128_t& other) const {
        uint128_t res;
        res.full = full + other.full; // Native optimization
        return res;
    }

    uint128_t operator-(const uint128_t& other) const {
        uint128_t res;
        res.full = full - other.full; // Native optimization
        return res;
    }

    // Safe Multiplication without __multi3
    uint128_t operator*(const uint128_t& other) const {
        uint128_t res;
        // Hardware natively supports 64x64 -> 128 bit multiplication
        res.full = (unsigned __int128)parts.low * other.parts.low;
        
        // Add the high cross-products (mathematically shifted by 64 bits)
        res.parts.high += (parts.high * other.parts.low) + (parts.low * other.parts.high);
        return res;
    }

    uint128_t operator/(const uint128_t& other) const { uint128_t q, r; div_mod(other, q, r); return q; }
    uint128_t operator%(const uint128_t& other) const { uint128_t q, r; div_mod(other, q, r); return r; }

    // --- BITWISE OPERATORS ---
    // Native bitwise operations on full 128-bit register are much faster than parts!
    
    uint128_t operator&(const uint128_t& other) const { uint128_t r; r.full = full & other.full; return r; }
    uint128_t operator|(const uint128_t& other) const { uint128_t r; r.full = full | other.full; return r; }
    uint128_t operator^(const uint128_t& other) const { uint128_t r; r.full = full ^ other.full; return r; }
    
    uint128_t operator<<(int shift) const { 
        uint128_t r; 
        r.full = full << shift; 
        return r; 
    }
    
    uint128_t operator>>(int shift) const { 
        uint128_t r; 
        r.full = full >> shift; 
        return r; 
    }

    // --- COMPARISON OPERATORS ---
    
    bool operator==(const uint128_t& other) const { return full == other.full; }
    bool operator!=(const uint128_t& other) const { return full != other.full; }
    bool operator>(const uint128_t& other) const  { return full > other.full; }
    bool operator<(const uint128_t& other) const  { return full < other.full; }
    bool operator>=(const uint128_t& other) const { return full >= other.full; }
    bool operator<=(const uint128_t& other) const { return full <= other.full; }
};

// User-defined literal to allow: uint128_t x = 1234567890_u128;
inline uint128_t operator"" _u128(const char* str) {
    return uint128_t(str);
}

#endif