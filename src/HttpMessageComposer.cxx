#include <isl/HttpMessageComposer.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <sstream>

namespace {

static const char * ContentLengthHeader = "Content-Length";
static const char * TransferEncodingHeader = "Transfer-Encoding";

}

namespace isl
{

HttpMessageComposer::HttpMessageComposer(const std::string& firstToken, const std::string& secondToken,
		const std::string& thirdToken) :
	_firstToken(firstToken),
	_secondToken(secondToken),
	_thirdToken(thirdToken),
	_packetPrefix(),
	_packetPostfix(),
	_envelope()
{}

HttpMessageComposer::~HttpMessageComposer()
{}

void HttpMessageComposer::reset(const std::string& firstToken, const std::string& secondToken,
		const std::string& thirdToken)
{
	_firstToken.clear();
	_secondToken.clear();
	_thirdToken.clear();
	_packetPrefix.clear();
	_packetPostfix.clear();
	_envelope.clear();
}

const std::string& HttpMessageComposer::compose(const Http::Headers& header, size_t dataLen)
{
	Http::Headers actualHeader(header);
	actualHeader.erase(ContentLengthHeader);
	actualHeader.erase(TransferEncodingHeader);
	if (dataLen > 0U) {
		std::ostringstream oss;
		oss << dataLen;
		actualHeader.insert(Http::Headers::value_type(ContentLengthHeader, oss.str()));
	}
	std::ostringstream envelopStream;
	composeFirstLine(envelopStream);
	composeHeader(header, envelopStream);
	envelopStream << "\r\n";
	_envelope = envelopStream.str();
	return _envelope;
}

HttpMessageComposer::Packet HttpMessageComposer::compose(const Http::Headers& header, char * buffer,
		size_t headerPartSize, size_t dataLen)
{
	const std::string& envelope = compose(header, dataLen);
	if (envelope.size() > headerPartSize) {
		throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Provided buffer for header (") <<
				headerPartSize << " bytes) does not have enough size to store a requested header (" <<
				envelope.size() << " bytes)");
	}
	char * packetPtr = buffer + headerPartSize - envelope.size(); 
	envelope.copy(packetPtr, envelope.size());
	return Packet(packetPtr, envelope.size() + dataLen);
}

const std::string& HttpMessageComposer::composeFirstChunk(const Http::Headers& header, size_t dataLen)
{
	if (dataLen <= 0U) {
		throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Could not compose envelope for empty chunk"));
	}
	Http::Headers actualHeader(header);
	actualHeader.erase(ContentLengthHeader);
	actualHeader.erase(TransferEncodingHeader);
	actualHeader.insert(Http::Headers::value_type(TransferEncodingHeader, "chunked"));
	// Composing chunk
	std::ostringstream envelopStream;
	composeFirstLine(envelopStream);
	composeHeader(header, envelopStream);
	envelopStream << "\r\n" << std::hex << dataLen << "\r\n";
	_envelope = envelopStream.str();
	return _envelope;
}

const std::string& HttpMessageComposer::composeChunk(size_t dataLen)
{
	if (dataLen <= 0U) {
		throw isl::Exception(isl::Error(SOURCE_LOCATION_ARGS, "Could not compose envelope for empty chunk"));
	}
	std::ostringstream envelopStream;
	envelopStream << "\r\n" << std::hex << dataLen << "\r\n";
	_envelope = envelopStream.str();
	return _envelope;
}

const std::string& HttpMessageComposer::composeLastChunk(const Http::Headers& header)
{
	std::ostringstream envelopStream;
	envelopStream << "\r\n0\r\n";
	composeHeader(header, envelopStream);
	envelopStream << "\r\n";
	_envelope = envelopStream.str();
	return _envelope;
}

} // namespace isl
