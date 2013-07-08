#ifndef ISL__STATE_SET__HXX
#define ISL__STATE_SET__HXX

#include <isl/WaitCondition.hxx>
#include <set>
#include <memory>
#include <functional>

namespace isl
{

//! State set
/*!
  Multithread state set, which is used in ISL for inter-thread control.
  \tparam S State type
  TODO: template <typename O> SetType modify(const O& operation);
*/
template <typename S> class StateSet
{
public:
	typedef S StateType;							//!< State type
	typedef std::set<StateType> SetType;					//!< Stat set type

	//! Creates an empty state set
	StateSet() :
		_set(),
		_cond()
	{}
	//! Creates and inializes state set
	/*!
	  \param set Constant reference to set to initialize from
	*/
	StateSet(const SetType& set) :
		_set(set),
		_cond()
	{}
	//! Returns a reference to state set condition variable
	WaitCondition& cond()
	{
		return _cond;
	}
	//! Awaits for state set condition to be satisfied
	/*!
	  \param c Condition functor
	  \param limit Time limit to await
	  \param isSatisfied Pointer to boolean memory location where condition result is to be put or 0 otherwise
	  \return Current state set
	  \tparam C Condition functor type
	*/
	template <typename C> SetType await(const C& c, const Timestamp& limit, bool * isSatisfied = 0)
	{
		MutexLocker locker(_cond.mutex());
		do {
			if (!c(_set)) {
				continue;
			}
			if (isSatisfied) {
				*isSatisfied = true;
			}
			return _set;
		} while (_cond.wait(limit));
		if (isSatisfied) {
			*isSatisfied = false;
		}
		return _set;
	}
	//! Awaits for state to be in state set
	/*!
	  \param state Constant reference to state to await for
	  \param limit Timestamp limit to wait for
	  \param isSatisfied Pointer to boolean memory location where condition result is to be put or 0 otherwise
	  \return Current state set
	*/
	inline SetType await(const StateType& state, const Timestamp& limit, bool * isSatisfied = 0)
	{
		return await(std::bind2nd(IsInSet(), state), limit, isSatisfied);
	}
	//! Awaits for any of the set items to be in state set
	/*!
	  \param set A constant reference to set which items are to be checked against state set (empty one is always satisfies a condition)
	  \param limit Timestamp limit to wait for
	  \param isSatisfied Pointer to boolean memory location where condition result is to be put or 0 otherwise
	  \return Current state set
	*/
	inline SetType awaitAny(const SetType& set, const Timestamp& limit, bool * isSatisfied = 0)
	{
		return await(std::bind2nd(AnyInSet(), set), limit, isSatisfied);
	}
	//! Awaits for all of the set items to be in state set
	/*!
	  \param set A constant reference to set which items are to be checked against state set (empty one does not satisfy a condition)
	  \param limit Timestamp limit to wait for
	  \param isSatisfied Pointer to boolean memory location where condition result is to be put or 0 otherwise
	  \return Current state set
	*/
	inline SetType awaitAll(const SetType& set, const Timestamp& limit, bool * isSatisfied = 0)
	{
		return await(std::bind2nd(AllInSet(), set), limit, isSatisfied);
	}
	//! Returns current state set
	SetType fetch()
	{
		MutexLocker locker(_cond.mutex());
		return _set;
	}
	//! Resets state set
	void reset()
	{
		MutexLocker locker(_cond.mutex());
		_set.clear();
		_cond.wakeAll();
	}
	//! Thread-unsafely resets state set
	void resetUnsafe()
	{
		_set.clear();
	}
	//! Inserts state into the state set
	/*!
	  \param state State to insert to the state set
	  \note It does not do wake-up if the state set have not been changed
	*/
	void insert(const StateType& state)
	{
		MutexLocker locker(_cond.mutex());
		if (_set.find(state) != _set.end()) {
			return;
		}
		_set.insert(state);
		_cond.wakeAll();
	}
	//! Inserts all set items into the state set
	/*!
	  \param set Set which items are to be inserted to the state set
	  \note It does not do wake-up if the state set have not been changed
	*/
	void insert(const SetType& set)
	{
		MutexLocker locker(_cond.mutex());
		bool setChanged = false;
		for (typename SetType::const_iterator i = set.begin(); i != set.end; ++i) {
			if (_set.find(*i) == _set.end()) {
				_set.insert(*i);
				setChanged = true;
			}
		}
		if (setChanged) {
			_cond.wakeAll();
		}
	}
	//! Removes state from the state set
	/*!
	  \param state State to remove from the state set
	  \note It does not do wake-up if the state set have not been changed
	*/
	void remove(const StateType& state)
	{
		MutexLocker locker(_cond.mutex());
		if (_set.find(state) == _set.end()) {
			return;
		}
		_set.erase(state);
		_cond.wakeAll();
	}
	//! Removes all set items from the state set
	/*!
	  \param set Set which items are to be removed from the state set
	  \note It does not do wake-up if the state set have not been changed
	*/
	void remove(const SetType& set)
	{
		MutexLocker locker(_cond.mutex());
		bool setChanged = false;
		for (typename SetType::const_iterator i = set.begin(); i != set.end; ++i) {
			if (_set.find(*i) != _set.end()) {
				_set.erase(*i);
				setChanged = true;
			}
		}
		if (setChanged) {
			_cond.wakeAll();
		}
	}
private:
	struct IsInSet : public std::binary_function<SetType, StateType, bool>
	{
		bool operator()(const SetType& set, const StateType& state) const
		{
			return set.find(state) != set.end();
		}
	};
	struct AnyInSet : public std::binary_function<SetType, SetType, bool>
	{
		bool operator()(const SetType& container, const SetType& content) const
		{
			for (typename SetType::const_iterator i = content.begin(); i != content.end(); ++i) {
				if (container.find(*i) != container.end()) {
					return true;
				}
			}
			return false;
		}
	};
	struct AllInSet : public std::binary_function<SetType, SetType, bool>
	{
		bool operator()(const SetType& container, const SetType& content) const
		{
			for (typename SetType::const_iterator i = content.begin(); i != content.end(); ++i) {
				if (container.find(*i) == container.end()) {
					return false;
				}
			}
			return true;
		}
	};

	SetType _set;
	WaitCondition _cond;
};

} // namespace isl

#endif
