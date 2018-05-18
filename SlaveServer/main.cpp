#pragma warning(disable: 4819)

#include "stdafx.h"
#include "includes.h"
#include "Network.h"
#include "CPUUsage.h"
#include "FaceRecognition.h"

unsigned __stdcall SendCPUThread(LPVOID param);
unsigned __stdcall RecvThread(LPVOID param);


WSADATA wsaData;
SOCKET hSocket;
SOCKADDR_IN servAddr;

WSABUF sendDataBuf, recvDataBuf;
int sendBytes = 0;
int recvBytes = 0;
int flags = 0;

int _round = 0;

WSAEVENT event;
WSAOVERLAPPED overlapped;

DWORD WINAPI CompletionThread(LPVOID pComPort);
int SendData(char* data, int size);
CRITICAL_SECTION cs;

CPUDATA cpuData;

int main()
{

	CPUUsage cpuUsage;
	HANDLE hSendThread, hRecvThread, hSendCPUThread;

	char ip[16];
	printf("Input Master server IP : ");
	scanf("%s", ip);

	// temp
	if (0 == strcmp(ip, "a"))
	{
		sprintf(ip, "210.94.178.162");
	}
	
	InitializeCriticalSection(&cs);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(ip);
	servAddr.sin_port = htons(9001);

	if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	Sleep(300);

	if (-1 == send(hSocket, "slave", 6, 0))
	{
		printf("Init value send Error!!\n");
		closesocket(hSocket);
		WSACleanup();
		exit(0);
	}
	else
	{
		printf("Master server connected!\n");
	}

	Sleep(100);

	//구조체에 이벤트 핸들 삽입해서 전달
	//memset(&overlapped, 0, sizeof(overlapped));
	//overlapped.hEvent = event;

	hSendCPUThread = (HANDLE)_beginthreadex(NULL, 0, SendCPUThread, 0, 0, NULL);
	hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, 0, 0, NULL);
	
		
	while (1)
	{
		cpuData.cpuUsageValue = cpuUsage.usage_f();

		//printf("\rCurrent CPU load = %3.2f %% ", cpuUsageValue);
		
		Sleep(100);
	}

	//WaitForSingleObject(hSendThread, INFINITE);
	//WaitForSingleObject(hRecvThread, INFINITE);

	closesocket(hSocket);
	WSACleanup();

	return 0;
}



