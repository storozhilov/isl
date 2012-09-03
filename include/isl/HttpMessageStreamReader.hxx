#ifndef ISL__HTTP_MESSAGE_STREAM_READER__HXX
#define ISL__HTTP_MESSAGE_STREAM_READER__HXX

#include <isl/AbstractIODevice.hxx>
#include <isl/AbstractError.hxx>
#include <isl/Http.hxx>
#include <isl/Char.hxx>
#include <string>
#include <vector>
#include <memory>

#ifndef ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE
#define ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE 1024		// 1 Kb
#endif
#ifndef ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_MAX_HEADER_NAME_LENGTH
#define ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_MAX_HEADER_NAME_LENGTH 256
#endif
#ifndef ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_MAX_HEADER_VALUE_LENGTH
#define ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_MAX_HEADER_VALUE_LENGTH 4096	// 4 Kb
#endif

namespace isl
{

//! Base class for HTTP-message stream readers
class HttpMessageStreamReader
{
public:
	enum Constants {
		DefaultBufferSize = ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_BUFFER_SIZE,
		DefaultMaxHeaderNameLength = ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_MAX_HEADER_NAME_LENGTH,
		DefaultMaxHeaderValueLength = ISL__HTTP_MESSAGE_STREAM_READER_DEFAULT_MAX_HEADER_VALUE_LENGTH
	};
	//! Parser states
	enum ParserState {
		ParsingMessage,
		ParsingFirstToken,
		ParsingFirstTokenSP,
		ParsingSecondToken,
		ParsingSecondTokenSP,
		ParsingThirdToken,
		ParsingFirstLineLF,
		ParsingHeader,
		ParsingHeaderName,
		ParsingHeaderValue,
		ParsingHeaderValueLF,
		ParsingHeaderValueLWS,
		ParsingEndOfHeader,
		ParsingIdentityBody,
		ParsingChunkSize,
		ParsingChunkSizeLF,
		ParsingChunkExtension,
		ParsingChunk,
		ParsingChunkCR,
		ParsingChunkLF,
		ParsingTrailerHeader,
		ParsingTrailerHeaderName,
		ParsingTrailerHeaderValue,
		ParsingTrailerHeaderValueLF,
		ParsingTrailerHeaderValueLWS,
		ParsingFinalLF,
		MessageCompleted
	};

	//! Constructs reader
	/*!
	  \param device Reference to the I/O-device to fetch data from
	  \param bufferSize Data reading buffer size
	*/
	HttpMessageStreamReader(AbstractIODevice& device, size_t bufferSize = DefaultBufferSize,
			size_t maxHeaderNameLength = DefaultMaxHeaderNameLength,
			size_t maxHeaderValueLength = DefaultMaxHeaderValueLength);
	virtual ~HttpMessageStreamReader();
	//! Inspects for bad HTTP-message
	inline bool isBad() const
	{
		return _errorAutoPtr.get();
	}
	//! Returnts pointer to the error object ot 0 if no error occured
	inline const AbstractError * error() const
	{
		return _errorAutoPtr.get();
	}
	//! Returns the current position of the HTTP-message parser
	inline size_t pos() const
	{
		return _pos;
	}
	//! Returns the current line of the HTTP-message parser
	inline size_t line() const
	{
		return _line;
	}
	//! Returns the current column of the HTTP-message parser
	inline size_t col() const
	{
		return _col;
	}
	//! Returns a constant referense to the HTTP-message header
	inline const Http::Params& header() const
	{
		return _header;
	}
	//! Returns maximum header field name length
	inline size_t maxHeaderNameLength() const
	{
		return _maxHeaderNameLength;
	}
	//! Sets maximum header field name length
	/*!
	  \param newValue New maximum header field name length
	*/
	inline void setMaxHeaderNameLength(size_t newValue)
	{
		_maxHeaderNameLength = newValue;
	}
	//! Returns maximum header field value length
	inline size_t maxHeaderValueLength() const
	{
		return _maxHeaderValueLength;
	}
	//! Sets maximum header field value length
	/*!
	  \param newValue New maximum header field value length
	*/
	inline void setMaxHeaderValueLength(size_t newValue)
	{
		_maxHeaderValueLength = newValue;
	}
	//! Reads an HTTP-message from the device and puts the HTTP-message body to the supplied buffer
	/*!
	  \param bodyBuffer Body buffer
	  \param bodyBufferSize Body buffer size
	  \param timeout Timeout for reading operation
	  \param bytesReadFromDevice Pointer to memory location where number of bytes have been fetched from the device is to be put
	  \return Number of body bytes fetched
	*/
	size_t read(char * bodyBuffer, size_t bodyBufferSize, const Timeout& timeout = Timeout::defaultTimeout(), size_t * bytesReadFromDevice = 0);
	//! Return the state of the parser
	inline ParserState parserState() const
	{
		return _parserState;
	}
	//! Returns TRUE if the HTTP-message has been completely fetched
	inline bool isCompleted() const
	{
		return _parserState == MessageCompleted;
	}

	//! Resets reader
	virtual void reset();
protected:
	//! Sets the message is bad
	/*!
	  \param error Constant reference to the error occured
	*/
	inline void setIsBad(const AbstractError& error)
	{
		_errorAutoPtr.reset(error.clone());
	}

	virtual bool isAllowedInFirstToken(char ch) const = 0;
	virtual void appendToFirstToken(char ch) = 0;
	virtual bool isAllowedInSecondToken(char ch) const = 0;
	virtual void appendToSecondToken(char ch) = 0;
	virtual bool isAllowedInThirdToken(char ch) const = 0;
	virtual void appendToThirdToken(char ch) = 0;
private:
	HttpMessageStreamReader();

	/*enum PrivateConstants {
		DefaultMaxHeaderNameLength = 256,
		DefaultMaxHeaderValueLength = 4096
	};*/
	
	//! Parsing next character method
	/*!
	  \param ch Next character to parse
	  \return true if next body byte has been fetched and saved to _bodyByte member
	*/
	bool parse(char ch);
	void appendHeader();

	void parseHeader(char ch, bool isTrailer);
	void parseHeaderName(char ch, bool isTrailer);
	void parseHeaderValue(char ch, bool isTrailer);
	void parseHeaderValueLF(char ch, bool isTrailer);
	void parseHeaderValueLWS(char ch, bool isTrailer);

	AbstractIODevice& _device;
	std::vector<char> _buffer;
	size_t _bufferSize;
	size_t _bufferPosition;
	ParserState _parserState;
	std::auto_ptr<AbstractError> _errorAutoPtr;
	size_t _pos;
	size_t _line;
	size_t _col;
	std::string _headerFieldName;
	std::string _headerFieldValue;
	Http::Params _header;
	size_t _contentLength;
	size_t _identityBodyBytesParsed;
	std::string _chunkSizeStr;
	size_t _chunkSize;
	size_t _chunkBytesParsed;
	char _bodyByte;
	size_t _maxHeaderNameLength;
	size_t _maxHeaderValueLength;

	friend class HttpRequestReader;
};

} // namespace isl

#endif
