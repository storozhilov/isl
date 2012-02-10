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
	inline void bind(unsigned int port)
	{
		bind(port, std::list<std::string>());
	}
	inline void bind(unsigned int port, const std::string& interface)
	{
		std::list<std::string> interfaces(1, interface);
		bind(port, interfaces);
	}
	void bind(unsigned int port, const std::list<std::string>& interfaces);
	void listen(unsigned int backLog);
	TcpSocket * accept(const Timeout& timeout = Timeout());
	inline bool connected() const
	{
		return _connected;
	}
	void connect(const std::string& address, unsigned int port);				// TODO Use HostAddress class?
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

