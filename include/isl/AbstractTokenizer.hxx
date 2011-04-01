#ifndef ISL__ABSTRACT_TOKENISER__HXX
#define ISL__ABSTRACT_TOKENISER__HXX

#include <isl/Enum.hxx>
#include <string>
#include <vector>
#include <list>

#include <iostream>	// To be removed

namespace isl
{

template <typename Ch> class BasicAbstractProduction
{
protected:
	// Subclass your states from this class
	class AbstractState
	{
	public:
		virtual ~AbstractState() {}

		virtual AbstractState * clone() const = 0;
		virtual std::wstring name() const = 0;
	};
	typedef Enum<AbstractState> State;
	// Predefined states:
	class StartingState : public AbstractState
	{
	public:
		virtual StartingState * clone() const
		{
			return new StartingState(*this);
		}
		virtual std::wstring name() const
		{
			return std::wstring(L"Start parsing");
		}
	};
	class ErrorState : public AbstractState
	{
	public:
		virtual ErrorState  * clone() const
		{
			return new ErrorState(*this);
		}
		virtual std::wstring name() const
		{
			return std::wstring(L"Error");
		}
	};

	typedef std::list<BasicAbstractProduction<Ch> *> ChildrenProductions;
	typedef typename ChildrenProductions::iterator ChildrenProductionIterator;
	typedef std::list<ChildrenProductionIterator> StartProductions;
	////typedef std::list<ChildrenProductions::iterator> CurrentProductions;
	class CurrentProduction {
	public:
		CurrentProduction(ChildrenProductionIterator production) :
			_state(StartingState()),
			_production(production)
		{}
		inline State state() const
		{
			return _state;
		}
		inline ChildrenProductionIterator production() const
		{
			return _production;
		}
	private:
		CurrentProduction();

		State _state;
		ChildrenProductionIterator _production;
	};
	//typedef std::list<CurrentProduction> CurrentProductions;

	//class Possible
public:
	BasicAbstractProduction() :
		_state(StartingState()),
		_childrenProductions(),
		_token()
	{}
	virtual ~BasicAbstractProduction()
	{
		for (typename ChildrenProductions::iterator i = _childrenProductions.begin(); i != _childrenProductions.end(); ++i) {
			delete (*i);
		}
	}

	virtual bool isTerminal() const = 0;
	virtual bool parseSymbolImplementation(Ch ch) = 0;			// True - symbol accepted, False - otherwise

	virtual bool isInErrorState() const
	{
		return isInState<ErrorState>();
	}

	template <typename S> inline bool isInState() const
	{
		return _state.template equals<S>();				// <-- Funny construction! :) Otherwise an error while compiling.
	}
	inline State state() const
	{
		return _state;
	}
	inline std::basic_string<Ch> token() const
	{
		return _token;
	}
	bool parseSymbol(Ch ch)							// True - symbol accepted, False - otherwise
	{
		//if (
		// 1. Call 
	}
protected:
	inline void appendToToken(Ch ch)
	{
		if (isTerminal()) {
			_token += ch;
		}
	}
private:
	State _state;
	ChildrenProductions _childrenProductions;
	std::basic_string<Ch> _token;
};

typedef BasicAbstractProduction<char> AbstractProduction;
typedef BasicAbstractProduction<wchar_t> WAbstractProduction;

class TestProduction : public WAbstractProduction
{
private:
	class InitialState : public AbstractState
	{
		virtual AbstractState * clone() const
		{
			return new InitialState(*this);
		}
		virtual std::wstring name() const
		{
			return std::wstring(L"InitialTest");
		}
	};
public:
	TestProduction() :
		WAbstractProduction()
	{}

	virtual bool isTerminal() const
	{
		return false;
	}
};

template <typename Ch> class BasicAbstractTokenizer
{
public:
	class AbstractToken
	{
	public:
		virtual ~AbstractToken() {}

		virtual AbstractToken * clone() const = 0;
		virtual std::basic_string<Ch> str() const = 0;
	};
	typedef Enum<AbstractToken> Token;
	typedef std::list<AbstractToken *> TokenList;

	class AbstractProduction
	{
	public:
		virtual ~AbstractProduction() {}

		virtual AbstractProduction * clone() const = 0;
	};

	bool tokenize(const Ch * data, unsigned int size, bool endOfInput)
	{
		for (unsigned int i = 0; i < size; ++i) {
			std::wcout << '"' << data[i] << "\" tokenized" << std::endl;
		}
	}
	inline bool tokenize(const std::basic_string<Ch>& str, bool endOfInput)
	{
		return tokenize(str.data(), str.size(), endOfInput);
	}
private:

};

} // namespace isl

#endif

