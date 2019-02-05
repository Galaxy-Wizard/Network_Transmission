// datagram_sendDlg.cpp : файл реализации
//

#include "stdafx.h"
#include "datagram_send.h"
#include "datagram_sendDlg.h"
#include "afxdialogex.h"

#include "blocksock.h"

#include "encrypt_xor.h"

#include <windns.h>
#pragma comment (lib, "dnsapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const UINT CONST_MESSAGE_HEADER_LENGTH = sizeof(message_header_type) + sizeof(message_reverse_control_request_fragment_command_type);

const UINT CONST_MINIMUM_DATA_LENGTH = 1;

const LONG CONST_SECONDS_BEFORE_SEND_FIRST_REQUEST_AGAIN = 1;

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

	DDX_Control(pDX, IDC_EDIT_ADDRESS_LOCAL, member_edit_address_local);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER_LOCAL, member_edit_port_number_local);

	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_COUNT_LOCAL, member_edit_port_count_local);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_LOCAL_EXTERNAL, member_edit_address_local_external);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_PORT_NUMBER_LOCAL_EXTERNAL, member_edit_port_number_local_external);

	DDX_Control(pDX, IDC_EDIT_FILE_NAME, member_edit_file_name);

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

	DDX_Control(pDX, IDC_EDIT_MAXIMUM_PACKET_LENGTH, member_edit_maximum_packet_length);

	DDX_Control(pDX, IDC_EDIT_MAXIMUM_OUT_SPEED_BYTES_PER_SECOND, member_maximum_out_speed_bytes_per_second);
	DDX_Control(pDX, IDC_EDIT_MAXIMUM_IN_SPEED_BYTES_PER_SECOND, member_maximum_in_speed_bytes_per_second);

	DDX_Control(pDX, IDC_CHECK_PAUSE_SEND, member_pause_send);
	DDX_Control(pDX, IDC_CHECK_PAUSE_RECEIVE, member_pause_receive);

	DDX_Control(pDX, IDC_EDIT_XOR_CODE, member_edit_xor_code);
}

