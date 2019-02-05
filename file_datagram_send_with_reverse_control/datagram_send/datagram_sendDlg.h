
// datagram_sendDlg.h : файл заголовка
//

#pragma once
#include "afxwin.h"


UINT __cdecl datagram_send_connection_thread(LPVOID parameter);

UINT __cdecl datagram_send_connection_thread_reverse_control(LPVOID parameter);

UINT __cdecl datagram_receive_connection_thread(LPVOID parameter);

UINT __cdecl datagram_receive_connection_thread_reverse_control(LPVOID parameter);

int CodeToString(const LPBYTE code, const size_t code_len, CString *str);

bool domain_name_to_internet_name(CStringA domain_name, CStringA &internet_name);

class Cdatagram_sendDlg;

struct message_header_type
{
	WORD data_size;
	ULONGLONG data_offset_in_file;
	ULONGLONG file_size;
};

struct message_reverse_control_request_fragment_command_type
{
	ULONGLONG fragment_number;
	ULONGLONG fragments_count;
};

struct thread_parameters_structure_type
{
	CString parameter_address;
	WORD parameter_port_number;
	CString parameter_file_name;

	USHORT parameter_maximum_message_length;

	CString parameter_address_local;
	WORD parameter_port_number_local;

	message_reverse_control_request_fragment_command_type parameter_message_reverse_control_request_fragment_command;

	message_header_type parameter_message_header;

	Cdatagram_sendDlg *parameter_main_dialog;
};

struct thread_parameters_structure_reverse_control_type
{
	CString parameter_address_remote_reverse_control;
	WORD parameter_port_number_remote_reverse_control;

	USHORT parameter_maximum_message_length_reverse_control;

	CString parameter_address_local_reverse_control;
	WORD parameter_port_number_local_reverse_control;

	CString parameter_file_name;

	Cdatagram_sendDlg *parameter_main_dialog;
};

struct framents_information_type
{
	ULONGLONG fragment_number;
	ULONGLONG fragments_count;

	WORD data_size;
	ULONGLONG data_offset_in_file;
	ULONGLONG file_size;

	BOOL done;
};


typedef std::map<ULONGLONG,framents_information_type> framents_information_map_type;
typedef std::pair<ULONGLONG,framents_information_type> framents_information_map_pair_type;

// диалоговое окно Cdatagram_sendDlg
class Cdatagram_sendDlg : public CDialogEx
{
// Создание
public:
	Cdatagram_sendDlg(CWnd* pParent = NULL);	// стандартный конструктор
	~Cdatagram_sendDlg();

// Данные диалогового окна
	enum { IDD = IDD_DATAGRAM_SEND_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	CEdit member_edit_address;
	CEdit member_edit_port_number;

	CEdit member_edit_file_name;

	UINT member_maximum_message_length;

	CEdit member_edit_address_local;
	CEdit member_edit_port_number_local;

	CEdit member_edit_file_name_incoming;

	CEdit member_edit_pause;

	CButton member_check_stop_listen;
	CButton member_check_stop_send;

	CEdit member_send_information;
	CEdit member_receive_information;

	ULONGLONG member_bytes_send;
	ULONGLONG member_bytes_received;

	ULONGLONG member_bytes_per_second_send;
	ULONGLONG member_bytes_per_second_received;

	CTime member_start_time_send;
	CTime member_start_time_receive;

	CCriticalSection member_send_critical_section;
	CCriticalSection member_receive_critical_section;


	CEdit member_edit_address_local_reverse_control;
	CEdit member_edit_port_number_local_reverse_control;

	CEdit member_edit_address_remote_reverse_control;
	CEdit member_edit_port_number_remote_reverse_control;

	framents_information_map_type framents_information_receive_map;
	framents_information_map_type framents_information_send_map;


	afx_msg void OnBnClickedButtonStartSend();
	afx_msg void OnBnClickedButtonStartReceive();
	afx_msg void OnBnClickedButtonSelectFile();
	afx_msg void OnBnClickedButtonSelectFileIncoming();
};
