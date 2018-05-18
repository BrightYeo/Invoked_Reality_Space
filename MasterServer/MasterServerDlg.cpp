
// MasterServerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "MasterServer.h"
#include "MasterServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPDATEDATA    WM_USER + 1


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


// CMasterServerDlg 대화 상자


// 생성자
CMasterServerDlg::CMasterServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMasterServerDlg::IDD, pParent)
	, g_Network_Status(_T(""))
	, g_Host_IP(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	isON = FALSE;
}
CMasterServerDlg::~CMasterServerDlg()
{
}

void CMasterServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_ListClient);
	DDX_Control(pDX, IDC_LIST3, m_ListSlave);
	DDX_Control(pDX, IDC_LIST4, m_Message);
	DDX_Text(pDX, IDC_STATIC_IOCP_STATUS, g_Network_Status);
	DDX_Text(pDX, IDC_EDIT1, g_Host_IP);
}

BEGIN_MESSAGE_MAP(CMasterServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_UPDATEDATA, OnUpdateData)
	ON_BN_CLICKED(IDC_BUTTON1, &CMasterServerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CMasterServerDlg 메시지 처리기

BOOL CMasterServerDlg::OnInitDialog()
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



	// IOCP 스레드 시작
	h_IOCP = CreateThread(NULL, 0, RunIOCP, this, 0, NULL);

	// list control 열 추가
	m_ListClient.InsertColumn(0, "Num", NULL, 40);
	m_ListClient.InsertColumn(1, "IP Address", NULL, 120);
	m_ListClient.InsertColumn(2, "Status", NULL, 90);
	m_ListClient.InsertColumn(3, "Num of Device", NULL, 120);
	m_ListClient.InsertColumn(4, "Slave", NULL, 120);

	m_ListSlave.InsertColumn(0, "Num", NULL, 40);
	m_ListSlave.InsertColumn(1, "IP Address", NULL, 120);
	m_ListSlave.InsertColumn(2, "Status", NULL, 80);
	m_ListSlave.InsertColumn(3, "CPU Usage", NULL, 80);
	m_ListSlave.InsertColumn(4, "Num of Job", NULL, 80);


	list_item1.mask = LVIF_TEXT;
	list_item1.iItem = 0;

	list_item2.mask = LVIF_TEXT;
	list_item2.iItem = 0;
	
	AfxBeginThread(Thread_UpdateData, this);


	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMasterServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMasterServerDlg::OnPaint()
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
HCURSOR CMasterServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMasterServerDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	// begin 추가
	// 각 리스트 박스의 라인 수가 client/slave 구조체 개수보다 작으면 한 줄 추가
	if (m_ListClient.GetItemCount() < sClientInfo.size())
	{
		// 한 줄 추가
		CreateListItem(0);

		// 구조체 리스트를 조회해서 리스트 박스에 추가된 아이피 중 중복되지 않는 걸 찾아서
		// 새롭게 리스트박스에 추가한다.
		for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
			if (-1 == ModifyListItem(0, it->ip, -1, "null", -1, "null", "null"))
				m_ListClient.SetItemText(m_ListClient.GetItemCount() - 1, 1, it->ip);
		
	}
	if (m_ListSlave.GetItemCount() < sSlaveInfo.size())
	{
		// 위와 같음
		CreateListItem(1);
		
		for (list<SLAVE_INFO>::iterator it = sSlaveInfo.begin(); it != sSlaveInfo.end(); it++)
			if (-1 == ModifyListItem(1, it->ip, -1, "null", -1, "null", "null"))
				m_ListSlave.SetItemText(m_ListSlave.GetItemCount() - 1, 1, it->ip);
	}
	// end 추가

	// begin 삭제
	// client/slave 구조체 개수가 각 리스트 박스의 라인 수보다 작으면 찾아서 삭제 (연결 끊긴 경우)
	if (m_ListClient.GetItemCount() > sClientInfo.size())
	{
		for (int i = 0; i < m_ListClient.GetItemCount(); i++)
		{
			bool isFind = false;
			for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
			{
				if (0 == strcmp(m_ListClient.GetItemText(i, 1), it->ip))
				{
					isFind = true;
					break;
				}
			}
			if (!isFind)
			{
				//없는 놈 찾았으니 삭제
				m_ListClient.DeleteItem(i);
				list_item1.iItem--;
				break;
			}
			isFind = false;
		}
	}
	if (m_ListSlave.GetItemCount() > sSlaveInfo.size())
	{
		for (int i = 0; i < m_ListSlave.GetItemCount(); i++)
		{
			bool isFind = false;
			for (list<SLAVE_INFO>::iterator it = sSlaveInfo.begin(); it != sSlaveInfo.end(); it++)
			{
				if (0 == strcmp(m_ListSlave.GetItemText(i, 1), it->ip))
				{
					isFind = true;
					break;
				}
			}
			if (!isFind)
			{
				//삭제
				m_ListSlave.DeleteItem(i);
				list_item2.iItem--;
				break;
			}
			isFind = false;
		}
	}
	// end 삭제


	// begin 업데이트
	// client & slave list box item 업데이트 (ip가 일치하는 리스트 찾아서 모조리 업데이트)
	for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
	{
		CString dev_cnt = "";
		CString slv_num = "";
		
		// assigned_Slave에 들어있는 데이터들을 "0 0 0 1" 과 같은 형태로 추출
		for (list<CLIENT_DEVICE>::iterator it2 = it->assigned_Slave.begin(); it2 != it->assigned_Slave.end(); it2++)
		{
			CString c1, c2;
			
			c1.Format(" %d", it2->device_num);
			dev_cnt = dev_cnt + c1;
			c2.Format(" %d", it2->slave_num);
			slv_num = slv_num + c2;
		}

		ModifyListItem(0, it->ip, it->num, it->ip, it->status, dev_cnt, slv_num);
	}
	for (list<SLAVE_INFO>::iterator it = sSlaveInfo.begin(); it != sSlaveInfo.end(); it++)
	{
		// c1 : cpu usage(int)를 CString으로 바꿈   c2 : number of job
		CString c1, c2;
		c1.Format("%03.2f %%", it->cpuUsage);
		c2.Format("%d", it->numJob);
		ModifyListItem(1, it->ip, it->num, it->ip, it->status, c1, c2);
	}
	// end 업데이트

	
	UpdateData(FALSE);
	return 0;
}

