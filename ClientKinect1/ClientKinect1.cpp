// ClientKinect1.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//
#include "stdafx.h"
#include "std.h"
#include "FaceRecognition.h"
#include "socket_func.h"

#pragma warning (disable : 4996)
#pragma comment(lib,"ws2_32.lib")

#define BUFSIZE 11000

// global
int Count = 0;
SOCKET mySocket;
char g_ClientIP[16];
char g_ServerIP[16];

// prototypes
unsigned int WINAPI SendData(void* arg);
unsigned int WINAPI RecvData(void* arg);
unsigned int WINAPI KinectSensingOn(void* arg);
void InitSocket(char* servAddr);
void SetStructData(struct ClientData* _stData);
void SetHostIP();

//
// Kinect data
//
HANDLE colorStreamHandle;
HANDLE nextColorFrameEvent;

//skeletonStream�� �̱� ���� �̺�Ʈ �ڵ鷯 ����
HANDLE skeletonStreamHandle;
HANDLE nextSkeletonFrameEvent;

HRESULT hr;// �ڵ鷯 �����

#pragma pack(push, 1)
struct ClientData
{
	char client_IP[16];											//(Ŭ���̾�Ʈ IP �ּ�)
	int device_Type;											//(��� ����)
	int device_Number;											//(��� ��ȣ)
	int purpose;
	int socket_Count;											// ����� ��� �� (client main���� ����)
	int data_Size;												//(�Ʒ� ������ ũ��)
	char data[10332];												//(������)
};
#pragma pack(pop)


