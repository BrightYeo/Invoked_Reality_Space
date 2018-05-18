#pragma once

#define BUFSIZE 11000
#define RESULT_BUFSIZE 128
#include "includes.h"

#pragma pack(push, 1)
struct ClientData
{
	char client_IP[16];											//(클라이언트 IP 주소)
	int device_Type;											//(장비 종류)
	int device_Number;											//(장비 번호)
	int purpose;
	int socket_Count;											// 연결된 장비 수 (client main에서 사용됨)
	int data_Size;												//(아래 데이터 크기)
	char data[10332];												//(데이터)
};

// for send back to server (결과데이터)
typedef struct
{
	char client_IP[16];
	int device_Type;
	int device_Number;
	int purpose;
	bool succeed;
	char data[80];
} ResultData;
#pragma pack(pop)

// CPU Usage
typedef struct
{
	float cpuUsageValue;
} CPUDATA;

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
