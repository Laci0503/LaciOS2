#include <video.h>

void draw_rectangle(uint32 startx, uint32 starty, uint32 w, uint32 h, rgb color){
    if(startx+w<=width /*&& starty>h*/){
        for(uint32 i=starty*width + startx; i<starty*width+startx+w;i++)framebuffer[i]=color;
        for(uint32 i=(starty-h+1)*width + startx; i<starty*width+startx;i+=width)framebuffer[i]=color;
        for(uint32 i=(starty-h+1)*width + startx+w-1; i<starty*width+startx+w;i+=width)framebuffer[i]=color;
        for(uint32 i=(starty-h+1)*width + startx; i<(starty-h+1)*width+startx+w;i++)framebuffer[i]=color;
    }
}
void draw_rectangle_filled(uint32 startx, uint32 starty, uint32 w, uint32 h, rgb color){
    if(startx+w<width && (int32)starty-(int32)h>0)
        for(uint32 i=starty*width;i>(starty-h+1)*width;i-=width)
            for(uint32 j=startx;i<startx+w;i++)
                framebuffer[i+j]=color;
}
rgb torgb(uint8 r, uint8 g, uint8 b){
    rgb res;
    res.resv=0;
    res.r=r;
    res.g=g;
    res.b=b;
    return res;
}
void draw_line(uint32 x1, uint32 y1, uint32 x2, uint32 y2, rgb color){
    if(x1<width && x2 < width && y1 < height && y2 < height){
        if(x1>x2){
            uint32 c=x1;
            x1=x2;
            x2=c;

            c=y1;
            y1=y2;
            y2=c;
        }

        double m=((double)y2-(double)y1)/((double)x2-(double)x1);

        if(m<=(double)1){
            for(uint32 i=0;i<x2-x1+1;i++){
                //put_pixel(i+x1,y1+i*m,color);
                framebuffer[i+x1+(y1+(uint32)(i*m))*width]=color;
            }
        }else{
            for(uint32 i=0;i<y2-y1+1;i++){
                //put_pixel((double)x1+(double)i/m,y1+i,color);
                framebuffer[x1+(uint32)((double)i/m)+(y1+i)*width]=color;
            }
        }
    }
}
void drawTriangle(uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint16 x3, uint16 y3, rgb color){
    draw_line(x1,y2,x2,y2,color);
    draw_line(x1,y1,x3,y3,color);
    draw_line(x2,y2,x3,y3,color);
}
