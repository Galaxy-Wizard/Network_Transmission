// blocksock.cpp (CBlockingSocketException, CBlockingSocket, CHttpBlockingSocket)
#include <stdafx.h>
#include "blocksock.h"

#pragma comment (lib, "Ws2_32.lib")

#pragma comment (lib, "Normaliz.lib")

#pragma comment (lib, "dnsapi.lib")

// Class CBlockingSocketException
IMPLEMENT_DYNAMIC(CBlockingSocketException, CException)

CBlockingSocketException::CBlockingSocketException(wchar_t* pchMessage)
{
	m_strMessage = pchMessage;
	m_nError = WSAGetLastError();
}

BOOL CBlockingSocketException::GetErrorMessage(LPWSTR lpstrError, UINT nMaxError,
		PUINT pnHelpContext /*= NULL*/)
{

	wchar_t text[10000];
	if(m_nError == 0) {
		wsprintf((LPWSTR)text, _T("%s: ошибка"), m_strMessage.GetBuffer());
	}
	else {
		wsprintf((LPWSTR)text, _T("%s: ошибка #%d"), m_strMessage.GetBuffer(), m_nError);
	}
	wcsncpy((wchar_t*)lpstrError, text, nMaxError - 1);
	return TRUE;
}

// Class CBlockingSocket
IMPLEMENT_DYNAMIC(CBlockingSocket, CObject)

void CBlockingSocket::Cleanup()
{
	// doesn't throw an exception because it's called in a catch block
	if(m_hSocket == NULL) return;
	VERIFY(closesocket(m_hSocket) != SOCKET_ERROR);
	m_hSocket = NULL;
}

void CBlockingSocket::Create(int address_family /* = AF_INET6*/,int nType /* = SOCK_STREAM */, UINT nProtocol /* = 0 */)
{
	ASSERT(m_hSocket == NULL);
	if((m_hSocket = socket(address_family, nType, nProtocol)) == INVALID_SOCKET) {
		throw new CBlockingSocketException(L"Создание сокета");
	}
}

void CBlockingSocket::Bind(LPSOCKADDR6 psa)
{
	ASSERT(m_hSocket != NULL);
	if(bind(m_hSocket, (sockaddr *)psa, sizeof(sockaddr_in6)) == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Привязка сокета");
	}
}

void CBlockingSocket::Listen()
{
	ASSERT(m_hSocket != NULL);
	if(listen(m_hSocket, 5) == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Прослушивание на сокете");
	}
}

BOOL CBlockingSocket::Accept(CBlockingSocket& sConnect, LPSOCKADDR6 psa)
{
	ASSERT(m_hSocket != NULL);
	ASSERT(sConnect.m_hSocket == NULL);
	int nLengthAddr = sizeof(sockaddr_in6);
	sConnect.m_hSocket = accept(m_hSocket, (sockaddr *)psa, &nLengthAddr);
	if(sConnect == INVALID_SOCKET) {
		// no exception if the listen was canceled
		if(WSAGetLastError() != WSAEINTR) {
			throw new CBlockingSocketException(L"Приём на сокете");
		}
		return FALSE;
	}
	return TRUE;
}

void CBlockingSocket::Close()
{
	if (NULL == m_hSocket)
		return;

	if(closesocket(m_hSocket) == SOCKET_ERROR) {
		// should be OK to close if closed already
		throw new CBlockingSocketException(L"Закрытие сокета");
	}
	m_hSocket = NULL;
}

void CBlockingSocket::Connect(LPSOCKADDR6 psa)
{
	ASSERT(m_hSocket != NULL);
	// should timeout by itself
	if(connect(m_hSocket, (sockaddr*)psa, sizeof(sockaddr_in6)) == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Соединение");
	}
}

int CBlockingSocket::Write(const char* pch, const int nSize, const int nSecs)
{
	int nBytesSent = 0;
	int nBytesThisTime;
	const char* pch1 = pch;
	do {
		nBytesThisTime = Send(pch1, nSize - nBytesSent, nSecs);
		nBytesSent += nBytesThisTime;
		pch1 += nBytesThisTime;
	} while(nBytesSent < nSize);
	return nBytesSent;
}

