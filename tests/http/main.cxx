#include <isl/common.hxx>
#include <isl/HttpMessageParser.hxx>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
	if (argc < 2) {
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
	}
}
