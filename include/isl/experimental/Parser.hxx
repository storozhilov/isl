#ifndef ISL__PARSER__HXX
#define ISL__PARSER__HXX

#include <isl/Grammar.hxx>

#include <vector>
#include <list>

//#define ISL__PARSER_DEBUGGING_ON 1

namespace isl
{

// Earley parser implementation (without lookahead)

template <typename Ch> class BasicParser
{
public:
	typedef BasicGrammar<Ch> GrammarType;

	BasicParser(GrammarType& grammar) :
		_grammar(grammar),
		_earleySets()//,
		//_pos(0)
	{}
	virtual ~BasicParser()
	{
		reset();
	}

	void reset()
	{
		for (typename EarleySets::iterator i = _earleySets.begin(); i != _earleySets.end(); ++i) {
			delete (*i);
		}
		_earleySets.clear();
	}
	bool parse(const std::basic_string<Ch>& str)
	{
		return parse(str.data(), str.size());
	}
	bool parse(const Ch * data, size_t size)
	{
		reset();

		// Init start states
		if (!_grammar.isValidSymbolHandle(_grammar._startSymbolHandle)) {
			// TODO
			throw std::runtime_error("Grammar has no start symbol");
		}
		std::pair<typename GrammarType::ConstProductionPos, typename GrammarType::ConstProductionPos> range =
			_grammar._productions.equal_range(_grammar._startSymbolHandle);
		if (range.first == _grammar._productions.end()) {
			// TODO
			throw std::runtime_error("Grammar has no productions with the start symbol as lhs");
		}
		//_earleySets.reserve(size);
		_earleySets.reserve(size + 1);
		EarleySet startEarleySet;
		for (typename GrammarType::ConstProductionPos i = range.first; i != range.second; ++i) {
			startEarleySet.push_back(EarleyItem(i, 0));
		}

		//_earleySets.push_back(startEarleySet);	// ERROR WAS THERE

		for (unsigned int pos = 0; pos < size; ++pos) {
			// Predictor section
			//for (typename EarleySet::iterator i = _earleySets[pos].begin
			
		}
	}
protected:
//	inline GrammarType& grammar()
//	{
//		return _grammar;
//	}
private:
	BasicParser();

	class EarleyItem
	{
	public:
		EarleyItem(typename GrammarType::ConstProductionPos productionPos, unsigned int refSetPos) :
			_productionPos(productionPos),
			_productionRhsPos(0),
			_refSetPos(refSetPos)
		{}
	private:
		EarleyItem();

		typename GrammarType::ConstProductionPos _productionPos;
		unsigned int _productionRhsPos;
		unsigned int _refSetPos;
	};
	typedef std::list<EarleyItem> EarleySet;
	typedef std::vector<EarleySet *> EarleySets;
	typedef typename EarleySets::iterator EarleySetPos;
	typedef typename EarleySets::const_iterator ConstEarleySetPos;

	bool addToEarleySet(unsigned int pos, const EarleySet& earleySet)
	{
		if (pos >= _earleySets.size()) {
			for (unsigned int i = 0; i < (pos - _earleySets.size() - 1); ++i) {
				_earleySets.push_back(new EarleySet());
			}
		}
		// TODO I'm here
	}

	BasicGrammar<Ch>& _grammar;
	EarleySets _earleySets;
	//unsigned int _pos;
};

typedef BasicParser<char> Parser;
typedef BasicParser<wchar_t> WParser;

} // namespace isl

#endif
