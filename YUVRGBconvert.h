#include <iostream>
#include <opencv.hpp>
#include <opencv2\opencv.hpp>
#include <ctime>
#include <windows.h>
#include <cstdio>
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;


void conv_yuv420_to_mat(Mat &dst, unsigned char* pYUV420, int width, int height);
void conv_yuv422_to_mat(Mat &dst, unsigned char* pYUV420, int width, int height);
void conv_yuv400_to_mat(Mat &dst, unsigned char* pYUV400, int nWidth, int nHeight, int bit_depth);

void DisplayYUV420(const char *s_path, int img_w, int img_h);
void DisplayYUV400(const char *s_path, int img_w, int img_h, int bit_depth);