#include "ncurses.h"
#include "signal.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "unistd.h"
/*
 * Idea: have player, ai, etc stuff on separate threads
 * and transfer data to them with mutexed queues
 */
#define NUM_BLOCKS 10
struct block {
	int x,y,dx,dy,color;
};

struct game {
	bool running;
	int w,h;
	struct block blocks[NUM_BLOCKS];
	int current;
	clock_t start;
};

void init(struct game *g);
void input(struct game *g, int key);
void display(struct game *g);
void update(struct game *g);
void draw_rect(int y, int x, int h, int w, int color);

int main(int argc, char **argv)
{
	struct game g;
	int ch;
	srand(time(0));
	init(&g);
	display(&g);
	while (g.running) {
		usleep(30000);
		while ((ch = getch()) != ERR)
			input(&g, ch);
		erase();
		display(&g);
		refresh();
		update(&g);
	}
	endwin();
	return 0;
}
void input(struct game *g, int key)
{
	switch(key) {
	case 'q':
		g->running = 0;
		break;
	case KEY_RESIZE:
		g->w = COLS;
		g->h = LINES;
		break;
	case KEY_LEFT:
		g->blocks[g->current].dx--;
		break;
	case KEY_RIGHT:
		g->blocks[g->current].dx++;
		break;
	case KEY_UP:
		g->blocks[g->current].dy--;
		break;
	case KEY_DOWN:
		g->blocks[g->current].dy++;
		break;
	case 's':
		g->blocks[g->current].dx = 0;
		g->blocks[g->current].dy = 0;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		g->current = key - '0';
		break;
	}
}
void display(struct game *g)
{
	int i;
	clock_t time_now;
	for (i = 0; i < NUM_BLOCKS; i++)
		draw_rect(g->blocks[i].y, g->blocks[i].x, 4, 10, g->blocks[i].color);
	attron(COLOR_PAIR(1));
	time_now = clock() - g->start;
	mvprintw(1, g->w - 20, "Time used: %.3f", ((float)time_now) / CLOCKS_PER_SEC);
	mvprintw(2, g->w - 20, "Current block: %d", g->current);
	attroff(COLOR_PAIR(1));
}
void update(struct game *g)
{
	int i;
	for (i = 0; i < NUM_BLOCKS; i++) {
		g->blocks[i].x += g->blocks[i].dx;
		g->blocks[i].y += g->blocks[i].dy;
	       	if (g->blocks[i].x < 0)
	       		g->blocks[i].x = g->w - 1;
       		else if (g->blocks[i].x >= g->w)
			g->blocks[i].x = 0;
		else if (g->blocks[i].y < 0)
		       	g->blocks[i].y = g->h - 1;
	       	else if (g->blocks[i].y >= g->h)
       			g->blocks[i].y = 0;
	}
}
void init(struct game *g)
{
	int i;
	initscr();
	if (has_colors() == FALSE) {
		endwin();
		puts("Cannot display color");
		exit(1);
	}
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_WHITE, COLOR_GREEN);
	init_pair(4, COLOR_WHITE, COLOR_RED);
	init_pair(5, COLOR_WHITE, COLOR_CYAN);
	init_pair(6, COLOR_WHITE, COLOR_YELLOW);
	init_pair(7, COLOR_WHITE, COLOR_MAGENTA);
	raw();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	refresh();
	flushinp();
	g->running = 1;
	g->current = 0;
	for (i = 0; i < NUM_BLOCKS; i++) {
		g->blocks[i].x = rand() % (COLS - 4) + 4;
		g->blocks[i].y = rand() % (LINES - 4) + 4;
		g->blocks[i].dx = rand() % 3 - 1;
		g->blocks[i].dy = rand() % 3 - 1;
		g->blocks[i].color = rand() % 6 + 2;
	}
	g->w = COLS;
	g->h = LINES;
	g->start = clock();
}
void exit_handler(int signum)
{
	endwin();
}
void draw_rect(int y, int x, int h, int w, int color)
{
	int i;
	if (y < 0 || x < 0 || x + w >= COLS || y + h >= LINES)
		return;
	attron(COLOR_PAIR(color));
	for(i = 0; i < h; i++) {
		mvprintw(y+i, x, "%*c", w, ' ');
	}
	attroff(COLOR_PAIR(color));
}
