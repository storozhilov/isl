#ifndef ISL__QUANTIFIER__HXX
#define ISL__QUANTIFIER__HXX

#include <limits>
#include <string>

namespace isl
{

class Quantifier
{
public:
	explicit Quantifier(unsigned int min) :
		_min(min),
		_max(infinity())
	{}
	Quantifier(unsigned int min, unsigned int max) :
		_min(std::min<unsigned int>(min, max)),
		_max(std::max<unsigned int>(min, max))
	{}

	inline unsigned int min() const
	{
		return _min;
	}
	inline unsigned int max() const
	{
		return _max;
	}

	static unsigned int infinity()
	{
		return std::numeric_limits<unsigned int>::max();
	}
	static Quantifier fromZeroToOne()
	{
		return Quantifier(0, 1);
	}
	static Quantifier onlyOne()
	{
		return Quantifier(1, 1);
	}
	static Quantifier fromOneToInfinity()
	{
		return Quantifier(1, infinity());
	}
	static Quantifier fromZeroToInfinity()
	{
		return Quantifier(0, infinity());
	}
#ifdef ISL__DEBUGGING_ON
	template <typename Ch> std::basic_string<Ch> debug() const
	{
		std::basic_ostringstream<Ch> oss;
		if (_min == 1 && _max == 1) {
			// Nothing to do
		} else if (_min == 0 && _max == 1) {
			oss << '?';
		} else if (_min == 0 && _max == infinity()) {
			oss << '*';
		} else if (_min == 1 && _max == infinity()) {
			oss << '+';
		} else {
			oss << '{';
			if (_min != 0) {
				oss << _min;
			}
			oss << ',';
			if (_max != infinity()) {
				oss << _max;
			}
			oss << '}';
		}
		return oss.str();
	}
#endif
private:
	unsigned int _min;
	unsigned int _max;
};

} // namespace isl

#endif