unsigned __stdcall RecvThread(LPVOID param)
{
	char buf[BUFSIZE];

	ClientData* stData;
	memset(&stData, 0x00, sizeof(stData));

	char fileName[30];
	//FILE *fp;
	mkdir("data");

	//
	// for face recognition
	//
	CvMat * trainPersonNumMat;  // the person numbers during training
	float * projectedTestFace;
	
	char cstr[256];
	BOOL saveNextFaces = FALSE;
	char newPersonName[256];
	int newPersonFaces;

	trainPersonNumMat = 0;  // the person numbers during training
	projectedTestFace = 0;
	saveNextFaces = FALSE;
	newPersonFaces = 0;

	printf("Recognizing person from pictures ...\n");

	// Load the previously saved training data
	if (loadTrainingData(&trainPersonNumMat)) {
		faceWidth = pAvgTrainImg->width;
		faceHeight = pAvgTrainImg->height;
	}
	else {
		printf("ERROR in recognizeFromCam(): Couldn't load the training data!\n");
		//exit(1);
	}

	projectedTestFace = (float *)cvAlloc(nEigens*sizeof(float));

	mkdir("data");

	/*
	faceCascade = (CvHaarClassifierCascade*)cvLoad(faceCascadeFilename, 0, 0, 0);
	if (!faceCascade) {
		printf("ERROR in recognizeFromCam(): Could not load Haar cascade Face detection classifier in '%s'.\n", faceCascadeFilename);
		exit(1);
	}
	*/
	
	//
	// BEGIN while()
	//
	while (true)
	{
		int ret = recv(hSocket, buf, BUFSIZE, 0);

		if (ret == BUFSIZE)
		{
			stData = (ClientData*)buf;
			
		//	char _dir[40];
		//	sprintf(_dir, "data/%s", stData->client_IP);
		//	mkdir(_dir);
			printf("%d] Recv : %d, %s \n", _round, ret, stData->client_IP);

			int iNearest, nearest, truth;
			IplImage *processedFaceImg = cvCreateImage(cvSize(92, 112), IPL_DEPTH_8U, 1);
			CvRect faceRect;
			int keyPressed = 0;
			FILE *trainFile;
			float confidence;


			if (stData->data_Size == 10)
			{
				// 'n' 실행
				newPersonFaces = 0;	// restart training a new person
				saveNextFaces = TRUE;
				strcpy(newPersonName, stData->data);
				continue;
			}
			else if (stData->data_Size == 20)
			{
				// 't' 실행

				// 같은 아이피면 train.txt와 facedata.xml을 공유해야 함
				// 다른 아이피일 경우 분리된 곳에 저장시켜야 함.

				saveNextFaces = FALSE;	// stop saving next faces.
				// Store the saved data into the training file.
				printf("Storing the training data for new person '%s'.\n", newPersonName);
				// Append the new person to the end of the training data.
				trainFile = fopen("train.txt", "a");
				for (int i = 0; i < newPersonFaces; i++)
				{
					sprintf(cstr, "data/%d_%s%d.pgm", nPersons + 1, newPersonName, i + 1);
					fprintf(trainFile, "%d %s %s\n", nPersons + 1, newPersonName, cstr);
				}
				fclose(trainFile);

				// Re-initialize the local data.
				projectedTestFace = 0;
				newPersonFaces = 0;

				// Retrain from the new database without shutting down.
				// Depending on the number of images in the training set and number of people, it might take 30 seconds or so.
				cvFree(&trainPersonNumMat);	// Free the previous data before getting new data
				trainPersonNumMat = retrainOnline();
				// Project the test images onto the PCA subspace
				cvFree(&projectedTestFace);	// Free the previous data before getting new data
				projectedTestFace = (float *)cvAlloc(nEigens*sizeof(float));

				printf("Recognizing person from pictures ...\n");
				continue;	// Begin with the next frame.
			}
			
			if (stData->data_Size > 1000 && stData->device_Type == 0)
			{
				if ( (stData->device_Number > 30) || (stData->device_Number < 0) )
				{
					continue;
				}
				// 받은 이미지 데이터 IplImage에 로드
				//processedFaceImg = cvCreateImage(cvSize(92, 112), IPL_DEPTH_8U, 1);
				cvZero(processedFaceImg);
				// 아래 memcpy 오류남..
				memcpy(processedFaceImg->imageData, stData->data + 14, stData->data_Size - 14);
			}
			else
			{
				continue;
			}


			if (saveNextFaces)
			{
				sprintf(cstr, "data/%d_%s%d.pgm", nPersons + 1, newPersonName, newPersonFaces + 1);
				//sprintf(cstr, "data/%s/%d_%s%d.pgm", stData->client_IP, nPersons + 1, newPersonName, newPersonFaces + 1);
				printf("Storing the current face of '%s' into image '%s'.\n", newPersonName, cstr);
				cvSaveImage(cstr, processedFaceImg, NULL);
				newPersonFaces++;
			}
			else
			{
				if (nEigens > 0)
				{
					cvEigenDecomposite(
						processedFaceImg,
						nEigens,
						eigenVectArr,
						0, 0,
						pAvgTrainImg,
						projectedTestFace);

					// Check which person it is most likely to be.
					iNearest = findNearestNeighbor(projectedTestFace, &confidence);
					nearest = trainPersonNumMat->data.i[iNearest];

					printf("Most likely person in camera: '%s' (confidence=%f.\n", personNames[nearest - 1].c_str(), confidence);

					char buf[128];
					ResultData rsltData;
					sprintf(rsltData.client_IP, "%s", stData->client_IP);
					rsltData.device_Number = stData->device_Number;
					rsltData.device_Type = stData->device_Type;
					rsltData.purpose = stData->purpose;
					rsltData.succeed = true;

					float rate = (float)(confidence * 100);
					sprintf(rsltData.data, "Device) [%d]  |  Purpose) : ""face recognition""  |  Result) : ""%s(%.2f%%)""", stData->device_Number ,personNames[nearest - 1].c_str(), rate);

					memcpy(buf, (char*)&rsltData, 128);
					SendData(buf, 128);
					
				}
			}


			//sprintf(fileName, "data/%d.pgm", _round);
			//cvSaveImage(fileName, image);
			
			
			

			
			//printf("sent. IP : %s, devnum : %d\n", rsltData.client_IP, rsltData.device_Number);

			//cvReleaseImage(&processedFaceImg);
			_round++;
			memset(&stData, 0x00, sizeof(stData));
			
		}

	}

	return 0;
}

// send data to master server
int SendData(char* data, int size)
{
	EnterCriticalSection(&cs);

	WSABUF buffer;

	buffer.len = size;
	buffer.buf = data;

	int ret = WSASend(hSocket, &buffer, 1, (LPDWORD)&sendBytes, 0, &overlapped, NULL);
	//printf("error : %d\n", WSAGetLastError());

	//전송 완료 확인
	WSAWaitForMultipleEvents(1, &event, TRUE, WSA_INFINITE, FALSE); //데이터 전송 끝났는지 확인. 

	LeaveCriticalSection(&cs);

	return ret;
}


// cpu usage 전송
unsigned __stdcall SendCPUThread(LPVOID param)
{
	while (true)
	{
		SendData((char*)&cpuData, 10);

		Sleep(200);
	}

	return 0;
}