#pragma once

#include "stdafx.h"
#include "includes.h"
#include "StructData.h"

using namespace std;


extern const char *faceCascadeFilename;

extern int SAVE_EIGENFACE_IMAGES;		// Set to 0 if you dont want images of the Eigenvectors saved to files (for debugging).
//#define USE_MAHALANOBIS_DISTANCE	// You might get better recognition accuracy if you enable this.


// Global variables
extern IplImage ** faceImgArr; // array of face images
extern CvMat    *  personNumTruthMat; // array of person numbers
//#define	MAX_NAME_LENGTH 256		// Give each name a fixed size for easier code.
//char **personNames = 0;			// array of person names (indexed by the person number). Added by Shervin.
extern vector<string> personNames;			// array of person names (indexed by the person number). Added by Shervin.
extern int faceWidth;	// Default dimensions for faces in the face recognition database. Added by Shervin.
extern int faceHeight;	//	"		"		"		"		"		"		"		"
extern int nPersons; // the number of people in the training set. Added by Shervin.
extern int nTrainFaces; // the number of training images
extern int nEigens; // the number of eigenvalues
extern IplImage * pAvgTrainImg; // the average image
extern IplImage ** eigenVectArr; // eigenvectors
extern CvMat * eigenValMat; // eigenvalues
extern CvMat * projectedTrainFaceMat; // projected training faces

extern CvCapture* camera;	// The camera device.



// Function prototypes
void printUsage();
void learn(char *szFileTrain);
void doPCA();
void storeTrainingData();
int  loadTrainingData(CvMat ** pTrainPersonNumMat);
int  findNearestNeighbor(float * projectedTestFace);
int findNearestNeighbor(float * projectedTestFace, float *pConfidence);
int  loadFaceImgArray(char * filename);
void recognizeFileList(char *szFileTest);
void recognizeFromCam(void);
IplImage* getCameraFrame(void);
IplImage* convertImageToGreyscale(const IplImage *imageSrc);
IplImage* cropImage(const IplImage *img, const CvRect region);
IplImage* resizeImage(const IplImage *origImg, int newWidth, int newHeight);
IplImage* convertFloatImageToUcharImage(const IplImage *srcImg);
void saveFloatImage(const char *filename, const IplImage *srcImg);
CvRect detectFaceInImage(const IplImage *inputImg, const CvHaarClassifierCascade* cascade);
CvMat* retrainOnline(void);