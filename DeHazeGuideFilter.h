#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define I3(a,b,c) ((c) * w * c + (b) * w + (a))
#define I2(a,b)   ((b) * w + (a))
#define RGB2GRAY(r,g,b) (r)*0.2989f+(g)*0.5870f+(b)*0.1140f

struct DehazeParas{

	int				iWidth, iHeight, iChannel, iSize;
	float           *fog, *im;
	float           *foggy, *clear;
	unsigned char   *I_ori;
	float			*win_dark;
	float			*jarkvec;
	float			*imvec;
	int				*indices;
	int				*indices_temp;
	float			*df, *df_gray;
	//int				*index;
	float			*t;
	float			*pre;
	float			*tDown;
	float			*filtered;
	//float			*inten;
	//float			*cha;
	float			*alpha2;
	float			*im_dark;
	int				*deHaze;
};


int DeHazeCPU(short *img, int radius, int tolerance, struct DehazeParas myParas);

