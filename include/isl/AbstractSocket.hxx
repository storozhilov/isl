#ifndef ISL__ABSTRACT_SOCKET__HXX
#define ISL__ABSTRACT_SOCKET__HXX

#include <isl/AbstractAsynchronousIODevice.hxx>
#include <list>
#include <string>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractSocket
------------------------------------------------------------------------------*/

// TODO To clarify about unix sockets in synchronous/asynchronous context

class AbstractSocket : public AbstractAsynchronousIODevice
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
	virtual unsigned int readImplementation(char * buffer, unsigned int bufferSize, const Timeout& timeout);
	virtual unsigned int writeImplementation(const char * buffer, unsigned int bufferSize, const Timeout& timeout);

	void closeSocket();

	int _descriptor;
};

} // namespace isl

#endif

