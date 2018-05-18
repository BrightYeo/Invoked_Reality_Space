//
// IOCP 및 통신 관련 구조체 및 프로토콜 정의
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

//#define MAX_CONNECTION 10  // 장비들 최대 연결 수

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


// MEMO - CLIENT_INFO와 SLAVE_INFO 구조체
// sock, sockAddr은 처음 init 데이터 수신 시 1회만 업데이트 (Client로부터 받은 데이터를 포워딩하기 위함)
// ip도 마찬가지로 처음 init 데이터 수신 후 sockAddr에서 데이터를 가져와 1회만 업데이트
// num은 중복되면 안된다. 지속적인 조회를 통해 고유값 유지
// status와 assigned_Slave는 지속적인 업데이트가 필요하며,
// assigned_Slave에는 클라이언트의 각 장비의 데이터를 처리하는 Slave의 번호를 저장한다.
// ex ) 클라이언트가 세 대의 장비 1,2,3을 가지고 있고, 각각 1, 1, 2번 Slave에서 해당 데이터를 처리한다면
//      assigned_Slave는 [0] : 1,1   [1] : 2,1   [2] : 3,2 가 된다.


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
	int dev_count;  // 사용 중인 장비 개수

	// 클라이언트의 각 디바이스 데이터를 처리하는 Slave 번호
	// 장비 번호와 Slave 번호가 쌍을 이룬다.
	std::list<CLIENT_DEVICE> assigned_Slave;

	void setIP() // sockAddr의 주소를 ip로 복사
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
	int numJob;  // job 개수

	void setIP() // sockAddr의 주소를 ip로 복사
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