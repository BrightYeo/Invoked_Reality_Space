
// ClientMainDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "IOCP.h"
#include "resource.h"

#pragma comment(lib, "ws2_32.lib")

#define RECV_BUFSIZE 1024
#define SEND_BUFSIZE 128


DWORD WINAPI Run(void * arg);

// CClientMainDlg 대화 상자
class CClientMainDlg : public CDialogEx
{
// 생성입니다.
public:
	CClientMainDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CLIENTMAIN_DIALOG };

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
	
	static UINT Thread_GetData(LPVOID pParam);
	afx_msg LRESULT OnGetData(WPARAM wParam, LPARAM lParam);

public:


	afx_msg void ConnectToServer();
	void AddMessage(CString msg);
	void AddMessage(int msg);

	void AddMessage2(int msg);

	CString serverIP;
	
	HANDLE h_IOCP, h_RECV;
	CListBox m_Message;
	CString m_Device1_IP;
	CString m_Device2_IP;
	CString m_Device3_IP;
	CString m_Device4_IP;
	CString m_Device1_Type;
	CString m_Device2_Type;
	CString m_Device3_Type;
	CString m_Device4_Type;
	CString m_Device1_Status;
	CString m_Device2_Status;
	CString m_Device3_Status;
	CString m_Device4_Status;
	//afx_msg void OnBnClickedButton2();
	//afx_msg void OnBnClickedButton5();

	CComboBox m_Combo_dev1;
	CComboBox m_Combo_dev2;
	CComboBox m_Combo_dev3;
	CComboBox m_Combo_dev4;
	CString g_Host_IP;
	afx_msg void OnBnClickedButton2();
	CListBox m_Message_2;
};
