
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

UINT __cdecl datagram_send_connection_thread(LPVOID parameter);

int CodeToString(const LPBYTE code, const size_t code_len, CString *str);

struct thread_parameters_structure_type
{
	CString parameter_address;
	WORD parameter_port_number;
	WORD parameter_ports_number;
	WORD parameter_thread_number;
	WORD parameter_threads_number;
	WORD parameter_requests_number;
	CString parameter_file_name;
	USHORT parameter_maximum_message_length;

	CString parameter_address_local;
	WORD parameter_port_number_local;

	Cdatagram_sendDlg *parameter_main_dialog;
};

struct message_header_type
{
	WORD data_size;
	ULONGLONG data_offset_in_file;
	ULONGLONG file_size;
};

const size_t CONST_MESSAGE_HEADER_LENGTH = sizeof(message_header_type);

const UINT MINIMUM_SIZE = 1000;

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
	DDX_Control(pDX, IDC_EDIT_PORTS_NUMBER, member_edit_ports_number);
	DDX_Control(pDX, IDC_EDIT_THREADS_NUMBER, member_edit_threads_number);
	DDX_Control(pDX, IDC_EDIT_REQUESTS_NUMBER, member_edit_requests_number);
	DDX_Control(pDX, IDC_EDIT_FILE_NAME, member_edit_file_name);

	DDX_Control(pDX, IDC_EDIT_ADDRESS_LOCAL, member_edit_address_local);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER_LOCAL, member_edit_port_number_local);

	DDX_Control(pDX, IDC_EDIT_FILE_NAME_INCOMING, member_edit_file_name_incoming);

	DDX_Control(pDX, IDC_CHECK_UNSTOPPABLE, member_check_unstoppable);
	DDX_Control(pDX, IDC_EDIT_PAUSE, member_edit_pause);

	DDX_Control(pDX, IDC_CHECK_STOP_LISTEN, member_check_stop_listen);

	DDX_Control(pDX, IDC_EDIT_SEND_INFORMATION, member_send_information);
	DDX_Control(pDX, IDC_EDIT_RECEIVE_INFORMATION, member_receive_information);

}

BEGIN_MESSAGE_MAP(Cdatagram_sendDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_SEND, &Cdatagram_sendDlg::OnBnClickedButtonStartSend)
	ON_BN_CLICKED(IDC_BUTTON_START_LISTEN, &Cdatagram_sendDlg::OnBnClickedButtonStartListen)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_INPUT, &Cdatagram_sendDlg::OnBnClickedButtonClearInput)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FILE, &Cdatagram_sendDlg::OnBnClickedButtonSelectFile)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FILE_INCOMING, &Cdatagram_sendDlg::OnBnClickedButtonSelectFileIncoming)
	ON_EN_KILLFOCUS(IDC_EDIT_PORTS_NUMBER, &Cdatagram_sendDlg::OnEnKillfocusEditPortsNumber)
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

	member_edit_ports_number.SetWindowTextW(CString(L""));

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
		local_socket.Create(SOCK_DGRAM);
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

	member_maximum_message_length = 0;
	for(;size_t(member_maximum_message_length)<CONST_MESSAGE_HEADER_LENGTH;)
	{
		member_maximum_message_length += MINIMUM_SIZE;
	}

	for(;member_maximum_message_length<UINT(optVal);)
	{
		member_maximum_message_length += MINIMUM_SIZE;
	}

	if(member_maximum_message_length>=UINT(optVal))
	{
		member_maximum_message_length -= MINIMUM_SIZE;
	}

	member_edit_pause.SetWindowTextW(CString(L"100"));


	member_bytes_send = 0;
	member_bytes_received = 0;

	member_bytes_per_second_send = 0;
	member_bytes_per_second_received = 0;

	member_start_time_send = CTime();
	member_start_time_receive = CTime();


//	{
//		CString local_diagnostic_message;
//		local_diagnostic_message.Format(CString(L"member_maximum_message_length = %d"),member_maximum_message_length);
//		::MessageBox(0,local_diagnostic_message,CString(L"Diagnostic"),MB_ICONINFORMATION);
//	}

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