int _tmain(int argc, _TCHAR* argv[])
{
	printf("                      ############################### \n"
		   "                      ###     Client - Device     ### \n"
		   "                      ###     Face recognition    ### \n"
		   "                      ############################### \n\n");
	printf("Input Main computer IP address : ");
	scanf("%s", g_ServerIP);

	if (0 == strcmp(g_ServerIP, "a"))
		sprintf(g_ServerIP, "210.94.178.162");

	printf("\n");

	//
	// BEGIN socket part
	// 
	// ���� �ʱ�ȭ
	InitSocket(g_ServerIP);
	
	// host ip ���� // ethernet ���� �ٸ� ��Ʈ��ũ �ּ� ���� ���� ����...
	SetHostIP();
	
	/*
	HANDLE hThread_Send, hThread_Recv;
	DWORD dwThreadID_Send = NULL, dwThreadID_Recv = NULL;

	hThread_Send = (HANDLE)_beginthreadex(NULL, 0, SendData, NULL, 0, (unsigned*)&dwThreadID_Send);
	if (hThread_Send == 0)
	{
		printf("send _beginthreadex Error!");
		return -1;
	}
	*/
	//
	// END socket part
	//

	//
	// BEGIN face recognition & data send
	//
	char buf[BUFSIZE];
	ClientData _stData;

	int cnt = 0; // sending count
	char fileName[128];

	int faceWidth = 92;	// Default dimensions for faces in the face recognition database.
	int faceHeight = 112;
	bool saveNextFaces = false;

	const char *faceCascadeFilename = "haarcascade_frontalface_alt.xml";
	CvHaarClassifierCascade* faceCascade;

	mkdir("data");
	//cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);

	faceCascade = (CvHaarClassifierCascade*)cvLoad(faceCascadeFilename, 0, 0, 0);
	if (!faceCascade) {
		printf("ERROR in recognizeFromCam(): Could not load Haar cascade Face detection classifier in '%s'.\n", faceCascadeFilename);
		exit(1);
	}

	FILE *fp;

	while (1)
	{
		Sleep(80);  // ������ ����

		IplImage *camImg = NULL;
		IplImage *greyImg = NULL;
		IplImage *faceImg = NULL;
		IplImage *sizedImg = NULL;
		IplImage *equalizedImg = NULL;
		IplImage *processedFaceImg = NULL;
		CvRect faceRect;
		IplImage *shownImg;
		int keyPressed = 0;

		// ���۹���, ����ü �ʱ�ȭ
		memset(buf, 0x00, BUFSIZE);
		memset(&_stData, 0x00, BUFSIZE);

		if (_kbhit())
			keyPressed = getch();

		//keyPressed = cvWaitKey(10);
		if (keyPressed == 'n')
		{
			char name[30];
			printf("Input name : ");
			scanf("%s", name);

			sprintf(_stData.client_IP, g_ClientIP);
			_stData.device_Number = 0;
			_stData.device_Type = 0;
			_stData.socket_Count = 0;
			_stData.purpose = 0;
			_stData.data_Size = 10;
			sprintf(_stData.data, "%s", name);
			
			saveNextFaces = true;

			// ������ ����ü�� ��� ����
			memcpy(buf, (char*)&_stData, sizeof(_stData));
			send(mySocket, buf, BUFSIZE, 0);
			cnt++;
			printf("TRAINING MODE SELECTED\n PRESS 'R' KEY TO STOP TRAINING\n", cnt);
		}
		else if (keyPressed == 't')
		{
			printf("START TRAINING............   ");

			sprintf(_stData.client_IP, g_ClientIP);
			_stData.device_Number = 0;
			_stData.device_Type = 0;
			_stData.socket_Count = 0;
			_stData.purpose = 0;
			_stData.data_Size = 20;
			sprintf(_stData.data, "%s", "null");

			saveNextFaces = false;

			// ������ ����ü�� ��� ����
			memcpy(buf, (char*)&_stData, sizeof(_stData));
			send(mySocket, buf, BUFSIZE, 0);
			cnt++;

			Sleep(15000);
			printf("TRAINING DONE.\n\n");
		}
		if (keyPressed == VK_ESCAPE)
		{
			break;	// Stop processing input.
		}
		else
		{
			camImg = getCameraFrame();
			if (!camImg) {
				printf("ERROR in recognizeFromCam(): Bad input image!\n");
				exit(1);
			}

			greyImg = convertImageToGreyscale(camImg);

			faceRect = detectFaceInImage(greyImg, faceCascade);

			if (faceRect.width > 0)
			{
				faceImg = cropImage(greyImg, faceRect);	// Get the detected face image.
				// Make sure the image is the same dimensions as the training images.
				sizedImg = resizeImage(faceImg, faceWidth, faceHeight);
				// Give the image a standard brightness and contrast, in case it was too dark or low contrast.
				equalizedImg = cvCreateImage(cvGetSize(sizedImg), 8, 1);	// Create an empty greyscale image
				cvEqualizeHist(sizedImg, equalizedImg);
				processedFaceImg = equalizedImg;

				if (!processedFaceImg)
				{
					printf("ERROR in recognizeFromCam(): Don't have input image!\n");
					exit(1);
				}
			}

			if (faceRect.width > 0)
			{
				sprintf(_stData.client_IP, g_ClientIP);
				_stData.device_Number = 0;
				_stData.device_Type = 0;
				_stData.socket_Count = 0;
				_stData.purpose = 0;
				_stData.data_Size = 10318;


				sprintf(fileName, "data/%d.pgm", cnt);
				cvSaveImage(fileName, processedFaceImg, NULL);

				fp = fopen(fileName, "rb");

				int fsize = 10304;
				fread(_stData.data, sizeof(unsigned char), fsize, fp);
				fclose(fp);
				
				remove(fileName); // delete file

				// ������ ����ü�� ��� ����
				memcpy(buf, (char*)&_stData, sizeof(_stData));
				send(mySocket, buf, BUFSIZE, 0);
				cnt++;
				printf("DATA SENT COUNT : %d\r", cnt);
			}

		}


		cvReleaseImage(&greyImg);
		cvReleaseImage(&faceImg);
		cvReleaseImage(&sizedImg);
		cvReleaseImage(&equalizedImg);
		/*
		// ȭ�鿡 �����ֱ�
		shownImg = cvCloneImage(camImg);
		if (faceRect.width > 0)  // Check if a face was detected.
		{
			cvRectangle(shownImg, cvPoint(faceRect.x, faceRect.y),
				        cvPoint(faceRect.x + faceRect.width - 1, faceRect.y + faceRect.height - 1),
				        CV_RGB(0, 255, 0), 1, 8, 0);
		}
		cvShowImage("Input", shownImg);
		cvReleaseImage(&shownImg);
		*/



	}
	cvReleaseCapture(&camera);
	cvReleaseHaarClassifierCascade(&faceCascade);
	WSACleanup();
	closesocket(mySocket);
	printf("\n\n�����մϴ�.\n\n");
	return 0;

}


void InitSocket(char* servAddr)
{
	Init_socket(&mySocket, SOCK_STREAM, FALSE, NULL, NULL);

	// ������ ���� ����
	SOCKADDR_IN serverAddr;
	set_sockaddrin(&serverAddr, 9000, FALSE, NULL, servAddr);

	// ����
	if (connect(mySocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		err_quit("���ӽ���");
	printf("����� %d�� \r", ++Count);

}


unsigned int WINAPI SendData(void* arg)
{
	
	return 0;
}


unsigned int WINAPI RecvData(void* arg)
{
	// �޼��� �޴´�
	char buf[15];
	int len;

	while (1)
	{
		len = recv(mySocket, buf, BUFSIZE, 0);
		if (len == -1)
			continue;
		else
			buf[len] = '\0';

		printf("���� �޼��� RECV : %s \r", buf);
	}

}


void SetHostIP()
{
	PHOSTENT hostInfo;
	char hostName[50];
	memset(hostName, 0, sizeof(hostName));
	memset(g_ClientIP, 0, sizeof(g_ClientIP));
	int nError = gethostname(hostName, sizeof(hostName));
	hostInfo = gethostbyname(hostName);
	strcpy(g_ClientIP, inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[0]));
}

