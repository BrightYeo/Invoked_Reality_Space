
// MasterServerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "MasterServer.h"
#include "MasterServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPDATEDATA    WM_USER + 1


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


// CMasterServerDlg ��ȭ ����


// ������
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


// CMasterServerDlg �޽��� ó����

BOOL CMasterServerDlg::OnInitDialog()
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



	// IOCP ������ ����
	h_IOCP = CreateThread(NULL, 0, RunIOCP, this, 0, NULL);

	// list control �� �߰�
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


	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CMasterServerDlg::OnPaint()
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
HCURSOR CMasterServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMasterServerDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	// begin �߰�
	// �� ����Ʈ �ڽ��� ���� ���� client/slave ����ü �������� ������ �� �� �߰�
	if (m_ListClient.GetItemCount() < sClientInfo.size())
	{
		// �� �� �߰�
		CreateListItem(0);

		// ����ü ����Ʈ�� ��ȸ�ؼ� ����Ʈ �ڽ��� �߰��� ������ �� �ߺ����� �ʴ� �� ã�Ƽ�
		// ���Ӱ� ����Ʈ�ڽ��� �߰��Ѵ�.
		for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
			if (-1 == ModifyListItem(0, it->ip, -1, "null", -1, "null", "null"))
				m_ListClient.SetItemText(m_ListClient.GetItemCount() - 1, 1, it->ip);
		
	}
	if (m_ListSlave.GetItemCount() < sSlaveInfo.size())
	{
		// ���� ����
		CreateListItem(1);
		
		for (list<SLAVE_INFO>::iterator it = sSlaveInfo.begin(); it != sSlaveInfo.end(); it++)
			if (-1 == ModifyListItem(1, it->ip, -1, "null", -1, "null", "null"))
				m_ListSlave.SetItemText(m_ListSlave.GetItemCount() - 1, 1, it->ip);
	}
	// end �߰�

	// begin ����
	// client/slave ����ü ������ �� ����Ʈ �ڽ��� ���� ������ ������ ã�Ƽ� ���� (���� ���� ���)
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
				//���� �� ã������ ����
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
				//����
				m_ListSlave.DeleteItem(i);
				list_item2.iItem--;
				break;
			}
			isFind = false;
		}
	}
	// end ����


	// begin ������Ʈ
	// client & slave list box item ������Ʈ (ip�� ��ġ�ϴ� ����Ʈ ã�Ƽ� ������ ������Ʈ)
	for (list<CLIENT_INFO>::iterator it = sClientInfo.begin(); it != sClientInfo.end(); it++)
	{
		CString dev_cnt = "";
		CString slv_num = "";
		
		// assigned_Slave�� ����ִ� �����͵��� "0 0 0 1" �� ���� ���·� ����
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
		// c1 : cpu usage(int)�� CString���� �ٲ�   c2 : number of job
		CString c1, c2;
		c1.Format("%03.2f %%", it->cpuUsage);
		c2.Format("%d", it->numJob);
		ModifyListItem(1, it->ip, it->num, it->ip, it->status, c1, c2);
	}
	// end ������Ʈ

	
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





// ����Ʈ�� �� �� �߰�. 
// �Ķ���Ͱ� 0�̸� client�� ,  1�̸� slave��
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

// ����Ʈ �������� ���� ���� (�ݵ�� CreateListItem() ���� �� �ε�Ǿ�� ��)
// position : 0�� client, 1�� slave
// ip : ������ �ּҸ� �Է��ϸ� �� ��° ���������� ã�� �� �ش� �����͸� ����
// param1~4 : client�� ���� num, IP, status, ��� ��, �Ҵ�� Slave��
//            slave��  ���� num, IP, status, CPU Usage
//            Ư�� �Ķ���� ���� "null"�� �Է��ϸ� �ش� ���� ���� ����(param1, param3, param4�� -1)
int CMasterServerDlg::ModifyListItem(int position, CString ip, int param1, CString param2, int param3, CString param4, CString param5)
{
	int seq = -1;

	// �Ķ���ͷ� ���� ip �ּҰ� ��ġ�ϴ� ����Ʈ�� �ִ��� ã�´�
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	
	if(!isON)
	{
		isON = true;
		UpdateData(TRUE);

		g_Network_Status = "Running";
		UpdateData(FALSE);
	}
	
}
