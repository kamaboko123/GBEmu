#ifndef GBEMU_COLOR_H
#define GBEMU_COLOR_H

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

typedef struct GBColor{
    GLubyte  r;
    GLubyte  g;
    GLubyte  b;
} GBColor;

class GBPalette{
private:
    GBColor c[4];
public:
    GBPalette();
    GBColor operator[](int i);
};

#endif