BEGIN_MESSAGE_MAP(Cdatagram_sendDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_SEND, &Cdatagram_sendDlg::OnBnClickedButtonStartSend)
	ON_BN_CLICKED(IDC_BUTTON_START_RECEIVE, &Cdatagram_sendDlg::OnBnClickedButtonStartReceive)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FILE, &Cdatagram_sendDlg::OnBnClickedButtonSelectFile)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_FILE_INCOMING, &Cdatagram_sendDlg::OnBnClickedButtonSelectFileIncoming)
	ON_EN_CHANGE(IDC_EDIT_MAXIMUM_PACKET_LENGTH, &Cdatagram_sendDlg::OnEnChangeEditMaximumPacketLength)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_SPEED_LIMITS, &Cdatagram_sendDlg::OnBnClickedButtonApplySpeedLimits)
	ON_EN_CHANGE(IDC_EDIT_XOR_CODE, &Cdatagram_sendDlg::OnEnChangeEditXorCode)
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

	member_maximum_message_length_from_udp_protocol_rounded = member_maximum_message_length;

	CString local_string_maximum_packet_length;
	local_string_maximum_packet_length.Format(CString(L"%d"),member_maximum_message_length);
	member_edit_maximum_packet_length.SetWindowTextW(local_string_maximum_packet_length);

	if(CONST_MESSAGE_HEADER_LENGTH>=member_maximum_message_length_from_udp_protocol_rounded)
	{
		CString local_string_error_message;
		local_string_error_message.Format(
			CString(L"Доступная максимальная длина пакета (%d), полученная от протокола udp, меньше или равна длине используемого программой заголовка (%d).\nРабота программы невозможна."),
			member_maximum_message_length_from_udp_protocol_rounded,
			CONST_MESSAGE_HEADER_LENGTH);
		::MessageBox(0,local_string_error_message, CString(L"Ошибка"),MB_ICONERROR);

		ExitProcess(0);
	}


	member_bytes_send = 0;
	member_bytes_received = 0;

	member_bytes_per_second_send = 0;
	member_bytes_per_second_received = 0;

	member_start_time_send = CTime();
	member_start_time_receive = CTime();



	member_edit_address_local.SetEventMask(ENM_CHANGE | member_edit_address_local.GetEventMask());
	member_edit_port_number_local.SetEventMask(ENM_CHANGE | member_edit_port_number_local.GetEventMask());
	member_edit_port_count_local.SetEventMask(ENM_CHANGE | member_edit_port_count_local.GetEventMask());
	member_edit_address_local_external.SetEventMask(ENM_CHANGE | member_edit_address_local_external.GetEventMask());
	member_edit_port_number_local_external.SetEventMask(ENM_CHANGE | member_edit_port_number_local_external.GetEventMask());
	member_edit_file_name.SetEventMask(ENM_CHANGE | member_edit_file_name.GetEventMask());
	member_edit_file_name_incoming.SetEventMask(ENM_CHANGE | member_edit_file_name_incoming.GetEventMask());
	member_edit_pause.SetEventMask(ENM_CHANGE | member_edit_pause.GetEventMask());

	member_send_information.SetEventMask(ENM_CHANGE | member_send_information.GetEventMask());
	member_receive_information.SetEventMask(ENM_CHANGE | member_receive_information.GetEventMask());
	member_edit_address_local_reverse_control.SetEventMask(ENM_CHANGE | member_edit_address_local_reverse_control.GetEventMask());
	member_edit_port_number_local_reverse_control.SetEventMask(ENM_CHANGE | member_edit_port_number_local_reverse_control.GetEventMask());
	member_edit_address_remote_reverse_control.SetEventMask(ENM_CHANGE | member_edit_address_remote_reverse_control.GetEventMask());
	member_edit_port_number_remote_reverse_control.SetEventMask(ENM_CHANGE | member_edit_port_number_remote_reverse_control.GetEventMask());
	member_edit_maximum_packet_length.SetEventMask(ENM_CHANGE | member_edit_maximum_packet_length.GetEventMask());


	member_maximum_in_speed_bytes_per_second.SetEventMask(ENM_CHANGE | member_maximum_in_speed_bytes_per_second.GetEventMask());
	member_maximum_out_speed_bytes_per_second.SetEventMask(ENM_CHANGE | member_maximum_out_speed_bytes_per_second.GetEventMask());


	member_maximum_in_speed_bytes_per_second.SetWindowTextW(CString(L"0"));
	member_maximum_out_speed_bytes_per_second.SetWindowTextW(CString(L"0"));

	member_edit_xor_code.SetEventMask(ENM_CHANGE | member_edit_xor_code.GetEventMask());

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

			try
			{
				CFile local_file(local_parameter_file_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone);
				local_file.Seek(local_message_header.data_offset_in_file,CFile::begin);
				local_message_header.data_size = local_file.Read(local_message_to_send+CONST_MESSAGE_HEADER_LENGTH,local_message_header.data_size);
			}
			catch(CFileException *local_file_exception)
			{
				wchar_t local_error_message[10000];
				local_file_exception->GetErrorMessage(local_error_message, 10000);
				::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
				break;
			}

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

			CString local_xor_code_string;
			char local_xor_code = 0;

			local_main_dialog->member_edit_xor_code.GetWindowTextW(local_xor_code_string);
			local_xor_code = char(_wtoi(local_xor_code_string));

			int local_message_length_to_send_in_bytes_this_time = int(local_message_header.data_size)+int(CONST_MESSAGE_HEADER_LENGTH);

			try
			{
				local_bytes_sent = local_blocking_socket.SendDatagram(encrypt_xor(local_message_to_send,local_message_length_to_send_in_bytes_this_time,local_xor_code),local_message_length_to_send_in_bytes_this_time,local_socket_address,CONST_WAIT_TIME);
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

					//if(local_framents_information_map_iterator!=local_main_dialog->framents_information_send_map.end())
					//{
					//	local_main_dialog->framents_information_send_list.push_back(local_framents_information_map_iterator->second);
					//}

					CString local_string_send_information;
					local_main_dialog->member_bytes_send += ULONGLONG(local_message_header.data_size);
					ULONGLONG local_bytes_send_by_all_threads = local_main_dialog->member_bytes_send;
					CTime local_current_time_send = CTime::GetTickCount();
					CTimeSpan local_time_span_send = local_current_time_send - local_main_dialog->member_start_time_send;
					double local_bytes_per_second_send = double(local_bytes_send_by_all_threads)/double(local_time_span_send.GetTotalSeconds());
					local_main_dialog->member_bytes_per_second_send = ULONGLONG(local_bytes_per_second_send);

					CString local_time_string = local_time_span_send.Format(L"Дни: %D Часы: %H Минуты: %M Секунды:%S");

					local_string_send_information.Format(CString(L"Байт отправлено: %d / %d (%2.2f %%)\r\nБайт в секунду (средняя скорость): %.2f\r\nВремя передачи:\r\n %s"),
						unsigned int(local_bytes_send_by_all_threads),
						unsigned int(local_message_header.file_size),
						double(100.0*local_bytes_send_by_all_threads/local_message_header.file_size),
						local_bytes_per_second_send,
						local_time_string.GetBuffer()
						);
					local_main_dialog->member_send_information.SetWindowTextW(local_string_send_information);

					//	Ограничение скорости применяется здесь
					{
						CString local_string_maximum_out_speed_bytes_per_second;

						local_string_maximum_out_speed_bytes_per_second = local_main_dialog->member_string_maximum_out_speed_bytes_per_second;

						int local_maximum_out_speed_bytes_per_second = _wtoi(local_string_maximum_out_speed_bytes_per_second);

						if(local_maximum_out_speed_bytes_per_second>0)
						{
							if(local_bytes_per_second_send>=double(local_maximum_out_speed_bytes_per_second))
							{
								DWORD local_time_to_sleep = (DWORD)((local_bytes_per_second_send - double(local_maximum_out_speed_bytes_per_second))/double(local_maximum_out_speed_bytes_per_second)*1000);
								//	1000 mili seconds in 1 second
											
								Sleep(local_time_to_sleep);
							}
						}
					}

					if(local_framents_information_map_iterator!=local_main_dialog->framents_information_send_map.end())
					{
						local_main_dialog->framents_information_send_list.push_back(local_framents_information_map_iterator->second);
					}

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




bool domain_name_to_internet_name(CStringW domain_name, CStringA &internet_name)
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

	if(DnsQuery_W(domain_name, DNS_TYPE_A, 0, NULL, &ppQueryResults,NULL)==ERROR_SUCCESS)
	{
		for(PDNS_RECORD ptr=ppQueryResults; ptr != NULL ;ptr=ptr->pNext)
		{
			if(ptr->wType==DNS_TYPE_A)
			{
				if(ptr->wDataLength!=0)
				{
					char local_address_buffer[100];
					inet_ntop(AF_INET,&ptr->Data.A.IpAddress,local_address_buffer,100);
					internet_name = local_address_buffer;
				
					break;
//					MessageBoxA(0,internet_name,CStringA("Information"),MB_ICONINFORMATION);
				}
			}
		}

		DnsFree(ppQueryResults,DnsFreeRecordList);

		return true;
	}

	return false;
}

