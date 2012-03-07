#include <isl/UUID.hxx>

#include <stdlib.h>
#include <time.h>

namespace isl
{

std::string UUID::generate()
{
	// TODO This is just dummy realization
	srandom(time(NULL));
	std::string result;
	for (int i = 0; i < 20; ++i) {
		result += 'A' + random() % 26;
	}
	return result;
}

} // namespace isl
