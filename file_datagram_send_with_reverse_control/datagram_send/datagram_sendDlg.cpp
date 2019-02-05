// datagram_sendDlg.cpp : файл реализации
//

#include "stdafx.h"
#include "datagram_send.h"
#include "datagram_sendDlg.h"
#include "afxdialogex.h"

#include "blocksock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const UINT CONST_MESSAGE_HEADER_LENGTH = sizeof(message_header_type) + sizeof(message_reverse_control_request_fragment_command_type);

const UINT CONST_MINIMUM_DATA_LENGTH = 1;

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
	DDX_Control(pDX, IDC_EDIT_FILE_NAME, member_edit_file_name);

	DDX_Control(pDX, IDC_EDIT_ADDRESS_LOCAL, member_edit_address_local);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER_LOCAL, member_edit_port_number_local);

	DDX_Control(pDX, IDC_EDIT_FILE_NAME_INCOMING, member_edit_file_name_incoming);

	DDX_Control(pDX, IDC_EDIT_PAUSE, member_edit_pause);

	DDX_Control(pDX, IDC_CHECK_STOP_RECEIVE, member_check_stop_listen);
	DDX_Control(pDX, IDC_CHECK_STOP_SEND, member_check_stop_send);

	DDX_Control(pDX, IDC_EDIT_SEND_INFORMATION, member_send_information);
	DDX_Control(pDX, IDC_EDIT_RECEIVE_INFORMATION, member_receive_information);


	DDX_Control(pDX, IDC_EDIT_ADDRESS_LOCAL_REVERSE_CONTROL, member_edit_address_local_reverse_control);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER_LOCAL_REVERSE_CONTROL, member_edit_port_number_local_reverse_control);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_REMOTE_REVERSE_CONTROL, member_edit_address_remote_reverse_control);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER_REMOTE_REVERSE_CONTROL, member_edit_port_number_remote_reverse_control);

}

BEGIN_MESSAGE_MAP(Cdatagram_sendDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_SEND, &Cdatagram_sendDlg::OnBnClickedButtonStartSend)
	ON_BN_CLICKED(IDC_BUTTON_START_RECEIVE, &Cdatagram_sendDlg::OnBnClickedButtonStartReceive)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FILE, &Cdatagram_sendDlg::OnBnClickedButtonSelectFile)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FILE_INCOMING, &Cdatagram_sendDlg::OnBnClickedButtonSelectFileIncoming)
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

		::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);

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

		::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);

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

		::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
	}

	local_socket.Close();

	member_maximum_message_length = CONST_MESSAGE_HEADER_LENGTH;

	for(;member_maximum_message_length<UINT(optVal);)
	{
		member_maximum_message_length += CONST_MINIMUM_DATA_LENGTH;
	}

	if(member_maximum_message_length>UINT(optVal))
	{
		member_maximum_message_length -= CONST_MINIMUM_DATA_LENGTH;
	}

	if(CONST_MESSAGE_HEADER_LENGTH+CONST_MINIMUM_DATA_LENGTH>UINT(optVal))
	{
		CString local_format_message;
		local_format_message.Format(CString(L"Максимум байт доступно для передаи: %d. Байт необходимо для передачи: %d"), UINT(optVal), CONST_MESSAGE_HEADER_LENGTH+CONST_MINIMUM_DATA_LENGTH);
		::MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
	}

	member_edit_pause.SetWindowTextW(CString(L"1"));


	member_bytes_send = 0;
	member_bytes_received = 0;

	member_bytes_per_second_send = 0;
	member_bytes_per_second_received = 0;

	member_start_time_send = CTime();
	member_start_time_receive = CTime();


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
	CString local_address_remote_reverse_control;
	CString local_port_number_remote_reverse_control;
	CString local_address_local_reverse_control;
	CString local_port_number_local_reverse_control;
	CString local_file_name;

	member_edit_address_remote_reverse_control.GetWindowTextW(local_address_remote_reverse_control);
	member_edit_port_number_remote_reverse_control.GetWindowTextW(local_port_number_remote_reverse_control);
	member_edit_address_local_reverse_control.GetWindowTextW(local_address_local_reverse_control);
	member_edit_port_number_local_reverse_control.GetWindowTextW(local_port_number_local_reverse_control);
	member_edit_file_name.GetWindowTextW(local_file_name);

	WORD local_port_number_remote_reverse_control_word = (WORD)_wtoi(local_port_number_remote_reverse_control);
	WORD local_port_number_local_reverse_control_word = (WORD)_wtoi(local_port_number_local_reverse_control);

	USHORT local_maximum_message_length = USHORT(member_maximum_message_length);

	{
		void *local_thread_parameters_structure = new thread_parameters_structure_reverse_control_type;


		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_address_remote_reverse_control = local_address_remote_reverse_control;
		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_port_number_remote_reverse_control = local_port_number_remote_reverse_control_word;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_maximum_message_length_reverse_control = local_maximum_message_length;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_address_local_reverse_control = local_address_local_reverse_control;
		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_port_number_local_reverse_control = local_port_number_local_reverse_control_word;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_file_name = local_file_name;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_main_dialog = this;


		CWinThread *local_thread = AfxBeginThread(datagram_send_connection_thread_reverse_control,local_thread_parameters_structure);
	}
}

