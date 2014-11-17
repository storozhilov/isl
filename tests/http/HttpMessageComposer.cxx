#include <gtest/gtest.h>
#include <isl/HttpMessageComposer.hxx>
#include <isl/Exception.hxx>
#include <string.h>

#define BUFFER_SIZE 4096U
#define HEADER_SIZE 1024U

using namespace isl;

TEST(HttpMessageComposer, compose)
{
	static const char * Data = "This is some data to send";
	static size_t DataLen = strlen(Data);
	static const char * BodylessNonChunkedEnvelope =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n\r\n";
	static const char * BodylessNonChunkedPacket =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n\r\n";
	static const char * NonChunkedEnvelope =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 25\r\n"
		"Content-Type: text/html\r\n"
		"\r\n";
	static const char * NonChunkedPacket =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 25\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"This is some data to send";
	static const char * FirstChunkEnvelope =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Transfer-Encoding: chunked\r\n"
		"\r\n"
		"19\r\n";
	static const char * ChunkEnvelope =
		"\r\n"
		"19\r\n";
	static const char * LastChunkEnvelope =
		"\r\n"
		"0\r\n"
		"Content-Length: 123456\r\n"
		"Content-Type: text/html\r\n"
		"Transfer-Encoding: foobar\r\n"
		"\r\n";

	char buf[BUFFER_SIZE];
	memcpy(buf + HEADER_SIZE, Data, DataLen);
	HttpMessageComposer composer("HTTP/1.1", "200", "OK");
	Http::Headers header;
	header.insert(Http::Headers::value_type("Content-Type", "text/html"));
	header.insert(Http::Headers::value_type("Content-Length", "123456"));
	header.insert(Http::Headers::value_type("Transfer-Encoding", "foobar"));
	
	const std::string& e1 = composer.compose(header);
	EXPECT_EQ(BodylessNonChunkedEnvelope, e1);
	HttpMessageComposer::Packet p1 = composer.compose(header, buf, HEADER_SIZE);
	EXPECT_EQ(BodylessNonChunkedPacket, std::string(p1.first, p1.second));
	const std::string& e2 = composer.compose(header, DataLen);
	EXPECT_EQ(NonChunkedEnvelope, e2);
	HttpMessageComposer::Packet p2 = composer.compose(header, buf, HEADER_SIZE, DataLen);
	EXPECT_EQ(NonChunkedPacket, std::string(p2.first, p2.second));

	EXPECT_THROW(composer.composeFirstChunk(header, 0U), Exception);
	const std::string& e3 = composer.composeFirstChunk(header, DataLen);
	EXPECT_EQ(FirstChunkEnvelope, e3);
	EXPECT_THROW(composer.composeChunk(0U), Exception);
	const std::string& e4 = composer.composeChunk(DataLen);
	EXPECT_EQ(ChunkEnvelope, e4);
	const std::string& e5 = composer.composeLastChunk(header);
	EXPECT_EQ(LastChunkEnvelope, e5);
}
