#include "stdafx.h"
#include "IOCP.h"
#include "ClientMainDlg.h"


CClientMainDlg *pDlg;

// IOCP

int SocketCount = 0;
//char host_IP[16];

WSADATA	wsaData;
HANDLE hCompletionPort;
SYSTEM_INFO SystemInfo;
SOCKADDR_IN servAddr;
LPPER_IO_DATA PerIoData;
LPPER_HANDLE_DATA PerHandleData;

SOCKET hServSock;
unsigned long RecvBytes;
unsigned long i, Flags;

S_DeviceInfo s_DeviceInfo;       // critical section �ʿ�?


// ------ Socket for send to Server -------
SOCKET hMasterSocket;
SOCKADDR_IN masterServAddr;
WSAEVENT wsaEvent;
WSABUF wsaBuf;
int sendByte = 0;
WSAOVERLAPPED overlapped;
bool isConnectedToServer;
CString server_IP;

DWORD tick_start = 0;
DWORD tick_end = 0;
bool b_tick = true;

DWORD WINAPI RecvResultData(LPVOID pParam)
{
	int ret = 0;
	char buf[128];
	int count = 0;

	while (1)
	{
		ret = recv(hMasterSocket, buf, 128, 0);
		
		if (ret == 128)
		{
			ResultData *rsltData;
			rsltData = (ResultData*)buf;
			
			// recv()�� ��� null���� �����ϱ� ������ �ݵ�� ������ ó���� �� �Ʒ����� ����Ǿ�� ��

			if (0 == strcmp(rsltData->client_IP, pDlg->g_Host_IP))
			{
				if (!b_tick)
				{
					tick_end = GetTickCount();
					int time_result = tick_end - tick_start;
					pDlg->AddMessage2(time_result);
					b_tick = true;
				}
				

				CString msg;
				msg.Format("%s", rsltData->data);
				pDlg->AddMessage(msg);
				count++;
				TRACE("RECV %d ] IP :%s , data : %s\n", count, rsltData->client_IP, rsltData->data);
			}
		}
	}
	return 0;
}

DWORD WINAPI Run(LPVOID pParam)
{
	pDlg = static_cast<CClientMainDlg*>(pParam);

	// device ����ü �ʱ�ȭ
	int p = 0;
	for (int p = 0; p < MAX_CONNECTION; p++)
	{
		s_DeviceInfo.status[p] = -1;  // disconnected
		s_DeviceInfo.ipAddr[p] = "";
		s_DeviceInfo.dev_Type[p] = -1;
		s_DeviceInfo.dev_Num[p] = -1;

		s_DeviceInfo.b_dev_Num[p] = false;
		s_DeviceInfo.b_dev_Type[p] = false;
		s_DeviceInfo.b_ipAddr[p] = false;
		s_DeviceInfo.b_status[p] = false;
	}


	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// host_IP ���������� ���� ȣ��Ʈ ������ �ּ� ����
	//SetHostIP();
	
	//1. Completion Port ����.
	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&SystemInfo);
	
	//2. Completion Port ���� ����� �ϷḦ ��� �ϴ� �����带 CPU ���� ��ŭ ����.
	for (i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
	{
		CreateThread(NULL, NULL, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);
	}

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(9000);

	bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	listen(hServSock, SOMAXCONN);


	while (TRUE)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);
		SocketCount++;
		pDlg->AddMessage("Client(Device) connected");
		
		// ���� ����
		int c_num = SetDeviceIP(inet_ntoa(clntAddr.sin_addr));
		if ((c_num > 30) || (c_num < 0))
			continue;
		 
		SetDeviceType(c_num, 0); // �� �κ� ���� �ʿ�
		SetDeviceStatus(c_num, 1);
		SetDeviceNumber(c_num, c_num);
		
		
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

int round1 = 0; //temp

FILE* fp;

