
// ClientMain.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CClientMainApp:
// �� Ŭ������ ������ ���ؼ��� ClientMain.cpp�� �����Ͻʽÿ�.
//

class CClientMainApp : public CWinApp
{
public:
	CClientMainApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CClientMainApp theApp;