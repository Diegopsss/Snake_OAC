#include "ripes_system.h"
#include <stdlib.h>
#define LED_MATRIX_0_WIDTH	(0x23)
#define LED_MATRIX_0_HEIGHT	(0x19)
volatile unsigned int * led_base = (int*) LED_MATRIX_0_BASE;

typedef struct pixel{
        int x;
        int y;
    } pixel;

typedef struct macropixel{
        pixel pixels[4];
    } macropixel;

void color_pixel(pixel p, int color){
        volatile unsigned int * temp = led_base;
        temp += LED_MATRIX_0_WIDTH * p.y;
        temp += p.x;
        *temp = color;
    }
void color_macropixel(macropixel *p, int color){
        for(int i = 0; i < 4; i++){
            color_pixel(p->pixels[i], color);
        }
    }

macropixel* to_macropixel(pixel p){
        macropixel *P = malloc(sizeof(macropixel));
        P->pixels[0].x = 2 *p.x;
        P->pixels[0].y = 2 *p.y;
        P->pixels[1].x = 2 *p.x + 1;
        P->pixels[1].y = 2 *p.y;
        P->pixels[2].x = 2 *p.x;
        P->pixels[2].y = 2 *p.y + 1;
        P->pixels[3].x = 2 *p.x + 1;
        P->pixels[3].y = 2 *p.y + 1;
        return P;
    }

