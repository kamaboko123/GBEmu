#include "color.hpp"

GBPalette::GBPalette(){
    /*
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
    */
    c[0].r = 0xe0;
    c[0].g = 0xf8;
    c[0].b = 0xd0;

    c[1].r = 0x88;
    c[1].g = 0xc0;
    c[1].b = 0x70;

    c[2].r = 0x34;
    c[2].g = 0x68;
    c[2].b = 0x56;

    c[3].r = 0x08;
    c[3].g = 0x18;
    c[3].b = 0x20;
}

GBColor GBPalette::operator[](int i){
    return c[i];
}