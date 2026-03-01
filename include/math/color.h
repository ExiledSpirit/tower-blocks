#pragma once

namespace math {

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    // Optional: Add a helper for common colors
    static Color White() { return { 255, 255, 255, 255 }; }
    static Color Black() { return { 0, 0, 0, 255 }; }
    static Color Gray()  { return { 150, 150, 150, 255 }; }
};

} // namespace math