void Cdatagram_sendDlg::OnBnClickedButtonStartSend()
{
	CString local_string_address;
	CString local_string_port_number;
	CString local_string_ports_number;
	CString local_string_threads_number;
	CString local_string_requests_number;
	CString local_string_file_name;

	member_edit_address.GetWindowTextW(local_string_address);
	member_edit_port_number.GetWindowTextW(local_string_port_number);
	member_edit_ports_number.GetWindowTextW(local_string_ports_number);
	member_edit_threads_number.GetWindowTextW(local_string_threads_number);
	member_edit_requests_number.GetWindowTextW(local_string_requests_number);
	member_edit_file_name.GetWindowTextW(local_string_file_name);

	WORD local_port_number = (WORD)_wtoi(local_string_port_number);
	WORD local_ports_number = (WORD)_wtoi(local_string_ports_number);
	int local_threads_number = _wtoi(local_string_threads_number);
	WORD local_requests_number = (WORD)_wtoi(local_string_requests_number);


	local_threads_number = int(local_ports_number);

	USHORT local_maximum_message_length = USHORT(member_maximum_message_length);

	member_bytes_send = 0;
		
	member_bytes_per_second_send = 0;

	member_start_time_send = CTime::GetTickCount();

	for(int local_threads_number_counter=0;local_threads_number_counter<local_threads_number;local_threads_number_counter++)
	{
		void *local_thread_parameters_structure = new thread_parameters_structure_type;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address = local_string_address;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number = local_port_number;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_ports_number = local_ports_number;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_threads_number = local_threads_number;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_thread_number = local_threads_number_counter;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_requests_number = local_requests_number;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_file_name = local_string_file_name;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_maximum_message_length = local_maximum_message_length;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address_local = CString();
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number_local = 0;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_main_dialog = this;

		CWinThread *local_thread = AfxBeginThread(datagram_send_connection_thread,local_thread_parameters_structure);
	}
}

bool domain_name_to_internet_name(CStringA domain_name, CStringA &internet_name);

