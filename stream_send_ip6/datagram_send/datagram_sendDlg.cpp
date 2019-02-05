
// datagram_sendDlg.cpp : файл реализации
//

#include "stdafx.h"
#include "datagram_send.h"
#include "datagram_sendDlg.h"
#include "afxdialogex.h"

#include "blocksock.h"

#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Диалоговое окно CAboutDlg используется для описания сведений о приложении

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Данные диалогового окна
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	// Реализация
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


// диалоговое окно Cdatagram_sendDlg



Cdatagram_sendDlg::Cdatagram_sendDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cdatagram_sendDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	member_maximum_message_length = 500;
}

Cdatagram_sendDlg::~Cdatagram_sendDlg()
{
	WSACleanup();
}

void Cdatagram_sendDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ADDRESS, member_edit_address);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER, member_edit_port_number);
	DDX_Control(pDX, IDC_EDIT_THREADS_NUMBER, member_edit_threads_number);
	DDX_Control(pDX, IDC_EDIT_REQUESTS_NUMBER, member_edit_requests_number);
	DDX_Control(pDX, IDC_EDIT_MESSAGE_UNICODE_16, member_edit_message_unicode_16);

	DDX_Control(pDX, IDC_EDIT_ADDRESS_LOCAL, member_edit_address_local);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER_LOCAL, member_edit_port_number_local);

	DDX_Control(pDX, IDC_EDIT_MESSAGE_UNICODE_16_INCOMING, member_edit_message_unicode_16_incoming);

	DDX_Control(pDX, IDC_CHECK_UNSTOPPABLE, member_check_unstoppable);
	DDX_Control(pDX, IDC_EDIT_PAUSE, member_edit_pause);

	DDX_Control(pDX, IDC_CHECK_STOP_LISTEN, member_check_stop_listen);
}

BEGIN_MESSAGE_MAP(Cdatagram_sendDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_SEND, &Cdatagram_sendDlg::OnBnClickedButtonStartSend)
	ON_BN_CLICKED(IDC_BUTTON_START_LISTEN, &Cdatagram_sendDlg::OnBnClickedButtonStartListen)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_INPUT, &Cdatagram_sendDlg::OnBnClickedButtonClearInput)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// обработчики сообщений Cdatagram_sendDlg

BOOL Cdatagram_sendDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Добавление пункта "О программе..." в системное меню.

	// IDM_ABOUTBOX должен быть в пределах системной команды.
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

	// Задает значок для этого диалогового окна. Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	member_edit_address.SetWindowTextW(CString(L""));
	member_edit_threads_number.SetWindowTextW(CString(L""));

	WSADATA wsd;
	int local_result = WSAStartup(0x0202,&wsd);

	if(local_result!=NO_ERROR)
	{
		const int local_error_message_size = 10000;
		wchar_t local_error_message[local_error_message_size];

		const int local_system_error_message_size = local_error_message_size;
		wchar_t local_system_error_message[local_system_error_message_size];

		CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

		wcscpy(local_system_error_message,CString(L"Ошибка при запуске подсистемы сети."));

		wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

		::MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);

		return FALSE;
	}

	int optVal;
	int optLen = sizeof(int);

	CSockAddr local_socket_address(CStringA("localhost"),(USHORT)100); 
	CBlockingSocket local_socket;
	try
	{
		local_socket.Create(SOCK_STREAM, IPPROTO_IP);
//		local_socket.Create(SOCK_RAW, IPPROTO_RAW);
	}
	catch(CBlockingSocketException *local_blocking_socket_exception)
	{
		const int local_error_message_size = 10000;
		wchar_t local_error_message[local_error_message_size];

		const int local_system_error_message_size = local_error_message_size;
		wchar_t local_system_error_message[local_system_error_message_size];

		CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

		local_blocking_socket_exception->GetErrorMessage(local_system_error_message,local_system_error_message_size);

		wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

		local_blocking_socket_exception->Delete();

		::MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);

	}

	if(getsockopt(local_socket,SOL_SOCKET,SO_MAX_MSG_SIZE,(char*)&optVal,&optLen)==SOCKET_ERROR)
	{
		const int local_error_message_size = 10000;
		wchar_t local_error_message[local_error_message_size];

		const int local_system_error_message_size = local_error_message_size;
		wchar_t local_system_error_message[local_system_error_message_size];

		CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

		wcscpy(local_system_error_message,CString(L"Ошибка при получении максимального допустимого размера сообщения."));

		wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

		::MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);
	}

	local_socket.Close();

