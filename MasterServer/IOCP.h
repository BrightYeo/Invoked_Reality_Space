//
// IOCP �� ��� ���� ����ü �� �������� ����
//
#pragma once
#pragma warning (disable : 4996)
#pragma warning (disable : 4819)
#pragma warning (disable : 4018)

#include "stdafx.h"
#include <list>
#include <cv.h>

#define BUFSIZE 11000
#define RESULT_BUFSIZE 128

#define MAX_SLAVE 20
#define MAX_CLIENT 30

//#define MAX_CONNECTION 10  // ���� �ִ� ���� ��

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


// MEMO - CLIENT_INFO�� SLAVE_INFO ����ü
// sock, sockAddr�� ó�� init ������ ���� �� 1ȸ�� ������Ʈ (Client�κ��� ���� �����͸� �������ϱ� ����)
// ip�� ���������� ó�� init ������ ���� �� sockAddr���� �����͸� ������ 1ȸ�� ������Ʈ
// num�� �ߺ��Ǹ� �ȵȴ�. �������� ��ȸ�� ���� ������ ����
// status�� assigned_Slave�� �������� ������Ʈ�� �ʿ��ϸ�,
// assigned_Slave���� Ŭ���̾�Ʈ�� �� ����� �����͸� ó���ϴ� Slave�� ��ȣ�� �����Ѵ�.
// ex ) Ŭ���̾�Ʈ�� �� ���� ��� 1,2,3�� ������ �ְ�, ���� 1, 1, 2�� Slave���� �ش� �����͸� ó���Ѵٸ�
//      assigned_Slave�� [0] : 1,1   [1] : 2,1   [2] : 3,2 �� �ȴ�.


typedef struct
{
	int device_num;
	int slave_num;
} CLIENT_DEVICE;

typedef struct
{
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int num;  // ID
	char ip[16];
	int status; // 0 : disconnected   1 : connected   2: running
	int dev_count;  // ��� ���� ��� ����

	// Ŭ���̾�Ʈ�� �� ����̽� �����͸� ó���ϴ� Slave ��ȣ
	// ��� ��ȣ�� Slave ��ȣ�� ���� �̷��.
	std::list<CLIENT_DEVICE> assigned_Slave;

	void setIP() // sockAddr�� �ּҸ� ip�� ����
	{
		sprintf(ip, inet_ntoa(sockAddr.sin_addr));
	}
	
} CLIENT_INFO;

typedef struct
{
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	int num;
	char ip[16];
	int status;
	float cpuUsage;
	int numJob;  // job ����

	void setIP() // sockAddr�� �ּҸ� ip�� ����
	{
		sprintf(ip, inet_ntoa(sockAddr.sin_addr));
	}

} SLAVE_INFO;

typedef struct
{
	float cpuUsageValue;
} CPUDATA;




// Threads
extern DWORD WINAPI RunIOCP(LPVOID pParam);
extern DWORD WINAPI CompletionThread(LPVOID pComPort);

// List items
extern LVITEM list_item1;
extern LVITEM list_item2;

// Client , Slave Info (DB)
extern std::list<CLIENT_INFO> sClientInfo;
extern std::list<SLAVE_INFO> sSlaveInfo;


void SetStructData(struct ClientData* _stData);
void SetHostIP();