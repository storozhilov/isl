#include <isl/Debug.hxx>
#include <sstream>

namespace isl
{

std::string composeSourceLocation(SOURCE_LOCATION_ARGS_DECLARATION)
{
	std::ostringstream sstr;
	sstr << SOURCE_LOCATION_ARGS_FILE << '(' << SOURCE_LOCATION_ARGS_LINE << "), " << SOURCE_LOCATION_ARGS_FUNCTION;
	return sstr.str();
}

} // namespace isl
