#pragma once

#pragma warning (disable : 4996)
#pragma warning (disable : 4819)


//#include<opencv\cv.h>
//#include<opencv\highgui.h>
//#include<opencv\cxcore.h>
//#include<opencv2\opencv.hpp>

#include <conio.h>
#include <string>
#include <vector>
#include <direct.h>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>

//#include<Windows.h>
//#include<NuiApi.h> //KinectSDK

#include <fstream> //파일입출력
#include <iostream> //파일입출력


using namespace std;
using namespace cv;

extern CvCapture* camera;	// The camera device.


IplImage* getCameraFrame(void);
IplImage* convertImageToGreyscale(const IplImage *imageSrc);
CvRect detectFaceInImage(const IplImage *inputImg, const CvHaarClassifierCascade* cascade);
IplImage* cropImage(const IplImage *img, const CvRect region);
IplImage* resizeImage(const IplImage *origImg, int newWidth, int newHeight);