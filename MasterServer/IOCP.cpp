#pragma once

#include "stdafx.h"
#include "IOCP.h"
#include "MasterServerDlg.h"

using namespace std;

DWORD WINAPI RunIOCP(LPVOID pParam);
DWORD WINAPI CompletionThread(LPVOID pComPort);

// List items
LVITEM list_item1 = { 0 };
LVITEM list_item2 = { 0 };

// Client , Slave Info (DB)
list<CLIENT_INFO> sClientInfo;
list<SLAVE_INFO> sSlaveInfo;

CMasterServerDlg *pDlg;
CRITICAL_SECTION cs;

DWORD WINAPI RunIOCP(LPVOID pParam)
{
	WSADATA	wsaData;
	HANDLE hCompletionPort;
	SYSTEM_INFO SystemInfo;
	SOCKADDR_IN servAddr;
	LPPER_IO_DATA PerIoData;
	LPPER_HANDLE_DATA PerHandleData;

	SOCKET hServSock;
	unsigned long RecvBytes;
	unsigned long Flags;

	pDlg = static_cast<CMasterServerDlg*>(pParam);
	InitializeCriticalSection(&cs);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
	{
		pDlg->ErrorHandling("WSAStartup() error!");
	}

	//SetHostIP();

	//1. Completion Port 생성.
	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&SystemInfo);

	//2. Completion Port 에서 입출력 완료를 대기 하는 쓰레드를 CPU 개수x2 만큼 생성.
	for (int i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
		CreateThread(NULL, NULL, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(9001);

	bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	listen(hServSock, SOMAXCONN);

	pDlg->g_Network_Status = "Running";


	while (TRUE)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);

		pDlg->AddMessage("Client/Slave connected");

		
		// 연결 된 클라이언트의 소켓 핸들 정보와 주소 정보를 설정.
		PerHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		PerHandleData->hClntSock = hClntSock;
		memcpy(&(PerHandleData->clntAddr), &clntAddr, addrLen);

		//3. Overlapped 소켓과 CompletionPort의 연결.
		CreateIoCompletionPort((HANDLE)hClntSock, hCompletionPort, (DWORD)PerHandleData, 0);

		// 연결 된 클라이언트를 위한 버퍼를 설정 하고 OVERLAPPED 구조체 변수 초기화.
		PerIoData = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = BUFSIZE;
		PerIoData->wsaBuf.buf = PerIoData->buffer;
		Flags = 0;

		//4. 중첩된 데이터 입력.
		WSARecv(PerHandleData->hClntSock,	// 데이터 입력 소켓.
			&(PerIoData->wsaBuf),		// 데이터 입력 버퍼 포인터.
			1,							// 데이터 입력 버퍼의 수.
			&RecvBytes,
			&Flags,
			&(PerIoData->overlapped),	// OVERLAPPED 구조체 포인터.
			NULL
			);
	}
	return 0;
}

int count_1 = 0;
int count_2 = 0;

