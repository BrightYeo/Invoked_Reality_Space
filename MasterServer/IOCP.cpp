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

	//1. Completion Port ����.
	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&SystemInfo);

	//2. Completion Port ���� ����� �ϷḦ ��� �ϴ� �����带 CPU ����x2 ��ŭ ����.
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

		
		// ���� �� Ŭ���̾�Ʈ�� ���� �ڵ� ������ �ּ� ������ ����.
		PerHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		PerHandleData->hClntSock = hClntSock;
		memcpy(&(PerHandleData->clntAddr), &clntAddr, addrLen);

		//3. Overlapped ���ϰ� CompletionPort�� ����.
		CreateIoCompletionPort((HANDLE)hClntSock, hCompletionPort, (DWORD)PerHandleData, 0);

		// ���� �� Ŭ���̾�Ʈ�� ���� ���۸� ���� �ϰ� OVERLAPPED ����ü ���� �ʱ�ȭ.
		PerIoData = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = BUFSIZE;
		PerIoData->wsaBuf.buf = PerIoData->buffer;
		Flags = 0;

		//4. ��ø�� ������ �Է�.
		WSARecv(PerHandleData->hClntSock,	// ������ �Է� ����.
			&(PerIoData->wsaBuf),		// ������ �Է� ���� ������.
			1,							// ������ �Է� ������ ��.
			&RecvBytes,
			&Flags,
			&(PerIoData->overlapped),	// OVERLAPPED ����ü ������.
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

	CPUDATA* cpuData;  // slave�κ��� ���� cpu ���� ����ü
	ResultData *recvData;  // slave�κ��� ���� ��� ������ ����ü

	while (1)
	{
		// 5. ������� �Ϸ� �� ������ ���� ����. 
		GetQueuedCompletionStatus(_hCompletionPort,			 // Completion Port
			&_BytesTransferred,		 // ���� �� ����Ʈ ��
			(LPDWORD)&_PerHandleData,
			(LPOVERLAPPED*)&_PerIoData, // OVERLAPPED ����ü ������.
			INFINITE
			);

		// IP Copy
		clntIP.Format("%s", inet_ntoa(_PerHandleData->clntAddr.sin_addr));


		if (_BytesTransferred == 0) // ������ ������ ��
		{
			// ����ü���� ������ ���� �� ã�Ƽ� �ش��ϴ� �� erase
			for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
			{
				if (0 == strcmp(it->ip, clntIP))
				{
					sClientInfo.erase(it);
					pDlg->AddMessage("Client disconnected");
					// list ���� ���ҽ�Ű�� �� update message���� ó���Ұ���. ���⼭�� ����ü�� ����
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
			//printf("EOF �߻� \n");

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
			// Client�� device ���� ���� ��ȣ
			int cenceled_dev = atoi(_PerIoData->buffer);
						
			for (list<CLIENT_INFO>::iterator iter = sClientInfo.begin(); iter != sClientInfo.end(); iter++)
			{
				if (0 == strcmp(clntIP, iter->ip))
				{
					// client ���ŵ� ��� ã�Ƽ� client ����ü�� assgiend_slave ����Ʈ���� ����
					
					for (list<CLIENT_DEVICE>::iterator iter2 = iter->assigned_Slave.begin();
						iter2 != iter->assigned_Slave.end(); iter2++)
					{
						if (iter2->device_num == cenceled_dev)
						{
							// numJob 1 ����
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
			// ���ο� Client or Slave�� ����

			// slave ��ȣ �Ȱ�ġ����.. ���� ���� ���� ��ĭ ã�Ƽ� slveInfo.num �Ҵ�
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

			// Slave ���
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
			// client ��ȣ �Ȱ�ġ����.. ���� ���� ���� ��ĭ ã�Ƽ� slveInfo.num �Ҵ�
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

			// Client ���
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
			// Slave�� heart beat ����(cpu usage)

			// Slave�� CPU Usage ������Ʈ
			cpuData = (CPUDATA*)&_PerIoData->buffer;

			// � slave���� ã�Ƽ� ����ü ������ ������Ʈ
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
			// Slave�κ��� ��� ������ ���� �� 
			// ���� ������ ����ü �� ip ������ �����ؼ� 
			// ��� ������ ����ü ���� �� �ش� ip�� client���� �����͸� �������Ѵ�.
			
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
			// Client�κ��� ���� ������ ���� (11000 byte)

			ClientData* stData;
			stData = (ClientData*)&_PerIoData->buffer;
			
			if ((stData->device_Number > 30) || (stData->device_Number < 0))
			{
				continue;
			}

			// dev count�� ���� �Ҵ�� dev num�� slave �� ������Ʈ
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
			

			// job assign ���
			for (list<SLAVE_INFO>::iterator iter = sSlaveInfo.begin(); iter != sSlaveInfo.end(); iter++)
			{
				if (iter->cpuUsage <= 70)
				{
					// client�� ����̽��� �߰����� ��� client ����ü�� assigned_slave ������Ʈ
					list<CLIENT_INFO>::iterator iter2;
					for (iter2 = sClientInfo.begin(); iter2 != sClientInfo.end(); iter2++)
					{
						if (0 == strcmp(clntIP, iter2->ip))
						{
							//EnterCriticalSection(&cs);

							if (iter2->assigned_Slave.size() < stData->socket_Count)
							{
								// assigned_slave->device_number�� ��ġ�°� ���� �� �߰�
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

			// ���� ���
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
			// ������ ���� üũ
			ClientData* stData;
			stData = (ClientData*)_PerIoData->buffer;

			TRACE("rett %d, ������ ���� : %s %s\n", rett, stData->client_IP, stData->data.Bottombuf);

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