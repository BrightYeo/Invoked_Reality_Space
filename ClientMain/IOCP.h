#pragma once
#pragma warning (disable : 4996)
#pragma warning (disable : 4819)

#include "stdafx.h"
#include <conio.h>
#include <thread>

#include <string>
#include <vector>
#include <direct.h>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>


#define BUFSIZE 11000
#define RESULT_BUFSIZE 128

#define MAX_CONNECTION 20  // 장비들 최대 연결 수

#pragma pack(push, 1)
struct ClientData
{
	char client_IP[16];											//(클라이언트 IP 주소)
	int device_Type;											//(장비 종류)
	int device_Number;											//(장비 번호)
	int purpose;
	int socket_Count;											// 연결된 장비 수 (client main에서 사용됨)
	int data_Size;												//(아래 데이터 크기)
	char data[10332];											//(데이터)
};

// for recv back from server (결과데이터)
typedef struct
{
	char client_IP[16];
	int device_Type;
	int device_Number;
	int purpose;
	bool succeed;
	char data[80];
}ResultData;
#pragma pack(pop)

typedef struct //소켓 정보를 구조체화.
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct // 소켓의 버퍼 정보를 구조체화.
{
	OVERLAPPED overlapped;
	char buffer[BUFSIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;


// 클라이언트 내부의 연결된 장비들 정보
typedef struct
{
	// bool은 Dlg에서 재참조를 막기 위한 변수 (값이 변하기 전에는 계속 false 상태)
	bool b_ipAddr[MAX_CONNECTION];
	bool b_dev_Type[MAX_CONNECTION];
	bool b_dev_Num[MAX_CONNECTION];
	bool b_status[MAX_CONNECTION];

	CString ipAddr[MAX_CONNECTION];
	int dev_Type[MAX_CONNECTION];   //  2Dcam : 0    Kinect : 1    Leap : 2    MIC : 3
	int dev_Num[MAX_CONNECTION];
	int status[MAX_CONNECTION];   // 0 : disconnected    1 : connected    2 : running
	
}S_DeviceInfo;


DWORD WINAPI CompletionThread(LPVOID CompletionPortIO);
void ErrorHandling(char *message);
DWORD WINAPI Run(LPVOID pParam);
DWORD WINAPI RecvResultData(LPVOID pParam);
void SetHostIP();

// Getter, Setter
int SetDeviceIP(CString ip, int index = -1);
CString GetDeviceIP(int index);
void SetDeviceType(int index, int type);
int GetDeviceType(int index);
void SetDeviceNumber(int index, int num);
int GetDeviceNumber(int index);
void SetDeviceStatus(int index, int status);
int GetDeviceStatus(int index);

int GetDeviceNumber2(int index); // temp

bool CheckDeviceIP(int index);
bool CheckDeviceType(int index);
bool CheckDeviceNum(int index);
bool CheckDeviceStatus(int index);

int FindDevice(CString ip);

int ConnectToMaster(CString serv_ip);
extern bool isConnectedToServer;


// network variables

extern int SocketCount;
//extern char host_IP[16];

extern WSADATA wsaData;
extern HANDLE hCompletionPort;
extern SYSTEM_INFO SystemInfo;
extern SOCKADDR_IN servAddr;
extern LPPER_IO_DATA PerIoData;
extern LPPER_HANDLE_DATA PerHandleData;

extern SOCKET hServSock;
extern unsigned long RecvBytes;
extern unsigned long i, Flags;

// ------ Socket for send to Server -------
extern SOCKET hMasterSocket;
extern SOCKADDR_IN masterServAddr;
extern WSAEVENT wsaEvent;
extern WSABUF wsaBuf;
extern int sendByte;
extern WSAOVERLAPPED overlapped;
extern bool isConnectedToServer;
extern CString server_IP;