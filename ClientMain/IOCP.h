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

#define MAX_CONNECTION 20  // ���� �ִ� ���� ��

#pragma pack(push, 1)
struct ClientData
{
	char client_IP[16];											//(Ŭ���̾�Ʈ IP �ּ�)
	int device_Type;											//(��� ����)
	int device_Number;											//(��� ��ȣ)
	int purpose;
	int socket_Count;											// ����� ��� �� (client main���� ����)
	int data_Size;												//(�Ʒ� ������ ũ��)
	char data[10332];											//(������)
};

// for recv back from server (���������)
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

typedef struct //���� ������ ����üȭ.
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct // ������ ���� ������ ����üȭ.
{
	OVERLAPPED overlapped;
	char buffer[BUFSIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;


// Ŭ���̾�Ʈ ������ ����� ���� ����
typedef struct
{
	// bool�� Dlg���� �������� ���� ���� ���� (���� ���ϱ� ������ ��� false ����)
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