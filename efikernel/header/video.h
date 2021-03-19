#ifndef _VIDEO_H
#define _VIDEO_H
#include <types.h>

typedef struct{
    uint8 b;
    uint8 g;
    uint8 r;
    uint8 resv;
} rgb;

rgb* framebuffer;
uint64 width;
uint64 height;

void draw_rectangle(uint32 startx, uint32 starty, uint32 width, uint32 height, rgb color); // Left bottom corner, size
void draw_rectangle_filled(uint32 startx, uint32 starty, uint32 w, uint32 h, rgb color); // Left bottom corner, size
rgb torgb(uint8 r, uint8 g, uint8 b);
void draw_line(uint32 x1, uint32 y1, uint32 x2, uint32 y2, rgb color);

#endif