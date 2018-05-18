
// ClientMainDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ClientMain.h"
#include "ClientMainDlg.h"
#include "afxdialogex.h"

#define WM_GETDATA    WM_USER + 1

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClientMainDlg 대화 상자



CClientMainDlg::CClientMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientMainDlg::IDD, pParent)
	, serverIP(_T(""))
	, m_Device1_IP(_T(""))
	, m_Device2_IP(_T(""))
	, m_Device3_IP(_T(""))
	, m_Device4_IP(_T(""))
	, m_Device1_Type(_T(""))
	, m_Device2_Type(_T(""))
	, m_Device3_Type(_T(""))
	, m_Device4_Type(_T(""))
	, m_Device1_Status(_T(""))
	, m_Device2_Status(_T(""))
	, m_Device3_Status(_T(""))
	, m_Device4_Status(_T(""))
	, g_Host_IP(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	serverIP = "210.94.178.162"; // temp
	
}

void CClientMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT11, serverIP);
	DDX_Control(pDX, IDC_LIST2, m_Message);

	DDX_Control(pDX, IDC_COMBO1, m_Combo_dev1);
	DDX_Control(pDX, IDC_COMBO2, m_Combo_dev2);
	DDX_Control(pDX, IDC_COMBO3, m_Combo_dev3);
	DDX_Control(pDX, IDC_COMBO4, m_Combo_dev4);

	DDX_Text(pDX, IDC_EDIT12, g_Host_IP);
	DDX_Control(pDX, IDC_LIST3, m_Message_2);
}

BEGIN_MESSAGE_MAP(CClientMainDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CClientMainDlg::ConnectToServer)
	ON_MESSAGE(WM_GETDATA, OnGetData)
	ON_BN_CLICKED(IDC_BUTTON2, &CClientMainDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CClientMainDlg 메시지 처리기

BOOL CClientMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다. 
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	h_IOCP = CreateThread(NULL, NULL, Run, this, 0, NULL);
	h_RECV = CreateThread(NULL, NULL, RecvResultData, this, 0, NULL);
	AddMessage("Network Initialized");

	AfxBeginThread(Thread_GetData, this);

	m_Combo_dev1.AddString("Face recognition");
	m_Combo_dev2.AddString("Face recognition");
	m_Combo_dev3.AddString("Face recognition");
	m_Combo_dev4.AddString("Face recognition");

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CClientMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CClientMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}

	
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CClientMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CClientMainDlg::AddMessage(CString msg)
{
	m_Message.InsertString(m_Message.GetCount(), msg);
	m_Message.SetCurSel(m_Message.GetCount()-1);
}
void CClientMainDlg::AddMessage(int msg)
{
	CString str_msg;
	str_msg.Format("%d", msg);
	m_Message.InsertString(m_Message.GetCount(), str_msg);
	m_Message.SetCurSel(m_Message.GetCount() - 1);
}

// (임시)전송 후 데이터 도착 시간을 확인하기 위함
void CClientMainDlg::AddMessage2(int msg)
{
	CString str_msg;
	str_msg.Format("%d", msg);
	m_Message_2.InsertString(m_Message_2.GetCount(), str_msg);
	m_Message_2.SetCurSel(m_Message_2.GetCount() - 1);
}



// UpdataData(TRUE) : 컨트롤(화면) -> 변수
// ~~ (FALSE) : 변수 -> 컨트롤
//


void CClientMainDlg::ConnectToServer()
{
	//UpdateData(TRUE);
	//CString ip;
	//ip.Format("%s", serverIP);
	//AfxMessageBox(serverIP);
	
	//AddMessage("aaaa");

//	IN_ADDR temp_addr;
//	temp_addr.S_un.S_addr = 765545822;
//	AfxMessageBox(inet_ntoa(temp_addr));


	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);

	if (!isConnectedToServer)
	{
		UpdateData(TRUE);

		// 텍스트박스의 아이피 주소를 받아온다.
		
		/////////////////////// begin Master server에 접속 ////////////////////////////

		hMasterSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (hMasterSocket == INVALID_SOCKET)
			AfxMessageBox("mastersocket error");

		memset(&masterServAddr, 0, sizeof(masterServAddr));
		masterServAddr.sin_family = AF_INET;
		masterServAddr.sin_addr.s_addr = inet_addr(serverIP);
		masterServAddr.sin_port = htons(9001);

		if (connect(hMasterSocket, (SOCKADDR*)&masterServAddr, sizeof(masterServAddr)) == SOCKET_ERROR)
			AddMessage("Connect Failed!");
		else
			AddMessage("Connected to server");

		wsaEvent = WSACreateEvent();
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = wsaEvent;

		Sleep(500);

		wsaBuf.len = strlen("client ");
		wsaBuf.buf = "client";

		if (SOCKET_ERROR == WSASend(hMasterSocket, &wsaBuf, 1, (LPDWORD)&sendByte, 0, &overlapped, NULL))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				AfxMessageBox("WSASend error");
				closesocket(hMasterSocket);
				WSACleanup();
				exit(0);
			}
		}

		WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);

		///////////////////////////end Master 서버에 접속///////////////////////////

		isConnectedToServer = TRUE;


	}
}


