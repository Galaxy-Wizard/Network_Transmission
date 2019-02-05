
// datagram_sendDlg.h : файл заголовка
//

#pragma once
#include "afxwin.h"


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
	CEdit member_edit_ports_number;
	CEdit member_edit_threads_number;
	CEdit member_edit_requests_number;
	CEdit member_edit_file_name;

	UINT member_maximum_message_length;

	CEdit member_edit_address_local;
	CEdit member_edit_port_number_local;

	CEdit member_edit_file_name_incoming;

	CButton member_check_unstoppable;
	CEdit member_edit_pause;

	CButton member_check_stop_listen;

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

	afx_msg void OnBnClickedButtonStartSend();
	afx_msg void OnBnClickedButtonStartListen();
	afx_msg void OnBnClickedButtonClearInput();
	afx_msg void OnBnClickedButtonSelectFile();
	afx_msg void OnBnClickedButtonSelectFileIncoming();
	afx_msg void OnEnKillfocusEditPortsNumber();
};
