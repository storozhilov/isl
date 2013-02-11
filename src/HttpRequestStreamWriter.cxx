#include <isl/HttpRequestStreamWriter.hxx>

namespace isl
{

HttpRequestStreamWriter::HttpRequestStreamWriter(const std::string& uri,
		const std::string& method, const std::string& version) :
	AbstractHttpMessageStreamWriter(),
	_method(method),
	_uri(uri),
	_version(version)
{}

std::string HttpRequestStreamWriter::composeFirstLine() const
{
	std::string firstLine(_method);
	firstLine.append(1, ' ');
	firstLine.append(_uri);
	firstLine.append(1, ' ');
	firstLine.append(_version);
	firstLine.append("\r\n");
	return firstLine;
}

} // namespace isl