//	member_maximum_message_length = optVal;

	member_maximum_message_length = 10000;

	member_edit_pause.SetWindowTextW(CString(L"10"));

//	{
//		CString local_diagnostic_message;
//		local_diagnostic_message.Format(CString(L"member_maximum_message_length = %d"),member_maximum_message_length);
//		::MessageBox(0,local_diagnostic_message,CString(L"Diagnostic"),MB_ICONINFORMATION);
//	}

	SetTimer(100,500,NULL);

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void Cdatagram_sendDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок. Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void Cdatagram_sendDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR Cdatagram_sendDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT __cdecl datagram_send_connection_thread(LPVOID parameter);

int CodeToString(const LPBYTE code, const size_t code_len, CString *str);

struct thread_parameters_structure_type
{
	CString parameter_address;
	WORD parameter_port_number;
	WORD parameter_requests_number;
	CString parameter_message_unicode_16;
	USHORT parameter_maximum_message_length;

	CString parameter_address_local;
	WORD parameter_port_number_local;

	Cdatagram_sendDlg *parameter_main_dialog;
};

struct thread_parameters_receive_structure_type
{
	USHORT parameter_maximum_message_length;

	Cdatagram_sendDlg *parameter_main_dialog;

	CBlockingSocket *parameter_blocking_socket_accepted;
	CSockAddr *parameter_socket_address_accepted;
};

void Cdatagram_sendDlg::OnBnClickedButtonStartSend()
{
	CString local_string_address;
	CString local_string_port_number;
	CString local_string_threads_number;
	CString local_string_requests_number;
	CString local_string_message_unicode_16;

	member_edit_address.GetWindowTextW(local_string_address);
	member_edit_port_number.GetWindowTextW(local_string_port_number);
	member_edit_threads_number.GetWindowTextW(local_string_threads_number);
	member_edit_requests_number.GetWindowTextW(local_string_requests_number);
	member_edit_message_unicode_16.GetWindowTextW(local_string_message_unicode_16);

	WORD local_port_number = (WORD)_wtoi(local_string_port_number);
	int local_threads_number = _wtoi(local_string_threads_number);
	WORD local_requests_number = (WORD)_wtoi(local_string_requests_number);

	USHORT local_maximum_message_length = member_maximum_message_length;

	for(int local_int_threads_number_counter=0;local_int_threads_number_counter<local_threads_number;local_int_threads_number_counter++)
	{
		void *local_thread_parameters_structure = new thread_parameters_structure_type;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address = local_string_address;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number = local_port_number;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_requests_number = local_requests_number;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_message_unicode_16 = local_string_message_unicode_16;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_maximum_message_length = local_maximum_message_length;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address_local = CString();
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number_local = 0;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_main_dialog = this;

		CWinThread *local_thread = AfxBeginThread(datagram_send_connection_thread,local_thread_parameters_structure);
	}
}

bool domain_name_to_internet_6_name(CStringA domain_name, CStringA &internet_name);
bool domain_name_to_internet_name(CStringA domain_name, CStringA &internet_name);