bool domain_name_to_internet_name(CStringA domain_name, CStringA &internet_name)
{
	return domain_name_to_internet_name(CStringW(domain_name),internet_name);
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

	WORD local_thread_number = (local_thread_parameters_structure)->parameter_thread_number;

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

		try
		{
			CFile local_file(local_parameter_file_name, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
			local_file.Close();
		}
		catch(CFileException *local_file_exception)
		{
			wchar_t local_error_message[10000];
			local_file_exception->GetErrorMessage(local_error_message, 10000);
			::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
			break;
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
					CString local_xor_code_string;
					char local_xor_code = 0;

					local_main_dialog->member_edit_xor_code.GetWindowTextW(local_xor_code_string);
					local_xor_code = char(_wtoi(local_xor_code_string));

					encrypt_xor(local_message_to_receive,local_bytes_received,local_xor_code);

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

					try
					{
						CFile local_file(local_parameter_file_name, CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone | CFile::modeNoTruncate);
						local_file_length = local_file.GetLength();
						local_file.Close();
					}
					catch(CFileException *local_file_exception)
					{
						wchar_t local_error_message[10000];
						local_file_exception->GetErrorMessage(local_error_message, 10000);
						::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
						continue;
					}

					if(local_file_length!=local_file_size)
					{
						try
						{
							CFile local_file(local_parameter_file_name, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
							local_file.SetLength(local_file_size);
							local_file.Close();
						}
						catch(CFileException *local_file_exception)
						{
							wchar_t local_error_message[10000];
							local_file_exception->GetErrorMessage(local_error_message, 10000);
							::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
							continue;
						}
					}

					if(local_bytes_received==int(local_message_header.data_size)+int(CONST_MESSAGE_HEADER_LENGTH))
					{
						framents_information_type local_framents_information;

						ZeroMemory(&local_framents_information,sizeof(framents_information_type));

						local_framents_information.fragment_number = local_message_reverse_control_request_fragment_command.fragment_number;
						local_framents_information.fragments_count = local_message_reverse_control_request_fragment_command.fragments_count;

						local_framents_information.receive_thread_number = local_thread_number;

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

							try
							{
								CFile local_file(local_parameter_file_name, CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone | CFile::modeNoTruncate);
								local_file.Seek(local_data_offset_in_file,CFile::begin);
								local_file.Write(local_message_to_receive+CONST_MESSAGE_HEADER_LENGTH,local_bytes_received-CONST_MESSAGE_HEADER_LENGTH);
								local_file.Close();
							}
							catch(CFileException *local_file_exception)
							{
								wchar_t local_error_message[10000];
								local_file_exception->GetErrorMessage(local_error_message, 10000);
								::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
								continue;
							}

							{
								CSingleLock local_single_lock_receive(&local_main_dialog->member_receive_critical_section);
								local_single_lock_receive.Lock();

								//local_main_dialog->framents_information_receive_list.push_back(local_framents_information);

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
									local_string_receive_information.Format(CString(L"Байт принято: %d / %d (%2.2f %%)\r\nБайт в секунду  (средняя скорость): %.2f\r\nВремя приёма:\r\n %s"),
										unsigned int(local_bytes_received_by_all_threads), unsigned int(local_file_size),
										double(100.0*local_bytes_received_by_all_threads/local_file_size),
										local_bytes_per_second_received,
										local_time_string.GetBuffer()
										);
								}
								local_main_dialog->member_receive_information.SetWindowTextW(local_string_receive_information);

								//	Ограничение скорости применяется здесь
								{
									CString local_string_maximum_in_speed_bytes_per_second;

									local_string_maximum_in_speed_bytes_per_second = local_main_dialog->member_string_maximum_in_speed_bytes_per_second;

									int local_maximum_in_speed_bytes_per_second = _wtoi(local_string_maximum_in_speed_bytes_per_second);

									if(local_maximum_in_speed_bytes_per_second>0)
									{
										if(local_bytes_per_second_received>=double(local_maximum_in_speed_bytes_per_second))
										{
											DWORD local_time_to_sleep = (DWORD)((local_bytes_per_second_received - double(local_maximum_in_speed_bytes_per_second))/double(local_maximum_in_speed_bytes_per_second)*1000);
											//	1000 mili seconds in 1 second

											Sleep(local_time_to_sleep);
										}
									}
								}

								local_main_dialog->framents_information_receive_list.push_back(local_framents_information);

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

		CString local_string_file_name;
		member_edit_file_name.GetWindowTextW(local_string_file_name);

		try
		{
			CFile local_file(local_string_file_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone);
			ULONGLONG local_file_length = local_file.GetLength();

			if(local_file_length==0)
			{
				::MessageBox(0,CString(L"Длина файла для передачи равна нулю."),CString(L"Ошибка"),MB_ICONERROR);
				member_edit_file_name.SetWindowTextW(CString(L""));
			}
		}
		catch(CFileException *local_file_exception)
		{
			wchar_t local_error_message[10000];
			local_file_exception->GetErrorMessage(local_error_message, 10000);
			::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
			member_edit_file_name.SetWindowTextW(CString(L""));
		}
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

		local_main_dialog->framents_information_send_list.clear();

		local_single_lock_send.Unlock();

		ULONGLONG local_file_length = 0;

		if(local_parameter_file_name.GetLength()==0)
		{
			break;
		}

		try
		{
			CFile local_file(local_parameter_file_name, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone);
			local_file_length = local_file.GetLength();
			local_file.Close();
		}
		catch(CFileException *local_file_exception)
		{
			wchar_t local_error_message[10000];
			local_file_exception->GetErrorMessage(local_error_message, 10000);
			::MessageBox(0,local_error_message,CString(L"Ошибка"),MB_ICONERROR);
			break;
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

			int local_pause_send = local_main_dialog->member_pause_send.GetCheck();

			if(local_pause_send>0)
			{
				continue;
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

					//	Адрес программы, куда отправлять данные (передаётся в заголовке пакета запроса данных).
					CString local_string_address;
					CString local_string_port_number;
					wchar_t local_string_port_number_buffer[10000];

					CString local_string_file_name;

					local_string_address = local_message_reverse_control_request_fragment_command.address;
					local_string_port_number = _itow(int(local_message_reverse_control_request_fragment_command.port), local_string_port_number_buffer, 10);

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
								//								continue;
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

		local_main_dialog->framents_information_receive_list.clear();

		local_main_dialog->member_bytes_received = 0;

		local_main_dialog->member_bytes_per_second_received = 0;

		CString local_string_address_local_external;
		CString local_string_port_number_local_external;

		local_main_dialog->member_edit_address_local_external.GetWindowTextW(local_string_address_local_external);
		local_main_dialog->member_edit_port_number_local_external.GetWindowTextW(local_string_port_number_local_external);

		WORD local_port_number_local_external = (WORD)_wtoi(local_string_port_number_local_external);


		CString local_string_file_name;

		CString local_string_address_local;
		CString local_string_port_number_local;

		CString local_string_port_count_local;

		local_main_dialog->member_edit_address_local.GetWindowTextW(local_string_address_local);

		local_main_dialog->member_edit_port_number_local.GetWindowTextW(local_string_port_number_local);
		local_main_dialog->member_edit_file_name_incoming.GetWindowTextW(local_string_file_name);

		local_main_dialog->member_edit_port_count_local.GetWindowTextW(local_string_port_count_local);


		USHORT local_maximum_message_length = USHORT(local_main_dialog->member_maximum_message_length);

		WORD local_port_number_local = (WORD)_wtoi(local_string_port_number_local);

		WORD local_port_count_local = (WORD)_wtoi(local_string_port_count_local);

		{
			for(WORD local_port_counter_local=local_port_number_local;local_port_counter_local<local_port_number_local+local_port_count_local;local_port_counter_local++)
			{
				void *local_thread_parameters_structure = new thread_parameters_structure_type;

				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address = CString();
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number = 0;
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_file_name = local_string_file_name;
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_maximum_message_length = local_maximum_message_length;

				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_address_local = local_string_address_local;
				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_port_number_local = local_port_counter_local;

				((thread_parameters_structure_type*)local_thread_parameters_structure)->parameter_thread_number = local_port_counter_local-local_port_number_local;

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

		bool local_request_first_packet_again = false;

		local_main_dialog->member_start_time_receive = CTime::GetTickCount();

		for(;;)
		{
			int local_check_stop_listen_state = local_main_dialog->member_check_stop_listen.GetCheck();

			if(local_check_stop_listen_state>0)
			{
				break;
			}

			int local_pause_receive = local_main_dialog->member_pause_receive.GetCheck();

			if(local_pause_receive>0)
			{
				continue;
			}


			CSingleLock local_single_lock_receive(&local_main_dialog->member_receive_critical_section);

			for(WORD local_free_thread_number=0;local_free_thread_number<local_port_count_local;local_free_thread_number++)
			{
				message_reverse_control_request_fragment_command_type local_message_reverse_control_request_fragment_command;

				ZeroMemory(&local_message_reverse_control_request_fragment_command,sizeof(message_reverse_control_request_fragment_command_type));


				//	Здесь проверяем что поток local_free_thread_number готов принять данные
				bool local_thread_is_free = true;
				local_single_lock_receive.Lock();
				{
					framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.begin();
					for(;local_framents_information_map_iterator!=local_main_dialog->framents_information_receive_map.end();local_framents_information_map_iterator++)
					{
						if(local_framents_information_map_iterator->second.done==FALSE)
						{
							if(local_framents_information_map_iterator->second.receive_thread_number==local_free_thread_number)
							{
								local_thread_is_free = false;
								break;
							}
						}
					}
				}
				local_single_lock_receive.Unlock();

				if(local_free_thread_number==0)
				{
					if(local_request_first_packet_again)
					{
						local_thread_is_free = true;
					}
				}

				if(!local_thread_is_free)
				{
					continue;
				}

				{
					framents_information_type local_framents_information;

					ZeroMemory(&local_framents_information,sizeof(framents_information_type));

					ULONGLONG local_fragment_number = 0;
					ULONGLONG local_fragments_count = 1;

					local_single_lock_receive.Lock();

					framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.begin();

					if(!local_main_dialog->framents_information_receive_map.empty())
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

							local_framents_information.receive_thread_number = local_free_thread_number;

							local_single_lock_receive.Lock();

							local_main_dialog->framents_information_receive_map.insert(framents_information_map_pair_type(local_fragment_number,local_framents_information));

							local_single_lock_receive.Unlock();

							break;
						}
					}

					local_message_reverse_control_request_fragment_command.fragment_number = local_fragment_number;
					local_message_reverse_control_request_fragment_command.fragments_count = local_fragments_count;
				}


				{
					if(local_free_thread_number>0)
					{
						bool local_first_fragment_received = false;

						local_single_lock_receive.Lock();

						framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.find(0);

						local_single_lock_receive.Unlock();

						if(local_framents_information_map_iterator!=local_main_dialog->framents_information_receive_map.end())
						{
							if(local_framents_information_map_iterator->second.done==TRUE)
							{
								local_first_fragment_received = true;
							}
						}

						if(!local_first_fragment_received)
						{
							continue;
						}
					}
				}

				BOOL local_all_fragments_received = TRUE;

				for(;;)
				{

					ULONGLONG local_fragments_count = local_message_reverse_control_request_fragment_command.fragments_count;

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

					{
						wcscpy(local_message_reverse_control_request_fragment_command.address, local_string_address_local_external.GetBuffer(CONST_ADDRESS_LENGTH));
						local_message_reverse_control_request_fragment_command.port = local_port_number_local_external+local_free_thread_number;
					}


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

					local_blocking_socket.Close();
				}

			}

			//	Здесь ждём освобождения хотя бы одного потока получения
			for(;;)
			{
				int local_check_stop_listen_state = local_main_dialog->member_check_stop_listen.GetCheck();

				if(local_check_stop_listen_state>0)
				{
					break;
				}

				WORD local_working_threads_count = 0;

				local_single_lock_receive.Lock();
				{
					framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.begin();
					for(;local_framents_information_map_iterator!=local_main_dialog->framents_information_receive_map.end();local_framents_information_map_iterator++)
					{
						if(local_framents_information_map_iterator->second.done==FALSE)
						{
							local_working_threads_count++;
						}
					}
				}
				local_single_lock_receive.Unlock();

				local_single_lock_receive.Lock();

				size_t local_framents_information_receive_map_size = local_main_dialog->framents_information_receive_map.size();

				local_single_lock_receive.Unlock();


				if(local_framents_information_receive_map_size>1)
				{
					bool local_exist_free_thread = local_working_threads_count<local_port_count_local;

					if(local_exist_free_thread)
					{
						break;
					}
				}
				else
				{
					bool local_first_fragment_received = false;

					CTime local_enter_time = CTime::GetTickCount();

					for(;;)
					{
						int local_check_stop_listen_state = local_main_dialog->member_check_stop_listen.GetCheck();

						if(local_check_stop_listen_state>0)
						{
							break;
						}

						local_single_lock_receive.Lock();

						framents_information_map_type::iterator local_framents_information_map_iterator = local_main_dialog->framents_information_receive_map.find(0);

						local_single_lock_receive.Unlock();

						if(local_framents_information_map_iterator!=local_main_dialog->framents_information_receive_map.end())
						{
							if(local_framents_information_map_iterator->second.done==TRUE)
							{
								local_request_first_packet_again = false;
								local_first_fragment_received = true;
								break;
							}
							else
							{
								CTime local_current_time = CTime::GetTickCount();
								CTimeSpan local_time_span = local_current_time - local_enter_time;

								LONG local_seconds_passed = local_time_span.GetSeconds();

								if(local_seconds_passed>=CONST_SECONDS_BEFORE_SEND_FIRST_REQUEST_AGAIN)
								{
									local_request_first_packet_again = true;

									break;
								}
							}
						}
						else
						{
							local_request_first_packet_again = true;

							break;
						}

						CString local_string_pause;
						local_main_dialog->member_edit_pause.GetWindowTextW(local_string_pause);
						int local_pause_number = _wtoi(local_string_pause);
						Sleep(local_pause_number);
					}
					break;
				}

				CString local_string_pause;
				local_main_dialog->member_edit_pause.GetWindowTextW(local_string_pause);
				int local_pause_number = _wtoi(local_string_pause);
				Sleep(local_pause_number);
			}

		}

		break;
	}

	delete local_thread_parameters_structure_reverse_control;

	return 1;
}


void Cdatagram_sendDlg::OnEnChangeEditMaximumPacketLength()
{
	CString local_string_maximum_packet_length;
	member_edit_maximum_packet_length.GetWindowTextW(local_string_maximum_packet_length);

	member_maximum_message_length = _wtoi(local_string_maximum_packet_length);

	if(member_maximum_message_length_from_udp_protocol_rounded<member_maximum_message_length)
	{
		member_maximum_message_length = member_maximum_message_length_from_udp_protocol_rounded;

		CString local_string_maximum_message_length_from_udp_protocol_rounded;
		local_string_maximum_message_length_from_udp_protocol_rounded.Format(CString(L"%d"),member_maximum_message_length);

		member_edit_maximum_packet_length.SetWindowTextW(local_string_maximum_message_length_from_udp_protocol_rounded);
	}

	if(CONST_MESSAGE_HEADER_LENGTH>=member_maximum_message_length)
	{
		member_maximum_message_length = CONST_MESSAGE_HEADER_LENGTH+1;

		CString local_string_maximum_message_length_from_udp_protocol_rounded;
		local_string_maximum_message_length_from_udp_protocol_rounded.Format(CString(L"%d"),member_maximum_message_length);

		member_edit_maximum_packet_length.SetWindowTextW(local_string_maximum_message_length_from_udp_protocol_rounded);
	}
}



void Cdatagram_sendDlg::OnBnClickedButtonApplySpeedLimits()
{
	member_maximum_in_speed_bytes_per_second.GetWindowTextW(member_string_maximum_in_speed_bytes_per_second);
	member_maximum_out_speed_bytes_per_second.GetWindowTextW(member_string_maximum_out_speed_bytes_per_second);
}


void Cdatagram_sendDlg::OnEnChangeEditXorCode()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления

	CString local_xor_code_string;
	int local_xor_code = 0;

	member_edit_xor_code.GetWindowTextW(local_xor_code_string);
	local_xor_code = _wtoi(local_xor_code_string);

	if(local_xor_code<0)
	{
		local_xor_code = 0;
	}
	else
	if(local_xor_code>255)
	{
		local_xor_code = 255;
	}
	else
	{
		return;
	}

	local_xor_code_string.Format(CString(L"%d"),local_xor_code);
	member_edit_xor_code.SetWindowTextW(local_xor_code_string);
}
