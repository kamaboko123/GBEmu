#include "color.hpp"

GBPalette::GBPalette(){
    c[0].r = 0xff;
    c[0].g = 0xff;
    c[0].b = 0xff;

    c[1].r = 0xca;
    c[1].g = 0xca;
    c[1].b = 0xca;

    c[2].r = 0x71;
    c[2].g = 0x71;
    c[2].b = 0x71;

    c[3].r = 0x00;
    c[3].g = 0x00;
    c[3].b = 0x00;
}

GBColor GBPalette::operator[](int i){
    return c[i];
}