#include <SDL2/SDL.h>
#include <math.h>

/* global variables */
double pi;
double const frame_movement = 0.0005; /* change this to make it go faster or slower */
int const width = 600;
int const height = 600;
int const total_points = 1000;
typedef enum { false, true } bool;

/* point utilites */
struct point { double x, y; };
struct point make_point(double const x, double const y)
{
    struct point result;
    result.x = x;
    result.y = y;
    return result;
}

void set_pixel(SDL_Renderer *renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(renderer, r,g,b,a);
	SDL_RenderDrawPoint(renderer, x, y);
}

void draw_circle(SDL_Renderer *renderer, int n_cx, int n_cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	/* if the first pixel in the screen is represented by (0,0) (which is in sdl)
	 * remember that the beginning of the circle is not in the middle of the pixel
	 * but to the left-top from it:
     */

	double error = (double)-radius;
	double x = (double)radius - 0.5;
	double y = (double)0.5;
	double cx = n_cx - 0.5;
	double cy = n_cy - 0.5;

	while (x >= y)
	{
		set_pixel(renderer, (int)(cx + x), (int)(cy + y), r, g, b, a);
		set_pixel(renderer, (int)(cx + y), (int)(cy + x), r, g, b, a);

		if (x != 0)
		{
			set_pixel(renderer, (int)(cx - x), (int)(cy + y), r, g, b, a);
			set_pixel(renderer, (int)(cx + y), (int)(cy - x), r, g, b, a);
		}

		if (y != 0)
		{
			set_pixel(renderer, (int)(cx + x), (int)(cy - y), r, g, b, a);
			set_pixel(renderer, (int)(cx - y), (int)(cy + x), r, g, b, a);
		}

		if (x != 0 && y != 0)
		{
			set_pixel(renderer, (int)(cx - x), (int)(cy - y), r, g, b, a);
			set_pixel(renderer, (int)(cx - y), (int)(cy - x), r, g, b, a);
		}

		error += y;
		++y;
		error += y;

		if (error >= 0)
		{
			--x;
			error -= x;
			error -= x;
		}
	}
}

void fill_circle(SDL_Renderer *renderer, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	/* Note that there is more to altering the bitrate of this
	 * method than just changing this value.  See how pixels are
	 * altered at the following web page for tips:
	 *   http://www.libsdl.org/intro.en/usingvideo.html
     */
    int dy;

	for (dy = 1; dy <= radius; ++dy)
	{
		/*  This loop is unrolled a bit, only iterating through half of the
		 *  height of the circle.  The result is used to draw a scan line and
		 *  its mirror image below it.
         *
		 *  The following formula has been simplified from our original.  We
		 *  are using half of the width of the circle because we are provided
		 *  with a center and we need left/right coordinates.
         */

		double const dx = floor(sqrt((2.0 * radius * dy) - (dy * dy)));
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
		SDL_RenderDrawLine(renderer, cx - dx, cy + dy - radius, cx + dx, cy + dy - radius);
		SDL_RenderDrawLine(renderer, cx - dx, cy - dy + radius, cx + dx, cy - dy + radius);
	}
}

/* NOTE: this function assumes that index is always positive */
struct point point_from_index(int const index, float const r)
{
    float angle = ((index % total_points) / (float)total_points) * (pi * 2) + pi;
    struct point result = make_point(r * cos(angle), r * sin(angle));
    return result;
}

/* NOTE: the drawing assumes that the window has an aspect ratio of 1x1 */
void draw(SDL_Renderer *renderer)
{
    /* the middle of the window */
    #define CENTER (width / 2)

    int i;
    double const r = width / 2.5;
    static double factor = 0;

    /* clear screen */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_circle(renderer, CENTER, height / 2, r, 255, 255, 255, 255);
    for(i = 0; i < total_points; ++i)
    {
        /* draw lines */
        {
            struct point a, b;
            
            /* there is no need to draw a line between the same two points */
            if(0 == i) goto point_draw;
            a = point_from_index(i, r);
            b = point_from_index(i * factor, r);
            
            /* draw a line between the two points */
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            SDL_RenderDrawLine(renderer, a.x + CENTER, a.y + CENTER, b.x + CENTER, b.y + CENTER);
        }

        /* draw points on the circle */
        point_draw:
        {
            struct point point;
            
            /* only draw some points on the circle */
            if(0 != (i % (total_points / 100 ? total_points / 100 : 1 /* prevent divsion by zero */))) continue;
            
            point = point_from_index(i, r);
            fill_circle(renderer, point.x + CENTER, point.y + CENTER, 8, 255, 255, 255, 255);
        }
    }

    /* increase factor by a small amount every frame */
    factor += frame_movement;
    SDL_RenderPresent(renderer);
    #undef CENTER
}

int main(int argc, char **argv)
{
    SDL_Window * window;
    SDL_Renderer * renderer;
    int int_must_be_4_bytes[sizeof(int) >= 4 ? 1 : -1];

    /* setup pi here as acos is not a compile-time constant */
    pi = acos(-1.0);

    /* remove warnings about unused arguments */
    (void)argc, (void)argv, (void)int_must_be_4_bytes;

	/* setup SDL2 */
	SDL_Init(SDL_INIT_VIDEO);
	
    /* create window and renderer */
    window = SDL_CreateWindow("",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    {
        SDL_Event event;

        for(;;)
        {
            /* check if we need to quit */
            while(SDL_PollEvent(&event))
                switch (event.type)
                {
                    case SDL_QUIT: goto window_quit;
                }

            /* drawing */
            {
                draw(renderer);
            }
        }

        window_quit:;
    }

    /* cleanup SDL2 */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
