// cs350 Fall 2015 
// ======================
// HW1 water fall model from continuing from Lab1
// 
// student: Pedro Gonzalez
// 
// 

//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
#include "fonts.h"
}



#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 5000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	bool bubbler; 
	Shape box[5];
	Shape circle;
	Particle particle[MAX_PARTICLES];
	int n;
	int lastMousex;
	int lastMousey;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);

#define BOX_HEIGHT 20
#define BOX_WIDTH 50
#define RADIUS 100

int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	//set bubbler to false
	game.bubbler = false;

	// circle coordinates
	game.circle.radius = RADIUS;
	game.circle.center.x = 460.0;
	game.circle.center.y = 1.0;

	//declare boxes
	for(int x = 0; x < 5; ++x)
	{
		game.box[x].width = BOX_WIDTH;
		game.box[x].height = BOX_HEIGHT;
		game.box[x].center.x = 100 + 50*x;
		game.box[x].center.y = 600 - 5*60 - 50*x;
	}

	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Hw1: Waterfall Model");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);

	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}
// random function
#define rnd() (float)rand() / (float)RAND_MAX

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	//position of particle
	Particle *p = &game->particle[game->n];
	// if bubbler is toggled, change water stream x position
	if(game->bubbler)
	{
		p->s.center.x = x + rnd() * 0.5;
		p->s.center.y = y + rnd() * 0.1;
	}
	else
	{
		p->s.center.x = x;
		p->s.center.y = y;
	}
	p->velocity.x =  1.0 + rnd() * 0.1;
	p->velocity.y = rnd() * 1.0 - 0.5;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{

	game->lastMousex = e->xbutton.x;
	game->lastMousey = WINDOW_HEIGHT - e->xbutton.y;
	static int savex = 0;
	static int savey = 0;
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;

			if(!game->bubbler)// if bubbler is not toggled, mouse movement for water is allowed
				for(int i = 0; i < 10; i++)
					makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			//std::cout << "right mouse b down" << std::endl;
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		int y = WINDOW_HEIGHT - e->xbutton.y;

		if(!game->bubbler)// if bubbler is not toggled, mouse movement for water is allowed
			for(int i = 0; i < 10; i++)
				makeParticle(game, e->xbutton.x, y);
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.
		if(key == XK_b || key == XK_B)// checks if b or B are pressed
		{
			// toggle the flow of water
			if(game->bubbler)
				game->bubbler = false;
			else
				game->bubbler = true;
			std::cout << "b button activated" << std::endl;
		}

	}
	return 0;
}

// checking for collisions
void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	int x_bubbler = 60;
	int y_bubbler = WINDOW_HEIGHT;

	if(game->bubbler) // if bubbler is toggled only stream water from the top, no mouse involved
	{
		for(int i = 0; i < 10; i++)
			makeParticle(game, x_bubbler, y_bubbler);
	}

	if(!game->bubbler) // if bubbler is not toggled, mouse movement for water is allowed
		for(int i = 0; i < 10; i++)
			makeParticle(game, game->lastMousex, game->lastMousey);

	for(int i = 0; i < game->n; ++i)
	{
		p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY; 
		//check for collision with shapes...
		// check for number of collisions with boxes
		for(int j = 0; j < 5; ++j)
		{
			// collision of box
			Shape * s = &game->box[j];
			if ( p->s.center.y < s->center.y + s->height &&
				p->s.center.x >= s->center.x - s->width &&
				p->s.center.x <= s->center.x + s->width &&
				p->s.center.y > s->center.y - s->height){

					p->velocity.y *= -0.25;
					p->s.center.y = s->center.y + s->height;
			}
		}

		// collisions with circle
		Shape * c = &game->circle;

		// x axis WORKS
		float x_p, y_p,
			x_c, y_c;

		// checks if the particles x position and y position are within the circles area
		if(p->s.center.x >= c->center.x - RADIUS && p->s.center.x <= c->center.x + RADIUS && p->s.center.y < RADIUS + 3)
		{
			x_p = p->s.center.x;
			y_p = p->s.center.y;

			x_c = c->center.x;
			y_c = c->center.y;

			float distance = sqrt( pow(x_p - x_c, 2) + pow(y_p - y_c, 2));

			if(distance - RADIUS <= 0)
			{
				// changes direction of water depenting where it lands on the circle
				if(p->s.center.x < c->center.x - 5)
				{
					p->velocity.x =  -(1.0 + rnd() * 0.1);
				}
				else
					p->velocity.y = rnd() * 0.1;

				p->velocity.y *= -0.1;
				p->s.center.y += 2;
			}
		}

		//check for off-screen
		if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT) {
			memcpy(&game->particle[i], &game->particle[game->n -1], 
				sizeof(Particle));
			game->n--;

		}
	}
}

void render(Game *game)
{
	float w, h;
	// clears buffer
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	Shape *s;
	glColor3ub(90,40,190);

	// render the boxes
	for(int i = 0; i < 5; ++i)
	{

		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;

		// draws a box
		glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
		glEnd();
		glPopMatrix();

	}

	// draw circle
	Shape circle = game->circle;
	glPushMatrix();
	glTranslatef(circle.center.x, circle.center.y, circle.center.z);
	//filled circle
	float x1,y1,x2,y2;// x1, y1 are starting points

	x1 = 0.0,y1=0.0;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x1,y1);

	double PI = 3.14159265;

	for (float angle=0.0; angle <= 360.0; angle += 1.0)
	{
		// change next point
		x2 = x1 + sin(angle * PI/180)*RADIUS;
		y2 = y1 + cos(angle * PI/180)*RADIUS;
		glVertex2f(x2,y2);
	}

	glEnd();
	glPopMatrix();
	//draw all particles here
	glPushMatrix();
	glColor3ub(150,160,250);

	int randColorWater = 0;
	for(int i = 0; i < game->n; ++i){
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();	 
		randColorWater+= 10; 
	}

	// the text for the waterfall model
	Rect r;
	r.bot = WINDOW_HEIGHT- 65;
	r.left = 60;
	r.center = 0;
	int spacer = 50;
	ggprint8b(&r, spacer, 0x00FFFF00, "Requirements");
	r.left += 70;
	ggprint8b(&r, spacer, 0x00FFFF00, "Design");
	r.left += 50;
	ggprint8b(&r, spacer, 0x00FFFF00, "Coding");
	r.left += 50;
	ggprint8b(&r, spacer, 0x00FFFF00, "Testing");
	r.left += 35;
	ggprint8b(&r, spacer, 0x00FFFF00, "Maintenance");
}