UINT __cdecl datagram_send_connection_thread(LPVOID parameter)
{
	thread_parameters_structure_type *local_thread_parameters_structure = (thread_parameters_structure_type *)parameter;

	if(local_thread_parameters_structure==NULL)
	{
		return 0;
	}

	CString local_parameter_address = (local_thread_parameters_structure)->parameter_address;
	WORD local_parameter_port_number = (local_thread_parameters_structure)->parameter_port_number;
	CString local_parameter_file_name = (local_thread_parameters_structure)->parameter_file_name;
	USHORT local_parameter_maximum_message_length = (local_thread_parameters_structure)->parameter_maximum_message_length;

	message_reverse_control_request_fragment_command_type local_message_reverse_control_request_fragment_command;
	ZeroMemory(&local_message_reverse_control_request_fragment_command,sizeof(message_reverse_control_request_fragment_command_type));

	memcpy(
		&local_message_reverse_control_request_fragment_command,
		&(local_thread_parameters_structure)->parameter_message_reverse_control_request_fragment_command,
		sizeof(message_reverse_control_request_fragment_command_type));

	message_header_type local_message_header;
	ZeroMemory(&local_message_header,sizeof(message_header_type));

	memcpy(
		&local_message_header,
		&((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_message_header,
		sizeof(message_header_type));

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure)->parameter_main_dialog;

	const size_t CONST_MESSAGE_LENGTH = local_parameter_maximum_message_length;
	const size_t CONST_MESSAGE_LENGTH_WITHOUT_HEADER = CONST_MESSAGE_LENGTH-CONST_MESSAGE_HEADER_LENGTH;
	const int CONST_WAIT_TIME = 10;

	char *local_message_to_send = new char[CONST_MESSAGE_LENGTH];

	for(;local_message_to_send!=NULL && local_main_dialog!=NULL;)
	{
		for(;;)
		{
			if(local_main_dialog->member_check_stop_send.GetCheck()>0)
			{
				break;
			}

			ZeroMemory(local_message_to_send,CONST_MESSAGE_LENGTH);

			if(local_parameter_file_name.GetLength()==0)
			{
				break;
			}

			CFile local_file(local_parameter_file_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone);
			local_file.Seek(local_message_header.data_offset_in_file,CFile::begin);
			local_message_header.data_size = local_file.Read(local_message_to_send+CONST_MESSAGE_HEADER_LENGTH,local_message_header.data_size);

			memcpy(local_message_to_send,&local_message_header,sizeof(message_header_type));

			memcpy(local_message_to_send+sizeof(message_header_type),&local_message_reverse_control_request_fragment_command,sizeof(message_reverse_control_request_fragment_command_type));

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

				MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);

				break;
			}



			CSockAddr local_socket_address(local_address_internet_address,local_parameter_port_number);


			int local_bytes_sent = 0;
			int local_error_number = 0;

			try
			{
				local_bytes_sent = local_blocking_socket.SendDatagram(local_message_to_send,int(local_message_header.data_size)+int(CONST_MESSAGE_HEADER_LENGTH),local_socket_address,CONST_WAIT_TIME);
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
					MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
				}
			}



			local_blocking_socket.Close();

			if(local_bytes_sent<CONST_MESSAGE_HEADER_LENGTH && local_error_number!=0)
			{
				CString local_format_message;
				local_format_message.Format(CString(L"Байт отправлено: %d"),local_bytes_sent);
				MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
			}

			if(local_bytes_sent==int(local_message_header.data_size)+int(CONST_MESSAGE_HEADER_LENGTH))
			{
				CSingleLock local_single_lock_send(&local_main_dialog->member_send_critical_section);

				local_single_lock_send.Lock();

				framents_information_map_type::iterator local_framents_information_map_iterator = 
					local_main_dialog->framents_information_send_map.find(local_message_reverse_control_request_fragment_command.fragment_number);

				if(local_framents_information_map_iterator!=local_main_dialog->framents_information_send_map.end())
				{
					local_framents_information_map_iterator->second.done = TRUE;
				}

				local_single_lock_send.Unlock();

				{
					local_single_lock_send.Lock();

					CString local_string_send_information;
					local_main_dialog->member_bytes_send += ULONGLONG(local_message_header.data_size);
					ULONGLONG local_bytes_send_by_all_threads = local_main_dialog->member_bytes_send;
					CTime local_current_time_send = CTime::GetTickCount();
					CTimeSpan local_time_span_send = local_current_time_send - local_main_dialog->member_start_time_send;
					double local_bytes_per_second_send = double(local_bytes_send_by_all_threads)/double(local_time_span_send.GetTotalSeconds());
					local_main_dialog->member_bytes_per_second_send = ULONGLONG(local_bytes_per_second_send);

					CString local_time_string = local_time_span_send.Format(L"Дни: %D Часы: %H Минуты: %M Секунды:%S");

					local_string_send_information.Format(CString(L"Байт отправлено: %d / %d (%2.2f %%)\r\nБайт в секунду: %.2f\r\nВремя приёма:\r\n %s"),
						unsigned int(local_bytes_send_by_all_threads),
						unsigned int(local_message_header.file_size),
						double(100.0*local_bytes_send_by_all_threads/local_message_header.file_size),
						local_bytes_per_second_send,
						local_time_string.GetBuffer()
						);
					local_main_dialog->member_send_information.SetWindowTextW(local_string_send_information);

					local_single_lock_send.Unlock();
				}
			}
			else
			{
				CString local_format_message;
				local_format_message.Format(CString(L"Недопустимое число байт для фрагмента: %d"),unsigned int(local_message_reverse_control_request_fragment_command.fragment_number));
				MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
			}

			{
				CString local_string_pause;
				local_main_dialog->member_edit_pause.GetWindowTextW(local_string_pause);
				int local_pause_number = _wtoi(local_string_pause);
				Sleep(local_pause_number);
			}

			break;
		}

		break;
	}

	if(local_message_to_send!=NULL)
	{
		delete[] local_message_to_send;
	}

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
					//					MessageBoxA(0,internet_name,CStringA("Информация"),MB_ICONINFORMATION);
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

