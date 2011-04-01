#ifndef ISL__INTERVAL_SET__HXX
#define ISL__INTERVAL_SET__HXX

#include <algorithm>
#include <list>
#include <string>
#include <sstream>

//#include <iostream>	// TODO To be removed

namespace isl
{

template <typename T> class IntervalSet
{
public:
	IntervalSet() :
		_intervals()
	{}
	IntervalSet(const T& val) :
		_intervals()
	{
		add(val);
	}
	IntervalSet(const T& bound1, const T& bound2) :
		_intervals()
	{
		add(bound1, bound2);
	}

	void add(const T& val)
	{
		add(val, val);
	}
	void add(const T& bound1, const T& bound2)
	{
		Interval newInterval(bound1, bound2);

		//std::cout << "Adding interval: " << newInterval.toString() << std::endl;

		IntervalsRange affectedIntervals = intersectedRange(newInterval);
		if (affectedIntervals.first == affectedIntervals.second) {
			insert(affectedIntervals.second, newInterval);
			return;
		}
		T affectedLeftBound = (*affectedIntervals.first).leftBound();
		typename Intervals::iterator beforeEndPos = affectedIntervals.second;
		--beforeEndPos;
		T affectedRightBound = (*beforeEndPos).rightBound();
		_intervals.erase(affectedIntervals.first, affectedIntervals.second);
		insert(affectedIntervals.second,
				 Interval(std::min(newInterval.leftBound(), affectedLeftBound), std::max(newInterval.rightBound(), affectedRightBound)));
	}
	void remove(const T& val)
	{
		remove(val, val);
	}
	void remove(const T& bound1, const T& bound2)
	{
		Interval newInterval(bound1, bound2);

		//std::cout << "Removing interval: " << newInterval.toString() << std::endl;

		IntervalsRange affectedIntervals = intersectedRange(newInterval);
		if (affectedIntervals.first == affectedIntervals.second) {
			return;
		}
		T affectedLeftBound = (*affectedIntervals.first).leftBound();
		typename Intervals::iterator beforeEndPos = affectedIntervals.second;
		--beforeEndPos;
		T affectedRightBound = (*beforeEndPos).rightBound();
		_intervals.erase(affectedIntervals.first, affectedIntervals.second);
		if (affectedLeftBound < newInterval.leftBound()) {
			_intervals.insert(affectedIntervals.second, Interval(affectedLeftBound, newInterval.leftBound() - 1));
		}
		if (newInterval.rightBound() < affectedRightBound) {
			_intervals.insert(affectedIntervals.second, Interval(newInterval.rightBound() + 1, affectedRightBound));
		}
	}
	bool contains(const T val) const
	{
		if (_intervals.empty()) {
			return false;
		}
		typename Intervals::const_iterator intervalPos = std::lower_bound(_intervals.begin(), _intervals.end(), Interval(val, val), less);
		return (intervalPos != _intervals.end()) && ((*intervalPos).leftBound() <= val);
	}
	std::string toString() const
	{
		std::ostringstream res;
		for (typename Intervals::const_iterator i = _intervals.begin(); i != _intervals.end(); ++i) {
			if (i != _intervals.begin()) {
				res << ", ";
			}
			res << (*i).toString();
		}
		return res.str();
	}
	inline bool empty() const
	{
		return _intervals.empty();
	}
	std::list< std::pair<T, T> > intervals() const
	{
		std::list< std::pair<T, T> > result;
		for (typename Intervals::const_iterator i = _intervals.begin(); i != _intervals.end(); ++i) {
			result.push_back(std::pair<T, T>((*i).leftBound(), (*i).rightBound()));
		}
		return result;
	}
private:
	class Interval
	{
	public:
		Interval(const T& leftBound, const T& rightBound) :
			_leftBound(std::min(leftBound, rightBound)),
			_rightBound(std::max(leftBound, rightBound))
		{}

		inline T leftBound() const
		{
			return _leftBound;
		}
		inline void setLeftBound(const T& val)
		{
			_leftBound = val;
		}
		inline T rightBound() const
		{
			return _rightBound;
		}
		inline void setRightBound(const T& val)
		{
			_rightBound = val;
		}
		inline bool contains(const T& val) const
		{
			return ((_leftBound <= val) && (val >= _rightBound));
		}
		std::string toString() const
		{
			std::ostringstream res;
			res << '[' << _leftBound << ", " << _rightBound << ']';
			return res.str();
		}
		bool intersects(const Interval& val) const
		{
			return (leftBound() <= val.leftBound() && rightBound() >= val.leftBound()) ||
				(val.leftBound() <= leftBound() && val.rightBound() >= leftBound());
		}
	private:
		Interval();

		T _leftBound;
		T _rightBound;
	};
	typedef std::list<Interval> Intervals;
	typedef std::pair<typename Intervals::iterator, typename Intervals::iterator> IntervalsRange;

	// Helper predicate for use in std::lower_bound() algorithm
	static inline bool less(const Interval& interval1, const Interval& interval2)
	{
		return (interval1.rightBound() < interval2.rightBound());
	}
	// Returns intervals range [first, second) intersected by the supplied one
	IntervalsRange intersectedRange(const Interval& interval)
	{
		if (_intervals.empty()) {

			//std::cout << "Empty interval set" << std::endl;

			return IntervalsRange(_intervals.end(), _intervals.end());
		}
		typename Intervals::iterator endPos = std::lower_bound(_intervals.begin(), _intervals.end(), interval, less);
		if (endPos == _intervals.end()) {
			if (_intervals.back().rightBound() < interval.leftBound()) {

				//std::cout << "All intervals are to the left of the supplied one" << std::endl;

				return IntervalsRange(_intervals.end(), _intervals.end());
			}
		} else {
			if ((*endPos).leftBound() <= interval.rightBound()) {
				++endPos;
			} else if (endPos == _intervals.begin()) {

				//std::cout << "All intervals are to the right of the supplied one" << std::endl;

				return IntervalsRange(_intervals.begin(), _intervals.begin());
			}
		}
		typename Intervals::iterator beginPos = endPos;
		--beginPos;
		if ((*beginPos).rightBound() < interval.leftBound()) {

			//std::cout << "Supplied interval is not intersecting any another one" << std::endl;

			return IntervalsRange(endPos, endPos);
		}
		while (beginPos != _intervals.begin()) {
			typename Intervals::iterator beforeBeginPos = beginPos;
			--beforeBeginPos;
			if ((*beforeBeginPos).rightBound() < interval.leftBound()) {
				break;
			}
			beginPos = beforeBeginPos;
		}

		//std::cout << "Intersected intervals: ";
		//for (typename Intervals::iterator i = beginPos; i != endPos; ++i) {
		//	if (i != beginPos) {
		//		std::cout << ", ";
		//	}
		//	std::cout << (*i).toString();
		//}
		//std::cout << std::endl;

		return IntervalsRange(beginPos, endPos);
	}
	// Inserting interval or merging with existing ones if applicable
	void insert(typename Intervals::iterator pos, const Interval& interval)
	{
		// Checkout left merging
		bool leftMergingRequired = false;
		typename Intervals::iterator beforePos;
		if (pos != _intervals.begin()) {
			beforePos = pos;
			--beforePos;
			if (((*beforePos).rightBound() + 1) == interval.leftBound()) {
				leftMergingRequired = true;
			}
		}
		// Checkout right merging
		bool rightMergingRequired = ((pos != _intervals.end()) && (interval.rightBound() == ((*pos).leftBound() - 1)));

		//std::cout << (leftMergingRequired ? "Left merging required" : "No left merging required") << std::endl;
		//std::cout << (rightMergingRequired ? "Right merging required" : "No right merging required") << std::endl;

		// Inserting/merging interval
		if (leftMergingRequired) {
			if (rightMergingRequired) {
				(*beforePos).setRightBound((*pos).rightBound());
				_intervals.erase(pos);
			} else {
				(*beforePos).setRightBound(interval.rightBound());
			}
		} else if (rightMergingRequired) {
			(*pos).setLeftBound(interval.leftBound());
		} else {
			_intervals.insert(pos, interval);
		}
	}

	Intervals _intervals;
};

} // namespace isl

#endif