int CBlockingSocket::Send(const char* pch, const int nSize, const int nSecs)
{
	ASSERT(m_hSocket != NULL);
	// returned value will be less than nSize if client cancels the reading
	FD_SET fd = {1, m_hSocket};
	TIMEVAL tv = {nSecs, 0};
	if(select(0, NULL, &fd, NULL, &tv) == 0) {
		throw new CBlockingSocketException(L"Тайм аут передачи");
	}
	int nBytesSent;
	if((nBytesSent = send(m_hSocket, pch, nSize, 0)) == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Передача");
	}
	return nBytesSent;
}

int CBlockingSocket::Receive(char* pch, const int nSize, const int nSecs)
{
	ASSERT(m_hSocket != NULL);
	FD_SET fd = {1, m_hSocket};
	TIMEVAL tv = {nSecs, 0};
	if(select(0, &fd, NULL, NULL, &tv) == 0) {
		throw new CBlockingSocketException(L"Тайм аут получения");
	}

	int nBytesReceived;
	if((nBytesReceived = recv(m_hSocket, pch, nSize, 0)) == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Получение");
	}
	return nBytesReceived;
}

int CBlockingSocket::ReceiveDatagram(char* pch, const int nSize, LPSOCKADDR6 psa, const int nSecs)
{
	ASSERT(m_hSocket != NULL);
	FD_SET fd = {1, m_hSocket};
	TIMEVAL tv = {nSecs, 0};
	if(select(0, &fd, NULL, NULL, &tv) == 0) {
		throw new CBlockingSocketException(L"Тайм аут получения");
	}

	// input buffer should be big enough for the entire datagram
	int nFromSize = sizeof(sockaddr_in6);
	int nBytesReceived = recvfrom(m_hSocket, pch, nSize, 0, (sockaddr*)psa, &nFromSize);
	if(nBytesReceived == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Получение датаграммы");
	}
	return nBytesReceived;
}

int CBlockingSocket::SendDatagram(const char* pch, const int nSize, LPSOCKADDR6 psa, const int nSecs)
{
	ASSERT(m_hSocket != NULL);
	FD_SET fd = {1, m_hSocket};
	TIMEVAL tv = {nSecs, 0};
	if(select(0, NULL, &fd, NULL, &tv) == 0) {
		throw new CBlockingSocketException(L"Тайм аут отправки");
	}

	int nBytesSent = sendto(m_hSocket, pch, nSize, 0, (sockaddr*)psa, sizeof(sockaddr_in6));
	if(nBytesSent == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Отправка датаграммы");
	}
	return nBytesSent;
}

void CBlockingSocket::GetPeerAddr(LPSOCKADDR6 psa)
{
	ASSERT(m_hSocket != NULL);
	// gets the address of the socket at the other end
	int nLengthAddr = sizeof(sockaddr_in6);
	if(getpeername(m_hSocket, (sockaddr*)psa, &nLengthAddr) == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Получение удалённого имени");
	}
}

void CBlockingSocket::GetSockAddr(LPSOCKADDR6 psa)
{
	ASSERT(m_hSocket != NULL);
	// gets the address of the socket at this end
	int nLengthAddr = sizeof(sockaddr_in6);
	if(getsockname(m_hSocket, (sockaddr*)psa, &nLengthAddr) == SOCKET_ERROR) {
		throw new CBlockingSocketException(L"Получение имени сокета");
	}
}

//static
CSockAddr CBlockingSocket::GetHostByName(const char* pchName, const USHORT ushPort /* = 0 */)
{
	char local_port[10];
	_itoa(ushPort,local_port,10);

	struct addrinfo hints;
	struct addrinfo *result = NULL;
	ZeroMemory( &hints, sizeof(hints) );
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;

	DWORD dwRetval = getaddrinfo(pchName, local_port, &hints, &result);
    if ( dwRetval != 0 ) {
        throw new CBlockingSocketException(L"Получение адреса по имени");
    }

	SOCKADDR_IN6 sockTemp;
	memcpy(&sockTemp,result->ai_addr,sizeof(SOCKADDR_IN6)); // address is already in network byte order
	freeaddrinfo(result);
	return sockTemp;
}

