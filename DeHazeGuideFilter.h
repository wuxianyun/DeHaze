#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <opencv.hpp>
#include <opencv2\opencv.hpp>
#include "opencv2/opencv_modules.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"


using namespace std;
using namespace cv;

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
	float			*t0, *tDown;
	float			*filtered;
	float			*inten;
	float			*cha;
	float			*alpha2;
	float			*im_dark;
	unsigned char	*deHaze;
};


int DeHazeCPU(Mat img, int radius, struct DehazeParas myParas);

