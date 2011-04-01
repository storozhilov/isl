#ifndef ISL__SOURCE_BROWSER_GENERATOR__HXX
#define ISL__SOURCE_BROWSER_GENERATOR__HXX

#include <isl/HTTPResponse.hxx>

namespace isl
{

class SourceBrowserGenerator : public HTTPResponse::AbstractGeneratorOK
{
public:
	SourceBrowserGenerator(AbstractHTTPTask * task, const std::wstring& rootPath);
private:
	SourceBrowserGenerator();
	SourceBrowserGenerator(const SourceBrowserGenerator&);

	SourceBrowserGenerator& operator=(const SourceBrowserGenerator&);

	virtual void generateImplementation();
	void generateNotFound();
	bool fileToBeDisplayed(const std::string& fileName);

	std::wstring _rootPath;
};

} // namespace isl

#endif