UINT __cdecl datagram_send_connection_thread(LPVOID parameter)
{
	thread_parameters_structure_type *local_thread_parameters_structure = (thread_parameters_structure_type *)parameter;

	CString local_parameter_address = (local_thread_parameters_structure)->parameter_address;
	WORD local_parameter_port_number = (local_thread_parameters_structure)->parameter_port_number;
	WORD local_parameter_ports_number = (local_thread_parameters_structure)->parameter_ports_number;
	WORD local_threads_number = (local_thread_parameters_structure)->parameter_threads_number;
	WORD local_threads_number_counter = (local_thread_parameters_structure)->parameter_thread_number;
	WORD local_parameter_requests_number = (local_thread_parameters_structure)->parameter_requests_number;
	CString local_string_file_name = (local_thread_parameters_structure)->parameter_file_name;
	USHORT local_maximum_message_length = (local_thread_parameters_structure)->parameter_maximum_message_length;

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure)->parameter_main_dialog;

	const size_t CONST_MESSAGE_LENGTH = local_maximum_message_length;
	const size_t CONST_MESSAGE_LENGTH_WITHOUT_HEADER = CONST_MESSAGE_LENGTH-CONST_MESSAGE_HEADER_LENGTH;
	const int CONST_WAIT_TIME = 10;

	char *local_message_to_send = new char[CONST_MESSAGE_LENGTH];

	if(local_message_to_send==NULL)
	{
		return 0;
	}

	ULONGLONG local_file_length = 0;

	{
		CFile local_file(local_string_file_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone);
		local_file_length = local_file.GetLength();
		local_file.Close();
	}

	for(;;)
	{
		
		for(WORD local_counter=0;local_counter<local_parameter_requests_number;local_counter++)
		{

			ZeroMemory(local_message_to_send,CONST_MESSAGE_LENGTH);

			ULONGLONG local_file_length_round = 0;
			for(;local_file_length_round<local_file_length;)
			{
				local_file_length_round += local_threads_number*CONST_MESSAGE_LENGTH_WITHOUT_HEADER;
			}

			ULONGLONG local_thread_data_to_send = local_file_length_round/local_threads_number;

			ULONGLONG local_thread_base_offset = local_thread_data_to_send*local_threads_number_counter;

			for(size_t local_current_packet_number=0;CONST_MESSAGE_LENGTH_WITHOUT_HEADER*local_current_packet_number<local_thread_data_to_send;local_current_packet_number++)
			{
				CFile local_file(local_string_file_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone);

				ULONGLONG local_data_base_offset_in_file = CONST_MESSAGE_LENGTH_WITHOUT_HEADER*local_current_packet_number;

				ULONGLONG local_data_offset_in_file = local_thread_base_offset + local_data_base_offset_in_file;

				WORD local_data_size = CONST_MESSAGE_LENGTH_WITHOUT_HEADER;


				local_file.Seek(local_data_offset_in_file,CFile::begin);
				local_data_size = local_file.Read(local_message_to_send+CONST_MESSAGE_HEADER_LENGTH,local_data_size);

				message_header_type local_message_header;

				ZeroMemory(&local_message_header,sizeof(message_header_type));


				local_message_header.data_size = local_data_size;
				local_message_header.data_offset_in_file = local_data_offset_in_file;
				local_message_header.file_size = local_file_length;

				if(local_data_offset_in_file>=local_file_length)
				{
					break;
				}

				memcpy(local_message_to_send,&local_message_header,CONST_MESSAGE_HEADER_LENGTH);

//				{
//					CString local_diagnostic_message;
//					local_diagnostic_message.Format(CString(L"local_message_length_to_send_in_bytes_this_time = %d"),local_message_length_to_send_in_bytes_this_time);
//					MessageBox(0,local_diagnostic_message,CString(L"Diagnostic"),MB_ICONINFORMATION);
//				}
	
				CBlockingSocket local_blocking_socket;

				CStringA local_address;

				CStringA local_address_internet_address;

				for(int local_counter=0;local_counter<local_parameter_address.GetLength();local_counter++)
				{
					local_address += BYTE(local_parameter_address[local_counter]);
				}

				if(domain_name_to_internet_name(local_address,local_address_internet_address)==false)
				{
					break;
				}

				CSockAddr local_socket_address(local_address_internet_address,local_parameter_port_number+local_threads_number_counter);

				try
				{
					local_blocking_socket.Create(SOCK_DGRAM);
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

					MessageBox(0,local_error_message,CString(L"Error"),MB_ICONERROR);

					return 0;
				}


				int local_bytes_sent = 0;
				int local_error_number = 0;

				try
				{
					local_bytes_sent = local_blocking_socket.SendDatagram(local_message_to_send,int(local_data_size)+int(CONST_MESSAGE_HEADER_LENGTH),local_socket_address,CONST_WAIT_TIME);
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

				if(local_bytes_sent<CONST_MESSAGE_HEADER_LENGTH && local_error_number!=0)
				{
					CString local_format_message;
					local_format_message.Format(CString(L"local_bytes_sent == %d"),local_bytes_sent);
					MessageBox(0,local_format_message,CString(L"Error"),MB_ICONERROR);
				}

				{
					CString local_string_pause;
					local_main_dialog->member_edit_pause.GetWindowTextW(local_string_pause);
					int local_pause_number = _wtoi(local_string_pause);
					Sleep(local_pause_number);
				}

				{
					CSingleLock local_single_lock_send(&local_main_dialog->member_send_critical_section);
					local_single_lock_send.Lock();
					
					CString local_string_send_information;
					local_main_dialog->member_bytes_send += ULONGLONG(local_data_size);
					ULONGLONG local_bytes_send_by_all_threads = local_main_dialog->member_bytes_send;
					CTime local_current_time_send = CTime::GetTickCount();
					CTimeSpan local_time_span_send = local_current_time_send - local_main_dialog->member_start_time_send;
					double local_bytes_per_second_send = double(local_bytes_send_by_all_threads)/double(local_time_span_send.GetTotalSeconds());
					local_main_dialog->member_bytes_per_second_send = ULONGLONG(local_bytes_per_second_send);
					local_string_send_information.Format(CString(L"Байт отправлено: %d / %d (%2.2f %%)\r\nБайт в секунду: %.2f"),unsigned int(local_bytes_send_by_all_threads),unsigned int(local_file_length), double(100.0*local_bytes_send_by_all_threads/local_file_length),local_bytes_per_second_send);
					local_main_dialog->member_send_information.SetWindowTextW(local_string_send_information);

					local_single_lock_send.Unlock();
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
	CString local_string_ports_number_local;
	CString local_string_file_name;

	member_edit_address_local.GetWindowTextW(local_string_address_local);
	member_edit_port_number_local.GetWindowTextW(local_string_port_number_local);
	member_edit_ports_number.GetWindowTextW(local_string_ports_number_local);
	member_edit_file_name_incoming.GetWindowTextW(local_string_file_name);
	
	USHORT local_maximum_message_length = USHORT(member_maximum_message_length);

	WORD local_port_number_local = (WORD)_wtoi(local_string_port_number_local);
	WORD local_ports_number_local = (WORD)_wtoi(local_string_ports_number_local);

	member_bytes_received = 0;

	member_bytes_per_second_received = 0;

	member_start_time_receive = CTime::GetTickCount();

	for(WORD local_ports_counter=0;local_ports_counter<local_ports_number_local;local_ports_counter++)
	{
		void *local_thread_parameters_structure = new thread_parameters_structure_type;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address = CString();
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number = 0;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_ports_number = local_ports_number_local;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_threads_number = local_ports_number_local;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_thread_number = 0;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_requests_number = 0;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_file_name = local_string_file_name;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_maximum_message_length = local_maximum_message_length;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address_local = local_string_address_local;
		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number_local = local_port_number_local+local_ports_counter;

		((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_main_dialog = this;

		CWinThread *local_thread = AfxBeginThread(datagram_receive_connection_thread,local_thread_parameters_structure);
	}
}

UINT __cdecl datagram_receive_connection_thread(LPVOID parameter)
{
	thread_parameters_structure_type *local_thread_parameters_structure = (thread_parameters_structure_type *)parameter;

	USHORT local_maximum_message_length = (local_thread_parameters_structure)->parameter_maximum_message_length;

	CString local_parameter_address_local = (local_thread_parameters_structure)->parameter_address_local;
	WORD local_parameter_port_number_local = (local_thread_parameters_structure)->parameter_port_number_local;
	CString local_string_file_name = (local_thread_parameters_structure)->parameter_file_name;

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure)->parameter_main_dialog;

	const size_t CONST_MESSAGE_LENGTH = local_maximum_message_length;
	const int CONST_WAIT_TIME_LISTEN = 10;

	char *local_message_to_receive = new char[CONST_MESSAGE_LENGTH+2];

	if(local_message_to_receive==NULL)
	{
		return 0;
	}

	CBlockingSocket local_blocking_socket;

	CStringA local_address;

	CStringA local_address_internet_address;

	for(int local_counter=0;local_counter<local_parameter_address_local.GetLength();local_counter++)
	{
		local_address += BYTE(local_parameter_address_local[local_counter]);
	}

	if(domain_name_to_internet_name(local_address,local_address_internet_address)==false)
	{
		return 0;
	}

	{
		CFile local_file(local_string_file_name, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
		local_file.Close();
	}

	CSockAddr local_socket_address(local_address_internet_address,local_parameter_port_number_local);

	try
	{
		local_blocking_socket.Create(SOCK_DGRAM);

		local_blocking_socket.Bind(local_socket_address);
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

		ZeroMemory(local_message_to_receive,CONST_MESSAGE_LENGTH+2);

		int local_bytes_received = 0;
			
		CSockAddr local_socket_address_peer;

		int local_error_number = 0;

		try
		{
			local_bytes_received = local_blocking_socket.ReceiveDatagram(local_message_to_receive,int(CONST_MESSAGE_LENGTH),(LPSOCKADDR)&local_socket_address_peer,CONST_WAIT_TIME_LISTEN);
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


		if(local_bytes_received<CONST_MESSAGE_HEADER_LENGTH && local_error_number!=0)
		{
			CString local_format_message;
			local_format_message.Format(CString(L"local_bytes_received == %d"),local_bytes_received);
			MessageBox(0,local_format_message,CString(L"Error"),MB_ICONERROR);
		}

		if(local_bytes_received>CONST_MESSAGE_HEADER_LENGTH)
		{
			message_header_type local_message_header;

			ZeroMemory(&local_message_header,sizeof(message_header_type));

			memcpy(&local_message_header,local_message_to_receive,CONST_MESSAGE_HEADER_LENGTH);

			WORD local_data_size = local_message_header.data_size;
			ULONGLONG local_data_offset_in_file = local_message_header.data_offset_in_file;
			ULONGLONG local_file_size = local_message_header.file_size;
			
			ULONGLONG local_file_length = 0;

			{
				CFile local_file(local_string_file_name, CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone | CFile::modeNoTruncate);
				local_file_length = local_file.GetLength();
				local_file.Close();
			}

			if(local_file_length!=local_file_size)
			{
				CFile local_file(local_string_file_name, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
				local_file.SetLength(local_file_size);
				local_file.Close();
			}

			{
				CFile local_file(local_string_file_name, CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone | CFile::modeNoTruncate);
				local_file.Seek(local_data_offset_in_file,CFile::begin);
				local_file.Write(local_message_to_receive+CONST_MESSAGE_HEADER_LENGTH,local_bytes_received-CONST_MESSAGE_HEADER_LENGTH);
				local_file.Close();
			}

			{
				CSingleLock local_single_lock_receive(&local_main_dialog->member_receive_critical_section);
				local_single_lock_receive.Lock();
					
				CString local_string_receive_information;
				local_main_dialog->member_bytes_received += ULONGLONG(local_bytes_received-CONST_MESSAGE_HEADER_LENGTH);
				ULONGLONG local_bytes_received_by_all_threads = local_main_dialog->member_bytes_received;
				CTime local_current_time_receive = CTime::GetTickCount();
				CTimeSpan local_time_span_receive = local_current_time_receive - local_main_dialog->member_start_time_receive;
				double local_bytes_per_second_received = double(local_bytes_received_by_all_threads)/double(local_time_span_receive.GetTotalSeconds());
				local_main_dialog->member_bytes_per_second_received = ULONGLONG(local_bytes_per_second_received);
				if(local_file_size>0)
				{
					local_string_receive_information.Format(CString(L"Байт принято: %d / %d (%2.2f %%)\r\nБайт в секунду: %.2f"),unsigned int(local_bytes_received_by_all_threads),unsigned int(local_file_size), double(100.0*local_bytes_received_by_all_threads/local_file_size),local_bytes_per_second_received);
				}
				local_main_dialog->member_receive_information.SetWindowTextW(local_string_receive_information);

				local_single_lock_receive.Unlock();
			}
		}


		/*/
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

		if(local_received_message.GetLength()!=0)
		{
//			::MessageBox(0,local_received_message,CString(L"Сообщение от ")+local_socket_address_peer.DottedDecimal(),MB_ICONINFORMATION);

			CString local_port_peer;
			local_port_peer.Format(CString(L"%d"),local_socket_address_peer.Port());

			CString local_new_line = CString(L"Сообщение от ")+local_socket_address_peer.DottedDecimal()+CString(L": ")+local_port_peer+CString(L": ")+local_received_message+CString(L"\n\r");

			CString local_window_text;
			local_main_dialog->member_edit_file_name_incoming.GetWindowTextW(local_window_text);

			CString local_window_text_new = local_window_text + CString(L"\n\r") + local_new_line;

			if(local_wrong_input)
			{
				local_window_text_new += CString(L"\n\r") + CString(L"Неверная длина сообщения") + CString(L"\n\r");
			}

			local_main_dialog->member_edit_file_name_incoming.SetWindowTextW(local_window_text_new);
		}
		/*/
		
	}

	local_blocking_socket.Close();

	delete []local_message_to_receive;

	delete local_thread_parameters_structure;

	return 1;
}



void Cdatagram_sendDlg::OnBnClickedButtonClearInput()
{
	member_edit_file_name_incoming.SetWindowTextW(CString(L""));
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




void Cdatagram_sendDlg::OnBnClickedButtonSelectFile()
{
	CFileDialog local_file_dialog(TRUE);

	if(local_file_dialog.DoModal()==IDOK)
	{
		member_edit_file_name.SetWindowTextW(local_file_dialog.GetPathName());
	}
}


void Cdatagram_sendDlg::OnBnClickedButtonSelectFileIncoming()
{
	CFileDialog local_file_dialog(FALSE);

	if(local_file_dialog.DoModal()==IDOK)
	{
		member_edit_file_name_incoming.SetWindowTextW(local_file_dialog.GetPathName());
	}
}


void Cdatagram_sendDlg::OnEnKillfocusEditPortsNumber()
{
	CString local_string_ports_number;
	member_edit_ports_number.GetWindowTextW(local_string_ports_number);
	member_edit_threads_number.SetWindowTextW(local_string_ports_number);
}
