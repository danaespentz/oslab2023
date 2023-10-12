/*
 * mandel-lib.c
 *
 * A library with useful functions
 * for computing the Mandelbrot Set and handling a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "mandel-lib.h"

/*****************************************
 *                                       *
 * Functions to manage a 256-color xterm *
 *                                       *
 *****************************************/

/* 3 functions to convert between RGB colors and the corresponding xterm-256 values
 * Wolfgang Frisch, xororand@frexx.de */


// whole colortable, filled by maketable()
static int initialized=0;
static unsigned char colortable[254][3];

// the 6 value iterations en the xterm color cube
static const unsigned char valuerange[] = { 0x00, 0x5F, 0x87, 0xAF, 0xD7, 0xFF };

// 16 basic colors
static const unsigned char basic16[16][3] =
{
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0xCD, 0x00, 0x00 }, // 1
	{ 0x00, 0xCD, 0x00 }, // 2
	{ 0xCD, 0xCD, 0x00 }, // 3
	{ 0x00, 0x00, 0xEE }, // 4
	{ 0xCD, 0x00, 0xCD }, // 5
	{ 0x00, 0xCD, 0xCD }, // 6
	{ 0xE5, 0xE5, 0xE5 }, // 7
	{ 0x7F, 0x7F, 0x7F }, // 8
	{ 0xFF, 0x00, 0x00 }, // 9
	{ 0x00, 0xFF, 0x00 }, // 10
	{ 0xFF, 0xFF, 0x00 }, // 11
	{ 0x5C, 0x5C, 0xFF }, // 12
	{ 0xFF, 0x00, 0xFF }, // 13
	{ 0x00, 0xFF, 0xFF }, // 14
	{ 0xFF, 0xFF, 0xFF }  // 15
};

// convert an xterm color value (0-253) to 3 unsigned chars rgb
static void xterm2rgb(unsigned char color, unsigned char* rgb)
{
	// 16 basic colors
	if(color<16)
	{
		rgb[0] = basic16[color][0];
		rgb[1] = basic16[color][1];
		rgb[2] = basic16[color][2];
	}

	// color cube color
	if(color>=16 && color<=232)
	{
		color-=16;
		rgb[0] = valuerange[(color/36)%6];
		rgb[1] = valuerange[(color/6)%6];
		rgb[2] = valuerange[color%6];
	}

	// gray tone
	if(color>=233 && color<=253)
	{
		rgb[0]=rgb[1]=rgb[2] = 8+(color-232)*0x0a;
	}
}

// fill the colortable for use with rgb2xterm
static void maketable()
{
	unsigned char c, rgb[3] = {0, 0, 0};
	for(c=0;c<=253;c++)
	{
		xterm2rgb(c,rgb);
		colortable[c][0] = rgb[0];
		colortable[c][1] = rgb[1];
		colortable[c][2] = rgb[2];
	}
}

// selects the nearest xterm color for a 3xBYTE rgb value
static unsigned char rgb2xterm(unsigned char* rgb)
{
	unsigned char c, best_match=0;
	double d, smallest_distance;

	if(!initialized)
		maketable();

	smallest_distance = 10000000000.0;

	for(c=0;c<=253;c++)
	{
		d = pow(colortable[c][0]-rgb[0],2.0) +
			pow(colortable[c][1]-rgb[1],2.0) +
			pow(colortable[c][2]-rgb[2],2.0);
		if(d<smallest_distance)
		{
			smallest_distance = d;
			best_match=c;
		}
	}

	return best_match;
}


/*******************************************
 *                                         *
 * A nice 256-color palette for drawing    *
 * the Mandelbrot Set.                     *
 *                                         *
 *******************************************/

