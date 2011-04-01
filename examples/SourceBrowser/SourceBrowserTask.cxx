#include "SourceBrowserTask.hxx"
#include "SourceBrowserGenerator.hxx"

namespace isl
{

/*------------------------------------------------------------------------------
 * SourceBrowserTask
------------------------------------------------------------------------------*/

SourceBrowserTask::SourceBrowserTask(TcpSocket * socket, const std::wstring& rootPath) :
	AbstractHTTPTask(socket),
	_rootPath(rootPath)
{}

HTTPResponse::AbstractGenerator * SourceBrowserTask::createGeneratorOK()
{
	return new SourceBrowserGenerator(this, _rootPath);
}

} // namespace isl
