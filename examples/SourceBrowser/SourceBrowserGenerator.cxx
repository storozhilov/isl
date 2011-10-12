#include "SourceBrowserGenerator.hxx"
#include <isl/NameListReleaser.hxx>
#include <isl/Utf8TextCodec.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/String.hxx>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fstream>
#include <errno.h>

namespace isl
{

/*------------------------------------------------------------------------------
 * SourceBrowserGenerator
------------------------------------------------------------------------------*/

SourceBrowserGenerator::SourceBrowserGenerator(AbstractHTTPTask * task, const std::wstring& rootPath) :
	HTTPResponse::AbstractGeneratorOK(task),
	_rootPath(rootPath)
{}

void SourceBrowserGenerator::generateImplementation()
{
	response().setHeaderField("Content-type", "text/html", true);
	std::string filename(Utf8TextCodec().encode(_rootPath));
	filename += request().uri();
	struct stat fileInfo;
	if (stat(filename.c_str(), &fileInfo) != 0) {
		generateNotFound();
		return;
	}
	if (S_ISREG(fileInfo.st_mode)) {
		if (!fileToBeDisplayed(filename)) {
			response().outputBuffer().write(
				"<html>\n"
				"  <head>\n"
				"    <title>Can not display file</title>\n"
				"  </head>\n"
				"  <body>\n"
				"    <h1>Can not display file</h1>\n"
				"    <p>Invalid file extension of <strong>" + request().uri() + "</strong></p>\n"
				"  </body>\n"
				"</html>");
			return;
		}
		response().outputBuffer().write(
			"<html>\n"
			"  <head>\n"
			"    <title>LibISL source file</title>\n"
			"  </head>\n"
			"  <body>\n"
			"    <pre>\n");
		std::ifstream fstr;
		fstr.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
		while (!fstr.eof()) {
			char buffer[1024];
			fstr.read(buffer, 1024);
			unsigned int bytesRead = fstr.gcount();
			std::string str(buffer, bytesRead);
			String::replace(str, "<", "&lt;");
			String::replace(str, ">", "&gt;");
			String::replace(str, "\"", "&quot;");
			response().outputBuffer().write(str);
		}
		response().outputBuffer().write(
			"    </pre>\n"
			"  </body>\n"
			"</html>");
	} else if (S_ISDIR(fileInfo.st_mode)) {
		struct dirent ** namelist;
		int filesCount = scandir(filename.c_str(), &namelist, 0, alphasort);
		if (filesCount < 0) {
			generateNotFound();
			return;
		}
		NameListReleaser releaser(namelist, filesCount);
		response().outputBuffer().write(
			"<html>\n"
			"  <head>\n"
			"    <title>Source browser</title>\n"
			"  </head>\n"
			"  <body>\n"
			"    <ul>\n");
		for (int i = 0; i < filesCount; ++i) {
			struct stat itemInfo;
			std::string itemName(filename);
			if (itemName.at(itemName.size() - 1) != '/') {
				itemName += '/';
			}
			itemName += namelist[i]->d_name;
			if (stat(itemName.c_str(), &itemInfo) != 0) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Stat, errno));
			}
			if (S_ISDIR(itemInfo.st_mode) && strcmp(namelist[i]->d_name, ".") && strcmp(namelist[i]->d_name, "..")) {
				response().outputBuffer().write(
					std::string("      <li><a href=\"") + request().uri() + namelist[i]->d_name + "/\">" +
					namelist[i]->d_name + "</a></li>\n");
			} else if (S_ISREG(itemInfo.st_mode) && fileToBeDisplayed(namelist[i]->d_name)) {
				response().outputBuffer().write(
					std::string("      <li><a href=\"") + request().uri() + namelist[i]->d_name + "\">" +
					namelist[i]->d_name + "</a></li>\n");
			} else {
				response().outputBuffer().write(std::string("      <li>") + namelist[i]->d_name + "</li>\n");
			}
		}
		response().outputBuffer().write(
			"    </ul>\n"
			"  </body>\n"
			"</html>");
	} else {
		generateNotFound();
	}
}

void SourceBrowserGenerator::generateNotFound()
{
	response().setStatusCode(HTTPResponse::StatusCode::construct<HTTPResponse::NotFoundStatusCode>());
	response().outputBuffer().write(
		"<html>\n"
		"  <head>\n"
		"    <title>404 Not found</title>\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>404 Not found</h1>\n"
		"    <p>Requested URI <strong>&quot;" + request().uri() + "&quot;</strong> is not found on the server</p>\n"
		"  </body>\n"
		"</html>");
}

bool SourceBrowserGenerator::fileToBeDisplayed(const std::string& fileName)
{
	return (fileName.rfind(".cxx") == (fileName.length() - 4) || fileName.rfind(".hxx") == (fileName.length() - 4));
}

} // namespace isl