UINT CMasterServerDlg::Thread_UpdateData(LPVOID pParam)
{
	CMasterServerDlg* pDlg = (CMasterServerDlg*)pParam;

	while (1)
	{
		if (pDlg->isON)
		{
			pDlg->SendMessage(WM_UPDATEDATA);
			Sleep(50);
		}
	}

	return 0;
}

void CMasterServerDlg::AddMessage(CString msg)
{
	m_Message.InsertString(m_Message.GetCount(), msg);
	m_Message.SetCurSel(m_Message.GetCount() - 1);
}
void CMasterServerDlg::AddMessage(int msg)
{
	CString str_msg;
	str_msg.Format("%d", msg);
	m_Message.InsertString(m_Message.GetCount(), str_msg);
	m_Message.SetCurSel(m_Message.GetCount() - 1);
}
void CMasterServerDlg::AddMessage(float msg)
{
	CString str_msg;
	str_msg.Format("%f", msg);
	m_Message.InsertString(m_Message.GetCount(), str_msg);
	m_Message.SetCurSel(m_Message.GetCount() - 1);
}





// 리스트에 한 줄 추가. 
// 파라미터가 0이면 client에 ,  1이면 slave에
void CMasterServerDlg::CreateListItem(int position)
{
	if (position == 0)
	{
		m_ListClient.InsertItem(&list_item1);
		list_item1.iItem++;
	}
	else
	{
		m_ListSlave.InsertItem(&list_item2);
		list_item2.iItem++;
	}
}

// 리스트 아이템의 내용 변경 (반드시 CreateListItem() 수행 후 로드되어야 함)
// position : 0은 client, 1은 slave
// ip : 아이피 주소를 입력하면 몇 번째 데이터인지 찾은 후 해당 데이터를 수정
// param1~4 : client는 각각 num, IP, status, 장비 수, 할당된 Slave들
//            slave는  각각 num, IP, status, CPU Usage
//            특정 파라미터 값에 "null"을 입력하면 해당 값은 변경 안함(param1, param3, param4는 -1)
int CMasterServerDlg::ModifyListItem(int position, CString ip, int param1, CString param2, int param3, CString param4, CString param5)
{
	int seq = -1;

	// 파라미터로 받은 ip 주소가 일치하는 리스트가 있는지 찾는다
	if (position == 0)  // Client
	{
		for (int i = 0; i < m_ListClient.GetItemCount(); i++)
		{
			if (0 == strcmp(ip, m_ListClient.GetItemText(i, 1)))
			{
				seq = i;
				break;
			}
		}
	}
	else if (position == 1) // Slave
	{
		for (int i = 0; i < m_ListSlave.GetItemCount(); i++)
		{
			if (0 == strcmp(ip, m_ListSlave.GetItemText(i, 1)))
			{
				seq = i;
				break;
			}
		}
	}

	if (seq == -1)
	{
		//AddMessage("Data not found.");
		return -1;
	}

	CString str_param1;
	str_param1.Format("%d", param1);

	CString str_param3;
	switch (param3)
	{
	case 0:  str_param3 = "Disconnected"; break;
	case 1:  str_param3 = "Connected"; break;
	case 2:  str_param3 = "Running"; break;
	default: break;
	}
	
	// modify data
	if (position == 0)
	{
		// client
		if (param1 != -1)
			m_ListClient.SetItemText(seq, 0, str_param1);
		if (param2 != "null")
			m_ListClient.SetItemText(seq, 1, param2);
		if (param3 != -1)
			m_ListClient.SetItemText(seq, 2, str_param3);
		if (param4 != "null")
			m_ListClient.SetItemText(seq, 3, param4);
		if (param5 != "null")
			m_ListClient.SetItemText(seq, 4, param5);
	}
	else
	{
		// slave
		if (param1 != -1)
			m_ListSlave.SetItemText(seq, 0, str_param1);
		if (param2 != "null")
			m_ListSlave.SetItemText(seq, 1, param2);
		if (param3 != -1)
			m_ListSlave.SetItemText(seq, 2, str_param3);
		if (param4 != "null")
			m_ListSlave.SetItemText(seq, 3, param4);
		if (param5 != "null")
			m_ListSlave.SetItemText(seq, 4, param5);
	}

	return 0;
}



void CMasterServerDlg::ErrorHandling(char *message)
{
	AddMessage(message);
	g_Network_Status = "Stopped";
	//fputs(message, stderr);
	//fputc('\n', stderr);
	//exit(1);
}


void CMasterServerDlg::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	if(!isON)
	{
		isON = true;
		UpdateData(TRUE);

		g_Network_Status = "Running";
		UpdateData(FALSE);
	}
	
}
