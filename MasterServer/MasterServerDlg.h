
// MasterServerDlg.h : ��� ����
//

#pragma once

#include "IOCP.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "resource.h"

#include <process.h>
#include <list>

using namespace std;

// CMasterServerDlg ��ȭ ����
class CMasterServerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CMasterServerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	~CMasterServerDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_MASTERSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.



// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	LRESULT OnUpdateData(WPARAM wParam, LPARAM lParam);
public:
	static UINT Thread_UpdateData(LPVOID pParam);

	void AddMessage(CString msg);
	void AddMessage(int msg);
	void AddMessage(float msg);
	void CreateListItem(int position);
	int ModifyListItem(int position, CString ip, int param1, CString param2, int param3, CString param4, CString param5);


	// IOCP
	bool isON;
	HANDLE h_IOCP;
	HANDLE h_CompletionThread;

	int SocketCount = 0;


	void ErrorHandling(char *message);

	// ------ Socket for send to Slaves -------
	
	CListCtrl m_ListClient;
	CListCtrl m_ListSlave;
	CListBox m_Message;
	CString g_Network_Status;
	CString g_Host_IP;
	afx_msg void OnBnClickedButton1();
};
