
// ClientMainDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "ClientMain.h"
#include "ClientMainDlg.h"
#include "afxdialogex.h"

#define WM_GETDATA    WM_USER + 1

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CClientMainDlg ��ȭ ����



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


// CClientMainDlg �޽��� ó����

BOOL CClientMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�. 
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	h_IOCP = CreateThread(NULL, NULL, Run, this, 0, NULL);
	h_RECV = CreateThread(NULL, NULL, RecvResultData, this, 0, NULL);
	AddMessage("Network Initialized");

	AfxBeginThread(Thread_GetData, this);

	m_Combo_dev1.AddString("Face recognition");
	m_Combo_dev2.AddString("Face recognition");
	m_Combo_dev3.AddString("Face recognition");
	m_Combo_dev4.AddString("Face recognition");

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CClientMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}

	
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
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

// (�ӽ�)���� �� ������ ���� �ð��� Ȯ���ϱ� ����
void CClientMainDlg::AddMessage2(int msg)
{
	CString str_msg;
	str_msg.Format("%d", msg);
	m_Message_2.InsertString(m_Message_2.GetCount(), str_msg);
	m_Message_2.SetCurSel(m_Message_2.GetCount() - 1);
}



// UpdataData(TRUE) : ��Ʈ��(ȭ��) -> ����
// ~~ (FALSE) : ���� -> ��Ʈ��
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

		// �ؽ�Ʈ�ڽ��� ������ �ּҸ� �޾ƿ´�.
		
		/////////////////////// begin Master server�� ���� ////////////////////////////

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

		///////////////////////////end Master ������ ����///////////////////////////

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
	// ���������� IOCP ���� Getter��κ��� �����͸� ������ ������Ʈ �Ѵ�.
	// ������Ʈ �� ȭ���� text box�� ����� �� UpdataData(FALSE) ����

	// Dlg�� ��ü�� ������ (�Ʒ����� Ŭ���� ��� �Լ��� ����ϱ� ����)
	CClientMainDlg* pDlg = (CClientMainDlg*)pParam;

	CString m_Device_IP[MAX_CONNECTION];
	CString m_Device_Type[MAX_CONNECTION];
	CString m_Device_Status[MAX_CONNECTION];
	
	while (1)
	{
		// ��Ʈ�� ���� ����
		// device index���� ����ִ� �� Ȯ��
		// ����ִ� �Ϳ� ���ؼ��� get~() �ؼ� ��Ʈ�� ������ ����

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
					//AfxMessageBox("��� �ʱ�ȭ( status -1��)");
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

		//m_Device1_IP ����
		
		// ��Ʈ�� ���� ���� �ٽ� �ֱ�
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}