void Cdatagram_sendDlg::OnBnClickedButtonStartReceive()
{
	CString local_address_remote_reverse_control;
	CString local_port_number_remote_reverse_control;
	CString local_address_local_reverse_control;
	CString local_port_number_local_reverse_control;
	CString local_file_name;

	member_edit_address_remote_reverse_control.GetWindowTextW(local_address_remote_reverse_control);
	member_edit_port_number_remote_reverse_control.GetWindowTextW(local_port_number_remote_reverse_control);
	member_edit_address_local_reverse_control.GetWindowTextW(local_address_local_reverse_control);
	member_edit_port_number_local_reverse_control.GetWindowTextW(local_port_number_local_reverse_control);
	member_edit_file_name.GetWindowTextW(local_file_name);

	WORD local_port_number_remote_reverse_control_word = (WORD)_wtoi(local_port_number_remote_reverse_control);
	WORD local_port_number_local_reverse_control_word = (WORD)_wtoi(local_port_number_local_reverse_control);

	USHORT local_maximum_message_length = USHORT(member_maximum_message_length);

	{
		void *local_thread_parameters_structure = new thread_parameters_structure_reverse_control_type;


		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_address_remote_reverse_control = local_address_remote_reverse_control;
		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_port_number_remote_reverse_control = local_port_number_remote_reverse_control_word;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_maximum_message_length_reverse_control = local_maximum_message_length;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_address_local_reverse_control = local_address_local_reverse_control;
		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_port_number_local_reverse_control = local_port_number_local_reverse_control_word;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_file_name = local_file_name;

		((thread_parameters_structure_reverse_control_type*)local_thread_parameters_structure)->parameter_main_dialog = this;


		CWinThread *local_thread = AfxBeginThread(datagram_receive_connection_thread_reverse_control,local_thread_parameters_structure);
	}
}

