#ifndef ISL__TCP_ADDR_INFO__HXX
#define ISL__TCP_ADDR_INFO__HXX

#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/SystemCallError.hxx>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sstream>
#include <list>

namespace isl
{

//! TCP-address info class
/*!
  Basically this class is a wrapper for getaddrinfo(3) system call
  TODO _hostAsAddress support
*/
class TcpAddrInfo
{
public:
	//! TCP-endoint: "host:port"
	struct Endpoint
	{
		//! Conbstructs TCP-endoint
		/*!
		  \param host TCP-host
		  \param port TCP-port
		*/
		Endpoint(const std::string& host, unsigned int port) :
			host(host),
			port(port)
		{}

		//! TCP-host
		std::string host;
		//! TCP-port
		unsigned int port;
	};
	//! Endpoints container
	typedef std::list<Endpoint> EndpointList;
	//! Address family enumeration
	enum Family {
		IpV4,
		IpV6
	};
	//! Constructor
	/*!
	  \param family Address family
	  \param host Host name/address
	*/
	TcpAddrInfo(Family family, const std::string& host) :
		_family(family),
		_host(host),
		_service(),
		_port(0),
		_hostAsAddress(false),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}
	//! Constructor
	/*!
	  \param family Address family
	  \param host Host name/address
	  \param port Port number
	*/
	TcpAddrInfo(Family family, const char * host, unsigned int port) :
		_family(family),
		_host(host),
		_service(),
		_port(port),
		_hostAsAddress(false),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}
	/*TcpAddrInfo(Family family, const std::string& host, bool hostAsAddress) :
		_family(family),
		_host(host),
		_service(),
		_port(0),
		_hostAsAddress(hostAsAddress),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}*/
	/*TcpAddrInfo(Family family, const std::string& host, unsigned int port) :
		_family(family),
		_host(host),
		_service(),
		_port(port),
		_hostAsAddress(false),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}*/
	/*TcpAddrInfo(Family family, const std::string& host, unsigned int port, bool hostAsAddress) :
		_family(family),
		_host(host),
		_service(),
		_port(port),
		_hostAsAddress(hostAsAddress),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}*/
	//! Constructor
	/*!
	  \param family Address family
	  \param host Host name/address
	  \param service Service name
	*/
	TcpAddrInfo(Family family, const std::string& host, const std::string& service) :
		_family(family),
		_host(host),
		_service(service),
		_port(0),
		_hostAsAddress(false),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}
	/*TcpAddrInfo(Family family, const std::string& host, const std::string& service, bool hostAsAddress) :
		_family(family),
		_host(host),
		_service(service),
		_port(0),
		_hostAsAddress(hostAsAddress),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}*/
	//! Copying constructor
	/*!
	  \param other Other TCP-address info instance to copy from
	*/
	TcpAddrInfo(const TcpAddrInfo& other) :
		_family(other._family),
		_host(other._host),
		_service(other._service),
		_port(other._port),
		_hostAsAddress(other._hostAsAddress),
		_addrinfo(0),
		_endpoints(),
		_canonicalName()
	{
		init();
	}
	//! Destructor
	~TcpAddrInfo()
	{
		resetAddrInfo();
	}
	//! Assignment operator
	/*!
	  \param other Other TCP-address info instance to assign from
	*/
	TcpAddrInfo& operator=(const TcpAddrInfo& other)
	{
		if (&other == this) {
			return *this;
		}
		_family = other._family;
		_host = other._host;
		_service = other._service;
		_port = other._port;
		_hostAsAddress = other._hostAsAddress;
		resetAddrInfo();
		_endpoints.clear();
		_canonicalName.clear();
		init();
		return *this;
	}
	//! Returns initial hostname/address
	inline const std::string& host() const
	{
		return _host;
	}
	//! Returns initial service name
	inline const std::string& service() const
	{
		return _service;
	}
	//! Returns initial port number
	inline unsigned int port() const
	{
		return _port;
	}
	//! Returns first endpoint from the endpoints list
	inline Endpoint firstEndpoint() const
	{
		return _endpoints.front();
	}
	//! Returns an entire endpoints list
	inline const EndpointList& endpoints() const
	{
		return _endpoints;
	}
	//! Returns canonical name for the host
	inline const std::string& canonicalName() const
	{
		return _canonicalName;
	}
	//! Returns a pointer to the 'struct addreinfo' structure
	const struct addrinfo * addrinfo() const
	{
		return _addrinfo;
	}
	//! Loopback interface address predefined value
	static const char LoopbackAddress[];
	//! Wildcard interface address predefined value
	static const char WildcardAddress[];
private:
	TcpAddrInfo();

	void init()
	{
		struct addrinfo hints;
		memset(&hints, 0, sizeof hints);
		hints.ai_family = (_family == IpV6) ? AF_INET6 : AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		std::string serviceStr;
		if (_service.empty() && _port > 0) {
			hints.ai_flags |= AI_NUMERICSERV;
			std::ostringstream oss;
			oss << _port;
			serviceStr = oss.str();
		} else {
			serviceStr = _service;
		}
		if (_host == WildcardAddress) {
			hints.ai_flags |= AI_PASSIVE;
		}
		if ((_host != WildcardAddress) && (_host != LoopbackAddress)) {
			hints.ai_flags |= AI_CANONNAME;
			if (_hostAsAddress) {
				hints.ai_flags |= AI_NUMERICHOST;
			}
		}
		int status = getaddrinfo((_host == LoopbackAddress) || (_host == WildcardAddress) ? 0 : _host.c_str(), serviceStr.empty() ? 0 : serviceStr.c_str(), &hints, &_addrinfo);
		if (status != 0) {
			throw Exception(Error(SOURCE_LOCATION_ARGS, gai_strerror(status)));
		}
		struct addrinfo * curAddrInfo = _addrinfo;
		while (curAddrInfo) {
			if (curAddrInfo->ai_family == AF_INET) {
				char addr[INET_ADDRSTRLEN];
				if (!inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in *>(curAddrInfo->ai_addr)->sin_addr), addr, INET_ADDRSTRLEN)) {
					throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
				}
				_endpoints.push_back(Endpoint(addr, ntohs(reinterpret_cast<sockaddr_in *>(curAddrInfo->ai_addr)->sin_port)));
			} else if (curAddrInfo->ai_family == AF_INET6) {
				char addr[INET6_ADDRSTRLEN];
				if (!inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in6 *>(curAddrInfo->ai_addr)->sin6_addr), addr, INET_ADDRSTRLEN)) {
					throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::InetNToP, errno));
				}
				_endpoints.push_back(Endpoint(addr, ntohs(reinterpret_cast<sockaddr_in6 *>(curAddrInfo->ai_addr)->sin6_port)));
			} else {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Invalid address family"));
			}
			curAddrInfo = curAddrInfo->ai_next;
		}
		if (_addrinfo->ai_canonname) {
			_canonicalName = _addrinfo->ai_canonname;
		}
	}
	inline void resetAddrInfo()
	{
		if (_addrinfo) {
			freeaddrinfo(_addrinfo);
		}
	}

	Family _family;
	std::string _host;
	std::string _service;
	unsigned int _port;
	bool _hostAsAddress;
	struct addrinfo * _addrinfo;
	EndpointList _endpoints;
	std::string _canonicalName;
};

} // namespace isl

#endif