DWORD WINAPI CompletionThread(LPVOID pComPort)
{
	HANDLE _hCompletionPort = (HANDLE)pComPort;
	DWORD _BytesTransferred;
	LPPER_HANDLE_DATA _PerHandleData;
	LPPER_IO_DATA _PerIoData;
	DWORD _flags;

	CString clntIP;
	int clntNum;
	

	char fileName[30];
	mkdir("data");

	char _data[BUFSIZE];
	FILE *fp;

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
		clntNum = FindDevice(clntIP);

		if (_BytesTransferred == 0) // ������ ������ ��
		{
			// ���� ������ ��� 4����Ʈ¥�� ������ ������ ���..
			char canceled_device[4];
			sprintf(canceled_device, "%d", GetDeviceNumber2(clntNum));
			wsaBuf.len = 4;
			wsaBuf.buf = canceled_device;
			WSASend(hMasterSocket, &wsaBuf, 1, (LPDWORD)&sendByte, 0, &overlapped, NULL);

			SetDeviceIP("", clntNum);  // �ִ��� ���������
			SetDeviceType(clntNum, -1);
			SetDeviceStatus(clntNum, -1);
			SetDeviceNumber(clntNum, -1);

			closesocket(_PerHandleData->hClntSock);
			SocketCount--;
			free(_PerHandleData);
			free(_PerIoData);
			
			pDlg->AddMessage("Client disconnected");

			continue;
		}
		else
		{
			SetDeviceStatus(clntNum, 2);
		}


		// RECEIVE AGAIN
		memset(&(_PerIoData->overlapped), 0, sizeof(OVERLAPPED));
		_PerIoData->wsaBuf.len = BUFSIZE;
		_PerIoData->wsaBuf.buf = _PerIoData->buffer;

		_flags = 0;
		int getSize = WSARecv(_PerHandleData->hClntSock,
			&(_PerIoData->wsaBuf),
			1,
			NULL,
			&_flags,
			&(_PerIoData->overlapped),
			NULL
			);
				

		// ������ ��ȯ & Master�� ����   // �޸� ���� ����

		if (isConnectedToServer && (_BytesTransferred == BUFSIZE))
		{
			// �����͸� ������ ������ ����ü ������ ������Ʈ �� ����
			ClientData* stData;
			//memset(&stData, 0x00, BUFSIZE);
			stData = (ClientData*)&_PerIoData->buffer;
			
			//�� �κ� ���� �ʿ�
			//int c_num = SetDeviceIP(inet_ntoa(_PerHandleData->clntAddr.sin_addr));
			//SetDeviceType(c_num, stData->device_Type);
			
			if (b_tick)
			{
				tick_start = GetTickCount();
				b_tick = false;
			}
			//int buf_Size = stData->data_Size;
			
			// �޸� ������ �ذ��Ϸ��� �õ��ߴ� ��
			//char *img_buf = new char[buf_Size];
			
			//memcpy(img_buf, stData->data, stData->data_Size);
			//img_buf[buf_Size] = '\0';

			//IplImage *image = cvCreateImage(cvSize(92, 112), IPL_DEPTH_8U, 1);
			//cvSetData(image, img_buf, 92);
			/*
			IplImage *image = cvCreateImage(cvSize(92, 112), IPL_DEPTH_8U, 1);
			memcpy(image->imageData, stData->data, 92 * 112 * sizeof(BYTE));
			
			char filename2[30];
			sprintf(filename2, "data/_%d.pgm", round1);
			cvSaveImage(filename2, image);
			*/

			
			//// ���丮�� �� �� ���� ���� ��
			//sprintf(fileName, "data/%d.pgm", round1);
			//fp = fopen(fileName, "wb");
			//fwrite(stData->data, sizeof(char), stData->data_Size, fp);
			
			
			//sprintf(fileName, "data/%d.pgm", _round);
			//cvSaveImage(fileName, img, NULL);
			
			round1++;
			
			//fclose(fp);
			//delete[]img_buf;
			//remove(fileName);
			
			stData->device_Number = clntNum;
			sprintf(stData->client_IP, "%s", pDlg->g_Host_IP);
			stData->socket_Count = SocketCount;
					
			wsaBuf.len = BUFSIZE;
			wsaBuf.buf = (char*)stData;
			
			if (SOCKET_ERROR == WSASend(hMasterSocket, &wsaBuf, 1, (LPDWORD)&sendByte, 0, &overlapped, NULL))
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					AfxMessageBox("WSASend (data to master) error");
				}
			}

			WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
			
		}
		/*
		else if (isConnectedToServer && (_BytesTransferred == 128))
		{
			// ��� ������ ����
			// �̰� IOCP�� �ȵ�.. hMasterSocket�� ���� ��������� 

			ResultData *rsltData;
			rsltData = (ResultData*)&_PerIoData->buffer;

			TRACE("recv IP : %s , data : %s\n", rsltData->client_IP, rsltData->data);

			memset(rsltData, 0x00, sizeof(rsltData));
		}
		*/
		
	}

	fclose(fp);
	return 0;
}


