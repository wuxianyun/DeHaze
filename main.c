
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DeHazeGuideFilter.h"

#include "YUVRGBconvert.h"

#include "getopt.h"                // 包含头文件

#ifdef _DEBUG
#pragma comment(lib, "getoptd.lib")   // 加载静态库文件（Windows）
#else
#pragma comment(lib, "getopt.lib")   // 加载静态库文件（Windows）
#endif

//#define _CRT_SECURE_NO_WARNINGS

struct globalArgs_t {

	char outFileName[200];    /* -o option */
	int width;              /* -w option */
	int height;             /* -h option */

	char *format;			/* -f option*/
	char inputFiles[200];          /* input files */
	int numInputFiles;          /* # of input files */

	int radius;             /* --randomize option */
} globalArgs;

static const char *optString = "i:o:w:h:r:k:";

static const struct option longOpts[] = {
	//{ "inputFiles", no_argument, NULL, 'I' },
	{ "width", required_argument, NULL, 'w' },
	{ "height", required_argument, NULL, 'h' },
	{ "input file", required_argument, NULL, 'i' },
	{ "radius", no_argument, NULL, 'r' },
	{ "cha", no_argument, NULL, 'k' },
	{ "output file", no_argument, NULL, 'o' },
	{ NULL, no_argument, NULL, 0 }
};

__int64 filesize64(FILE *stream)
{
	__int64 curpos, length;
	curpos = _ftelli64(stream);
	_fseeki64(stream, 0L, SEEK_END);
	length = _ftelli64(stream);
	_fseeki64(stream, curpos, SEEK_SET);
	return length;
}

