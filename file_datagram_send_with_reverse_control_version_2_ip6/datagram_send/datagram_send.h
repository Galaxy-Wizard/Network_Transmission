
// datagram_send.h : ������� ���� ��������� ��� ���������� PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�������� stdafx.h �� ��������� ����� ����� � PCH"
#endif

#include "resource.h"		// �������� �������


// Cdatagram_sendApp:
// � ���������� ������� ������ ��. datagram_send.cpp
//

class Cdatagram_sendApp : public CWinApp
{
public:
	Cdatagram_sendApp();

// ���������������
public:
	virtual BOOL InitInstance();

// ����������

	DECLARE_MESSAGE_MAP()
};

extern Cdatagram_sendApp theApp;