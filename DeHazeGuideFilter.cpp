/********************************************************************
 created:	2014/12/01
 created:	1:12:2014   17:25
 filename: 	E:\Hazeremove\DeHaze\DeHazeGuideFilter\DeHazeGuideFilter.c
 file path:	E:\Hazeremove\DeHaze\DeHazeGuideFilter
 file base:	DeHazeGuideFilter
 file ext:	c
 author:		Xianyun Wu
 
 purpose:	C code for DeHaze using guidefilter
 *********************************************************************/
#include "DeHazeGuideFilter.h"

void boxfilter( float *src, float *dst, int height, int width, int radius);
void guidefilter(float *I, float *p, float *dst, int height, int width, int channel, int radius, float eps);

float max(float a, float b)
{
	if(a>b) return a;
	else return b;
}
float min(float a, float b)
{
	if(a>b) return b;
	else return a;
}
unsigned char round_me(float x)
{
    if(x>255) return 255;
    else if(x<0) return 0;
    else return x;
}

float rgb2gray(float *p)
{
    //inv([1.0 0.956 0.621; 1.0 -0.272 -0.647; 1.0 -1.106 1.703])
    return p[0]*0.2989f+p[1]*0.5870f+p[2]*0.1140f;
}

//dst=src1+src2
void matrix_add(float *src1, float *src2, float *dst, int row, int column)//a+b
{
    int i,j;
    for(i=0;i<row;i++)
    {
        for(j=0;j<column;j++)
        {
            dst[i*column+j] = src1[i*column+j] + src2[i*column+j];
        }
    }
}

//dst=src1-src2
void matrix_subtract(float *src1, float *src2, float *dst, int row, int column)//subtract
{
    int i,j;
    for(i=0;i<row;i++)
    {
        for(j=0;j<column;j++)
        {
            dst[i*column+j] = src1[i*column+j] - src2[i*column+j];
        }
    }
}

//dst=src+sum
void matrix_add_all(float *src, float *dst, int row, int column, float sum)//all add one value
{
    int i,j;
    for(i=0;i<row;i++)
    {
        for(j=0;j<column;j++)
        {
            dst[i*column+j] = src[i*column+j]+sum;
        }
    }
}

//dst=src./div
void matrix_divide(float *src, float *dst, int row, int column, float div)//all divide one value
{
    int i,j;
    for(i=0;i<row;i++)
    {
        for(j=0;j<column;j++)
        {
            dst[i*column+j] = src[i*column+j]/div;
        }
    }
}

//dot divide  dst=src1./src2
void matrix_dot_divide(float *src1, float *src2, float *dst, int row, int column)
{
    int i,j;
    for(i=0;i<row;i++)
    {
        for(j=0;j<column;j++)
        {
            dst[i*column+j] = src1[i*column+j]/src2[i*column+j];
        }
    }
}

//dot multiple  dst=src1.*src2
void matrix_dot_multiple(float *src1, float *src2, float *dst, int row, int column)
{
    int i,j;
    for(i=0;i<row;i++)
    {
        for(j=0;j<column;j++)
        {
            dst[i*column+j] = src1[i*column+j]*src2[i*column+j];
        }
    }
}

//gray erode for flat: find the minimum value in square neighborhood with radius
void imerode(float *src, float *dst, int height, int width, int channel, int radius)
{
    int i,j,m,n;
    float min;
    int index_x,index_y;
    
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
			min=255.0f;
            //for(k=0;k<channel;k++)
            for(m=-radius;m<=radius;m++)//Square neighborhood
            {
                for(n=-radius;n<=radius;n++)
                {
                    index_x = i+m;
                    index_y = j+n;
                    if(index_x<0||index_x>=height||index_y<0||index_y>=width)
                        continue;
                    else
                    {
                        if(min>src[index_x*width*channel+index_y*channel])
                            min=src[index_x*width*channel+index_y*channel];
                        if(min>src[index_x*width*channel+index_y*channel+1])
                            min=src[index_x*width*channel+index_y*channel+1];
                        if(min>src[index_x*width*channel+index_y*channel+2])
                            min=src[index_x*width*channel+index_y*channel+2];
                    }
                }
            }
            dst[i*width+j]   = min;
        }
    }
}

