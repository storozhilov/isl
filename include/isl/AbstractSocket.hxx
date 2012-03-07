#ifndef ISL__ABSTRACT_SOCKET__HXX
#define ISL__ABSTRACT_SOCKET__HXX

#include <isl/AbstractIODevice.hxx>
#include <list>
#include <string>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractSocket
------------------------------------------------------------------------------*/

class AbstractSocket : public AbstractIODevice
{
public:
	AbstractSocket();
	~AbstractSocket();

	inline int descriptor() const
	{
		return _descriptor;
	}
protected:
	AbstractSocket(int descriptor);

	virtual int createDescriptor() = 0;

	virtual void closeImplementation();
private:
	AbstractSocket(const AbstractSocket&);							// No copy

	AbstractSocket& operator=(const AbstractSocket&);					// No copy

	virtual void openImplementation();
	virtual size_t readImplementation(char * buffer, size_t bufferSize, const Timeout& timeout);
	virtual size_t writeImplementation(const char * buffer, size_t bufferSize, const Timeout& timeout);

	void closeSocket();

	int _descriptor;
};

} // namespace isl

#endif

