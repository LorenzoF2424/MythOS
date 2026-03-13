

#ifndef POINT_H
#define POINT_H

#include <stdint.h>

struct point {
    int32_t x;
    int32_t y;

    point() : x(0), y(0) {}

    point(int32_t p_x, int32_t p_y) : x(p_x), y(p_y) {}

    bool operator==(const point& other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const point& other) const {
        return !(*this == other);
    }

    point operator+(const point& other) const {
        return point(x + other.x, y + other.y);
    }
};





#endif // POINT_H