int main(int argc, char *argv[])
{
	int				i;
	int				data_flag = 0;
	char			input_name[200];
	int				iWidth, iHeight, iChannel, iSize;
	int				radius=15;
	int				tolerance = 1600;//default value

	struct DehazeParas myParas;
	int				opt;
	int				longIndex;


	opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	while (opt != -1) {
		switch (opt) {
		case 'w':
			globalArgs.width = (int) atoi(optarg); /* true */
			break;

		case 'h':
			globalArgs.height = (int)atoi(optarg);
			break;

		case 'o':
			strcpy(globalArgs.outFileName, optarg);
			break;

		case 'i':
			//globalArgs.inputFiles = optarg;
			strcpy(globalArgs.inputFiles, optarg);
			break;

		case 'r':
			radius = (int) atoi(optarg);
			break;

		//by Wxyun 201809122322, add -k parameter
		case 'k':
			tolerance = (int)atoi(optarg);
			break;

		case 0:     /* long option without a short arg */
			//if (strcmp("randomize", longOpts[longIndex].name) == 0) {
			//	globalArgs.randomized = 1;
			//}
			break;

		default:
			/* You won't actually get here. */
			break;
		}

		opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	}

	printf("----------------Current version: 1.03---------------------------\n");
	printf("----------------Update data: 20180916---------------------------\n");

	
	iWidth = globalArgs.width;
	iHeight = globalArgs.height;
	iChannel = 3;
	iSize = iWidth*iHeight;

	FILE *readfile;
	FILE *outfile;
	sprintf(input_name, globalArgs.inputFiles);

	readfile = fopen(input_name, "rb");
	if (readfile == NULL)
	{
		printf( "Open the YUV file %s failed" , input_name );
		exit(1);
	}
	outfile = fopen(globalArgs.outFileName, "wb");
	if (outfile == NULL)
	{
		printf("Open the output file %s failed", globalArgs.outFileName);
		exit(1);
	}
//初始化去雾参数
	{

		myParas.iWidth = iWidth;
		myParas.iHeight = iHeight;
		myParas.iChannel = 3;

		myParas.iSize = myParas.iWidth*myParas.iHeight;
		//myParas.I_ori = (unsigned char	*)malloc(iWidth*iHeight*iChannel*sizeof(unsigned char));//Buffer for input image
		myParas.fog = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
		myParas.deHaze = (int *)malloc(iHeight*iWidth*iChannel*sizeof(int));//DeHaze image
		myParas.foggy = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));
		myParas.im = (float *)malloc(iSize * iChannel * sizeof(float));
		myParas.im_dark = (float *)malloc(iHeight*iWidth*sizeof(float));
		myParas.t = (float *)malloc(iSize*sizeof(float));
		myParas.pre = (float *)malloc(iSize*iChannel*sizeof(float));
		//myParas.t0 = (float *)malloc(iSize*sizeof(float));
		myParas.tDown = (float *)malloc(iSize*sizeof(float));
		myParas.filtered = (float *)malloc(iHeight*iWidth*sizeof(float));
		myParas.win_dark = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
		myParas.indices = (int *)malloc(iSize*sizeof(int));
		myParas.df = (float *)malloc(iSize / 100 * sizeof(float) * 3);
		myParas.df_gray = (float *)malloc(iSize / 100 * sizeof(float));
		//myParas.inten = (float *)malloc(iHeight*iWidth*sizeof(float));
		//myParas.cha = (float *)malloc(iHeight*iWidth*sizeof(float));
		myParas.alpha2 = (float *)malloc(iHeight*iWidth*sizeof(float));
		myParas.clear = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));

	}

	__int64 filelength;

	filelength = filesize64(readfile);

	short* pYUV422 = (short *)malloc(iWidth * iHeight * 3 * sizeof(short) );

	short *img = (short *)malloc(iSize * iChannel * sizeof(short));


	int cntlen = 0;
	int framecnt = 0;

	__int64 current_len = 0;
	while (current_len <= filelength)
	{
		printf("Process frame %d, ", framecnt);

		current_len += iWidth * iHeight * iChannel * sizeof(short);
		printf("process length %I64u, ", current_len);
		printf("file length %I64u. \n", filelength);

		fread(pYUV422, 1, iWidth * iHeight * iChannel * sizeof(short), readfile);
		//conv_yuv420_to_mat(img, pYUV422, iWidth, iHeight);

		short *yptr, *uptr, *vptr;

		yptr = pYUV422;
		uptr = yptr + iHeight*iWidth;
		vptr = uptr + iHeight*iWidth;
		for (i = 0; i < iSize; i++)
		{
			short y = yptr[i];
			short u = uptr[i];
			short v = vptr[i];
			//by Wxyun 201809122322, change rgb sequence
			img[3 * i+0] = yuv2r(y, u, v);
			img[3 * i+1] = yuv2g(y, u, v);
			img[3 * i+2] = yuv2b(y, u, v);
		}
		

		//FILE *testfile;
		//testfile = fopen("test.raw", "wb");
		//fwrite(img, iSize*iChannel*sizeof(short), 1, testfile);
		//fclose(testfile);
		
		//by Wxyun 201809122322, add tolerance parameter

		DeHazeCPU(img, radius, tolerance, myParas); //radius = 7
		fwrite(img, 1, iWidth * iHeight * iChannel * sizeof(short), outfile);

		framecnt++;

	}


	free(pYUV422);
	free(img);
	fclose(readfile);
	fclose(outfile);

	//清理去雾参数
	{
		free(myParas.indices);  myParas.indices = NULL;
		free(myParas.im);       myParas.im = NULL;
		free(myParas.pre);      myParas.pre = NULL;
		//free(myParas.t0);       myParas.t0 = NULL;
		free(myParas.tDown);    myParas.tDown = NULL;
		free(myParas.t);        myParas.t = NULL;
		free(myParas.df);       myParas.df = NULL;
		free(myParas.df_gray);  myParas.df_gray = NULL;
		//free(myParas.index);    myParas.index = NULL;
		free(myParas.win_dark); myParas.win_dark = NULL;
		//free(myParas.I_ori);    myParas.I_ori = NULL;
		free(myParas.fog);      myParas.fog = NULL;
		free(myParas.foggy);	myParas.foggy = NULL;
		free(myParas.filtered); myParas.filtered = NULL;
		//free(myParas.inten);    myParas.inten = NULL;
		//free(myParas.cha);		myParas.cha = NULL;
		free(myParas.alpha2);	myParas.alpha2 = NULL;
		free(myParas.clear);    myParas.clear = NULL;
		free(myParas.im_dark);  myParas.im_dark = NULL;
		free(myParas.deHaze);   myParas.deHaze = NULL;
	}


	

	return 0;
}