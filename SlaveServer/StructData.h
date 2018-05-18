#pragma once

#define BUFSIZE 11000
#define RESULT_BUFSIZE 128
#include "includes.h"

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

// for send back to server (���������)
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
