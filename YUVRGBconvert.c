#include "YUVRGBconvert.h"
#include <math.h>
/*
* @brief yuv2rgb
* @param y [0, 255]
* @param u [0, 255]
* @param v [0, 255]
* @return #ABGR
*/

int qRound(float x)
{
	return (int)(x + 0.5);
}

int qBound(int min, int x, int max)
{

	if (x < min)
		x = min;
	else if (x > max)
		x = max;

	return x;
}

int yuv2r(int y, int u, int v)
{
	int R = qRound(y + 1.403 * (v - 2048));
	R = qBound(0, R, 4095);
	return R;
}
int yuv2g(int y, int u, int v)
{
	int R = qRound(y-0.343 * (u - 2048) - 0.714 * (v - 2048));
	R = qBound(0, R, 4095);
	return R;
}
int yuv2b(int y, int u, int v)
{
	int R = qRound(y + 1.770 * (u - 2048));
	R = qBound(0, R, 4095);
	return R;
}

int rgb2y(int R, int G, int B)
{
	int	Y = 0.299 * R + 0.587 * G + 0.114 * B;
	return qBound(0, Y, 4095);
}

int rgb2u(int R, int G, int B)
{
	int	U = -0.169 * R - 0.331 * G + 0.500 * B + 2048;
	return qBound(0, U, 4095);
}

int rgb2v(int R, int G, int B)
{
	int	V = 0.500 * R - 0.419 * G - 0.081 * B + 2048;
	return qBound(0, V, 4095);
}
/*
inline void rgb2yuv(short rgb, short *y, short *u, short *v)
{
	int R = qRed(rgb);
	int B = qBlue(rgb);
	int G = qGreen(rgb);

	int Y, U, V;
	Y = 0.299 * R + 0.587 * G + 0.114 * B;
	U = -0.169 * R - 0.331 * G + 0.500 * B + 128;
	V = 0.500 * R - 0.419 * G - 0.081 * B + 128;

	y = qBound(0, Y, 255);
	u = qBound(0, U, 255);
	v = qBound(0, V, 255);
}

inline short yuv2rgb_fast(short y, short u, short v)
{
	u = u - 128;
	v = v - 128;

	int r_dif = v + ((v * 103) >> 8);
	int g_dif = -((u * 88) >> 8) - ((v * 183) >> 8);
	int b_dif = u + ((u * 198) >> 8);

	int R = y + r_dif;
	int G = y + g_dif;
	int B = y + b_dif;

	R = qBound(0, R, 255);
	G = qBound(0, G, 255);
	B = qBound(0, B, 255);

	return qRgb(R, G, B);
}

*/