//gray dilate for flat: find the maximum value in square neighborhood with radius
void imdilate(float *src, float *dst, int height, int width, int channel, int radius)
{
    int i,j,m,n;
    float max_r,max_g,max_b;
    int index_x,index_y;
    
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
			max_r=max_g=max_b=0.0f;
            //for(k=0;k<channel;k++)
            for(m=-radius;m<=radius;m++)//Square neighborhood
            {
                for(n=-radius;n<=radius;n++)
                {
                    index_x = i+m;
                    index_y = j+n;
                    if(index_x<0||index_x>=height||index_y<0||index_y>=width)
                        continue;
                    else
                    {
                        if(max_r<src[index_x*width*channel+index_y*channel])
                            max_r=src[index_x*width*channel+index_y*channel];
                        if(max_g<src[index_x*width*channel+index_y*channel+1])
                            max_g=src[index_x*width*channel+index_y*channel+1];
                        if(max_b<src[index_x*width*channel+index_y*channel+2])
                            max_b=src[index_x*width*channel+index_y*channel+2];
                    }
                }
            }
            dst[i*width*channel+j*channel]   = max_r;
            dst[i*width*channel+j*channel+1] = max_g;
            dst[i*width*channel+j*channel+2] = max_b;
        }
    }
}

//return file length
int filesize(FILE *stream)
{
    int curpos, length;
    curpos = ftell(stream);
    fseek(stream, 0L, SEEK_END);
    length = ftell(stream);
    fseek(stream, curpos, SEEK_SET);
    return length;
}

//sort from min to max, swap with index
void sort(float *p, int *index, int size)//use reduction sort, sort with index
{
    int i,j;
    float temp;
    int   itemp;
    for(i=0;i<size-1;i++)
    {
        for(j=i+1;j<size;j++)//min->max
        {
            if(p[i]>p[j])
            {
                temp=p[i];
                p[i]=p[j];
                p[j]=temp;
                itemp=index[i];
                index[i]=index[j];
                index[j]=itemp;
            }
        }
    }
}