static struct { double red; double green; double blue; } mandel256[] = {
	{0.000,0.000,0.734},
	{0.000,0.300,0.734},
	{0.000,0.734,0.000},
	{0.734,0.734,0.000},
	{0.734,0.000,0.000},
	{0.734,0.000,0.734},
	{0.000,0.734,0.734},
	{0.750,0.750,0.750},
	{0.750,0.859,0.750},
	{0.641,0.781,0.938},
	{0.500,0.000,0.000},
	{0.000,0.500,0.000},
	{0.500,0.500,0.000},
	{0.000,0.000,0.500},
	{0.500,0.000,0.500},
	{0.000,0.500,0.500},
	{0.234,0.359,0.234},
	{0.359,0.359,0.234},
	{0.484,0.359,0.234},
	{0.609,0.359,0.234},
	{0.734,0.359,0.234},
	{0.859,0.359,0.234},
	{0.984,0.359,0.234},
	{0.234,0.484,0.234},
	{0.359,0.484,0.234},
	{0.484,0.484,0.234},
	{0.609,0.484,0.234},
	{0.734,0.484,0.234},
	{0.859,0.484,0.234},
	{0.984,0.484,0.234},
	{0.234,0.609,0.234},
	{0.359,0.609,0.234},
	{0.484,0.609,0.234},
	{0.609,0.609,0.234},
	{0.734,0.609,0.234},
	{0.859,0.609,0.234},
	{0.984,0.609,0.234},
	{0.234,0.734,0.234},
	{0.359,0.734,0.234},
	{0.484,0.734,0.234},
	{0.609,0.734,0.234},
	{0.734,0.734,0.234},
	{0.859,0.734,0.234},
	{0.984,0.734,0.234},
	{0.234,0.859,0.234},
	{0.359,0.859,0.234},
	{0.484,0.859,0.234},
	{0.609,0.859,0.234},
	{0.734,0.859,0.234},
	{0.859,0.859,0.234},
	{0.984,0.859,0.234},
	{0.234,0.984,0.234},
	{0.359,0.984,0.234},
	{0.484,0.984,0.234},
	{0.609,0.984,0.234},
	{0.734,0.984,0.234},
	{0.859,0.984,0.234},
	{0.984,0.984,0.234},
	{0.234,0.234,0.359},
	{0.359,0.234,0.359},
	{0.484,0.234,0.359},
	{0.609,0.234,0.359},
	{0.734,0.234,0.359},
	{0.859,0.234,0.359},
	{0.984,0.234,0.359},
	{0.234,0.359,0.359},
	{0.359,0.359,0.359},
	{0.484,0.359,0.359},
	{0.609,0.359,0.359},
	{0.734,0.359,0.359},
	{0.859,0.359,0.359},
	{0.984,0.359,0.359},
	{0.234,0.484,0.359},
	{0.359,0.484,0.359},
	{0.484,0.484,0.359},
	{0.609,0.484,0.359},
	{0.734,0.484,0.359},
	{0.859,0.484,0.359},
	{0.984,0.484,0.359},
	{0.234,0.609,0.359},
	{0.359,0.609,0.359},
	{0.484,0.609,0.359},
	{0.609,0.609,0.359},
	{0.734,0.609,0.359},
	{0.859,0.609,0.359},
	{0.984,0.609,0.359},
	{0.234,0.734,0.359},
	{0.359,0.734,0.359},
	{0.484,0.734,0.359},
	{0.609,0.734,0.359},
	{0.734,0.734,0.359},
	{0.859,0.734,0.359},
	{0.984,0.734,0.359},
	{0.234,0.859,0.359},
	{0.359,0.859,0.359},
	{0.484,0.859,0.359},
	{0.609,0.859,0.359},
	{0.734,0.859,0.359},
	{0.859,0.859,0.359},
	{0.984,0.859,0.359},
	{0.234,0.984,0.359},
	{0.359,0.984,0.359},
	{0.484,0.984,0.359},
	{0.609,0.984,0.359},
	{0.734,0.984,0.359},
	{0.859,0.984,0.359},
	{0.984,0.984,0.359},
	{0.234,0.234,0.484},
	{0.359,0.234,0.484},
	{0.484,0.234,0.484},
	{0.609,0.234,0.484},
	{0.734,0.234,0.484},
	{0.859,0.234,0.484},
	{0.984,0.234,0.484},
	{0.234,0.359,0.484},
	{0.359,0.359,0.484},
	{0.484,0.359,0.484},
	{0.609,0.359,0.484},
	{0.734,0.359,0.484},
	{0.859,0.359,0.484},
	{0.984,0.359,0.484},
	{0.234,0.484,0.484},
	{0.359,0.484,0.484},
	{0.484,0.484,0.484},
	{0.609,0.484,0.484},
	{0.734,0.484,0.484},
	{0.859,0.484,0.484},
	{0.984,0.484,0.484},
	{0.234,0.609,0.484},
	{0.359,0.609,0.484},
	{0.484,0.609,0.484},
	{0.609,0.609,0.484},
	{0.734,0.609,0.484},
	{0.859,0.609,0.484},
	{0.984,0.609,0.484},
	{0.234,0.734,0.484},
	{0.359,0.734,0.484},
	{0.484,0.734,0.484},
	{0.609,0.734,0.484},
	{0.734,0.734,0.484},
	{0.859,0.734,0.484},
	{0.984,0.734,0.484},
	{0.234,0.859,0.484},
	{0.359,0.859,0.484},
	{0.484,0.859,0.484},
	{0.609,0.859,0.484},
	{0.734,0.859,0.484},
	{0.859,0.859,0.484},
	{0.984,0.859,0.484},
	{0.234,0.984,0.484},
	{0.359,0.984,0.484},
	{0.484,0.984,0.484},
	{0.609,0.984,0.484},
	{0.734,0.984,0.484},
	{0.859,0.984,0.484},
	{0.984,0.984,0.484},
	{0.234,0.234,0.609},
	{0.359,0.234,0.609},
	{0.484,0.234,0.609},
	{0.609,0.234,0.609},
	{0.734,0.234,0.609},
	{0.859,0.234,0.609},
	{0.984,0.234,0.609},
	{0.234,0.359,0.609},
	{0.359,0.359,0.609},
	{0.484,0.359,0.609},
	{0.609,0.359,0.609},
	{0.734,0.359,0.609},
	{0.859,0.359,0.609},
	{0.984,0.359,0.609},
	{0.234,0.484,0.609},
	{0.359,0.484,0.609},
	{0.484,0.484,0.609},
	{0.609,0.484,0.609},
	{0.734,0.484,0.609},
	{0.859,0.484,0.609},
	{0.984,0.484,0.609},
	{0.234,0.609,0.609},
	{0.359,0.609,0.609},
	{0.484,0.609,0.609},
	{0.609,0.609,0.609},
	{0.734,0.609,0.609},
	{0.859,0.609,0.609},
	{0.984,0.609,0.609},
	{0.234,0.734,0.609},
	{0.359,0.734,0.609},
	{0.484,0.734,0.609},
	{0.609,0.734,0.609},
	{0.734,0.734,0.609},
	{0.859,0.734,0.609},
	{0.984,0.734,0.609},
	{0.234,0.859,0.609},
	{0.359,0.859,0.609},
	{0.484,0.859,0.609},
	{0.609,0.859,0.609},
	{0.734,0.859,0.609},
	{0.859,0.859,0.609},
	{0.984,0.859,0.609},
	{0.234,0.984,0.609},
	{0.359,0.984,0.609},
	{0.484,0.984,0.609},
	{0.609,0.984,0.609},
	{0.734,0.984,0.609},
	{0.859,0.984,0.609},
	{0.984,0.984,0.609},
	{0.234,0.234,0.734},
	{0.359,0.234,0.734},
	{0.484,0.234,0.734},
	{0.609,0.234,0.734},
	{0.734,0.234,0.734},
	{0.859,0.234,0.734},
	{0.984,0.234,0.734},
	{0.234,0.359,0.734},
	{0.359,0.359,0.734},
	{0.484,0.359,0.734},
	{0.609,0.359,0.734},
	{0.734,0.359,0.734},
	{0.859,0.359,0.734},
	{0.984,0.359,0.734},
	{0.234,0.484,0.734},
	{0.359,0.484,0.734},
	{0.484,0.484,0.734},
	{0.609,0.484,0.734},
	{0.734,0.484,0.734},
	{0.859,0.484,0.734},
	{0.984,0.484,0.734},
	{0.234,0.609,0.734},
	{0.359,0.609,0.734},
	{0.484,0.609,0.734},
	{0.609,0.609,0.734},
	{0.734,0.609,0.734},
	{0.859,0.609,0.734},
	{0.984,0.609,0.734},
	{0.234,0.734,0.734},
	{0.359,0.734,0.734},
	{0.484,0.734,0.734},
	{0.609,0.734,0.734},
	{0.734,0.734,0.734},
	{0.859,0.734,0.734},
	{0.984,0.734,0.734},
	{0.234,0.859,0.734},
	{0.359,0.859,0.734},
	{0.484,0.859,0.734},
	{0.609,0.859,0.734},
	{0.734,0.859,0.734},
	{0.859,0.859,0.734},
	{0.984,0.969,0.938},
	{0.625,0.625,0.641},
	{0.500,0.500,0.500},
	{0.984,0.000,0.000},
	{0.000,0.984,0.000},
	{0.984,0.984,0.000},
	{0.000,0.000,0.984},
	{0.984,0.000,0.984},
	{0.000,0.984,0.984},
	{0.000,0.000,0.000}
};