DWORD WINAPI CompletionThread(LPVOID pComPort)
{
	HANDLE _hCompletionPort = (HANDLE)pComPort;
	DWORD _BytesTransferred;
	LPPER_HANDLE_DATA _PerHandleData;
	LPPER_IO_DATA _PerIoData;
	DWORD _flags;
	
	CString clntIP;

	CPUDATA* cpuData;  // slave로부터 받을 cpu 정보 구조체
	ResultData *recvData;  // slave로부터 받을 결과 데이터 구조체

	while (1)
	{
		// 5. 입출력이 완료 된 소켓의 정보 얻음. 
		GetQueuedCompletionStatus(_hCompletionPort,			 // Completion Port
			&_BytesTransferred,		 // 전송 된 바이트 수
			(LPDWORD)&_PerHandleData,
			(LPOVERLAPPED*)&_PerIoData, // OVERLAPPED 구조체 포인터.
			INFINITE
			);

		// IP Copy
		clntIP.Format("%s", inet_ntoa(_PerHandleData->clntAddr.sin_addr));


		if (_BytesTransferred == 0) // 접속이 끊겼을 때
		{
			// 구조체에서 아이피 같은 것 찾아서 해당하는 것 erase
			for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
			{
				if (0 == strcmp(it->ip, clntIP))
				{
					sClientInfo.erase(it);
					pDlg->AddMessage("Client disconnected");
					// list 갯수 감소시키는 건 update message에서 처리할것임. 여기서는 구조체만 삭제
					break;
				}
			}
			for (list<SLAVE_INFO>::iterator it = sSlaveInfo.begin(); it != sSlaveInfo.end(); it++)
			{
				if (0 == strcmp(it->ip, clntIP))
				{
					sSlaveInfo.erase(it);
					pDlg->AddMessage("Slave disconnected");
					break;
				}
			}

			closesocket(_PerHandleData->hClntSock);

			free(_PerHandleData);
			free(_PerIoData);
			//printf("EOF 발생 \n");

			continue;
		}


		// RECEIVE AGAIN
		memset(&(_PerIoData->overlapped), 0, sizeof(OVERLAPPED));
		_PerIoData->wsaBuf.len = BUFSIZE;
		_PerIoData->wsaBuf.buf = _PerIoData->buffer;

		_flags = 0;
		WSARecv(_PerHandleData->hClntSock,
			&(_PerIoData->wsaBuf),
			1,
			NULL,
			&_flags,
			&(_PerIoData->overlapped),
			NULL
			);

		//count_2++;
		//TRACE("recv byte : %d, count : %d IP : %s\n", _BytesTransferred, count_2, clntIP);


		// IP Copy
		clntIP.Format("%s", inet_ntoa(_PerHandleData->clntAddr.sin_addr));


		if (_BytesTransferred == 4)
		{
			// Client의 device 접속 끊김 신호
			int cenceled_dev = atoi(_PerIoData->buffer);
						
			for (list<CLIENT_INFO>::iterator iter = sClientInfo.begin(); iter != sClientInfo.end(); iter++)
			{
				if (0 == strcmp(clntIP, iter->ip))
				{
					// client 제거된 장비 찾아서 client 구조체의 assgiend_slave 리스트에서 제거
					
					for (list<CLIENT_DEVICE>::iterator iter2 = iter->assigned_Slave.begin();
						iter2 != iter->assigned_Slave.end(); iter2++)
					{
						if (iter2->device_num == cenceled_dev)
						{
							// numJob 1 감소
							for (list<SLAVE_INFO>::iterator iter3 = sSlaveInfo.begin();
								iter3 != sSlaveInfo.end(); iter3++)
							{
								if (iter2->slave_num == iter3->num)
								{
									iter3->numJob--;
									break;
								}
							}

							iter->assigned_Slave.erase(iter2);
							break;
						}
					}
					break;
				}
			}
			
		}
		else if (_BytesTransferred == 6)
		{
			// 새로운 Client or Slave의 접속

			// slave 번호 안겹치도록.. 가장 낮은 숫자 빈칸 찾아서 slveInfo.num 할당
			int slv_num = 0;
			bool b_out = false;
			for (int i = 0; i < MAX_SLAVE; i++)
			{
				for (list<SLAVE_INFO>::iterator it = sSlaveInfo.begin(); it != sSlaveInfo.end(); it++)
				{
					if (i != it->num)
					{
						slv_num = i;
						b_out = true;
						break;
					}
				}
				if (b_out)
					break;
			}

			// Slave 등록
			SLAVE_INFO slveInfo;
			slveInfo.num = slv_num;
			slveInfo.sock = _PerHandleData->hClntSock;
			slveInfo.sockAddr = _PerHandleData->clntAddr;
			slveInfo.setIP();
			slveInfo.status = 1;
			slveInfo.numJob = 0;

			sSlaveInfo.push_back(slveInfo);


		}
		else if (_BytesTransferred == 7)
		{
			// client 번호 안겹치도록.. 가장 낮은 숫자 빈칸 찾아서 slveInfo.num 할당
			int clnt_num = 0;
			bool b_out = false;
			for (int i = 0; i < MAX_SLAVE; i++)
			{
				for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
				{
					if (i != it->num)
					{
						clnt_num = i;
						b_out = true;
						break;
					}
				}
				if (b_out)
					break;
			}

			// Client 등록
			CLIENT_INFO clntInfo;
			clntInfo.num = clnt_num;
			clntInfo.sock = _PerHandleData->hClntSock;
			clntInfo.sockAddr = _PerHandleData->clntAddr;
			clntInfo.setIP();
			clntInfo.status = 1;
			clntInfo.dev_count = 0;

			sClientInfo.push_back(clntInfo);

		}
		else if (_BytesTransferred == 10)
		{
			// Slave의 heart beat 수신(cpu usage)

			// Slave의 CPU Usage 업데이트
			cpuData = (CPUDATA*)&_PerIoData->buffer;

			// 어떤 slave인지 찾아서 구조체 데이터 업데이트
			for (list<SLAVE_INFO>::iterator iter = sSlaveInfo.begin(); iter != sSlaveInfo.end(); iter++)
			{
				if (0 == strcmp(clntIP, iter->ip))
				{
					iter->status = 2; //running
					iter->cpuUsage = cpuData->cpuUsageValue;
				}
			}

			memset(cpuData, 0x00, sizeof(cpuData));
		}
		else if (_BytesTransferred == 128)
		{
			// Slave로부터 결과 데이터 수신 시 
			// 먼저 수신한 구조체 중 ip 변수를 참조해서 
			// 결과 데이터 구조체 만든 후 해당 ip의 client에게 데이터를 포워딩한다.
			
			recvData = (ResultData*)&_PerIoData->buffer;

			for (list<CLIENT_INFO>::iterator iter = sClientInfo.begin(); iter != sClientInfo.end(); iter++)
			{
				if (0 == strcmp(recvData->client_IP, iter->ip))
				{
					//pDlg->AddMessage(recvData->data);
					
					WSASend(iter->sock, &_PerIoData->wsaBuf, 1, NULL, 0, NULL, NULL);
					
					break;
				}
			}

			memset(recvData, 0x00, sizeof(recvData));
			
		}
		else if (_BytesTransferred == BUFSIZE)
		{
			// Client로부터 센서 데이터 수신 (11000 byte)

			ClientData* stData;
			stData = (ClientData*)&_PerIoData->buffer;
			
			if ((stData->device_Number > 30) || (stData->device_Number < 0))
			{
				continue;
			}

			// dev count에 따른 할당된 dev num과 slave 값 업데이트
			for (list<CLIENT_INFO>::iterator iter = sClientInfo.begin(); iter != sClientInfo.end(); iter++)
			{
				if (0 == strcmp(clntIP, iter->ip))
				{
					iter->dev_count = stData->socket_Count;
					break;
				}
			}

			count_1++;
			TRACE("sockcnt : %d, cnt : %d, recv IP : %s\n", stData->socket_Count, count_1, stData->client_IP);
			

			// job assign 모듈
			for (list<SLAVE_INFO>::iterator iter = sSlaveInfo.begin(); iter != sSlaveInfo.end(); iter++)
			{
				if (iter->cpuUsage <= 70)
				{
					// client의 디바이스가 추가됐을 경우 client 구조체의 assigned_slave 업데이트
					list<CLIENT_INFO>::iterator iter2;
					for (iter2 = sClientInfo.begin(); iter2 != sClientInfo.end(); iter2++)
					{
						if (0 == strcmp(clntIP, iter2->ip))
						{
							//EnterCriticalSection(&cs);

							if (iter2->assigned_Slave.size() < stData->socket_Count)
							{
								// assigned_slave->device_number가 겹치는게 없을 때 추가
								bool isExist = false;
								list<CLIENT_DEVICE>::iterator iter3;
								for (iter3 = iter2->assigned_Slave.begin();
									iter3 != iter2->assigned_Slave.end();
									iter3++)
								{
									if (iter3->device_num == stData->device_Number)
									{
										isExist = true;
										break;
									}
								}

								if (!isExist)
								{
									CLIENT_DEVICE clnt_Dvce;
									clnt_Dvce.device_num = stData->device_Number;
									clnt_Dvce.slave_num = iter->num;
									iter2->assigned_Slave.push_back(clnt_Dvce);

									iter->numJob++;
									pDlg->AddMessage("A Client's device is added");
								}
							}

							//LeaveCriticalSection(&cs);
						}
					}
				}
			}

			bool isSent = false;

			// 전송 모듈
			for (list<SLAVE_INFO>::iterator iter = sSlaveInfo.begin(); iter != sSlaveInfo.end(); iter++)
			{
				if (isSent)
					break;

				list<CLIENT_INFO>::iterator iter2;
				for (iter2 = sClientInfo.begin(); iter2 != sClientInfo.end(); iter2++)
				{
					if (0 == strcmp(clntIP, iter2->ip))
					{
						list<CLIENT_DEVICE>::iterator iter3;
						for (iter3 = iter2->assigned_Slave.begin();
							iter3 != iter2->assigned_Slave.end();
							iter3++)
						{
							if ((iter->num == iter3->slave_num) && (stData->device_Number == iter3->device_num))
							{
								EnterCriticalSection(&cs);

								WSASend(iter->sock, &_PerIoData->wsaBuf, 1, NULL, 0, NULL, NULL);
								TRACE("sent back to client : 1\n");
								isSent = true;
								
								LeaveCriticalSection(&cs);
								break;
							}
						}
						break;
					}
				}

			}

			/*
			// 데이터 수신 체크
			ClientData* stData;
			stData = (ClientData*)_PerIoData->buffer;

			TRACE("rett %d, 데이터 수신 : %s %s\n", rett, stData->client_IP, stData->data.Bottombuf);

			memset(stData, 0x00, sizeof(stData));
			*/
		}

	}
	return 0;
}
/*
void SetHostIP()
{
	
	PHOSTENT hostInfo;
	char hostName[50];
	memset(hostName, 0, sizeof(hostName));
	int nError = gethostname(hostName, sizeof(hostName));
	hostInfo = gethostbyname(hostName);
	//pDlg->g_Host_IP = inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[0]);
	pDlg->g_Host_IP = "52.69.177.44";
	
}
*/