#ifndef ISL__TCP_SOCKET__HXX
#define ISL__TCP_SOCKET__HXX

#include <isl/AbstractSocket.hxx>
#include <string>

namespace isl
{

/*------------------------------------------------------------------------------
 * TcpSocket
------------------------------------------------------------------------------*/

class TcpSocket : public AbstractSocket
{
public:
	TcpSocket();

	inline std::wstring localAddress() const
	{
		return _localAddress;
	}
	inline unsigned int localPort() const
	{
		return _localPort;
	}
	inline std::wstring remoteAddress() const
	{
		return _remoteAddress;
	}
	inline unsigned int remotePort() const
	{
		return _remotePort;
	}
	void bind(unsigned int port, const std::list<std::wstring>& interfaces = std::list<std::wstring>());
	void listen(unsigned int backLog);
	TcpSocket * accept(const Timeout& timeout = Timeout());
	inline bool connected() const
	{
		return _connected;
	}
	void connect(const std::wstring& hostName, unsigned int port);				// TODO Use HostAddress class
private:
	TcpSocket(const TcpSocket&);								// No copy
	TcpSocket(int descriptor);

	TcpSocket& operator=(const TcpSocket&);							// No copy

	virtual void closeImplementation();
	virtual int createDescriptor();

	std::wstring _localAddress;
	unsigned int _localPort;
	std::wstring _remoteAddress;
	unsigned int _remotePort;
	bool _connected;

	friend class AbstractTcpListener;

};

} // namespace isl

#endif