//guidefilter
//int main(int argc, char *argv[])
int DeHazeCPU(Mat img, int radius, struct DehazeParas myParas)
{
    int				iHeight,iWidth,iChannel,iSize;
    char			img_name[400],dehaze_img_name[400];
    FILE            *fp_in,*fp_out,*fp_test;
    int				img_len;
    int				i,j,m,n;
    int				numpx;//choose 0.1% points
    float			atomsLight[3];
    float			w=0.95f;
	int				k;
	clock_t			start,end;

	float           *fog, *im;
	float           *foggy, *clear;
	unsigned char   *I_ori;
	float			*win_dark;
	float			*jarkvec;
	float			*imvec;
	int				*indices;
	int				*indices_temp;
	float			*df, *df_gray;
	int				*index;
	float			*t;
	float			*pre;
	float			*t0, *tDown;
	float			*filtered;
	float			*inten;
	float			*cha;
	float			*alpha2;
	float			*im_dark;
	unsigned char	*deHaze;

	iWidth	 = myParas.iWidth    ;
	iHeight	 = myParas.iHeight   ;
	iChannel = myParas.iChannel  ;
	iSize    = myParas.iSize     ;

	I_ori    = myParas.I_ori     ;
	fog      = myParas.fog       ;
	deHaze   = myParas.deHaze    ;
	foggy    = myParas.foggy     ;
	im       = myParas.im        ;
	im_dark  = myParas.im_dark   ;
	t        = myParas.t         ;
	pre      = myParas.pre       ;
	t0       = myParas.t0        ;
	tDown    = myParas.tDown     ;
	filtered = myParas.filtered  ;
	win_dark = myParas.win_dark  ;
	indices  = myParas.indices   ;
	df       = myParas.df        ;
	df_gray  = myParas.df_gray   ;
	inten    = myParas.inten     ;
	cha      = myParas.cha       ;
	alpha2   = myParas.alpha2    ;
	clear    = myParas.clear     ;
                               
	iWidth=img.cols;
	iHeight=img.rows;
	iChannel=img.channels();
	printf("Width: %d Height: %d Channel: %d\n",iWidth,iHeight,iChannel);
	

	for(i=0;i<iHeight;i++){
		for(j=0;j<iWidth;j++){
			for(n=0;n<iChannel;n++){
				fog[(i*iWidth+j)*iChannel+n]	= img.data[(i*iWidth+j)*iChannel+n]/1023.0f;//I_ori[(i*iWidth+j)*iChannel+n];           
			}
		}
	}
	memcpy(pre,fog,iSize*iChannel*sizeof(float));
    
	start = clock();
    imerode(fog, win_dark, iHeight, iWidth, iChannel, radius);

	end = clock();
	printf("1.Image erode time is %dms.\n",end-start);
	

    numpx	= (int)floor(iSize/1000);//choose 0.1% points
    
	int count[256] = { 0 };
	int thres = numpx;
	for (i = 0; i < iSize; i++)
	{
		count[(int) (win_dark[i]*255) ] ++;
	}
	int cnt = 0;
	for (i = 255; i > 0; i--)
	{
		cnt += count[i];
		if (cnt > thres)
		{
			break;
		}
	}

	int indice = i;
	int indice_index = 0;
	for (i = 0; i < iSize; i++)
	{
		if ((int)(win_dark[i] * 255) == indice)
			indice_index = i;
	}
	
    for(i=0;i<numpx;i++)
    {
        df_gray[i]=0;
        for(j=0;j<iChannel;j++)
        {
			df[i * 3 + j] = fog[indice_index * 3 + j];//pick out those pixels
        }
        df_gray[i]=RGB2GRAY(df[i*3],df[i*3+1],df[i*3+2]);//RGB to GRAY
    }
    index = (int *)malloc(numpx*sizeof(int));
    for(i=0;i<numpx;i++)
    {
        index[i]=0;
    }
    //sort(df_gray,index,numpx);//sort again
	float maxdark=0.0;
	int   maxindex=0;
	for(i=0;i<iSize;i++)
	{
		if(win_dark[i]>maxdark)
		{
			maxdark = win_dark[i];
			maxindex = i;
		}
	}
	
    atomsLight[0]=fog[maxindex*3];
    atomsLight[1]=fog[maxindex*3+1];
    atomsLight[2]=fog[maxindex*3+2];
	 
	//start = clock();    
    
    for(i=0;i<iHeight;i++)
    {
        for(j=0;j<iWidth;j++)
        {
            for(n=0;n<iChannel;n++)
            {
                im[(i*iWidth+j)*iChannel+n]	= fog[(i*iWidth+j)*iChannel+n]/atomsLight[n];
                //printf("%d,%d,%d,%x\n",i,j,n,*(I_ori+(i*w+j)*c+n));
            }
        }
    }
    
    imerode(im,im_dark,iHeight,iWidth,iChannel,radius);
    
	end = clock();
	printf("2.Image erode time is %dms.\n",end-start);

    for(i=0;i<iSize;i++)
    {
        t[i] = 1.0f-w*im_dark[i];
    }
    
	for(i=0;i<iSize;i++)
    {
        t0[i]=0.1f;
        if(t0[i]>t[i])
            tDown[i]=t0[i];
        else
            tDown[i]=t[i];
    }    

    for(i=0;i<iHeight;i++)
    {
        for(j=0;j<iWidth;j++)
        {
            for(n=0;n<iChannel;n++)
            {
                pre[(i*iWidth+j)*iChannel+n]	= (fog[(i*iWidth+j)*iChannel+n]-atomsLight[n])/tDown[i]+atomsLight[n];
                //printf("%d,%d,%d,%x\n",i,j,n,*(I_ori+(i*w+j)*c+n));
            }
        }
    }
        

    imdilate(fog,foggy,iHeight,iWidth,iChannel,radius);
	
    for(i=0;i<iHeight*iWidth;i++)
    {
        foggy[i]=rgb2gray(foggy+i*3);
    }
    
    guidefilter(foggy,t,filtered,iHeight,iWidth,iChannel,radius*5,1e-05f);
    
	//Debug guidefilter, filtered wrong!!!
	end = clock();
	printf("3.DeHaze time is %dms.\n",end-start);

   
	k=30;
    for(m=0;m<iHeight;m++)
    {
        for(n=0;n<iWidth;n++)
        {
            cha[m*iWidth+n]=0;
            for(i=0;i<iChannel;i++)
            {
                cha[m*iWidth+n]+=fog[m*iWidth*iChannel+n*iChannel+i];
            }
            inten[m*iWidth+n]	=	cha[m*iWidth+n]/3;//inten(m,n)=mean(I(m,n,:))
			cha[m*iWidth+n]		=	fabs( inten[m*iWidth+n]-atomsLight[0] );
			//cha=abs(inten-atmosLight(1));  
			alpha2[m*iWidth+n]	=	min( max( (float) k/(cha[m*iWidth+n]*255.0f),1.0f)*max( (float)filtered[m*iWidth+n],0.1), 1);
			//alpha2=min(max(k./cha,1.0).*max(filtered,0.1),1);
        }
    }

    
	for(i=0;i<iHeight;i++)
    {
        for(j=0;j<iWidth;j++)
        {
			//t0[i*iWidth+j]		=	0.1f;
			tDown[i*iWidth+j]	=	max(0.1f,alpha2[i*iWidth+j]);
			//t0=ones(m,n)*0.1;tDown=max(t0,alpha2);
		}
	}
	for(i=0;i<iHeight;i++)
    {
        for(j=0;j<iWidth;j++)
        {
            for(n=0;n<iChannel;n++)
            {
                clear[(i*iWidth+j)*iChannel+n]		=
                (fog[i*iWidth*iChannel+j*iChannel+n]-atomsLight[n])/tDown[i*iWidth+j]+atomsLight[n];
            }
        }
    }


    for(i=0;i<iHeight;i++)
    {
        for(j=0;j<iWidth;j++)
        {
            for(n=0;n<iChannel;n++)
            {
                deHaze[(i*iWidth+j)*iChannel+n]	=	round_me( clear[(i*iWidth+j)*iChannel+n]*255 );
            }
        }
    }

	end = clock();
	printf("4.DeHaze time is %dms.\n",end-start);

	
	for(i=0;i<iHeight;i++){
		for(j=0;j<iWidth;j++){
			for(n=0;n<iChannel;n++){
				img.data[(i*iWidth+j)*iChannel+n]	= deHaze[(i*iWidth+j)*iChannel+n];//I_ori[(i*iWidth+j)*iChannel+n];           
			}
		}
	}
    
    return 0;
}

