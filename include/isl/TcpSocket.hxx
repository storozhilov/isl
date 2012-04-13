#ifndef ISL__TCP_SOCKET__HXX
#define ISL__TCP_SOCKET__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/TcpAddrInfo.hxx>
#include <list>
#include <string>
#include <memory>

namespace isl
{

//! TCP-socket implementation
/*!
  This is a TCP-socket implementation with asynchronous mode support - you can read from it in one thread and write to it in another one.
*/
class TcpSocket : public AbstractIODevice
{
public:
	// Constructor
	TcpSocket();
	// Destructor
	virtual ~TcpSocket();
	// Returns a TCP-socket descriptor
	inline int descriptor() const
	{
		return _descriptor;
	}
	//! Returns a constant reference to local address info if socket has been connected or throws an exception otherwise
	const TcpAddrInfo& localAddr() const;
	//! Returns a constant reference to remote address info if socket has been connected or throws an exception otherwise
	const TcpAddrInfo& remoteAddr() const;
	//! Tread-safely returns connected status
	inline bool connected() const
	{
		ReadLocker locker(_connectedRwLock);
		return _connected;
	}
	//! Binds socket to an interface
	/*!
	  \param addrInfo Address info to bind to
	*/
	void bind(const TcpAddrInfo& addrInfo);
	//! Switching socket to the listening state
	/*!
	  \param backLog Listen backlog
	*/
	void listen(unsigned int backLog);
	//! Accepting TCP-connection
	/*!
	  \param timeout Timeout to wait for incoming connection
	*/
	std::auto_ptr<TcpSocket> accept(const Timeout& timeout = Timeout());
	//! Connects to an inteface
	/*!
	  \param addrInfo Interface address info to connect to
	*/
	void connect(const TcpAddrInfo& addrInfo);
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
	void fetchPeersData();

	virtual void openImplementation();
	virtual void closeImplementation();
	virtual size_t readImplementation(char * buffer, size_t bufferSize, const Timeout& timeout);
	virtual size_t writeImplementation(const char * buffer, size_t bufferSize, const Timeout& timeout);

	int _descriptor;
	std::auto_ptr<TcpAddrInfo> _localAddr;
	std::auto_ptr<TcpAddrInfo> _remoteAddr;
	bool _connected;
	mutable ReadWriteLock _connectedRwLock;

	friend class AbstractTcpListener;

};

} // namespace isl

#endif

