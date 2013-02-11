#include <isl/DateTime.hxx>
#include <isl/TimeSpec.hxx>
#include <iostream>

int main(int argc, char *argv[])
{
	//struct timespec ts1 = isl::TimeSpec::now();
	/*struct timespec ts2 = isl::operator+(ts1, isl::Timeout(0, 999999999).timeSpec());
	struct timespec ts3 = isl::operator-(ts1, isl::Timeout(1, 999999999).timeSpec());
	std::clog << "ts1 = {" << ts1.tv_sec << ", " << ts1.tv_nsec << "}" << std::endl;
	std::clog << "ts2 = {" << ts2.tv_sec << ", " << ts2.tv_nsec << "}" << std::endl;
	std::clog << "ts3 = {" << ts3.tv_sec << ", " << ts3.tv_nsec << "}" << std::endl;

	isl::DateTime dt(2012, 12, 21, 10, 30, 45, 800000000);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt = dt + isl::Timeout(1, 200000001);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt += isl::Timeout(0, 999999999);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt -= isl::Timeout(0, 999999999);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt = dt - isl::Timeout(1, 200000001);
	std::clog << "dt = " << dt.toString() << std::endl;

	isl::DateTime sf(2012, 12, 21, 23, 59, 59);
	isl::DateTime fb(2012, 12, 22, 0, 0, 1);*/
	//size_t expirationsAmount = isl::DateTime::expirationsInFrame(sf, fb, isl::Timeout(0, 200000000));
	//std::clog << "expirationsAmount = " << expirationsAmount << std::endl;
}