UINT __cdecl datagram_receive_connection_thread(LPVOID parameter)
{
	thread_parameters_structure_type *local_thread_parameters_structure = (thread_parameters_structure_type *)parameter;

	USHORT local_parameter_maximum_message_length = (local_thread_parameters_structure)->parameter_maximum_message_length;

	CString local_parameter_address_local = (local_thread_parameters_structure)->parameter_address_local;
	WORD local_parameter_port_number_local = (local_thread_parameters_structure)->parameter_port_number_local;
	CString local_parameter_file_name = (local_thread_parameters_structure)->parameter_file_name;

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure)->parameter_main_dialog;

	const size_t CONST_MESSAGE_LENGTH = local_parameter_maximum_message_length;
	const int CONST_WAIT_TIME_LISTEN = 10;

	char *local_message_to_receive = new char[CONST_MESSAGE_LENGTH+2];

	for(;local_message_to_receive!=NULL && local_main_dialog!=NULL;)
	{

		CBlockingSocket local_blocking_socket;

		CStringA local_address;

		CStringA local_address_internet_address;

		for(int local_counter=0;local_counter<local_parameter_address_local.GetLength();local_counter++)
		{
			local_address += BYTE(local_parameter_address_local[local_counter]);
		}

		if(domain_name_to_internet_name(local_address,local_address_internet_address)==false)
		{
			break;
		}

		if(local_parameter_file_name.GetLength()==0)
		{
			break;
		}

		{
			CFile local_file(local_parameter_file_name, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
			local_file.Close();
		}

		CSockAddr local_socket_address(local_address_internet_address,local_parameter_port_number_local);

		for(;;)
		{
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

				MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);

				break;
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
						MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
					}
				}


				if(local_bytes_received<CONST_MESSAGE_HEADER_LENGTH && local_error_number!=0)
				{
					CString local_format_message;
					local_format_message.Format(CString(L"Байт принято: %d"),local_bytes_received);
					MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
				}

				if(local_bytes_received>CONST_MESSAGE_HEADER_LENGTH)
				{
					message_header_type local_message_header;

					ZeroMemory(&local_message_header,sizeof(message_header_type));

					memcpy(&local_message_header,local_message_to_receive,sizeof(message_header_type));

					message_reverse_control_request_fragment_command_type local_message_reverse_control_request_fragment_command;

					ZeroMemory(&local_message_reverse_control_request_fragment_command,sizeof(message_reverse_control_request_fragment_command_type));

					memcpy(&local_message_reverse_control_request_fragment_command,local_message_to_receive+sizeof(message_header_type),sizeof(message_reverse_control_request_fragment_command_type));

					WORD local_data_size = local_message_header.data_size;
					ULONGLONG local_data_offset_in_file = local_message_header.data_offset_in_file;
					ULONGLONG local_file_size = local_message_header.file_size;

					ULONGLONG local_file_length = 0;

					{
						CFile local_file(local_parameter_file_name, CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone | CFile::modeNoTruncate);
						local_file_length = local_file.GetLength();
						local_file.Close();
					}

					if(local_file_length!=local_file_size)
					{
						CFile local_file(local_parameter_file_name, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
						local_file.SetLength(local_file_size);
						local_file.Close();
					}

					if(local_bytes_received==int(local_message_header.data_size)+int(CONST_MESSAGE_HEADER_LENGTH))
					{
						framents_information_type local_framents_information;

						ZeroMemory(&local_framents_information,sizeof(framents_information_type));

						local_framents_information.fragment_number = local_message_reverse_control_request_fragment_command.fragment_number;
						local_framents_information.fragments_count = local_message_reverse_control_request_fragment_command.fragments_count;

						local_framents_information.data_size = local_data_size;
						local_framents_information.data_offset_in_file = local_data_offset_in_file;
						local_framents_information.file_size = local_file_length;
						local_framents_information.done = TRUE;

						CSingleLock local_single_lock_receive(&local_main_dialog->member_receive_critical_section);

						local_single_lock_receive.Lock();

						framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.find(local_framents_information.fragment_number);

						local_single_lock_receive.Unlock();


						if(local_framents_information_map_iterator!=local_main_dialog->framents_information_receive_map.end())
						{
							local_single_lock_receive.Lock();

							local_framents_information_map_iterator->second = local_framents_information;

							local_single_lock_receive.Unlock();

							{
								CFile local_file(local_parameter_file_name, CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone | CFile::modeNoTruncate);
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

								CString local_time_string = local_time_span_receive.Format(L"Дни: %D Часы: %H Минуты: %M Секунды:%S");

								if(local_file_size>0)
								{
									local_string_receive_information.Format(CString(L"Байт принято: %d / %d (%2.2f %%)\r\nБайт в секунду: %.2f\r\nВремя приёма:\r\n %s"),
										unsigned int(local_bytes_received_by_all_threads), unsigned int(local_file_size),
										double(100.0*local_bytes_received_by_all_threads/local_file_size),
										local_bytes_per_second_received,
										local_time_string.GetBuffer()
										);
								}
								local_main_dialog->member_receive_information.SetWindowTextW(local_string_receive_information);

								local_single_lock_receive.Unlock();
							}
						}
						else
						{
							CString local_format_message;
							local_format_message.Format(CString(L"Принят недопустимый фрагмент: %d"),unsigned int(local_framents_information.fragment_number));
							MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
						}

					}
				}
			}

			local_blocking_socket.Close();

			break;
		}
		break;
	}

	if(local_message_to_receive!=NULL)
	{
		delete []local_message_to_receive;
	}

	delete local_thread_parameters_structure;

	return 1;
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




