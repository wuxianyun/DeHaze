#include <iostream>
#include <opencv.hpp>
#include <opencv2\opencv.hpp>
//#include "showfft.h"
#include <ctime>
#include <windows.h>
#include <cstdio>
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

void conv_yuv420_to_mat(Mat &dst, unsigned char* pYUV420, int width, int height)
{
	if (!pYUV420) {
		return;
	}

	IplImage *yuvimage, *rgbimg, *yimg, *uimg, *vimg, *uuimg, *vvimg;

	int nWidth = width;
	int nHeight = height;
	rgbimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 3);
	yuvimage = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 3);

	yimg = cvCreateImageHeader(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
	uimg = cvCreateImageHeader(cvSize(nWidth / 2, nHeight / 2), IPL_DEPTH_8U, 1);
	vimg = cvCreateImageHeader(cvSize(nWidth / 2, nHeight / 2), IPL_DEPTH_8U, 1);

	uuimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
	vvimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);

	cvSetData(yimg, pYUV420, nWidth);
	cvSetData(uimg, pYUV420 + nWidth*nHeight, nWidth / 2);
	cvSetData(vimg, pYUV420 + long(nWidth*nHeight*1.25), nWidth / 2);
	cvResize(uimg, uuimg, CV_INTER_LINEAR);
	cvResize(vimg, vvimg, CV_INTER_LINEAR);

	cvMerge(yimg, uuimg, vvimg, NULL, yuvimage);
	cvCvtColor(yuvimage, rgbimg, CV_YCrCb2RGB);

	cvReleaseImage(&uuimg);
	cvReleaseImage(&vvimg);
	cvReleaseImageHeader(&yimg);
	cvReleaseImageHeader(&uimg);
	cvReleaseImageHeader(&vimg);

	cvReleaseImage(&yuvimage);

	dst = cvarrToMat(rgbimg, true);

	cvReleaseImage(&rgbimg);
}

void conv_yuv422_to_mat(Mat &dst, unsigned char* pYUV422, int width, int height)
{
	if (!pYUV422) {
		return;
	}

	IplImage *yuvimage, *rgbimg, *yimg, *uimg, *vimg, *uuimg, *vvimg;

	int nWidth = width;
	int nHeight = height;
	rgbimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 3);
	yuvimage = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 3);

	yimg = cvCreateImageHeader(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
	uimg = cvCreateImageHeader(cvSize(nWidth / 2, nHeight ), IPL_DEPTH_8U, 1);
	vimg = cvCreateImageHeader(cvSize(nWidth / 2, nHeight ), IPL_DEPTH_8U, 1);

	uuimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
	vvimg = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);

	cvSetData(yimg, pYUV422, nWidth);
	cvSetData(uimg, pYUV422 + nWidth*nHeight, nWidth / 2);
	cvSetData(vimg, pYUV422 + long(nWidth*nHeight*1.5), nWidth / 2);
	cvResize(uimg, uuimg, CV_INTER_LINEAR);
	cvResize(vimg, vvimg, CV_INTER_LINEAR);

	cvMerge(yimg, uuimg, vvimg, NULL, yuvimage);
	cvCvtColor(yuvimage, rgbimg, CV_YCrCb2RGB);

	cvReleaseImage(&uuimg);
	cvReleaseImage(&vvimg);
	cvReleaseImageHeader(&yimg);
	cvReleaseImageHeader(&uimg);
	cvReleaseImageHeader(&vimg);

	cvReleaseImage(&yuvimage);

	dst = cvarrToMat(rgbimg, true);

	cvReleaseImage(&rgbimg);
}

void conv_yuv400_to_mat(Mat &dst, unsigned char* pYUV400, int nWidth, int nHeight, int bit_depth)
{
	IplImage *yimg;

	if (!pYUV400) {
		return;
	}

	if (bit_depth == 8) {
		yimg = cvCreateImageHeader(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
	}
	else {
		yimg = cvCreateImageHeader(cvSize(nWidth, nHeight), IPL_DEPTH_16U, 1);
	}

	cvSetData(yimg, pYUV400, nWidth);

	dst = cvarrToMat(yimg, true);
	cvReleaseImageHeader(&yimg);
}


/* ---------------------------------------------------------------------------
*/
void DisplayYUV420(const char *s_path, int img_w, int img_h)
{
	printf("yuv file img_w: %d, img_h: %d \n", img_w, img_h);

	FILE* pFileIn = fopen(s_path, "rb");
	int bufLen = img_w * img_h * 3 / 2;
	unsigned char* pYuvBuf = new unsigned char[bufLen];
	int iCount = 0;


	for (iCount = 0; iCount < 200; iCount++) {
		Mat rgbImg;
		fread(pYuvBuf, bufLen * sizeof(unsigned char), 1, pFileIn);

		conv_yuv420_to_mat(rgbImg, pYuvBuf, img_w, img_h);

		cv::imshow("img", rgbImg);
		cv::waitKey(1);

		printf("%d \n", iCount);
	}

	delete[] pYuvBuf;
	fclose(pFileIn);
}

/* ---------------------------------------------------------------------------
*/
void DisplayYUV400(const char *s_path, int img_w, int img_h, int bit_depth)
{
	printf("yuv file img_w: %d, img_h: %d \n", img_w, img_h);

	FILE* pFileIn = fopen(s_path, "rb");
	int size_of_pixel = (bit_depth <= 8) ? 1 : 2;
	int bufLen = img_w * img_h * size_of_pixel;
	unsigned char* pYuvBuf = new unsigned char[bufLen];
	int iCount = 0;


	for (iCount = 0; iCount < 200; iCount++) {
		Mat rgbImg;
		fread(pYuvBuf, bufLen * sizeof(unsigned char), 1, pFileIn);

		conv_yuv400_to_mat(rgbImg, pYuvBuf, img_w, img_h, bit_depth);

		cv::imshow("img", rgbImg);
		cv::waitKey(0);

		printf("%d \n", iCount);
	}

	delete[] pYuvBuf;
	fclose(pFileIn);
}