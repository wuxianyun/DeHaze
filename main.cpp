#include <iostream>
#include <opencv.hpp>
#include <opencv2\opencv.hpp>
#include <ctime>
#include <windows.h>
#include <cstdio>
#include "opencv2/highgui.hpp"

#include "YUVRGBconvert.h"
#include "DeHazeGuideFilter.h"

using namespace std;
using namespace cv;


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

	struct DehazeParas myParas;

	if (argc < 2)
	{
		printf("Please input correct arguments.\n");
		printf(" 1. Input data type(0: Camera, 1: Image(bmp,jpg,tiff), 2: YUV420 Video, 3: YUV422 Video, 4: Avi Video\n");
		scanf("%d", &data_flag);

		if (data_flag > 0)
		{
			printf(" 2. Input data name\n");
			scanf("%s", input_name);
		}
		
	}
	else
	{
		data_flag = (int)atoi(argv[1]);
		if (data_flag <0 || data_flag >4 )
		{
			printf("Please input correct arguments.\n");
			exit(0);
		}
	}


	if(data_flag == 0)//camera
	{
		Mat frame;
		int frame_width;
		int frame_height;

		VideoCapture cap(0);
		assert(cap.isOpened());
		cap.read( frame);
		frame_width = frame.cols;
		frame_height = frame.rows;
		int i,j,n;
	
		Mat img(frame.rows, frame.cols, CV_8UC3);//RGB

		iWidth = frame.cols;
		iHeight = frame.rows;
		iChannel = frame.channels();
		iSize = iWidth*iHeight;

		//初始化去雾参数
		{			
			myParas.iWidth = frame_width;
			myParas.iHeight = frame_height;
			myParas.iChannel = frame.channels();

			myParas.iSize = frame_width*frame_height;
			myParas.I_ori = (unsigned char	*)malloc(iWidth*iHeight*iChannel*sizeof(unsigned char));//Buffer for input image
			myParas.fog = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
			myParas.deHaze = (unsigned char *)malloc(iHeight*iWidth*iChannel*sizeof(unsigned char));//DeHaze image
			myParas.foggy = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));
			myParas.im = (float *)malloc(iSize * 3 * sizeof(float));
			myParas.im_dark = (float *)malloc(iHeight*iWidth*sizeof(float));
			myParas.t = (float *)malloc(iSize*sizeof(float));
			myParas.pre = (float *)malloc(iSize*iChannel*sizeof(float));
			myParas.t0 = (float *)malloc(iSize*sizeof(float));
			myParas.tDown = (float *)malloc(iSize*sizeof(float));
			myParas.filtered = (float *)malloc(iHeight*iWidth*sizeof(float));
			myParas.win_dark = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
			myParas.indices = (int *)malloc(iSize*sizeof(int));
			myParas.df = (float *)malloc(iSize / 1000 * sizeof(float) * 3);
			myParas.df_gray = (float *)malloc(iSize / 1000 * sizeof(float));
			myParas.inten = (float *)malloc(iHeight*iWidth*sizeof(float));
			myParas.cha = (float *)malloc(iHeight*iWidth*sizeof(float));
			myParas.alpha2 = (float *)malloc(iHeight*iWidth*sizeof(float));
			myParas.clear = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));
		}

		for (;;)
		{
			cap.read( frame);
			for(i=0;i<frame_height;i++){
			for(j=0;j<frame_width;j++){
				for(n=0;n<3;n++){
					img.data[(i*frame_width+j)*3+n]	= frame.data[(i*frame_width+j)*3+n];//I_ori[(i*iWidth+j)*iChannel+n];           
				}
			}
			}

			DeHazeCPU(  img , 7, myParas); //radius = 7
			imshow("ori", frame);
			imshow("DeHaze", img);

			//imwrite("ori.bmp",frame);
			//imwrite("Dehaze.bmp",img);
			waitKey(5);
		}

		//清理去雾参数
		{
			free(myParas.indices);  myParas.indices = NULL;
			free(myParas.im);       myParas.im = NULL;
			free(myParas.pre);      myParas.pre = NULL;
			free(myParas.t0);       myParas.t0 = NULL;
			free(myParas.tDown);    myParas.tDown = NULL;
			free(myParas.t);        myParas.t = NULL;
			free(myParas.df);       myParas.df = NULL;
			free(myParas.df_gray);  myParas.df_gray = NULL;
			free(myParas.win_dark); myParas.win_dark = NULL;
			free(myParas.I_ori);    myParas.I_ori = NULL;
			free(myParas.fog);      myParas.fog = NULL;
			free(myParas.foggy);	myParas.foggy = NULL;
			free(myParas.filtered); myParas.filtered = NULL;
			free(myParas.inten);    myParas.inten = NULL;
			free(myParas.cha);		myParas.cha = NULL;
			free(myParas.alpha2);	myParas.alpha2 = NULL;
			free(myParas.clear);    myParas.clear = NULL;
			free(myParas.im_dark);  myParas.im_dark = NULL;
			free(myParas.deHaze);   myParas.deHaze = NULL;
		}
	}
	else 
	{
		if(data_flag == 4)//Open AVI Files
		{
			CvCapture *cap=cvCaptureFromAVI(argv[2]);
			IplImage *frame = cvQueryFrame(cap);

			Mat img( frame->height,frame->width,CV_8UC3);

			iWidth = frame->width;
			iHeight = frame->height;
			iChannel = frame->nChannels;
			iSize = iWidth*iHeight;

			//初始化去雾参数
			{

				myParas.iWidth = frame->width;
				myParas.iHeight = frame->height;
				myParas.iChannel = frame->nChannels;
				myParas.iSize = frame->width*frame->height;

				myParas.I_ori = (unsigned char	*)malloc(iWidth*iHeight*iChannel*sizeof(unsigned char));//Buffer for input image
				myParas.fog = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
				myParas.deHaze = (unsigned char *)malloc(iHeight*iWidth*iChannel*sizeof(unsigned char));//DeHaze image
				myParas.foggy = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));
				myParas.im = (float *)malloc(iSize * 3 * sizeof(float));
				myParas.im_dark = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.t = (float *)malloc(iSize*sizeof(float));
				myParas.pre = (float *)malloc(iSize*iChannel*sizeof(float));
				myParas.t0 = (float *)malloc(iSize*sizeof(float));
				myParas.tDown = (float *)malloc(iSize*sizeof(float));
				myParas.filtered = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.win_dark = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
				myParas.indices = (int *)malloc(iSize*sizeof(int));
				myParas.df = (float *)malloc(iSize / 1000 * sizeof(float) * 3);
				myParas.df_gray = (float *)malloc(iSize / 1000 * sizeof(float));
				myParas.inten = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.cha = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.alpha2 = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.clear = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));

			}

			cvNamedWindow("Before DeHaze");
			while(frame)
			{
				frame = cvQueryFrame(cap);

				for(int i=0;i<frame->height;i++){
					for(int j=0;j<frame->width;j++){
						for(int n=0;n<3;n++){
							img.data[(i*frame->width+j)*3+n]= frame->imageData[(i*frame->width+j)*3+n];//I_ori[(i*iWidth+j)*iChannel+n];           
						}
					}
				}

				cvShowImage("Before DeHaze", frame);

				DeHazeCPU(img, 7, myParas); //radius = 7

				
				imshow("After DeHaze",img);
				cvWaitKey(20);
			}
			cvDestroyWindow("Before DeHaze");

			//清理去雾参数
			{
				free(myParas.indices);  myParas.indices = NULL;
				free(myParas.im);       myParas.im = NULL;
				free(myParas.pre);      myParas.pre = NULL;
				free(myParas.t0);       myParas.t0 = NULL;
				free(myParas.tDown);    myParas.tDown = NULL;
				free(myParas.t);        myParas.t = NULL;
				free(myParas.df);       myParas.df = NULL;
				free(myParas.df_gray);  myParas.df_gray = NULL;
				free(myParas.win_dark); myParas.win_dark = NULL;
				free(myParas.I_ori);    myParas.I_ori = NULL;
				free(myParas.fog);      myParas.fog = NULL;
				free(myParas.foggy);	myParas.foggy = NULL;
				free(myParas.filtered); myParas.filtered = NULL;
				free(myParas.inten);    myParas.inten = NULL;
				free(myParas.cha);		myParas.cha = NULL;
				free(myParas.alpha2);	myParas.alpha2 = NULL;
				free(myParas.clear);    myParas.clear = NULL;
				free(myParas.im_dark);  myParas.im_dark = NULL;
				free(myParas.deHaze);   myParas.deHaze = NULL;
			}

			//return 0;
		}
		else if (data_flag == 3 || data_flag == 2)//Open 2:YUV420 3:YUV422 Files
		{
			if (argc < 5)
			{
				printf("Please input: width height YUV_name\n");
				exit(0);
			}
			

			int iWidth = (int)atoi(argv[2]);
			int iHeight = (int)atoi(argv[3]);
			int iChannel = 3;
			iSize = iWidth*iHeight;

			FILE *readfile;
			sprintf(input_name, argv[4]);

			readfile = fopen(input_name, "rb");

			if (readfile == NULL)
			{
				cerr << "Open the YUV file %s failed" << input_name << endl;
				exit(1);
			}

			//初始化去雾参数
			{

				myParas.iWidth = iWidth;
				myParas.iHeight = iHeight;
				myParas.iChannel = 3;

				myParas.iSize = myParas.iWidth*myParas.iHeight;
				myParas.I_ori = (unsigned char	*)malloc(iWidth*iHeight*iChannel*sizeof(unsigned char));//Buffer for input image
				myParas.fog = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
				myParas.deHaze = (unsigned char *)malloc(iHeight*iWidth*iChannel*sizeof(unsigned char));//DeHaze image
				myParas.foggy = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));
				myParas.im = (float *)malloc(iSize * 3 * sizeof(float));
				myParas.im_dark = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.t = (float *)malloc(iSize*sizeof(float));
				myParas.pre = (float *)malloc(iSize*iChannel*sizeof(float));
				myParas.t0 = (float *)malloc(iSize*sizeof(float));
				myParas.tDown = (float *)malloc(iSize*sizeof(float));
				myParas.filtered = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.win_dark = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
				myParas.indices = (int *)malloc(iSize*sizeof(int));
				myParas.df = (float *)malloc(iSize / 1000 * sizeof(float) * 3);
				myParas.df_gray = (float *)malloc(iSize / 1000 * sizeof(float));
				myParas.inten = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.cha = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.alpha2 = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.clear = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));

			}

			__int64 filelength;

			filelength = filesize64(readfile);

			unsigned char* pYUV422 = (unsigned char*)malloc(iWidth * iHeight * 2);

			Mat img(iHeight, iWidth, CV_8UC3);


			int cntlen = 0;
			while (cntlen < filelength)
			{
				//每次读入一幅图，进行YUV到RGB转换操作，然后转换后图像去雾
				if (data_flag == 2)
				{
					cntlen += iWidth * iHeight * 1.5;
					fread(pYUV422, 1, iWidth * iHeight * 1.5, readfile);
					conv_yuv420_to_mat(img, pYUV422, iWidth, iHeight);

				}
				else if (data_flag == 3)
				{
					cntlen += iWidth * iHeight * 2;
					fread(pYUV422, 1, iWidth * iHeight * 2, readfile);
					conv_yuv422_to_mat(img, pYUV422, iWidth, iHeight);
				}


				imshow("Before DeHaze", img);
				cvWaitKey(20);

				DeHazeCPU(img, 7, myParas); //radius = 7

				imshow("After DeHaze", img);
				cvWaitKey(20);
			}
			

			free(pYUV422);
			fclose(readfile);

			//清理去雾参数
			{
				free(myParas.indices);  myParas.indices = NULL;
				free(myParas.im);       myParas.im = NULL;
				free(myParas.pre);      myParas.pre = NULL;
				free(myParas.t0);       myParas.t0 = NULL;
				free(myParas.tDown);    myParas.tDown = NULL;
				free(myParas.t);        myParas.t = NULL;
				free(myParas.df);       myParas.df = NULL;
				free(myParas.df_gray);  myParas.df_gray = NULL;
				//free(myParas.index);    myParas.index = NULL;
				free(myParas.win_dark); myParas.win_dark = NULL;
				free(myParas.I_ori);    myParas.I_ori = NULL;
				free(myParas.fog);      myParas.fog = NULL;
				free(myParas.foggy);	myParas.foggy = NULL;
				free(myParas.filtered); myParas.filtered = NULL;
				free(myParas.inten);    myParas.inten = NULL;
				free(myParas.cha);		myParas.cha = NULL;
				free(myParas.alpha2);	myParas.alpha2 = NULL;
				free(myParas.clear);    myParas.clear = NULL;
				free(myParas.im_dark);  myParas.im_dark = NULL;
				free(myParas.deHaze);   myParas.deHaze = NULL;
			}

			//return 0;
		}
		else if(data_flag==1)// data_flag ==1  处理单幅图像去雾
		{
			for(int ii=0;ii<argc-2;ii++)
			{
				Mat frame=imread(argv[ii+2]);
				printf("Open file %s\n", argv[ii + 2]);
				iWidth = frame.cols;
				iHeight = frame.rows;
				iChannel = frame.channels();
				iSize = iWidth*iHeight;

				//初始化去雾参数
				{

					myParas.iWidth = iWidth;
					myParas.iHeight = iHeight;
					myParas.iChannel = 3;

					myParas.iSize = myParas.iWidth*myParas.iHeight;
					myParas.I_ori = (unsigned char	*)malloc(iWidth*iHeight*iChannel*sizeof(unsigned char));//Buffer for input image
					myParas.fog = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
					myParas.deHaze = (unsigned char *)malloc(iHeight*iWidth*iChannel*sizeof(unsigned char));//DeHaze image
					myParas.foggy = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));
					myParas.im = (float *)malloc(iSize * 3 * sizeof(float));
					myParas.im_dark = (float *)malloc(iHeight*iWidth*sizeof(float));
					myParas.t = (float *)malloc(iSize*sizeof(float));
					myParas.pre = (float *)malloc(iSize*iChannel*sizeof(float));
					myParas.t0 = (float *)malloc(iSize*sizeof(float));
					myParas.tDown = (float *)malloc(iSize*sizeof(float));
					myParas.filtered = (float *)malloc(iHeight*iWidth*sizeof(float));
					myParas.win_dark = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
					myParas.indices = (int *)malloc(iSize*sizeof(int));
					myParas.df = (float *)malloc(iSize / 1000 * sizeof(float) * 3);
					myParas.df_gray = (float *)malloc(iSize / 1000 * sizeof(float));
					myParas.inten = (float *)malloc(iHeight*iWidth*sizeof(float));
					myParas.cha = (float *)malloc(iHeight*iWidth*sizeof(float));
					myParas.alpha2 = (float *)malloc(iHeight*iWidth*sizeof(float));
					myParas.clear = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));

				}

				Mat img( frame.rows,frame.cols,CV_8UC3);

				int i,j,n;
				for(i=0;i<frame.rows;i++){
					for(j=0;j<frame.cols;j++){
						for(n=0;n<3;n++){
							img.data[(i*frame.cols+j)*3+n]	= frame.data[(i*frame.cols+j)*3+n];//I_ori[(i*iWidth+j)*iChannel+n];           
						}
					}
				}
				char szName[200];
				sprintf(szName,"%s before Dehaze",argv[ii+2]);
				imshow(szName, img);
				cvWaitKey(50);

				DeHazeCPU(img, 7, myParas); //radius = 7
				//imshow("ori", frame);
				sprintf(szName,"%s after Dehaze",argv[ii+2]);
				imshow(szName, img);
				cvWaitKey(50);
				
				sprintf(szName, "Dehaze_%s", argv[ii + 2]);
				imwrite(szName, img);
				printf("Write to file %s\n", szName);

				//清理去雾参数
				{
					free(myParas.indices);  myParas.indices = NULL;
					free(myParas.im);       myParas.im = NULL;
					free(myParas.pre);      myParas.pre = NULL;
					free(myParas.t0);       myParas.t0 = NULL;
					free(myParas.tDown);    myParas.tDown = NULL;
					free(myParas.t);        myParas.t = NULL;
					free(myParas.df);       myParas.df = NULL;
					free(myParas.df_gray);  myParas.df_gray = NULL;
					//free(myParas.index);    myParas.index = NULL;
					free(myParas.win_dark); myParas.win_dark = NULL;
					free(myParas.I_ori);    myParas.I_ori = NULL;
					free(myParas.fog);      myParas.fog = NULL;
					free(myParas.foggy);	myParas.foggy = NULL;
					free(myParas.filtered); myParas.filtered = NULL;
					free(myParas.inten);    myParas.inten = NULL;
					free(myParas.cha);		myParas.cha = NULL;
					free(myParas.alpha2);	myParas.alpha2 = NULL;
					free(myParas.clear);    myParas.clear = NULL;
					free(myParas.im_dark);  myParas.im_dark = NULL;
					free(myParas.deHaze);   myParas.deHaze = NULL;
				}
			}
			waitKey();
		}
		else if (data_flag == 5)//5:YUV444 10bit
		{
			if (argc < 5)
			{
				printf("Please input: width height YUV444_name\n");
				exit(0);
			}


			int iWidth = (int)atoi(argv[2]);
			int iHeight = (int)atoi(argv[3]);
			int iChannel = 3;
			iSize = iWidth*iHeight;

			FILE *readfile;
			sprintf(input_name, argv[4]);//读入YUV444文件

			readfile = fopen(input_name, "rb");

			if (readfile == NULL)
			{
				cerr << "Open the YUV file %s failed" << input_name << endl;
				exit(1);
			}

			//初始化去雾参数
			{

				myParas.iWidth = iWidth;
				myParas.iHeight = iHeight;
				myParas.iChannel = 3;

				myParas.iSize = myParas.iWidth*myParas.iHeight;
				myParas.I_ori = (unsigned char	*)malloc(iWidth*iHeight*iChannel*sizeof(unsigned char));//Buffer for input image
				myParas.fog = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
				myParas.deHaze = (unsigned char *)malloc(iHeight*iWidth*iChannel*sizeof(unsigned char));//DeHaze image
				myParas.foggy = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));
				myParas.im = (float *)malloc(iSize * 3 * sizeof(float));
				myParas.im_dark = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.t = (float *)malloc(iSize*sizeof(float));
				myParas.pre = (float *)malloc(iSize*iChannel*sizeof(float));
				myParas.t0 = (float *)malloc(iSize*sizeof(float));
				myParas.tDown = (float *)malloc(iSize*sizeof(float));
				myParas.filtered = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.win_dark = (float			*)malloc(iWidth*iHeight*iChannel*sizeof(float));//Normalized image
				myParas.indices = (int *)malloc(iSize*sizeof(int));
				myParas.df = (float *)malloc(iSize / 1000 * sizeof(float) * 3);
				myParas.df_gray = (float *)malloc(iSize / 1000 * sizeof(float));
				myParas.inten = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.cha = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.alpha2 = (float *)malloc(iHeight*iWidth*sizeof(float));
				myParas.clear = (float *)malloc(iHeight*iWidth*iChannel*sizeof(float));

			}

			__int64 filelength;

			filelength = filesize64(readfile);

			unsigned char* pYUV422 = (unsigned char*)malloc(iWidth * iHeight * 2);

			Mat img(iHeight, iWidth, CV_8UC3);


			int cntlen = 0;
			while (cntlen < filelength)
			{
				//每次读入一幅图，进行YUV到RGB转换操作，然后转换后图像去雾
				if (data_flag == 2)
				{
					cntlen += iWidth * iHeight * 1.5;
					fread(pYUV422, 1, iWidth * iHeight * 1.5, readfile);
					conv_yuv420_to_mat(img, pYUV422, iWidth, iHeight);

				}
				else if (data_flag == 3)
				{
					cntlen += iWidth * iHeight * 2;
					fread(pYUV422, 1, iWidth * iHeight * 2, readfile);
					conv_yuv422_to_mat(img, pYUV422, iWidth, iHeight);
				}


				imshow("Before DeHaze", img);
				cvWaitKey(20);

				DeHazeCPU(img, 7, myParas); //radius = 7

				imshow("After DeHaze", img);
				cvWaitKey(20);
			}


			free(pYUV422);
			fclose(readfile);

			//清理去雾参数
			{
				free(myParas.indices);  myParas.indices = NULL;
				free(myParas.im);       myParas.im = NULL;
				free(myParas.pre);      myParas.pre = NULL;
				free(myParas.t0);       myParas.t0 = NULL;
				free(myParas.tDown);    myParas.tDown = NULL;
				free(myParas.t);        myParas.t = NULL;
				free(myParas.df);       myParas.df = NULL;
				free(myParas.df_gray);  myParas.df_gray = NULL;
				//free(myParas.index);    myParas.index = NULL;
				free(myParas.win_dark); myParas.win_dark = NULL;
				free(myParas.I_ori);    myParas.I_ori = NULL;
				free(myParas.fog);      myParas.fog = NULL;
				free(myParas.foggy);	myParas.foggy = NULL;
				free(myParas.filtered); myParas.filtered = NULL;
				free(myParas.inten);    myParas.inten = NULL;
				free(myParas.cha);		myParas.cha = NULL;
				free(myParas.alpha2);	myParas.alpha2 = NULL;
				free(myParas.clear);    myParas.clear = NULL;
				free(myParas.im_dark);  myParas.im_dark = NULL;
				free(myParas.deHaze);   myParas.deHaze = NULL;
			}

			//return 0;
		}
	}
	

	return 0;
}