UINT __cdecl datagram_send_connection_thread_reverse_control(LPVOID parameter)
{
	thread_parameters_structure_reverse_control_type *local_thread_parameters_structure_reverse_control = (thread_parameters_structure_reverse_control_type *)parameter;

	if(local_thread_parameters_structure_reverse_control==NULL)
	{
		return 1;
	}

	USHORT local_maximum_message_length_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_maximum_message_length_reverse_control;

	CString local_address_remote_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_address_remote_reverse_control;
	WORD local_port_number_remote_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_port_number_remote_reverse_control;

	CString local_address_local_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_address_local_reverse_control;
	WORD local_port_number_local_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_port_number_local_reverse_control;

	CString local_parameter_file_name = (local_thread_parameters_structure_reverse_control)->parameter_file_name;

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure_reverse_control)->parameter_main_dialog;

	const size_t CONST_MESSAGE_LENGTH = local_maximum_message_length_reverse_control;
	const int CONST_WAIT_TIME_LISTEN = 10;

	char *local_message_to_receive = new char[CONST_MESSAGE_LENGTH+2];

	for(;local_message_to_receive!=NULL && local_main_dialog!=NULL;)
	{

		CBlockingSocket local_blocking_socket;

		CStringA local_address;

		CStringA local_address_internet_address;

		for(int local_counter=0;local_counter<local_address_local_reverse_control.GetLength();local_counter++)
		{
			local_address += BYTE(local_address_local_reverse_control[local_counter]);
		}

		if(domain_name_to_internet_name(local_address,local_address_internet_address)==false)
		{
			break;
		}

		CSingleLock local_single_lock_send(&local_main_dialog->member_send_critical_section);
						
		local_single_lock_send.Lock();

		local_main_dialog->framents_information_send_map.clear();

		local_single_lock_send.Unlock();

		ULONGLONG local_file_length = 0;

		if(local_parameter_file_name.GetLength()==0)
		{
			break;
		}

		{
			CFile local_file(local_parameter_file_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone);
			local_file_length = local_file.GetLength();
			local_file.Close();
		}


		ULONGLONG local_file_length_round = 0;

		ULONGLONG local_fragments_count = 0;

		for(;local_file_length_round<local_file_length;)
		{
			local_file_length_round += CONST_MESSAGE_LENGTH-CONST_MESSAGE_HEADER_LENGTH;

			local_fragments_count++;
		}

		ULONGLONG local_fragment_number = 0;

		WORD local_data_size = 0;
		ULONGLONG local_data_offset_in_file = 0;

		for(;local_data_offset_in_file+CONST_MESSAGE_LENGTH-CONST_MESSAGE_HEADER_LENGTH<local_file_length;local_data_offset_in_file+=CONST_MESSAGE_LENGTH-CONST_MESSAGE_HEADER_LENGTH)
		{
			local_data_size = CONST_MESSAGE_LENGTH-CONST_MESSAGE_HEADER_LENGTH;

			framents_information_type local_framents_information;

			ZeroMemory(&local_framents_information,sizeof(framents_information_type));

			local_framents_information.fragment_number = local_fragment_number;
			local_framents_information.fragments_count = local_fragments_count;

			local_framents_information.data_size = local_data_size;
			local_framents_information.data_offset_in_file = local_data_offset_in_file;
			local_framents_information.file_size = local_file_length;

			local_framents_information.done = FALSE;

			local_single_lock_send.Lock();

			local_main_dialog->framents_information_send_map.insert(framents_information_map_pair_type(local_fragment_number,local_framents_information));

			local_single_lock_send.Unlock();

			local_fragment_number++;
		}

		{
			local_data_offset_in_file = local_fragment_number*(CONST_MESSAGE_LENGTH-CONST_MESSAGE_HEADER_LENGTH);

			local_data_size = WORD(local_file_length-local_data_offset_in_file);

			framents_information_type local_framents_information;

			ZeroMemory(&local_framents_information,sizeof(framents_information_type));

			local_framents_information.fragment_number = local_fragment_number;
			local_framents_information.fragments_count = local_fragments_count;

			local_framents_information.data_size = local_data_size;
			local_framents_information.data_offset_in_file = local_data_offset_in_file;
			local_framents_information.file_size = local_file_length;

			local_framents_information.done = FALSE;

			local_single_lock_send.Lock();

			local_main_dialog->framents_information_send_map.insert(framents_information_map_pair_type(local_fragment_number,local_framents_information));

			local_single_lock_send.Unlock();

			local_fragment_number++;
		}

		local_main_dialog->member_bytes_send = 0;

		local_main_dialog->member_bytes_per_second_send = 0;

		local_main_dialog->member_start_time_send = CTime::GetTickCount();

		CSockAddr local_socket_address(local_address_internet_address,local_port_number_local_reverse_control);

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

			MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);

			break;
		}

		for(;;)
		{
			int local_check_stop_send_state = local_main_dialog->member_check_stop_send.GetCheck();

			if(local_check_stop_send_state>0)
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
					MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
				}
			}


			if(local_bytes_received!=sizeof(message_reverse_control_request_fragment_command_type) && local_error_number!=0)
			{
				CString local_format_message;
				local_format_message.Format(CString(L"Байт принято: %d"),local_bytes_received);
				MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
			}

			if(local_bytes_received==sizeof(message_reverse_control_request_fragment_command_type))
			{
				message_reverse_control_request_fragment_command_type local_message_reverse_control_request_fragment_command;

				memcpy(&local_message_reverse_control_request_fragment_command,local_message_to_receive,sizeof(message_reverse_control_request_fragment_command_type));

				local_message_reverse_control_request_fragment_command.fragments_count = local_fragments_count;

				//	Здесь отправляем запрошенный фрагмент в новом потоке.
				{
					ULONGLONG local_fragment_number = local_message_reverse_control_request_fragment_command.fragment_number;

					CString local_string_address;
					CString local_string_port_number;
					CString local_string_file_name;

					local_main_dialog->member_edit_address.GetWindowTextW(local_string_address);
					local_main_dialog->member_edit_port_number.GetWindowTextW(local_string_port_number);
					local_main_dialog->member_edit_file_name.GetWindowTextW(local_string_file_name);

					WORD local_port_number = (WORD)_wtoi(local_string_port_number);

					USHORT local_maximum_message_length = USHORT(local_main_dialog->member_maximum_message_length);

					{
						void *local_thread_parameters_structure = new thread_parameters_structure_type;

						((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address = local_string_address;
						((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number = local_port_number;
						((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_file_name = local_string_file_name;
						((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_maximum_message_length = local_maximum_message_length;

						((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address_local = CString();
						((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number_local = 0;

						memcpy(
							&((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_message_reverse_control_request_fragment_command,
							&local_message_reverse_control_request_fragment_command,
							sizeof(message_reverse_control_request_fragment_command_type));

						message_header_type local_message_header;

						ZeroMemory(&local_message_header,sizeof(message_header_type));

						CSingleLock local_single_lock_send(&local_main_dialog->member_send_critical_section);

						local_single_lock_send.Lock();

						framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_send_map.find(local_fragment_number);

						local_single_lock_send.Unlock();

						if(local_framents_information_map_iterator!=local_main_dialog->framents_information_send_map.end())
						{
							local_single_lock_send.Lock();

							framents_information_type local_framents_information = local_framents_information_map_iterator->second;

							if(local_framents_information.done==TRUE)
							{
								continue;
							}

							local_single_lock_send.Unlock();

							local_message_header.data_size = local_framents_information.data_size;
							local_message_header.data_offset_in_file = local_framents_information.data_offset_in_file;
							local_message_header.file_size = local_framents_information.file_size;


							memcpy(
								&((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_message_header,
								&local_message_header,
								sizeof(message_header_type));

							((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_main_dialog = local_main_dialog;

							CWinThread *local_thread = AfxBeginThread(datagram_send_connection_thread,local_thread_parameters_structure);

						}
						else
						{
							CString local_format_message;
							local_format_message.Format(CString(L"Фрагмент %d не найден.\r\nВсего фрагментов %d"),unsigned int(local_fragment_number),unsigned int(local_fragments_count));
							MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
						}
					}
				}

			}
		}

		local_blocking_socket.Close();

		break;
	}

	if(local_message_to_receive!=NULL)
	{
		delete []local_message_to_receive;
	}

	delete local_thread_parameters_structure_reverse_control;

	return 1;
}





UINT __cdecl datagram_receive_connection_thread_reverse_control(LPVOID parameter)
{
	thread_parameters_structure_reverse_control_type *local_thread_parameters_structure_reverse_control = (thread_parameters_structure_reverse_control_type *)parameter;

	if(local_thread_parameters_structure_reverse_control==NULL)
	{
		return 1;
	}

	USHORT local_maximum_message_length_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_maximum_message_length_reverse_control;

	CString local_address_remote_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_address_remote_reverse_control;
	WORD local_port_number_remote_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_port_number_remote_reverse_control;

	CString local_parameter_address_local_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_address_local_reverse_control;
	WORD local_parameter_port_number_local_reverse_control = (local_thread_parameters_structure_reverse_control)->parameter_port_number_local_reverse_control;

	CString local_file_name = (local_thread_parameters_structure_reverse_control)->parameter_file_name;

	Cdatagram_sendDlg *local_main_dialog = (local_thread_parameters_structure_reverse_control)->parameter_main_dialog;

	const size_t CONST_MESSAGE_LENGTH = local_maximum_message_length_reverse_control;
	const int CONST_WAIT_TIME = 10;

	char *local_message_to_send = new char[CONST_MESSAGE_LENGTH];

	for(;local_message_to_send!=NULL && local_main_dialog!=NULL;)
	{
		ZeroMemory(local_message_to_send,CONST_MESSAGE_LENGTH);

		CBlockingSocket local_blocking_socket;

		CStringA local_address;

		CStringA local_address_internet_address;

		for(int local_counter=0;local_counter<local_address_remote_reverse_control.GetLength();local_counter++)
		{
			local_address += BYTE(local_address_remote_reverse_control[local_counter]);
		}

		if(domain_name_to_internet_name(local_address,local_address_internet_address)==false)
		{
			break;
		}

		local_main_dialog->framents_information_receive_map.clear();

		local_main_dialog->member_bytes_received = 0;

		local_main_dialog->member_bytes_per_second_received = 0;

		local_main_dialog->member_start_time_receive = CTime::GetTickCount();

		{
			CString local_string_address_local;
			CString local_string_port_number_local;
			CString local_string_file_name;

			local_main_dialog->member_edit_address_local.GetWindowTextW(local_string_address_local);
			local_main_dialog->member_edit_port_number_local.GetWindowTextW(local_string_port_number_local);
			local_main_dialog->member_edit_file_name_incoming.GetWindowTextW(local_string_file_name);

			USHORT local_maximum_message_length = USHORT(local_main_dialog->member_maximum_message_length);

			WORD local_port_number_local = (WORD)_wtoi(local_string_port_number_local);

			{
				void *local_thread_parameters_structure = new thread_parameters_structure_type;

				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address = CString();
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number = 0;
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_file_name = local_string_file_name;
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_maximum_message_length = local_maximum_message_length;

				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address_local = local_string_address_local;
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number_local = local_port_number_local;

				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_main_dialog = local_main_dialog;


				message_reverse_control_request_fragment_command_type local_message_reverse_control_request_fragment_command;

				ZeroMemory(&local_message_reverse_control_request_fragment_command,sizeof(message_reverse_control_request_fragment_command_type));

				memcpy(
					&((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_message_reverse_control_request_fragment_command,
					&local_message_reverse_control_request_fragment_command,
					sizeof(message_reverse_control_request_fragment_command_type));

				message_header_type local_message_header;

				ZeroMemory(&local_message_header,sizeof(message_header_type));

				memcpy(
					&((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_message_header,
					&local_message_header,
					sizeof(message_header_type));

				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_main_dialog = local_main_dialog;

				CWinThread *local_thread = AfxBeginThread(datagram_receive_connection_thread,local_thread_parameters_structure);
			}
		}

		for(;;)
		{
			int local_check_stop_listen_state = local_main_dialog->member_check_stop_listen.GetCheck();

			if(local_check_stop_listen_state>0)
			{
				break;
			}

			CSingleLock local_single_lock_receive(&local_main_dialog->member_receive_critical_section);

			message_reverse_control_request_fragment_command_type local_message_reverse_control_request_fragment_command;

			ZeroMemory(&local_message_reverse_control_request_fragment_command,sizeof(message_reverse_control_request_fragment_command_type));

			{
				framents_information_type local_framents_information;

				ZeroMemory(&local_framents_information,sizeof(framents_information_type));

				ULONGLONG local_fragment_number = 0;
				ULONGLONG local_fragments_count = 1;

				local_single_lock_receive.Lock();

				framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.begin();

				if(local_main_dialog->framents_information_receive_map.empty())
				{
					local_framents_information.fragment_number = local_fragment_number;
					local_framents_information.done = FALSE;

					local_main_dialog->framents_information_receive_map.insert(framents_information_map_pair_type(local_fragment_number,local_framents_information));
				}
				else
				{
					if(local_framents_information_map_iterator->second.done!=FALSE)
					{
						local_fragments_count = local_framents_information_map_iterator->second.fragments_count;
					}
				}

				local_single_lock_receive.Unlock();

				for(ULONGLONG local_fragments_counter=0;local_fragments_counter<local_fragments_count;local_fragments_counter++)
				{
					local_single_lock_receive.Lock();

					local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.find(local_fragments_counter);

					local_single_lock_receive.Unlock();

					if(local_framents_information_map_iterator==local_main_dialog->framents_information_receive_map.end())
					{
						local_fragment_number = local_fragments_counter;


						local_framents_information.fragment_number = local_fragment_number;
						local_framents_information.done = FALSE;

						local_main_dialog->framents_information_receive_map.insert(framents_information_map_pair_type(local_fragment_number,local_framents_information));

						break;
					}
				}

				local_message_reverse_control_request_fragment_command.fragment_number = local_fragment_number;
				local_message_reverse_control_request_fragment_command.fragments_count = local_fragments_count;
			}

			BOOL local_all_fragments_received = TRUE;

			for(;;)
			{

				ULONGLONG local_fragments_count = local_message_reverse_control_request_fragment_command.fragments_count;

				if(local_fragments_count==0)
				{
					local_all_fragments_received = FALSE;

					break;
				}

				for(ULONGLONG local_fragments_counter=0;local_fragments_counter<local_fragments_count;local_fragments_counter++)
				{
					local_single_lock_receive.Lock();

					framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.find(local_fragments_counter);

					local_single_lock_receive.Unlock();

					if(local_framents_information_map_iterator==local_main_dialog->framents_information_receive_map.end())
					{
						local_all_fragments_received = FALSE;

						break;
					}
					else
					{
						if(local_framents_information_map_iterator->second.done==FALSE)
						{
							local_all_fragments_received = FALSE;

							break;
						}
					}
				}
				break;
			}

			if(local_all_fragments_received==TRUE)
			{
				break;
			}


			//	Здесь отправляем запрос на отправку фрагмента в новом потоке.
			{
				CBlockingSocket local_blocking_socket;

				CStringA local_address;

				CStringA local_address_internet_address;

				for(int local_counter=0;local_counter<local_address_remote_reverse_control.GetLength();local_counter++)
				{
					local_address += BYTE(local_address_remote_reverse_control[local_counter]);
				}

				if(domain_name_to_internet_name(local_address,local_address_internet_address)==false)
				{
					break;
				}

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

					MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);

					break;
				}



				CSockAddr local_socket_address(local_address_internet_address,local_port_number_remote_reverse_control);


				int local_bytes_sent = 0;
				int local_error_number = 0;


				memcpy(local_message_to_send,&local_message_reverse_control_request_fragment_command,sizeof(message_reverse_control_request_fragment_command_type));


				try
				{
					local_bytes_sent = local_blocking_socket.SendDatagram(local_message_to_send,sizeof(message_reverse_control_request_fragment_command_type),local_socket_address,CONST_WAIT_TIME);
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
						MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
					}
				}



				if(local_bytes_sent!=sizeof(message_reverse_control_request_fragment_command_type) && local_error_number!=0)
				{
					CString local_format_message;
					local_format_message.Format(CString(L"Байт отправлено: %d"),local_bytes_sent);
					MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
				}


				for(;;)
				{
					ULONGLONG local_current_fragment = local_message_reverse_control_request_fragment_command.fragment_number;

					local_single_lock_receive.Lock();

					framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.find(local_current_fragment);

					local_single_lock_receive.Unlock();

					if(local_framents_information_map_iterator!=local_main_dialog->framents_information_receive_map.end())
					{
						if(local_framents_information_map_iterator->second.done==TRUE)
						{
							break;
						}
					}

					int local_check_stop_listen_state = local_main_dialog->member_check_stop_listen.GetCheck();

					if(local_check_stop_listen_state>0)
					{
						break;
					}

					/*/
					//	Повторяем, пока 0 пакет не будет принят или не отменят приём

					local_single_lock_receive.Lock();

					local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.find(0);

					local_single_lock_receive.Unlock();

					if(local_framents_information_map_iterator!=local_main_dialog->framents_information_receive_map.end())
					{
						if(local_framents_information_map_iterator->second.done==FALSE)
						{
							try
							{
								local_bytes_sent = local_blocking_socket.SendDatagram(local_message_to_send,sizeof(message_reverse_control_request_fragment_command_type),local_socket_address,CONST_WAIT_TIME);
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
									MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
								}
							}
							//	Пауза для синхронизации 0 пакета
							CString local_string_pause;
							local_main_dialog->member_edit_pause.GetWindowTextW(local_string_pause);
							int local_pause_number = _wtoi(local_string_pause);
							Sleep(local_pause_number);
						}
					}
					else
					{
						CString local_format_message;
						local_format_message.Format(CString(L"Пакет не найден в списке на приём: %d"),0);
						MessageBox(0,local_format_message,CString(L"Ошибка"),MB_ICONERROR);
					}
					/*/


					CString local_string_pause;
					local_main_dialog->member_edit_pause.GetWindowTextW(local_string_pause);
					int local_pause_number = _wtoi(local_string_pause);
					Sleep(local_pause_number);
				}


				local_blocking_socket.Close();

			}
		}

		break;
	}

	delete local_thread_parameters_structure_reverse_control;

	return 1;
}
