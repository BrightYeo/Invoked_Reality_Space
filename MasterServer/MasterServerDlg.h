
// MasterServerDlg.h : 헤더 파일
//

#pragma once

#include "IOCP.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "resource.h"

#include <process.h>
#include <list>

using namespace std;

// CMasterServerDlg 대화 상자
class CMasterServerDlg : public CDialogEx
{
// 생성입니다.
public:
	CMasterServerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	~CMasterServerDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MASTERSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.



// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
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