UINT __cdecl datagram_send_connection_thread(LPVOID parameter)
{
	thread_parameters_structure_type *local_thread_parameters_structure = (thread_parameters_structure_type *)parameter;

	CString local_parameter_address = (local_thread_parameters_structure)->parameter_address;
	WORD local_parameter_port_number = (local_thread_parameters_structure)->parameter_port_number;
	WORD local_parameter_requests_number = (local_thread_parameters_structure)->parameter_requests_number;
	CString local_string_message_unicode_16 = (local_thread_parameters_structure)->parameter_message_unicode_16;
	USHORT local_maximum_message_length = (local_thread_parameters_structure)->parameter_maximum_message_length;

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure)->parameter_main_dialog;

	const size_t CONST_MESSAGE_LENGTH = local_maximum_message_length;
	const int CONST_WAIT_TIME = 10;

	char *local_message_to_send = new char[CONST_MESSAGE_LENGTH+2];

	if(local_message_to_send==NULL)
	{
		return 0;
	}

	for(;;)
	{

		for(WORD local_counter=0;local_counter<local_parameter_requests_number;local_counter++)
		{

			ZeroMemory(local_message_to_send,CONST_MESSAGE_LENGTH+2);

			/*/
			srand(100);

			for(size_t local_counter=0;local_counter<CONST_MESSAGE_LENGTH;local_counter++)
			{
			local_message[local_counter] = char(256.0*rand()/UINT(-1));
			}
			/*/

			size_t local_message_length_in_bytes = local_string_message_unicode_16.GetLength()*sizeof(wchar_t);
			size_t local_message_length_to_send_in_bytes_this_time = local_message_length_in_bytes;
			size_t local_message_length_to_send_in_bytes = local_message_length_in_bytes;

			size_t local_message_length_left_to_send_in_bytes = local_message_length_in_bytes;

			if(CONST_MESSAGE_LENGTH<local_message_length_to_send_in_bytes_this_time)
			{
				local_message_length_to_send_in_bytes_this_time = CONST_MESSAGE_LENGTH;
			}

			//			{
			//				CString local_diagnostic_message;
			//				local_diagnostic_message.Format(CString(L"local_message_length_to_send_in_bytes_this_time = %d"),local_message_length_to_send_in_bytes_this_time);
			//				MessageBox(0,local_diagnostic_message,CString(L"Diagnostic"),MB_ICONINFORMATION);
			//			}

			for(;local_message_length_left_to_send_in_bytes>0;)
			{
				size_t local_message_length_shift_from_beginning_in_wchar_t = (local_message_length_in_bytes-local_message_length_left_to_send_in_bytes)/sizeof(wchar_t);

				//				{
				//					CString local_diagnostic_message;
				//					local_diagnostic_message.Format(CString(L"local_message_length_shift_from_beginning = %d"),local_message_length_shift_from_beginning);
				//				MessageBox(0,local_diagnostic_message,CString(L"Diagnostic"),MB_ICONINFORMATION);
				//				}

				if(CONST_MESSAGE_LENGTH<local_message_length_left_to_send_in_bytes)
				{
					local_message_length_to_send_in_bytes_this_time = CONST_MESSAGE_LENGTH;
					local_message_length_left_to_send_in_bytes -= CONST_MESSAGE_LENGTH;
				}
				else
				{
					local_message_length_to_send_in_bytes_this_time = local_message_length_left_to_send_in_bytes;
					local_message_length_left_to_send_in_bytes = 0;
				}

				for(size_t local_counter=0;local_counter<local_message_length_to_send_in_bytes_this_time/sizeof(wchar_t);local_counter++)
				{
					((wchar_t*)local_message_to_send)[local_counter] = local_string_message_unicode_16[local_counter+local_message_length_shift_from_beginning_in_wchar_t];
				}

				//				{
				//					CString local_diagnostic_message;
				//					local_diagnostic_message.Format(CString(L"local_message_length_to_send_in_bytes_this_time = %d"),local_message_length_to_send_in_bytes_this_time);
				//					MessageBox(0,local_diagnostic_message,CString(L"Diagnostic"),MB_ICONINFORMATION);
				//				}

				CBlockingSocket local_blocking_socket;

				CStringA local_address;
	
				wchar_t *local_address_unicode = new wchar_t[CONST_MESSAGE_LENGTH+2];

				if(local_address_unicode==NULL)
				{
					break;
				}

				ZeroMemory(local_address_unicode,(CONST_MESSAGE_LENGTH+2)*sizeof(wchar_t));
					
				CStringA local_address_internet_address;
					
				if(IdnToAscii(0,local_parameter_address,local_parameter_address.GetLength(),local_address_unicode,CONST_MESSAGE_LENGTH)==0)
				{
					const int local_error_message_size = 10000;
					wchar_t local_error_message[local_error_message_size];

					const int local_system_error_message_size = local_error_message_size;
					wchar_t local_system_error_message[local_system_error_message_size];

					wcscpy(local_system_error_message,L"IdnToAscii завершилась неудачей");

					CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

					wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

					delete []local_address_unicode;

					MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);

					break;
				}

				for(size_t local_counter=0;local_counter<CONST_MESSAGE_LENGTH;local_counter++)
				{
					local_address += BYTE(local_address_unicode[local_counter]);
						
					if(local_address_unicode[local_counter]==L'\0')
					{
						break;
					}
				}


				if(domain_name_to_internet_6_name(local_address,local_address_internet_address)==false)
				{
					delete []local_address_unicode;
					
					break;
				}

				CSockAddr local_socket_address(local_address_internet_address,local_parameter_port_number);

				try
				{
					local_blocking_socket.Create(SOCK_STREAM, IPPROTO_IP);

					local_blocking_socket.Connect(local_socket_address);
				}
				catch(CBlockingSocketException *local_blocking_socket_exception)
				{
					const int local_error_message_size = 10000;
					wchar_t local_error_message[local_error_message_size];

					const int local_system_error_message_size = local_error_message_size;
					wchar_t local_system_error_message[local_system_error_message_size];

					CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

					local_blocking_socket_exception->GetErrorMessage(local_system_error_message,local_system_error_message_size);

					wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

					local_blocking_socket_exception->Delete();

					delete []local_address_unicode;

					delete[] local_message_to_send;

					delete local_thread_parameters_structure;

					local_blocking_socket.Close();

					MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);

					return 0;
				}

				int local_bytes_sent = 0;
				int local_error_number = 0;

				try
				{
					//					local_bytes_sent = local_blocking_socket.SendDatagram(local_message_to_send,local_message_length_to_send_in_bytes_this_time,local_socket_address,CONST_WAIT_TIME);
					local_bytes_sent = local_blocking_socket.Send(local_message_to_send,local_message_length_to_send_in_bytes_this_time,CONST_WAIT_TIME);
				}
				catch(CBlockingSocketException *local_blocking_socket_exception)
				{
					const int local_error_message_size = 10000;
					wchar_t local_error_message[local_error_message_size];

					const int local_system_error_message_size = local_error_message_size;
					wchar_t local_system_error_message[local_system_error_message_size];

					CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

					local_blocking_socket_exception->GetErrorMessage(local_system_error_message,local_system_error_message_size);

					wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

					local_error_number = local_blocking_socket_exception->GetErrorNumber();

					local_blocking_socket_exception->Delete();

					if(local_error_number!=0)
					{
						MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);
					}
				}

				local_blocking_socket.Close();

				if(local_bytes_sent==0 && local_error_number==0)
				{
					MessageBox(0,CString(L"local_bytes_sent==0"),CString(L"Error"),MB_ICONERROR);
				}
			}
		}

		if(local_main_dialog->member_check_unstoppable.GetCheck()>0)
		{
			CString local_string_pause;
			local_main_dialog->member_edit_pause.GetWindowTextW(local_string_pause);
			int local_pause_number = _wtoi(local_string_pause);
			Sleep(local_pause_number);
		}
		else
		{
			break;
		}
	}

	delete[] local_message_to_send;

	delete local_thread_parameters_structure;

	return 1;
}




