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

typedef struct segment{
	pixel location;
	struct segment* next;
} segment;

typedef struct snake{
	segment* head;
} snake;


// para direcciones usar x y y, aka: izquierda = -1 0, derecha = 1, 0, arriba = 0, 1 ,etc.
int check_apple(snake *s, pixel *a, int x, int y){
	if(a->x == s->head->location.x + x && a->y == s->head->location.y + y){
		return 1;
	}
	return 0;
}

int check_colision(snake *s){
	if(s->head->location.x < 0 || s->head->location.x >= LED_MATRIX_0_WIDTH || s->head->location.y < 0 || s->head->location.y >= LED_MATRIX_0_HEIGHT){
		return 1;
	}
	segment *n = s->head;
	while(n->next != NULL){
		n = n->next;
		if(s->head->location.x == n->location.x && s->head->location.y == n->location.y){
			return 1;
		}
	}

	return 0;
}

void move_snake(snake *s, int x, int y){
	segment *n = s->head;
		pixel current = n->location;
		n->location.x += x;
		n->location.y += y;
	while(n->next != NULL){
		n = n->next;
		pixel temp = n->location;
		n->location = current;
		current = temp;
	}
	color_macropixel(to_macropixel(n->location), 0);
}

pixel *new_apple(snake *s){
	int x;
	int y;
	while(1){
		x = rand() % LED_MATRIX_0_WIDTH;	
		y = rand() % LED_MATRIX_0_HEIGHT;		
		segment *n = s->head;
		int f = 1;
		while(n != NULL){
			if(n->location.x == x && n->location.y == y){
				f = 0;
				break;
			}
			n = n-> next;
		}
		if(f){
			break;
		}
	}
	pixel *apple = malloc(sizeof(pixel));
	apple->x = x;
	apple->y = y;
	return apple;
}

pixel *move_apple(snake *s, pixel *a){
	segment *new = malloc(sizeof(segment));
	new->location.x = a->x;
	new->location.y = a->y;
	new->next = s->head;
	s->head = new;
	free(a);
	return new_apple(s);
	
}
