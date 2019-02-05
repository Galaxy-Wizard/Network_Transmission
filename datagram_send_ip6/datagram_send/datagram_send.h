
// datagram_send.h : главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// Cdatagram_sendApp:
// О реализации данного класса см. datagram_send.cpp
//

class Cdatagram_sendApp : public CWinApp
{
public:
	Cdatagram_sendApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern Cdatagram_sendApp theApp;