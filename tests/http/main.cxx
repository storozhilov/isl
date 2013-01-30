#include <isl/common.hxx>
#include <isl/HttpMessageParser.hxx>
#include <iostream>
#include <string.h>
//#include <fstream>

#define BUF_SIZE 8

const char buf[] =
	// Bodyless HTTP-request
	"GET /index.html HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"\r\n"
	// Chuncked-encoded HTTP-request
	"GET /index.html?q=%D0%B0%D0%B1%D0%B2 HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"X-Foo: bar\r\n"
	"Transfer-Encoding: chunked\r\n"
	"\r\n"
	"a\r\n"
	"1234567890\r\n"
	"b\r\n"
	"12345678901\r\n"
	"0\r\n"
	"X-Bar: foo\r\n"
	"\r\n"
	// Identity-encoded HTTP-request
	"GET /index.html HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"X-Foo: bar\r\n"
	"Content-Length: 10\r\n"
	"\r\n"
	"1234567890"
	// Trailerless chuncked-encoded HTTP-request
	"GET /index.html HTTP/1.1\r\n"
	"Host: localhost\r\n"
	"X-Foo: bar\r\n"
	"Transfer-Encoding: chunked\r\n"
	"\r\n"
	"a\r\n"
	"1234567890\r\n"
	"b\r\n"
	"12345678901\r\n"
	"0\r\n"
	"\r\n";

int main(int argc, char *argv[])
{



	std::ostringstream body;
	size_t bytesParsed = 0;
	isl::HttpMessageParser parser(20, 4096, 20);
	while (bytesParsed < strlen(buf)) {
		body.str("");
		bytesParsed += parser.parse(buf + bytesParsed, sizeof(buf) - bytesParsed, body);
		if (parser.isBad()) {
			std::cerr << parser.error()->message() << std::endl;
			return 1;
		}
		if (parser.isCompleted()) {
			std::cout << "-------------------------------------" << std::endl;
			std::cout << "HTTP-message has been parsed:" << std::endl << std::endl <<
				parser.firstToken() << ' ' << parser.secondToken() << ' ' << parser.thirdToken() << std::endl;
			for (isl::Http::Params::const_iterator i = parser.header().begin(); i != parser.header().end(); ++i) {
				std::cout << i->first << ": " << i->second << std::endl;
			}
			if (!body.str().empty()) {
				std::cout << std::endl << std::endl << body.str() << std::endl;
			}
		}
	}

	/*char * readBuf = buf;
	char bodyBuf[BUF_SIZE];
	size_t bytesRead = 0;
	size_t bytesParsed = 0;
	isl::HttpMessageParser parser(20, 4096, 20);
	while (bytesRead < strlen(buf)) {
		if (bytesParsed >= BUF_SIZE) {
			// Refilling the read buffer
			
			bytesParsed = 0;
		}
		if (bytesParsed < BUF_SIZE) {
			// Parsing the rest of the read buffer

		}
	}*/


	/*char bodyBuf[4096];
	size_t bytesParsed = 0;
	isl::HttpMessageParser parser(20, 4096, 20);
	std::ostringstream body;
	while (bytesParsed < strlen(buf)) {
		std::pair<size_t, size_t> res = parser.parse(buf + bytesParsed, sizeof(buf) - bytesParsed, bodyBuf, sizeof(bodyBuf));
		if (parser.isBad()) {
			std::cerr << parser.error()->message() << std::endl;
			return 1;
		}
		if (parser.isCompleted()) {
			std::cout << "HTTP-message has been parsed, first token is: '" << parser.firstToken() << "'" << std::endl;
			body 
		}
	}*/
	/*if (argc < 2) {
		std::cout << "Usage ./http <http_message_file>" << std::endl;
	}
	std::fstream f(argv[1]);
	if (f.fail()) {
		std::cerr << "Error opening file " << argv[1] << " for reading" << std::endl;
		return 1;
	}
	isl::HttpMessageParser p(20, 4096, 20);
	while (!f.eof()) {
		char ch = (char) f.get();
		if (f.eof()) {
			break;
		}
		if (ch == '\n') {
			p.parse('\r');
			if (p.isBad()) {
				std::cout << p.error()->message() << std::endl;
				return 1;
			}
		}
		if (p.parse(ch)) {
			std::cout << ch;
		}
		if (p.isBad()) {
			std::cout << p.error()->message() << std::endl;
			return 1;
		}
		if (p.isCompleted()) {
			std::cout << std::endl;
			std::cout << "Complete HTTP-message \"" << p.firstToken() << ' ' << p.secondToken() << ' ' << p.thirdToken() << "\" has been received, header is:" << std::endl;
			for (isl::Http::Params::const_iterator i = p.header().begin(); i != p.header().end(); ++i) {
				std::cout << '\t' << i->first << ": " << i->second << std::endl;
			}
		}
	}*/
}
