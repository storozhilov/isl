#ifndef ISL__ABSTRACT_GRAMMAR__HXX
#define ISL__ABSTRACT_GRAMMAR__HXX

#include <string>
#include <map>
#include <vector>

namespace isl
{

template <typename Ch> class BasicAbstractGrammar
{
public:
	BasicAbstractGrammar() :
		_symbols(),
		_symbolNamesCache(),
		_productions(),
		_startSymbolIndex(-1)
	{}
	virtual ~BasicAbstractGrammar()
	{
		for (typename Symbols::iterator i = _symbols.begin(); i != _symbols.end(); ++i) {
			delete (*i);
		}
	}

	//typedef std::wstring SymbolName;					// ??? or
	typedef std::string SymbolName;						// ??? or replace by std::string at all...

	SymbolName toString() const
	{
		SymbolName result;
		for (typename Productions::const_iterator i = _productions.begin(); i != _productions.end(); ++i) {
			if (!result.empty()) {
				result += '\n';
			}
			result += productionToString((*i).first, (*i).second);
		}
		return result;
	}
protected:
	class AbstractSymbol
	{
	public:
		AbstractSymbol(const SymbolName& name, bool isTerminal) :
			_name(name),
			_isTerminal(isTerminal)
		{}
		virtual ~AbstractSymbol()
		{}

		virtual AbstractSymbol * clone() const = 0;

		inline SymbolName name() const
		{
			return _name;
		}
		inline bool isTerminal() const
		{
			return _isTerminal;
		}
		bool canAppend(const std::basic_string<Ch>& str, Ch ch) const
		{
			if (!isTerminal) {
				// TODO
				throw std::runtime_error(std::string("Appending to nonterminal symbol '") + _name + "' is not possible");
			}
			return canAppendImplementation(str, ch);
		}
		bool completed(const std::basic_string<Ch>& str) const
		{
			if (!isTerminal) {
				// TODO
				throw std::runtime_error(std::string("Nonterminal '") + _name + "' could not be completed");	// ???
			}
			return completedImplementation(str);
		}
	protected:
		virtual bool canAppendImplementation(const std::basic_string<Ch>& str, Ch ch) const
		{
			throw std::runtime_error("Method AbstractSymbol::canAppendImplementation() should be reimplemented in subclass");
		}
		virtual bool completedImplementation(const std::basic_string<Ch>& str) const
		{
			throw std::runtime_error("Method AbstractSymbol::completed() should be reimplemented in subclass");
		}
	private:
		AbstractSymbol();

		SymbolName _name;
		bool _isTerminal;
	};

	class ProductionRule
	{
	public:
		ProductionRule(const AbstractSymbol& lhs) :
			_lhsName(lhs.name()),
			_rhsItems()
		{}

		void addRhsItem(const AbstractSymbol& item, bool isOptional)
		{
			_rhsItems.push_back(RhsItem(item.name(), isOptional));
		}
		void reset()
		{
			_rhsItems.clear();
		}
		void reset(const AbstractSymbol& lhs)
		{
			_lhsName = lhs.name();
			_rhsItems.clear();
		}
	private:
		ProductionRule();

		typedef std::pair<SymbolName, bool> RhsItem;
		typedef std::vector<RhsItem> RhsItems;

		SymbolName _lhsName;
		RhsItems _rhsItems;
	};

	typedef std::vector<AbstractSymbol *> ProductionSource;

	unsigned int addSymbol(const AbstractSymbol& symbol)
	{
		if (symbol.name().empty()) {
			// TODO
			throw std::runtime_error("Empty name of the symbol to add to the grammar");
		}
		if (_symbolNamesCache.find(symbol.name()) != _symbolNamesCache.end()) {
			// TODO
			throw std::runtime_error(std::string("Symbol '") + symbol.name() + "' already exists in grammar");
		}
		_symbols.push_back(symbol.clone());
		_symbolNamesCache.insert(typename SymbolNamesCache::value_type(symbol.name(), _symbols.size() - 1));
		return _symbols.size() - 1;
	}
	unsigned int setStartSymbol(const AbstractSymbol& symbol)
	{
		_startSymbolIndex = symbolIndex(symbol);
	}
	unsigned int symbolIndex(const AbstractSymbol& symbol) const
	{
		SymbolNamesCache::const_iterator pos = _symbolNamesCache.find(symbol.name());
		if (pos == _symbolNamesCache.end()) {
			// TODO
			throw std::runtime_error(std::string("Symbol '") + symbol.name() + "' not found in grammar");
		}
		return (*pos).second;
	}
	void addProduction(const AbstractSymbol& dest, const ProductionSource& source)
	{
		ProductionSourceIndexes sourceIndexes;
		for (typename ProductionSource::const_iterator i = source.begin(); i != source.end(); ++i) {
			sourceIndexes.push_back(symbolIndex(**i));
		}
		addProduction(symbolIndex(dest), sourceIndexes);
	}
private:
	BasicAbstractGrammar(const BasicAbstractGrammar&);					// No copy

	BasicAbstractGrammar& operator=(const BasicAbstractGrammar&);				// No copy

	typedef std::vector<AbstractSymbol *> Symbols;
	typedef std::map<SymbolName, unsigned int> SymbolNamesCache;
	typedef std::vector<unsigned int> ProductionSourceIndexes;
	typedef std::multimap<unsigned int, ProductionSourceIndexes> Productions;

	SymbolName productionToString(unsigned int destIndex, const ProductionSourceIndexes& sourceIndexes) const
	{
		SymbolName result = _symbols[destIndex]->name();
		result += ": ";
		for (unsigned int i = 0; i < sourceIndexes.size(); ++i) {
			if (i > 0) {
				result += ' ';
			}
			result += _symbols[sourceIndexes[i]]->name();
		}
		return result;
	}
	bool productionExists(unsigned int destIndex, const ProductionSourceIndexes& sourceIndexes) const
	{
		std::pair<Productions::const_iterator, Productions::const_iterator> range = _productions.equal_range(destIndex);
		for (typename Productions::const_iterator i = range.first; i != range.second; ++i) {
			if ((*i).second.size() != sourceIndexes.size()) {
				continue;
			}
			for (unsigned int j = 0; j < (*i).second.size(); ++j) {
				if ((*i).second[j] != sourceIndexes[j]) {
					continue;
				}
			}
			return true;
		}
		return false;
	}

	void addProduction(unsigned int destIndex, const ProductionSourceIndexes& sourceIndexes)
	{
		if (productionExists(destIndex, sourceIndexes)) {
			// TODO
			throw std::runtime_error(std::string("Production '") + productionToString(destIndex, sourceIndexes) + "' is already exists");
		}
		_productions.insert(typename Productions::value_type(destIndex, sourceIndexes));
	}

	Symbols _symbols;
	SymbolNamesCache _symbolNamesCache;
	Productions _productions;
	int _startSymbolIndex;
};

typedef BasicAbstractGrammar<char> AbstractGrammar;
typedef BasicAbstractGrammar<wchar_t> WAbstractGrammar;

class TestGrammar : public WAbstractGrammar
{
public:
	TestGrammar() :
		WAbstractGrammar()
	{
		StartSymbol s;
		addSymbol(s);
		setStartSymbol(s);
		ASymbol a;
		addSymbol(a);
		BSymbol b;
		addSymbol(b);
		ProductionSource productionSource;
		// Adding "S: a S b" production
		productionSource.push_back(&a);
		productionSource.push_back(&s);
		productionSource.push_back(&b);
		addProduction(s, productionSource);
		// Adding "S: b a" production
		productionSource.clear();
		productionSource.push_back(&b);
		productionSource.push_back(&a);
		addProduction(s, productionSource);

		// testing rules
		ProductionRule rule(s);
		rule.addRhsItem(a, false);
		rule.addRhsItem(s, false);
		rule.addRhsItem(b, false);
		//addRule(rule);
		rule.reset();
		rule.addRhsItem(b, false);
		rule.addRhsItem(a, false);
		//addRule(rule);
	}
private:
	TestGrammar(const TestGrammar&);							// No copy

