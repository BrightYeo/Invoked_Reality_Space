
// MasterServer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CMasterServerApp:
// �� Ŭ������ ������ ���ؼ��� MasterServer.cpp�� �����Ͻʽÿ�.
//

class CMasterServerApp : public CWinApp
{
public:
	CMasterServerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CMasterServerApp theApp;