//static
const char* CBlockingSocket::GetHostByAddr(LPSOCKADDR6 psa)
{
	hostent* pHostEnt = gethostbyaddr((char*) &((LPSOCKADDR_IN6) psa)
				->sin6_addr, sizeof(((LPSOCKADDR_IN6) psa)->sin6_addr), PF_INET6);
	if(pHostEnt == NULL) {
		throw new CBlockingSocketException(L"Получение адреса по адресу");
	}
	return pHostEnt->h_name; // caller shouldn't delete this memory
}

// Class CHttpBlockingSocket
IMPLEMENT_DYNAMIC(CHttpBlockingSocket, CBlockingSocket)

CHttpBlockingSocket::CHttpBlockingSocket()
{
	m_pReadBuf = new char[nSizeRecv];
	m_nReadBuf = 0;
}

CHttpBlockingSocket::~CHttpBlockingSocket()
{
	delete [] m_pReadBuf;
}

int CHttpBlockingSocket::ReadHttpHeaderLine(char* pch, const int nSize, const int nSecs)
// reads an entire header line through CRLF (or socket close)
// inserts zero string terminator, object maintains a buffer
{
	int nBytesThisTime = m_nReadBuf;
	int nLineLength = 0;
	char* pch1 = m_pReadBuf;
	char* pch2;
	do {
		// look for lf (assume preceded by cr)
		if((pch2 = (char*) memchr(pch1 , '\n', nBytesThisTime)) != NULL) {
			ASSERT((pch2) > m_pReadBuf);
			ASSERT(*(pch2 - 1) == '\r');
			nLineLength = (pch2 - m_pReadBuf) + 1;
			if(nLineLength >= nSize) nLineLength = nSize - 1;
			memcpy(pch, m_pReadBuf, nLineLength); // copy the line to caller
			m_nReadBuf -= nLineLength;
			memmove(m_pReadBuf, pch2 + 1, m_nReadBuf); // shift remaining characters left
			break;
		}
		pch1 += nBytesThisTime;
		nBytesThisTime = Receive(m_pReadBuf + m_nReadBuf, nSizeRecv - m_nReadBuf, nSecs);
		if(nBytesThisTime <= 0) { // sender closed socket or line longer than buffer
			throw new CBlockingSocketException(L"Чтение заголовка");
		}
		m_nReadBuf += nBytesThisTime;
	}
	while(TRUE);
	*(pch + nLineLength) = '\0';
	return nLineLength;
}

int CHttpBlockingSocket::ReadHttpResponse(char* pch, const int nSize, const int nSecs)
// reads remainder of a transmission through buffer full or socket close
// (assume headers have been read already)
{
	int nBytesToRead, nBytesThisTime, nBytesRead = 0;
	if(m_nReadBuf > 0) { // copy anything already in the recv buffer
		memcpy(pch, m_pReadBuf, m_nReadBuf);
		pch += m_nReadBuf;
		nBytesRead = m_nReadBuf;
		m_nReadBuf = 0;
	}
	do { // now pass the rest of the data directly to the caller
		nBytesToRead = min(nSizeRecv, nSize - nBytesRead);
		nBytesThisTime = Receive(pch, nBytesToRead, nSecs);
		if(nBytesThisTime <= 0) break; // sender closed the socket
		pch += nBytesThisTime;
		nBytesRead += nBytesThisTime;
	}
	while(nBytesRead <= nSize);
	return nBytesRead;
}

void LogBlockingSocketException(LPVOID pParam, char* pch, CBlockingSocketException* pe)
{	// pParam holds the HWND for the destination window (in another thread)
	CString strGmt = CTime::GetCurrentTime().FormatGmt("%m/%d/%y %H:%M:%S GMT");
	wchar_t text1[10000], text2[1500];
	pe->GetErrorMessage((LPWSTR)text2, 1500);
	wsprintf((wchar_t*)text1, L"Сетевая ошибка --%s %s -- %s\r\n", pch, text2, strGmt.GetBuffer());
	::SendMessage((HWND) pParam, EM_SETSEL, (WPARAM) 65534, 65535);
	::SendMessage((HWND) pParam, EM_REPLACESEL, (WPARAM) 0, (LPARAM) text1);
}
