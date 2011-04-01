#ifndef ISL__SOURCE_BROWSER__HXX
#define ISL__SOURCE_BROWSER__HXX

#include <isl/TaskDispatcher.hxx>
#include "SourceBrowserListener.hxx"

namespace isl
{

class SourceBrowser : public AbstractSubsystem
{
public:
	SourceBrowser(AbstractSubsystem * owner, unsigned int workersAmount, unsigned int port, const std::wstring& rootPath);
private:
	SourceBrowser();
	SourceBrowser(const SourceBrowser&);

	SourceBrowser& operator=(const SourceBrowser&);

	virtual void onStartCommand();
	virtual void onStopCommand();

	SourceBrowserListener _listener;
	TaskDispatcher _taskDispatcher;
};

} // namespace isl

#endif