//boxfilter called by guidefilter
//radius=radius*5;
void boxfilter( float *src, float *dst, int height, int width, int radius)
{
    float *imCum;
    int i,j;
    
    imCum=(float *)malloc(width*height*sizeof(float));
    assert(imCum!=NULL);
    //cumulative sum over Y axis ???
    for(i=0;i<width;i++)
    {
        imCum[i] = src[i];//First row copy form source
        for(j=1;j<height;j++)
        {
            imCum[j*width+i] = imCum[(j-1)*width+i]+src[j*width+i];
        }
    }
    //difference over Y axis
    for(i=0;i<=height-1;i++)
    {
        if(i<=radius)
        {
            for(j=0;j<width;j++)
            {
                dst[i*width+j] = imCum[(i+radius)*width+j];
            }
        }
        else if(i<=height-radius-1)//r+1:height-r
        {
            for(j=0;j<width;j++)
            {
                dst[i*width+j] = imCum[(i+radius)*width+j]-imCum[(i-radius)*width+j];
            }
        }
        else//height-r:height
        {
            for(j=0;j<width;j++)
            {
                dst[i*width+j] = imCum[(height-1)*width+j]-imCum[(i-radius)*width+j];
            }
        }
    }
    
    //cumulative sum over X axis ???
    for(i=0;i<height;i++)
    {
        imCum[i*width] = dst[i*width];//First row copy form dst
        for(j=1;j<width;j++)
        {
            imCum[i*width+j] = imCum[i*width+j-1]+dst[i*width+j];
        }
    }
    //difference over X axis
    for(i=0;i<=width-1;i++)
    {
        if(i<=radius)
        {
            for(j=0;j<height;j++)
            {
                dst[j*width+i] = imCum[j*width+i+radius];
            }
        }
        else if(i<=width-radius-1)//r+1:height-r
        {
            for(j=0;j<height;j++)
            {
                dst[j*width+i] = imCum[j*width+i+radius]-imCum[j*width+i-radius];
            }
        }
        else//height-r:height
        {
            for(j=0;j<height;j++)
            {
                dst[j*width+i] = imCum[j*width+width-1]-imCum[j*width+i-radius];
            }
        }
    }
    //Debug dst
    
    free(imCum);imCum=NULL;
}

