/*
Luigi Quattrociocchi
April 12 2021

Double Pendulum thing ; inspired by coding train and 3d spinning donut
https://thecodingtrain.com/CodingChallenges/093-double-pendulum.html
https://www.myphysicslab.com/pendulum/double-pendulum-en.html
https://www.a1k0n.net/2006/09/15/obfuscated-c-donut.html
https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/olcPixelGameEngine.h

*/


#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


void tick(char* b);
void show(char* b);
void loop();

int main() {
	printf("\x1b[2J"); // clear screen, cursor to home
	loop();
	return 0;
}



const int SW = 80, SH = 22; 	// width, height
const float hStretch = 2; 		// 1:1 aspect ratio
const int buffSZ = SW * SH; 	// buffer length
const int step = 30000;		// timestep (~ 30fps)



void drawPixel_f(char* b, int x, int y, char fill) {
	if ((0 <= x && x < SW) && (0 <= y && y < SH))
		// in screen space
		b[x + y * SW] = fill;
}
// fill argument function overload
void drawPixel(char* b, int x, int y) {
	drawPixel_f(b, x, y, '#');
}


// line drawing routine yoinked from my dawgz oneLoneCoder
// https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/olcPixelGameEngine.h
void drawLine_f(char* b, int x1, int y1, int x2, int y2, char fill) {
	int dx = x2 - x1;
	int dy = y2 - y1;

	// simple lines
	if (dx == 0) {
		if (y2 < y1) {
			int t = y2;
			y2 = y1;
			y1 = t;
		}

		for (int y = y1; y <= y2; ++y)
			drawPixel_f(b, x1, y, fill);
		return;
	}

	if (dy == 0) {
		if (x2 < x1) {
			int t = x2;
			x2 = x1;
			x1 = t;
		}

		for (int x = x1; x <= x2; x++)
			drawPixel_f(b, x, y1, fill);
		return;
	}
	

	// other lines
	int dx1 = abs(dx);
	int dy1 = abs(dy);
	int px = 2 * dy1 - dx1;
	int py = 2 * dx1 - dy1;
	int x, y, xe, ye;
	if (dy1 <= dx1) {
		if (dx >= 0) {
			x = x1;
			y = y1;
			xe = x2;
		}
		else {
			x = x2;
			y = y2;
			xe = x1;
		}

		drawPixel_f(b, x, y, fill);

		for (int i = 0; x < xe; i++) {
			++x;
			if (px < 0)
				px += 2 * dy1;
			else {
				y += ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) ? 1 : -1;
				px += 2 * (dy1 - dx1);
			}
			drawPixel_f(b, x, y, fill);
		}
	}
	else {
		if (dy >= 0) {
			x = x1;
			y = y1;
			ye = y2;
		}
		else {
			x = x2;
			y = y2;
			ye = y1;
		}

		drawPixel_f(b, x, y, fill);

		for (int i = 0; y < ye; i++) {
			++y;
			if (py <= 0)
				py += 2 * dx1;
			else {
				x += ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) ? 1 : -1;
				py += 2 * (dx1 - dy1);
			}
			drawPixel_f(b, x, y, fill);
		}
	}
}

void drawLine(char* b, int x1, int y1, int x2, int y2) {
	drawLine_f(b, x1, y1, x2, y2, '#');
}




// arbitrary numbers are way out of wack, but whatever
// NOTE: stuff will overflow when the acceleration goes too high
// for most starting values, either spiral out of control or barely move

// TODO: wrap stuff in a struct or something
const int originX = SW / 2;
const int originY = SH / 3;
const float grav = 0.1;
const float damp = 0.8;
const float len1 = 7;
const float len2 = 7;
const float mass1 = 4;
const float mass2 = 4;
float ang1_s = 3 * M_PI_4;
float ang2_s = -M_PI_4;
float ang1_v = 0;
float ang2_v = 0;


void tick(char* b) {

	// https://www.myphysicslab.com/pendulum/double-pendulum-en.html
	// wack math and stuff

	float ang1_a = (-grav * (2 * mass1 + mass2) * sin(ang1_s) + -mass2 * grav * sin(ang1_s -2 * ang2_s) + -2 * sin(ang1_s - ang2_s) * mass2 * (ang2_v * ang2_v * len2 + ang1_v * ang1_v * len1 * cos(ang1_s - ang2_s))) / (len1 * (2 * mass1 + mass2 - mass2 * cos(2 * ang1_s -2 * ang2_s)));

	float ang2_a = (2 * sin(ang1_s - ang2_s) * ((ang1_v * ang1_v * len1 * (mass1 + mass2)) + grav * (mass1 + mass2) * cos(ang1_s) + ang2_v * ang2_v * len2 * mass2 * cos(ang1_s - ang2_s))) / (len2 * (2 * mass1 + mass2 - mass2 * cos(2 * ang1_s -2 * ang2_s)));

	ang1_a *= damp;
	ang2_a *= damp;
	ang1_v += ang1_a;
	ang2_v += ang2_a;
	ang1_s += ang1_v;
	ang2_s += ang2_v;

	// world coords
	float x1 = len1 * sin(ang1_s);
	float y1 = len1 * cos(ang1_s);
	float x2 = x1 + len2 * sin(ang2_s);
	float y2 = y1 + len2 * cos(ang2_s);

	// screen coords
	int sx0 = originX;
	int sy0 = originY;
	int sx1 = (int) (hStretch * x1) + sx0;
	int sy1 = (int) y1 + sy0;
	int sx2 = (int) (hStretch * x2) + sx0;
	int sy2 = (int) y2 + sy0;
	


	// blank screen
	memset(b, ' ', buffSZ * sizeof(char));
	// put stuff to draw in buffer
	drawLine(b, sx0, sy0, sx1, sy1);
	drawLine(b, sx1, sy1, sx2, sy2);
	drawPixel_f(b, sx0, sy0, '@');
	drawPixel_f(b, sx1, sy1, '@');
	drawPixel_f(b, sx2, sy2, '@');
}

void show(char* b) {
	printf("\x1b[H"); // cursor to home
	for (int i = 0; i <= buffSZ; ++i)
		putchar(i % SW ? b[i] : '\n');
}

void loop() {
	char buffer[buffSZ];
	while (1) {
		tick(buffer);
		show(buffer);
		usleep(step);
	}
}