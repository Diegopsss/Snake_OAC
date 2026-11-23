#include "ripes_system.h"
#include <stdlib.h>

// ----- Configuración: tamaño del tablero, colores y velocidad
#define GRID_W (LED_MATRIX_0_WIDTH / 2)
#define GRID_H (LED_MATRIX_0_HEIGHT / 2)

#define COLOR_SNAKE  0x006400
#define COLOR_HEAD   0x00FF00
#define COLOR_APPLE  0xFF0000
#define COLOR_FONDO  0x000000

#define DELAY        3000

#define HEAP_SIZE    16384
// ----- Fin Configuración: tamaño del tablero, colores y velocidad

typedef struct pixel{
	int x;
	int y;
} pixel;

typedef struct macropixel{
	pixel pixels[4];
} macropixel;

typedef struct segment{
	pixel location;
	struct segment* next;
} segment;

typedef struct snake{
	segment* head;
} snake;

volatile unsigned int * led_base = (unsigned int*) LED_MATRIX_0_BASE;	// --- Cast corregido a unsigned int*

// ----- Inicializando los registros del D-Pad
volatile unsigned int * d_pad_up    = (unsigned int*) D_PAD_0_UP;
volatile unsigned int * d_pad_down  = (unsigned int*) D_PAD_0_DOWN;
volatile unsigned int * d_pad_left  = (unsigned int*) D_PAD_0_LEFT;
volatile unsigned int * d_pad_right = (unsigned int*) D_PAD_0_RIGHT;
// ----- Fin Inicializando los registros del D-Pad

// ----- Memoria dinámica propia (malloc/free sin sistema operativo)
static unsigned char heap_pool[HEAP_SIZE];
static unsigned int heap_top = 0;

void *malloc(size_t n){
	n = (n + 7) & ~7;
	if(heap_top + n > sizeof(heap_pool)){
		return NULL;
	}
	void *p = &heap_pool[heap_top];
	heap_top += n;
	return p;
}

void free(void *p){
	(void)p;
}
// ----- Fin Memoria dinámica propia (malloc/free sin sistema operativo)

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

// ----- Dibujo de bloques de 2x2 y de la víbora
void color_block(pixel p, int color){
	macropixel m;
	m.pixels[0].x = 2 * p.x;      m.pixels[0].y = 2 * p.y;
	m.pixels[1].x = 2 * p.x + 1;  m.pixels[1].y = 2 * p.y;
	m.pixels[2].x = 2 * p.x;      m.pixels[2].y = 2 * p.y + 1;
	m.pixels[3].x = 2 * p.x + 1;  m.pixels[3].y = 2 * p.y + 1;
	color_macropixel(&m, color);
}

void draw_snake(snake *s, int color){
	segment *n = s->head;
	while(n != NULL){
		color_block(n->location, color);
		n = n->next;
	}
}
// ----- Fin Dibujo de bloques de 2x2 y de la víbora

// ----- Lectura del D-Pad y cambio de dirección
int read_dpad(int *dx, int *dy){
	if(*d_pad_up && *dy != 1){
		*dx = 0;  *dy = -1;
		return 1;
	}
	if(*d_pad_down && *dy != -1){
		*dx = 0;  *dy = 1;
		return 1;
	}
	if(*d_pad_left && *dx != 1){
		*dx = -1; *dy = 0;
		return 1;
	}
	if(*d_pad_right && *dx != -1){
		*dx = 1;  *dy = 0;
		return 1;
	}
	return 0;
}
// ----- Fin Lectura del D-Pad y cambio de dirección

int check_apple(snake *s, pixel *a, int x, int y){
	if(a->x == s->head->location.x + x && a->y == s->head->location.y + y){
		return 1;
	}
	return 0;
}

int check_colision(snake *s){
	if(s->head->location.x < 0 || s->head->location.x >= GRID_W || s->head->location.y < 0 || s->head->location.y >= GRID_H){	// --- Límites del tablero en bloques de 2x2
		return 1;
	}
	segment *n = s->head;
	while(n != NULL){
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
}

pixel *new_apple(snake *s){
	int x;
	int y;
	while(1){
		x = rand() % GRID_W;	// --- Manzana dentro del tablero de bloques
		y = rand() % GRID_H;	// --- Manzana dentro del tablero de bloques
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

// ----- Main: inicialización, loop del juego y game over
int main(){
	snake s;
	s.head = malloc(sizeof(segment));
	s.head->location.x = GRID_W / 2;
	s.head->location.y = GRID_H / 2;
	s.head->next = NULL;

	int dx = 1, dy = 0;

	unsigned int semilla = 0;
	draw_snake(&s, COLOR_SNAKE);
	color_block(s.head->location, COLOR_HEAD);
	while(!read_dpad(&dx, &dy)){
		semilla++;
	}
	srand(semilla);

	pixel *apple = new_apple(&s);
	color_block(*apple, COLOR_APPLE);

	while(1){
		for(int i = 0; i < DELAY; i++){
			read_dpad(&dx, &dy);
		}

		draw_snake(&s, COLOR_FONDO);

		if(check_apple(&s, apple, dx, dy)){
			apple = move_apple(&s, apple);
			color_block(*apple, COLOR_APPLE);
		} else {
			move_snake(&s, dx, dy);
		}

		if(check_colision(&s)){
			break;
		}

		draw_snake(&s, COLOR_SNAKE);
		color_block(s.head->location, COLOR_HEAD);
	}

	for(int y = 0; y < LED_MATRIX_0_HEIGHT; y++){
		for(int x = 0; x < LED_MATRIX_0_WIDTH; x++){
			pixel p = {x, y};
			color_pixel(p, COLOR_APPLE);
		}
	}

	return 0;
}
// ----- Fin Main: inicialización, loop del juego y game over