//guidefilter
void guidefilter(float *I, float *p, float *dst, int height, int width, int channel, int radius, float eps)
{
    float	*N;
    float	*One;
    float	*mean_I;
    float	*mean_p,*mean_Ip,*cov_Ip,*mean_II,*var_I,*a,*b,*mean_a,*mean_b;
    float	*box_temp;
    int		i,j;
    
    N		=	(float *)malloc(width*height*sizeof(float));
    One		=	(float *)malloc(width*height*sizeof(float));
    mean_I  =   (float *)malloc(width*height*sizeof(float));
    mean_p  =   (float *)malloc(width*height*sizeof(float));
    mean_Ip =   (float *)malloc(width*height*sizeof(float));
    cov_Ip  =   (float *)malloc(width*height*sizeof(float));
    mean_II =   (float *)malloc(width*height*sizeof(float));
    var_I   =   (float *)malloc(width*height*sizeof(float));
    a       =   (float *)malloc(width*height*sizeof(float));
    b       =   (float *)malloc(width*height*sizeof(float));
    mean_a  =   (float *)malloc(width*height*sizeof(float));
    mean_b  =   (float *)malloc(width*height*sizeof(float));
    box_temp=   (float *)malloc(width*height*sizeof(float));
    assert(N		!=NULL);
    assert(One		!=NULL);
    assert(mean_I   !=NULL);
    assert(mean_p   !=NULL);
    assert(mean_Ip  !=NULL);
    assert(cov_Ip   !=NULL);
    assert(mean_II  !=NULL);
    assert(var_I    !=NULL);
    assert(a        !=NULL);
    assert(b        !=NULL);
    assert(mean_a   !=NULL);
    assert(mean_b   !=NULL);
    assert(box_temp   !=NULL);
    
    for(i=0;i<height;i++)//ones(height, width);
    {
        for(j=0;j<width;j++)
        {
            One[i*width+j] = 1;
        }
    }
    
    //N = boxfilter(ones(hei, wid), r);
    boxfilter( One, N, height, width, radius);
    
    //mean_I = boxfilter(I, r) ./ N;
    boxfilter( I, mean_I, height, width, radius );
    matrix_dot_divide(mean_I, N, mean_I, height, width );
    
    //mean_p = boxfilter(p, r) ./ N;
    boxfilter( p, mean_p, height, width, radius );
    matrix_dot_divide(mean_p, N, mean_p, height, width );
    
    //mean_Ip = boxfilter(I.*p, r) ./ N;
    matrix_dot_multiple( I, p, box_temp, height, width );
    boxfilter( box_temp, mean_Ip, height, width, radius );
    matrix_dot_divide( mean_Ip, N, mean_Ip, height, width );
    
    //cov_Ip = mean_Ip - mean_I .* mean_p;
    matrix_dot_multiple( mean_I, mean_p, box_temp, height, width );
    matrix_subtract( mean_Ip, box_temp, cov_Ip, height, width );
    
    //mean_II = boxfilter(I.*I, r) ./ N;
    matrix_dot_multiple( I, I, box_temp, height, width );
    boxfilter( box_temp, mean_II, height, width, radius );
	matrix_dot_divide( mean_II, N, mean_II, height, width );
    
    //var_I = mean_II - mean_I .* mean_I;
    matrix_dot_multiple( mean_I, mean_I, box_temp, height, width );
    matrix_subtract( mean_II, mean_I, var_I, height, width );
    
    //a = cov_Ip ./ (var_I + eps);
    matrix_add_all( var_I, var_I, height, width, eps );
    matrix_dot_divide( cov_Ip, var_I, a, height, width );
    
    //b = mean_p - a .* mean_I;
    matrix_dot_multiple( a, mean_I, box_temp, height, width );
    matrix_subtract( mean_p, box_temp, b, height, width );
    
    //mean_a = boxfilter(a, r) ./ N;
    boxfilter( a, box_temp, height, width, radius );
    matrix_dot_divide( box_temp, N, mean_a, height, width );
    
    //mean_b = boxfilter(b, r) ./ N;
    boxfilter( b, box_temp, height, width, radius );
    matrix_dot_divide( box_temp, N, mean_b, height, width );
    
    //q = mean_a .* I + mean_b;
    matrix_dot_multiple( mean_a, I, dst, height, width );
    matrix_add( dst, mean_b, dst, height, width );
    
    free(N		); N		=NULL;
    free(One	); One		=NULL;   
    free(mean_I ); mean_I   =NULL;
    free(mean_p ); mean_p   =NULL;
    free(mean_Ip); mean_Ip  =NULL;
    free(cov_Ip ); cov_Ip   =NULL;
    free(mean_II); mean_II  =NULL;
    free(var_I  ); var_I    =NULL;
    free(a      ); a        =NULL;
    free(b      ); b        =NULL;
    free(mean_a ); mean_a   =NULL;
    free(mean_b ); mean_b   =NULL;
    free(box_temp); box_temp=NULL;
}