LRESULT CClientMainDlg::OnGetData(WPARAM wParam, LPARAM lParam)
{
	//UpdateData(FALSE);
	return 0;
}

UINT CClientMainDlg::Thread_GetData(LPVOID pParam)
{
	// 지속적으로 IOCP 내의 Getter들로부터 데이터를 가져와 업데이트 한다.
	// 업데이트 시 화면의 text box에 출력한 후 UpdataData(FALSE) 수행

	// Dlg의 객체를 가져옴 (아래에서 클래스 멤버 함수를 사용하기 위함)
	CClientMainDlg* pDlg = (CClientMainDlg*)pParam;

	CString m_Device_IP[MAX_CONNECTION];
	CString m_Device_Type[MAX_CONNECTION];
	CString m_Device_Status[MAX_CONNECTION];
	
	while (1)
	{
		// 컨트롤 변수 선언
		// device index들이 살아있는 지 확인
		// 살아있는 것에 대해서만 get~() 해서 컨트롤 변수에 삽입

		for (int i = 0; i < MAX_CONNECTION; i++)
		{

			if (TRUE == CheckDeviceIP(i))
			{
				m_Device_IP[i] = GetDeviceIP(i);  // get IP
			}

			if (TRUE == CheckDeviceType(i))
			{
				switch (GetDeviceType(i))  // get dev Type
				{
				case -1:
					m_Device_Type[i] = "";
					break;
				case 0:
					m_Device_Type[i] = "2D Camera";
					break;
				case 1:
					m_Device_Type[i] = "Kinect";
					break;
				case 2:
					m_Device_Type[i] = "Leap motion";
					break;
				case 3:
					m_Device_Type[i] = "Sound";
					break;
				default:
					break;
				}
			}


			if (TRUE == CheckDeviceStatus(i))
			{
				switch (GetDeviceStatus(i))  // get stat
				{
				case -1:
					//AfxMessageBox("장비 초기화( status -1됨)");
					m_Device_Status[i] = "";
					break;
				case 0:
					m_Device_Status[i] = "Disconnected";
					break;
				case 1:
					m_Device_Status[i] = "Connected";
					break;
				case 2:
					m_Device_Status[i] = "Running";
					break;
				default:
					break;
				}
			}
		}

		//m_Device1_IP 기존
		
		// 컨트롤 변수 값에 다시 넣기
		pDlg->SetDlgItemText(STATIC_1_IP, m_Device_IP[0]);
		pDlg->SetDlgItemText(STATIC_2_IP, m_Device_IP[1]);
		pDlg->SetDlgItemText(STATIC_3_IP, m_Device_IP[2]);
		pDlg->SetDlgItemText(STATIC_4_IP, m_Device_IP[3]);
		pDlg->SetDlgItemText(STATIC_1_TYPE, m_Device_Type[0]);
		pDlg->SetDlgItemText(STATIC_2_TYPE, m_Device_Type[1]);
		pDlg->SetDlgItemText(STATIC_3_TYPE, m_Device_Type[2]);
		pDlg->SetDlgItemText(STATIC_4_TYPE, m_Device_Type[3]);
		pDlg->SetDlgItemText(STATIC_1_STATUS, m_Device_Status[0]);
		pDlg->SetDlgItemText(STATIC_2_STATUS, m_Device_Status[1]);
		pDlg->SetDlgItemText(STATIC_3_STATUS, m_Device_Status[2]);
		pDlg->SetDlgItemText(STATIC_4_STATUS, m_Device_Status[3]);

		//pDlg->SendMessage(WM_GETDATA); // UpdateData(FALSE)
		Sleep(100);
	}
}

/*
int aaaa = 0;
void CClientMainDlg::OnBnClickedButton2()
{
	PHOSTENT hostInfo;
	char hostName[50];
	memset(hostName, 0, sizeof(hostName));
	int nError = gethostname(hostName, sizeof(hostName));
	hostInfo = gethostbyname(hostName);
	
	if (aaaa == 0)
		return;
	g_Host_IP = inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[--aaaa]);
	UpdateData(FALSE);
}


void CClientMainDlg::OnBnClickedButton5()
{
	PHOSTENT hostInfo;
	char hostName[50];
	memset(hostName, 0, sizeof(hostName));
	int nError = gethostname(hostName, sizeof(hostName));
	hostInfo = gethostbyname(hostName);

	//if (inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[++aaaa]) != NULL)
	g_Host_IP = inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[++aaaa]);
	UpdateData(FALSE);
}
*/

void CClientMainDlg::OnBnClickedButton2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}
