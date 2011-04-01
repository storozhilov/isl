#ifndef ISL__SOURCE_BROWSER_LISTENER__HXX
#define ISL__SOURCE_BROWSER_LISTENER__HXX

#include <isl/AbstractTcpListener.hxx>

namespace isl
{

class SourceBrowserListener : public AbstractTcpListener
{
public:
	SourceBrowserListener(AbstractSubsystem * owner, TaskDispatcher& taskDispatcher, unsigned int port,
			const std::wstring& rootPath);
private:
	SourceBrowserListener();
	SourceBrowserListener(const SourceBrowserListener&);							// No copy

	SourceBrowserListener& operator=(const SourceBrowserListener&);						// No copy
	
	virtual AbstractTcpTask * createTask(TcpSocket * socket);

	std::wstring _rootPath;
};

} // namespace isl

#endif