bool domain_name_to_internet_6_name(CStringW domain_name, CStringA &internet_name)
{
//	DNS_STATUS
//WINAPI
//DnsQuery_W(
//    IN      PCWSTR          pszName,
//    IN      WORD            wType,
//    IN      DWORD           Options,                         
//    IN      PIP4_ARRAY      aipServers            OPTIONAL,
//    IN OUT  PDNS_RECORD *   ppQueryResults        OPTIONAL,
//    IN OUT  PVOID *         pReserved             OPTIONAL
//    );


	PDNS_RECORD   ppQueryResults;

	ZeroMemory(&ppQueryResults,sizeof(ppQueryResults));

	if(DnsQuery_W(domain_name, DNS_TYPE_AAAA, 0, NULL, &ppQueryResults,NULL)==ERROR_SUCCESS)
	{
		for(PDNS_RECORD ptr=ppQueryResults; ptr != NULL ;ptr=ptr->pNext)
		{
			if(ptr->wType==DNS_TYPE_AAAA)
			{
				if(ptr->wDataLength!=0)
				{
					char local_address_buffer[100];
					inet_ntop(AF_INET6,&ptr->Data.AAAA.Ip6Address.IP6Byte,local_address_buffer,100);
					internet_name = local_address_buffer;
				
					return true;
//					MessageBoxA(0,internet_name,CStringA("Information"),MB_ICONINFORMATION);
				}
			}
		}

		DnsFree(ppQueryResults,DnsFreeRecordList);

		return true;
	}

	return false;
}

