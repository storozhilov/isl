#include "SourceBrowserTask.hxx"
#include "SourceBrowserGenerator.hxx"

//namespace isl
//{

/*------------------------------------------------------------------------------
 * SourceBrowserTask
------------------------------------------------------------------------------*/

SourceBrowserTask::SourceBrowserTask(isl::TcpSocket * socket, const std::wstring& rootPath) :
	isl::AbstractHTTPTask(socket),
	_rootPath(rootPath)
{}

isl::HTTPResponse::AbstractGenerator * SourceBrowserTask::createGeneratorOK()
{
	return new SourceBrowserGenerator(this, _rootPath);
}

//} // namespace isl
