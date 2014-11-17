#ifndef ISL__HTTP_MESSAGE_COMPOSER__HXX
#define ISL__HTTP_MESSAGE_COMPOSER__HXX

#include <isl/Http.hxx>
#include <isl/Timestamp.hxx>
#include <string>
#include <map>
#include <list>
#include <string.h>
#include <ostream>

namespace isl
{

class HttpMessageComposer
{
public:
	typedef std::pair<const std::string&, const std::string&> Envelope;
	typedef std::pair<const char *, size_t> Packet;

	HttpMessageComposer(const std::string& firstToken, const std::string& secondToken,
			const std::string& thirdToken);
	virtual ~HttpMessageComposer();

	void reset(const std::string& firstToken, const std::string& secondToken,
			const std::string& thirdToken);
	//! Composes envelop to prepend data for single, non-chunked transmission
	const std::string& compose(const Http::Headers& header, size_t dataLen = 0U);
	//! Prepends data with envelope to compose a packet for single, non-chunked transmission
	/*!
	 * \param header Reference to header to use
	 * \param buffer Pointer to result buffer to compose packet into
	 * \param headerLen Length of the header part in result buffer
	 * \param dataLen Length of the data to send
	 * \return Pointer to HTTP-packet and it's length
	 * \note The idea behind this helper method is to have
	 *       HTTP-packet as single piece of data to be able to send it
	 *       using only one send(2) operation.
	 */
	Packet compose(const Http::Headers& header, char * buffer, size_t headerPartSize,
			size_t dataLen = 0U);
	const std::string& composeFirstChunk(const Http::Headers& header, size_t dataLen);
	Packet composeFirstChunk(const Http::Headers& header, char * buffer,
			size_t headerPartSize, size_t dataLen);
	const std::string& composeChunk(size_t dataLen);
	const std::string& composeLastChunk(const Http::Headers& header);
private:
	inline static void composeHeader(const Http::Headers& header, std::ostream& target)
	{
		for (Http::Headers::const_iterator i = header.begin(); i != header.end(); ++i) {
			target << i->first << ": " << i->second  << "\r\n";
		}
	}
	inline void composeFirstLine(std::ostream& target)
	{
		target << _firstToken << ' ' << _secondToken << ' ' << _thirdToken << "\r\n";
	}

	std::string _firstToken;
	std::string _secondToken;
	std::string _thirdToken;
	std::string _packetPrefix;
	std::string _packetPostfix;
	std::string _envelope;
};

} // namespace isl

#endif