/*******************************************
 *                                         *
 * Functions to compute the Mandelbrot set *
 *                                         *
 *******************************************/

/*
 * This function takes a (x,y) point on the complex plane
 * and uses the escape time algorithm to return a color value
 * used to draw the Mandelbrot Set.
 */
int mandel_iterations_at_point(double x, double y, int max)
{
	double x0 = x;
	double y0 = y;
	int iter = 0;

	while ( (x * x + y * y <= 4) && iter < max) {
		double xt = x * x - y * y + x0;
		double yt = 2 * x * y + y0;

		x = xt;
		y = yt;

		++iter;
	}

	return iter;
}

/*
 * This function takes a color value as returned
 * by mandelbrot_iterations() and uses the 256-color
 * palette defined above to return an approximation for 256-color
 * xterms.
 */
unsigned char xterm_color(int color_val)
{
	unsigned char rgb[3];

	if (color_val > 255)
		color_val = 255;

	rgb[0] = 255.0 * mandel256[color_val].red;
	rgb[1] = 255.0 * mandel256[color_val].green;
	rgb[2] = 255.0 * mandel256[color_val].blue;
	color_val = rgb2xterm(rgb);

	assert(0 <= color_val && color_val <= 255);
	return color_val;
}

/*
 * Insist until all count bytes beginning at
 * address buff have been written to file descriptor fd.
 */
ssize_t insist_write(int fd, const char *buf, size_t count)
{
	ssize_t ret;
	size_t orig_count = count;

	while (count > 0) {
		ret = write(fd, buf, count);
		if (ret < 0)
			return ret;
		buf += ret;
		count -= ret;
	}

	return orig_count;
}

/*
 * This function outputs the proper control sequence
 * to change the current color of a 256-color xterm.
 */
void set_xterm_color(int fd, unsigned char color)
{
	char buf[100];
	snprintf(buf, 100, "\033[38;5;%dm", color);

	if (insist_write(fd, buf, strlen(buf)) != strlen(buf)) {
		perror("set_xterm_color: insist_write");
		exit(1);
	}
}

/*
 * Reset all character attributes before leaving,
 * to ensure the prompt is not drawn in a funny color
 */
void reset_xterm_color(int fd)
{
	if (insist_write(fd, "\033[0m", 4) != 4) {
		perror("reset_xterm_color: insist_write");
		exit(1);
	}
}