	TestGrammar& operator==(const TestGrammar&);						// No copy

	class StartSymbol : public AbstractSymbol
	{
	public:
		StartSymbol() :
			AbstractSymbol("S", false)
		{}
		virtual AbstractSymbol * clone() const
		{
			return new StartSymbol();
		}
	};
	class ASymbol : public AbstractSymbol
	{
	public:
		ASymbol() :
			AbstractSymbol("a", true)
		{}
		virtual AbstractSymbol * clone() const
		{
			return new ASymbol();
		}
	private:
		virtual bool canAppendImplementation(const std::wstring& str, wchar_t ch) const
		{
			return (str.empty() && (ch == L'a'));
		}
		virtual bool completedImplementation(const std::wstring& str) const
		{
			return str == L"a";
		}
	};
	class BSymbol : public AbstractSymbol
	{
	public:
		BSymbol() :
			AbstractSymbol("b", true)
		{}
		virtual AbstractSymbol * clone() const
		{
			return new BSymbol();
		}
	private:
		virtual bool canAppendImplementation(const std::wstring& str, wchar_t ch) const
		{
			return (str.empty() && (ch == L'b'));
		}
		virtual bool completedImplementation(const std::wstring& str) const
		{
			return str == L"b";
		}
	};
};

template <typename Ch> class BasicParser
{
public:
	BasicParser(BasicAbstractGrammar<Ch>& grammar) :
		_grammar(grammar)
	{}
private:
	BasicParser();

	// ParseTreeNode

	BasicAbstractGrammar<Ch>& _grammar;
};

typedef BasicParser<char> Parser;
typedef BasicParser<wchar_t> WParser;

} // namespace isl

#endif