int ConnectToMaster(CString serv_ip)
{
	// ������ ó�� �������� ����
	try
	{
		/*
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			AfxMessageBox("WSA err");
			return FALSE;
		}

		hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (hSocket == INVALID_SOCKET)
		{
			AfxMessageBox("hSocket err");
			return FALSE;
		}
		*/

		///////////////



		/*
		// Master �������� �ʱ� Main client���� �ƴϸ� Slave server���� �Ǵ��ϱ� ���� ������ ����
		if ( send(hSocket, "client", 7, 0) == -1 )
		{
			AfxMessageBox("init sending error!");
			closesocket(hSocket);
			WSACleanup();
		}
		*/

		isConnectedToServer = TRUE;
	}
	catch (int a)
	{
		AfxMessageBox("Server Connection Error");
		return FALSE;
	}
		
	return TRUE;
}


void ErrorHandling(char *message)
{
	AfxMessageBox(message);
}

void SetHostIP()
{
	/*
	PHOSTENT hostInfo;
	char hostName[50];
	memset(hostName, 0, sizeof(hostName));
	int nError = gethostname(hostName, sizeof(hostName));
	hostInfo = gethostbyname(hostName);
	//pDlg->g_Host_IP = inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[0]);
	pDlg->g_Host_IP = "52.68.109.53";
	*/
}

// Get & Set, Check bool, Find DeviceData
#pragma region

int SetDeviceIP(CString ip, int index)
{
	// ���⼭ �����ϴ� index ���� �ĺ��ϴ� ���� ������ ����

	if (-1 == index)  // index�� ���� ��� �չ�ȣ �ڵ� �Ҵ�
	{
		int new_index = 0;

		for (new_index = 0; new_index < MAX_CONNECTION; new_index++)
		{
			if (s_DeviceInfo.ipAddr[new_index] == "")
				break;
		}

		if (new_index == MAX_CONNECTION) // �� ���� ����
		{
			return -1;
		}
		index = new_index;
	}
	
	s_DeviceInfo.ipAddr[index] = ip;
	s_DeviceInfo.b_ipAddr[index] = true;

	return index;
}
CString GetDeviceIP(int index)
{
	if (s_DeviceInfo.b_ipAddr[index] == false)
		return NULL;

	s_DeviceInfo.b_ipAddr[index] = false;
	return s_DeviceInfo.ipAddr[index];
}

void SetDeviceType(int index, int type)
{
	s_DeviceInfo.dev_Type[index] = type;
	s_DeviceInfo.b_dev_Type[index] = true;
}
int GetDeviceType(int index)
{
	if (s_DeviceInfo.b_dev_Type[index] == false)
		return NULL;
	
	s_DeviceInfo.b_dev_Type[index] = false;
	return s_DeviceInfo.dev_Type[index];
}

void SetDeviceNumber(int index, int num)
{
	s_DeviceInfo.dev_Num[index] = num;
	s_DeviceInfo.b_dev_Num[index] = true;
}
int GetDeviceNumber(int index)
{
	if(s_DeviceInfo.b_dev_Num[index] == false)
		return NULL;

	s_DeviceInfo.b_dev_Num[index] = false;
	return s_DeviceInfo.dev_Num[index];
}

void SetDeviceStatus(int index, int status)
{
	s_DeviceInfo.status[index] = status;
	s_DeviceInfo.b_status[index] = true;
}
int GetDeviceStatus(int index)
{
	if (s_DeviceInfo.b_status[index] == false)
		return NULL;

	s_DeviceInfo.b_status[index] = false;
	return s_DeviceInfo.status[index];
}

bool CheckDeviceIP(int index)
{
	return s_DeviceInfo.b_ipAddr[index];
}
bool CheckDeviceType(int index)
{
	return s_DeviceInfo.b_dev_Type[index];
}
bool CheckDeviceNum(int index)
{
	return s_DeviceInfo.b_dev_Num[index];
}
bool CheckDeviceStatus(int index)
{
	return s_DeviceInfo.b_status[index];
}

int FindDevice(CString ip)
{
	// IP �ּҸ� ����� ����� ����̽� �� index�� ã�� �̾Ƴ���.   -1�� ���ϵǸ� ����

	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		if (0 == strcmp(ip, s_DeviceInfo.ipAddr[i]))
		{
			return i;
		}
	}

	return -1;
}


int GetDeviceNumber2(int index) // temp
{
	return s_DeviceInfo.dev_Num[index];
}

#pragma endregion


// CString Ÿ���� char* Ÿ������ ��ȯ
/*
char* ip = new char[serv_ip.GetLength()];
strcpy(ip, serv_ip.GetBuffer());
*/

