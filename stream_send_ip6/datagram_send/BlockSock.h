// blocksock.h

// needs winsock.h in the precompiled headers

typedef const struct sockaddr_in6* LPSOCKADDR6;

class CBlockingSocketException : public CException
{
	DECLARE_DYNAMIC(CBlockingSocketException)
public:
// Constructor
	CBlockingSocketException(wchar_t* pchMessage);

public:
	~CBlockingSocketException() {}
	virtual BOOL GetErrorMessage(LPWSTR lpstrError, UINT nMaxError,
		PUINT pnHelpContext = NULL);

	virtual int GetErrorNumber()
	{
		return m_nError;
	}

private:
	int m_nError;
	CString m_strMessage;
};

extern void LogBlockingSocketException(LPVOID pParam, char* pch, CBlockingSocketException* pe);

class CSockAddr : public sockaddr_in6 {
public:
	// constructors
	CSockAddr()
		{ sin6_family = AF_INET6;
		  sin6_port = 0;
		  sin6_addr = in6addr_any;} // Default
	CSockAddr(const SOCKADDR& sa) { memcpy(this, &sa, sizeof(SOCKADDR)); }
	CSockAddr(const SOCKADDR_IN6& sin) { memcpy(this, &sin, sizeof(SOCKADDR_IN6)); }
	CSockAddr(const in6_addr ulAddr, const USHORT ushPort = 0) // parms are host byte ordered
		{ sin6_family = AF_INET6;
		  sin6_port = htons(ushPort);
	      memcpy(&sin6_addr,&ulAddr,sizeof(sin6_addr)); }
	CSockAddr(const char* pchIP, const USHORT ushPort = 0) // dotted IP addr string
		{ sin6_family = AF_INET6;
		  sin6_port = htons(ushPort);
		  inet_pton(AF_INET6,pchIP,&sin6_addr); } // already network byte ordered
	// Return the address in dotted-decimal format
	CString DottedDecimal()
		{
			char local_address_buffer[100];
			inet_ntop(AF_INET6,&sin6_addr,local_address_buffer,100);
			return CString(local_address_buffer);
		} // constructs a new CString object
	// Get port and address (even though they're public)
	USHORT Port() const
		{ return ntohs(sin6_port); }
	in6_addr IPAddr() const
		{ return sin6_addr; }
	// operators added for efficiency
	const CSockAddr& operator=(const SOCKADDR& sa)
		{ memcpy(this, &sa, sizeof(SOCKADDR));
		  return *this; }
	const CSockAddr& operator=(const SOCKADDR_IN6& sin)
		{ memcpy(this, &sin, sizeof(SOCKADDR_IN6));
		  return *this; }
	operator SOCKADDR()
		{ return *((LPSOCKADDR) this); }
	operator LPSOCKADDR()
		{ return (LPSOCKADDR) this; }
	operator LPSOCKADDR_IN6()
		{ return (LPSOCKADDR_IN6) this; }
};

// member functions truly block and must not be used in UI threads
// use this class as an alternative to the MFC CSocket class
class CBlockingSocket : public CObject
{
	DECLARE_DYNAMIC(CBlockingSocket)
public:
	SOCKET m_hSocket;
	CBlockingSocket() { m_hSocket = NULL; }
	void Cleanup();
	void Create(int nType = SOCK_STREAM, UINT nProtocol = 0);
	void Close();
	void Bind(LPSOCKADDR6 psa);
	void Listen();
	void Connect(LPSOCKADDR6 psa);
	BOOL Accept(CBlockingSocket& s, LPSOCKADDR6 psa);
	int Send(const char* pch, const int nSize, const int nSecs);
	int Write(const char* pch, const int nSize, const int nSecs);
	int Receive(char* pch, const int nSize, const int nSecs);
	int SendDatagram(const char* pch, const int nSize, LPSOCKADDR6 psa, 
		const int nSecs);
	int ReceiveDatagram(char* pch, const int nSize, LPSOCKADDR6 psa, 
		const int nSecs);
	void GetPeerAddr(LPSOCKADDR6 psa);
	void GetSockAddr(LPSOCKADDR6 psa);
	static CSockAddr GetHostByName(const char* pchName, 
		const USHORT ushPort = 0);
	static const char* GetHostByAddr(LPSOCKADDR6 psa);
	operator SOCKET()
		{ return m_hSocket; }
};

class CHttpBlockingSocket : public CBlockingSocket
{
public:
	DECLARE_DYNAMIC(CHttpBlockingSocket)
	enum {nSizeRecv = 1000}; // max receive buffer size (> hdr line length)
	CHttpBlockingSocket();
	~CHttpBlockingSocket();
	int ReadHttpHeaderLine(char* pch, const int nSize, const int nSecs);
	int ReadHttpResponse(char* pch, const int nSize, const int nSecs);
private:
	char* m_pReadBuf; // read buffer
	int m_nReadBuf; // number of bytes in the read buffer
};