bool domain_name_to_internet_6_name(CStringA domain_name, CStringA &internet_name)
{
	return domain_name_to_internet_6_name(CStringW(domain_name),internet_name);
}

bool domain_name_to_internet_name(CStringA domain_name, CStringA &internet_name)
{
	hostent *hostEnt;

	hostEnt = gethostbyname(domain_name);
	if (hostEnt!=NULL)
	{
		if(hostEnt->h_addrtype!=AF_INET)
		{
			return false;
		}

		if (hostEnt->h_addr_list!=NULL) 
		{
			for(int local_addresses_counter=0;local_addresses_counter<hostEnt->h_length;local_addresses_counter++)
			{
				if(hostEnt->h_addr_list[local_addresses_counter]==NULL)
				{
					return false;
				}

				{
					struct in_addr local_internet_name;

					local_internet_name.s_addr = *(u_long *) hostEnt->h_addr_list[local_addresses_counter];

					internet_name = inet_ntoa(local_internet_name);
					//					MessageBoxA(0,internet_name,CStringA("Information"),MB_ICONINFORMATION);
					return true;
				}
			}
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return false;
}

UINT __cdecl datagram_receive_connection_thread(LPVOID parameter);

void Cdatagram_sendDlg::OnBnClickedButtonStartListen()
{
	CString local_string_address_local;
	CString local_string_port_number_local;

	member_edit_address_local.GetWindowTextW(local_string_address_local);
	member_edit_port_number_local.GetWindowTextW(local_string_port_number_local);

	USHORT local_maximum_message_length = member_maximum_message_length;

	WORD local_port_number_local = (WORD)_wtoi(local_string_port_number_local);

	{
		void *local_thread_parameters_structure = new thread_parameters_structure_type;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address = CString();
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number = 0;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_requests_number = 0;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_message_unicode_16 = CString();
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_maximum_message_length = local_maximum_message_length;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address_local = local_string_address_local;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number_local = local_port_number_local;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_main_dialog = this;

		CWinThread *local_thread = AfxBeginThread(datagram_receive_connection_thread,local_thread_parameters_structure);
	}
}

const int MAXIMUM_TRY_NUMBER = 100;
const int TRY_PAUSE = 1;

UINT __cdecl datagram_receive_connection_thread_receive(LPVOID parameter)
{
	thread_parameters_receive_structure_type *local_thread_parameters_receive_structure = (thread_parameters_receive_structure_type *)parameter;

	if(local_thread_parameters_receive_structure==NULL)
	{
		MessageBox(0,CString(L"local_thread_parameters_receive_structure==NULL"),CString(L"Error"),MB_ICONERROR);
		return 0;
	}

	USHORT local_maximum_message_length = local_thread_parameters_receive_structure->parameter_maximum_message_length;

	Cdatagram_sendDlg *local_main_dialog = local_thread_parameters_receive_structure->parameter_main_dialog;

	if(local_main_dialog==NULL)
	{
		MessageBox(0,CString(L"local_main_dialog==NULL"),CString(L"Error"),MB_ICONERROR);
		return 0;
	}

	CBlockingSocket *local_blocking_socket_accepted = local_thread_parameters_receive_structure->parameter_blocking_socket_accepted;

	if(local_blocking_socket_accepted==NULL)
	{
		MessageBox(0,CString(L"local_blocking_socket_accepted==NULL"),CString(L"Error"),MB_ICONERROR);
		return 0;
	}

	CSockAddr *local_socket_address_accepted = local_thread_parameters_receive_structure->parameter_socket_address_accepted;

	if(local_socket_address_accepted==NULL)
	{
		MessageBox(0,CString(L"local_socket_address_accepted==NULL"),CString(L"Error"),MB_ICONERROR);
		return 0;
	}

	const int CONST_MESSAGE_LENGTH = local_maximum_message_length;
	const int CONST_WAIT_TIME_LISTEN = 10;

	char *local_message_to_receive = new char[CONST_MESSAGE_LENGTH+2];

	if(local_message_to_receive==NULL)
	{
		return 0;
	}

	ZeroMemory(local_message_to_receive,CONST_MESSAGE_LENGTH+2);

	int local_bytes_received = 0;

	CSockAddr local_socket_address_peer = *local_socket_address_accepted;

	for(int local_try_counter=0;local_try_counter<MAXIMUM_TRY_NUMBER;local_try_counter++)
	{
		int local_bytes_received_local = 0;
		try
		{
			local_bytes_received_local = local_blocking_socket_accepted->Receive(local_message_to_receive+local_bytes_received,CONST_MESSAGE_LENGTH-local_bytes_received,CONST_WAIT_TIME_LISTEN);
		}
		catch(CBlockingSocketException *local_blocking_socket_exception)
		{
			const int local_error_message_size = 10000;
			wchar_t local_error_message[local_error_message_size];

			const int local_system_error_message_size = local_error_message_size;
			wchar_t local_system_error_message[local_system_error_message_size];

			CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

			local_blocking_socket_exception->GetErrorMessage(local_system_error_message,local_system_error_message_size);

			wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

			int local_error_number = local_blocking_socket_exception->GetErrorNumber();

			local_blocking_socket_exception->Delete();

			if(local_error_number!=0)
			{
				MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);
			}
		}
	
		if(local_bytes_received_local<0)
		{
			break;
		}
		else
		{
			local_bytes_received += local_bytes_received_local;
		}

		if(local_bytes_received>=CONST_MESSAGE_LENGTH)
		{
			break;
		}

		Sleep(TRY_PAUSE);
	}

	CString local_received_message;

	if(local_bytes_received>0 && local_bytes_received<=CONST_MESSAGE_LENGTH)
	{
		local_received_message = (wchar_t*)local_message_to_receive;

		if(local_received_message.GetLength()*sizeof(wchar_t)!=local_bytes_received)
		{
			CString local_code_string;
			CodeToString((LPBYTE)local_message_to_receive,local_bytes_received,&local_code_string);
			
			local_received_message = local_code_string;
		}
	}


	bool local_wrong_input = false;
	if((local_bytes_received&1)!=0)
	{
		const int local_error_message_size = 10000;
		wchar_t local_error_message[local_error_message_size];

		const int local_system_error_message_size = local_error_message_size;
		wchar_t local_system_error_message[local_system_error_message_size];

		CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

		wcscpy(local_system_error_message,CString(L"Ошибка при получении сообщения: длина сообщения нечётная."));

		wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

		//			::MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);
		local_wrong_input = true;
	}

//	if(local_bytes_received<20 && local_bytes_received>0)
//	{
//		CString local_test;
//		local_test.Format(L"local_bytes_received = %d\n",local_bytes_received);
//		MessageBox(0,local_test+local_received_message,L"local_bytes_received<20 && local_bytes_received>0",0);
//	}

	if(local_received_message.GetLength()!=0)
	{
		CSingleLock local_single_lock(&local_main_dialog->member_critical_section);
		local_single_lock.Lock();

		//			::MessageBox(0,local_received_message,CString(L"Сообщение от ")+local_socket_address_peer.DottedDecimal(),MB_ICONINFORMATION);

		CString local_port_peer;
		local_port_peer.Format(CString(L"%d"),local_socket_address_peer.Port());

		CString local_new_line = CString(L"Сообщение от ")+local_socket_address_peer.DottedDecimal()+CString(L": ")+local_port_peer+CString(L": ")+local_received_message;

		//CString local_new_line = CString(L"Сообщение: ")+local_received_message;

		CString local_text_new = local_new_line;

		local_main_dialog->member_received_messages.push_back(local_text_new);

		if(local_wrong_input)
		{
			local_main_dialog->member_received_messages.push_back(CString(L"Неверная длина сообщения"));
		}

		local_single_lock.Unlock();
	}

	delete []local_message_to_receive;

	local_blocking_socket_accepted->Close();

	delete local_blocking_socket_accepted;
	delete local_socket_address_accepted;

	delete local_thread_parameters_receive_structure;

	return 1;
}

UINT __cdecl datagram_receive_connection_thread(LPVOID parameter)
{
	thread_parameters_structure_type *local_thread_parameters_structure = (thread_parameters_structure_type *)parameter;

	USHORT local_maximum_message_length = (local_thread_parameters_structure)->parameter_maximum_message_length;

	CString local_parameter_address_local = (local_thread_parameters_structure)->parameter_address_local;
	WORD local_parameter_port_number_local = (local_thread_parameters_structure)->parameter_port_number_local;

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure)->parameter_main_dialog;

	const int CONST_MESSAGE_LENGTH = local_maximum_message_length;
	const int CONST_WAIT_TIME_LISTEN = 10;

	CBlockingSocket local_blocking_socket;

	CStringA local_address;

	wchar_t *local_address_unicode = new wchar_t[CONST_MESSAGE_LENGTH+2];

	if(local_address_unicode==NULL)
	{
		delete local_thread_parameters_structure;

		return 0;
	}

	ZeroMemory(local_address_unicode,(CONST_MESSAGE_LENGTH+2)*sizeof(wchar_t));

	CStringA local_address_internet_address;

	if(IdnToAscii(0,local_parameter_address_local,local_parameter_address_local.GetLength(),local_address_unicode,CONST_MESSAGE_LENGTH)==0)
	{
		const int local_error_message_size = 10000;
		wchar_t local_error_message[local_error_message_size];

		const int local_system_error_message_size = local_error_message_size;
		wchar_t local_system_error_message[local_system_error_message_size];

		wcscpy(local_system_error_message,L"IdnToAscii завершилась неудачей");

		CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

		wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

		delete []local_address_unicode;

		delete local_thread_parameters_structure;

		MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);

		return 0;
	}

	for(int local_counter=0;local_counter<CONST_MESSAGE_LENGTH;local_counter++)
	{
		local_address += BYTE(local_address_unicode[local_counter]);

		if(local_address_unicode[local_counter]==L'\0')
		{
			break;
		}
	}

	if(domain_name_to_internet_6_name(local_address,local_address_internet_address)==false)
	{
		delete []local_address_unicode;

		delete local_thread_parameters_structure;

		return 0;
	}

	CSockAddr local_socket_address(local_address_internet_address,local_parameter_port_number_local);

	CBlockingSocket local_blocking_socket_accepted;
	CSockAddr local_socket_address_accepted;

	try
	{
		local_blocking_socket.Create(SOCK_STREAM, IPPROTO_IP);

		local_blocking_socket.Bind(local_socket_address);
		local_blocking_socket.Listen();
	}
	catch(CBlockingSocketException *local_blocking_socket_exception)
	{
		const int local_error_message_size = 10000;
		wchar_t local_error_message[local_error_message_size];

		const int local_system_error_message_size = local_error_message_size;
		wchar_t local_system_error_message[local_system_error_message_size];

		CString local_time_string = CTime::GetCurrentTime().FormatGmt("%d/%m/%y %H:%M:%S GMT");

		local_blocking_socket_exception->GetErrorMessage(local_system_error_message,local_system_error_message_size);

		wsprintf((wchar_t*)local_error_message, L"Сетевая ошибка -- %s -- %s\r\n", local_system_error_message, local_time_string.GetBuffer());

		local_blocking_socket_exception->Delete();

		delete []local_address_unicode;

		delete local_thread_parameters_structure;

		MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);

		return 0;
	}

	for(;;)
	{
		int local_check_stop_listen_state = local_main_dialog->member_check_stop_listen.GetCheck();

		if(local_check_stop_listen_state>0)
		{
			break;
		}

		CBlockingSocket *local_blocking_socket_accepted = new CBlockingSocket;
		CSockAddr *local_socket_address_accepted = new CSockAddr;

		local_blocking_socket.Accept(*local_blocking_socket_accepted,*local_socket_address_accepted);

		void *local_thread_parameters_receive_structure = new thread_parameters_receive_structure_type;

		((thread_parameters_receive_structure_type*)local_thread_parameters_receive_structure)->parameter_blocking_socket_accepted = local_blocking_socket_accepted;
		((thread_parameters_receive_structure_type*)local_thread_parameters_receive_structure)->parameter_socket_address_accepted = local_socket_address_accepted;
		((thread_parameters_receive_structure_type*)local_thread_parameters_receive_structure)->parameter_main_dialog = local_main_dialog;
		CWinThread *local_thread = AfxBeginThread(datagram_receive_connection_thread_receive,local_thread_parameters_receive_structure);
	}

	local_blocking_socket.Close();

	delete []local_address_unicode;

	delete local_thread_parameters_structure;

	return 1;
}

void Cdatagram_sendDlg::OnBnClickedButtonClearInput()
{
	CSingleLock local_single_lock(&member_critical_section);
	local_single_lock.Lock();
	
	member_received_messages.clear();

	local_single_lock.Unlock();
}


void Cdatagram_sendDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного

	if(nIDEvent==100)
	{
		CString local_window_text;

		CSingleLock local_single_lock(&member_critical_section);
		local_single_lock.Lock();

		for(std::list<CString>::iterator local_iterator=member_received_messages.begin();local_iterator!=member_received_messages.end();local_iterator++)
		{
			local_window_text += *local_iterator + CString(L"\r\n");
		}

		local_single_lock.Unlock();

		member_edit_message_unicode_16_incoming.SetWindowTextW(local_window_text);
	}

	CDialogEx::OnTimer(nIDEvent);
}

int CodeToString(const LPBYTE code, const size_t code_len, CString *str)
{
	str->Empty();
    BYTE *pcode = (BYTE *)code;
    for(size_t i = 0; i < code_len; i++)
	{
		CString local_string;
		local_string.Format(L"%02x ",*pcode),
        str->Append(local_string);
		pcode++;
    }
    return 0;
}


