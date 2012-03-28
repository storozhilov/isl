#ifndef ISL__TCP_SOCKET__HXX
#define ISL__TCP_SOCKET__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/ReadWriteLock.hxx>
#include <list>
#include <string>
#include <memory>

namespace isl
{

//! TcpSocket with asynchronous mode support - you can read wrom it in one thread and write to it in another one.
/*!\
  TODO Documentation!!!
*/
class TcpSocket : public AbstractIODevice
{
public:
	TcpSocket();
	virtual ~TcpSocket();

	inline int descriptor() const
	{
		return _descriptor;
	}
	// TODO Should return std::string or isl::IpInterface
	inline std::wstring localAddress() const
	{
		return _localAddress;
	}
	inline unsigned int localPort() const
	{
		return _localPort;
	}
	// TODO Should return std::string or isl::IpInterface
	inline std::wstring remoteAddress() const
	{
		return _remoteAddress;
	}
	inline unsigned int remotePort() const
	{
		return _remotePort;
	}
	inline bool connected() const
	{
		ReadLocker locker(_connectedRwLock);
		return _connected;
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
	std::auto_ptr<TcpSocket> accept(const Timeout& timeout = Timeout());
	void connect(const std::string& address, unsigned int port);				// TODO Use IpInterface class?
private:
	TcpSocket(const TcpSocket&);								// No copy
	TcpSocket(int descriptor);

	TcpSocket& operator=(const TcpSocket&);							// No copy

	inline void setIsConnected(bool newIsConnected)
	{
		WriteLocker locker(_connectedRwLock);
		_connected = newIsConnected;
	}
	void closeSocket();

	virtual void openImplementation();
	virtual void closeImplementation();
	virtual size_t readImplementation(char * buffer, size_t bufferSize, const Timeout& timeout);
	virtual size_t writeImplementation(const char * buffer, size_t bufferSize, const Timeout& timeout);

	int _descriptor;
	std::wstring _localAddress;
	unsigned int _localPort;
	std::wstring _remoteAddress;
	unsigned int _remotePort;
	bool _connected;
	mutable ReadWriteLock _connectedRwLock;

	friend class AbstractTcpListener;

};

} // namespace isl

#endif

