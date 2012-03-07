#ifndef ISL__GRAMMAR__HXX
#define ISL__GRAMMAR__HXX

#include <isl/IntervalSet.hxx>
#include <isl/Quantifier.hxx>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <stdexcept>
#include <sstream>

#include <algorithm>

#include <stdarg.h>

namespace isl
{

template <typename Ch> class BasicParser;

template <typename Ch> class BasicGrammar
{
public:
	BasicGrammar() :
		_symbols(),
		_identifiers(),
		_terminals(),
		_compositions(),
		_productions(),
		_startSymbolHandle(0)
	{}
	virtual ~BasicGrammar()
	{
		reset();
	}

	typedef std::basic_string<Ch> SourceType;
	typedef unsigned int SymbolHandle;
	typedef std::list<SymbolHandle> SymbolHandleList;
	typedef unsigned int CompositionHandle;

	void reset()
	{
		for (CompositionPos i = _compositions.begin(); i != _compositions.end(); ++i) {
			delete (*i);
		}
		_compositions.clear();
		_productions.clear();
		for (SymbolPos i = _symbols.begin(); i != _symbols.end(); ++i) {
			delete (*i);
		}
		_symbols.clear();
		_identifiers.clear();
		_terminals.clear();
		_startSymbolHandle = 0;
	}
	SymbolHandle newNonTerminal(const SourceType& identifier = SourceType())
	{
		Symbol * newSymbol = new Symbol(*this, identifier);
		return newSymbol->handle();
	}
	SymbolHandle newTerminal(Ch ch, const SourceType& identifier = SourceType())
	{
		Symbol * newSymbol = new Symbol(*this, ch, identifier);
		return newSymbol->handle();
	}
	SymbolHandle newTerminal(const std::set<Ch>& charSet, const SourceType& identifier = SourceType())
	{
		Symbol * newSymbol = new Symbol(*this, charSet, identifier);
		return newSymbol->handle();
	}
	SymbolHandle newTerminal(const IntervalSet<Ch>& intervalSet, const SourceType& identifier = SourceType())
	{
		Symbol * newSymbol = new Symbol(*this, intervalSet, identifier);
		return newSymbol->handle();
	}
	SymbolHandle newTerminal(Ch ch1, Ch ch2, const SourceType& identifier = SourceType())
	{
		Symbol * newSymbol = new Symbol(*this, ch1, ch2, identifier);
		return newSymbol->handle();
	}
	SymbolHandle symbolHandle(const SourceType& identifier) const
	{
		ConstIdentifierPos pos = _identifiers.find(identifier);
		if (pos == _identifiers.end()) {
			// TODO
			throw std::runtime_error("Invalid identifier");
		}
		return (*pos).second;
	}
	void setStartSymbol(SymbolHandle startSymbolHandle)
	{
		if (!isValidSymbolHandle(startSymbolHandle)) {
			// TODO
			throw std::runtime_error("Invalid symbol handle");
		}
		_startSymbolHandle = startSymbolHandle;
	}
	CompositionHandle newComposition(SymbolHandle lhsHandle)
	{
		_compositions.push_back(new Composition(*this, lhsHandle));
		return _compositions.size();
	}
	void addToComposition(CompositionHandle compositionHandle, const SymbolHandle symbolHandle,
			const Quantifier& quantifier = Quantifier::onlyOne())
	{
		SymbolHandleList symbolHandles(1, symbolHandle);
		addToComposition(compositionHandle, symbolHandles);
	}
	void addToComposition(CompositionHandle compositionHandle, const Quantifier& quantifier, unsigned int symbolsAmount,
			SymbolHandle symbolHandle, ...)
	{
		SymbolHandleList symbolHandles(1, symbolHandle);
		va_list args;
		va_start(args, symbolHandle);
		for (int i = 1; i < symbolsAmount; ++i) {
			symbolHandles.push_back(va_arg(args, SymbolHandle));
		}
		va_end(args);
		addToComposition(compositionHandle, symbolHandles, quantifier);
	}
	void addToComposition(CompositionHandle compositionHandle, const SymbolHandleList& symbolHandles,
			const Quantifier& quantifier = Quantifier::onlyOne())
	{
		if (!isValidCompositionHandle(compositionHandle)) {
			// TODO
			throw std::runtime_error("Invalid composition handle");
		}
		_compositions[compositionHandle - 1]->addToRhs(symbolHandles, quantifier);
	}
	void addToComposition(CompositionHandle compositionHandle, const SourceType& terminals,
			const Quantifier& quantifier = Quantifier::onlyOne())
	{
		SymbolHandleList symbolHandles;
		for (size_t i = 0; i < terminals.size(); ++i) {
			TerminalPos pos = _terminals.find(terminals.at(i));
			if (pos == _terminals.end()) {
				symbolHandles.push_back(newTerminal(terminals.at(i)));
			} else {
				symbolHandles.push_back((*pos).second);
			}
		}
		addToComposition(compositionHandle, symbolHandles, quantifier);
	}
	void populateComposition(CompositionHandle compositionHandle)
	{
		if (!isValidCompositionHandle(compositionHandle)) {
			// TODO
			throw std::runtime_error("Invalid composition handle");
		}
		_compositions[compositionHandle - 1]->populate();
	}
	void newProduction(SymbolHandle lhsHandle, unsigned int rhsSymbolsAmount, SymbolHandle firstRhsSymbolHandle, ...)
	{
		SymbolHandleList rhsSymbolHandles(1, firstRhsSymbolHandle);
		va_list args;
		va_start(args, firstRhsSymbolHandle);
		for (int i = 1; i < rhsSymbolsAmount; ++i) {
			rhsSymbolHandles.push_back(va_arg(args, SymbolHandle));
		}
		va_end(args);
		newProduction(lhsHandle, rhsSymbolHandles);

	}
	void newProduction(SymbolHandle lhsHandle, const SymbolHandleList& rhsSymbolHandles)
	{
		// Checkout handles passed
		if (!isValidSymbolHandle(lhsHandle)) {
			// TODO
			throw std::runtime_error("Invalid lhs symbol handle");
		}
		ProductionRhs rhs;
		rhs.reserve(rhsSymbolHandles.size());
		for (typename SymbolHandleList::const_iterator i = rhsSymbolHandles.begin(); i != rhsSymbolHandles.end(); ++i) {
			if (!isValidSymbolHandle(*i)) {
				// TODO
				throw std::runtime_error("Invalid rhs symbol handle");
			}
			rhs.push_back(*i);
		}
		if (productionExists(lhsHandle, rhs)) {
			// TODO
			throw std::runtime_error("Production already exists");
		}
		// Inserting the production
		_productions.insert(Productions::value_type(lhsHandle, rhs));
	}
#ifdef ISL__DEBUGGING_ON
	SourceType debugSymbols() const
	{
		std::basic_ostringstream<Ch> oss;
		for (ConstSymbolPos i = _symbols.begin(); i != _symbols.end(); ++i) {
			if (i != _symbols.begin()) {
				oss << std::endl;
			}
			oss << (*i)->debug();
			if (isValidSymbolHandle(_startSymbolHandle) && ((*i)->handle() == _startSymbolHandle)) {
				oss << " (start symbol)";
			}
		}
		return oss.str();
	}
	SourceType debugCompositions() const
	{
		std::basic_ostringstream<Ch> oss;
		for (ConstCompositionPos i = _compositions.begin(); i != _compositions.end(); ++i) {
			if (i != _compositions.begin()) {
				oss << std::endl;
			}
			oss << (*i)->debug();
		}
		return oss.str();
	}
	SourceType debugProductions() const
	{
		std::basic_ostringstream<Ch> oss;
		for (ConstProductionPos i = _productions.begin(); i != _productions.end(); ++i) {
			if (i != _productions.begin()) {
				oss << std::endl;
			}
			oss << symbol((*i).first).debug() << ":";
			bool isPrintingString = false;
			for (typename ProductionRhs::const_iterator j = (*i).second.begin(); j != (*i).second.end(); ++j) {
				if (isPrintingString) {
					if (!symbol(*j).isCharacter()) {
						oss << "' ";
					}
				} else {
					if (symbol(*j).isCharacter()) {
						oss << " '";
					} else {
						oss << ' ';
					}
				}
				if (symbol(*j).isCharacter()) {
					oss << symbol(*j).character();
					isPrintingString = true;
				} else {
					isPrintingString = false;
					oss << symbol(*j).debug();
				}
			}
			if (isPrintingString) {
				oss << '\'';
			}
		}
		return oss.str();
	}
#endif
protected:
private:
	typedef typename std::map<SourceType, SymbolHandle> Identifiers;	// Identifiers cache
	typedef typename Identifiers::iterator IdentifierPos;
	typedef typename Identifiers::const_iterator ConstIdentifierPos;
	typedef typename std::map<Ch, SymbolHandle> Terminals;			// Terminals cache
	typedef typename Terminals::iterator TerminalPos;
	typedef typename Terminals::const_iterator ConstTerminalPos;

	class Symbol
	{
	public:
		Symbol(BasicGrammar<Ch>& grammar, const SourceType& identifier = SourceType()) :
			_handle(0),
			_isAnonymous(identifier.empty()),
			_identifierPos(),
			_symbolType(NonTerminal),
			_terminalPos(),
			_characterSet(),
			_characterIntervalSet()
		{
			init(grammar, identifier);
		}
		Symbol(BasicGrammar<Ch>& grammar, Ch ch, const SourceType& identifier = SourceType()) :
			_handle(0),
			_isAnonymous(identifier.empty()),
			_identifierPos(),
			_symbolType(Character),
			_terminalPos(),
			_characterSet(),
			_characterIntervalSet()
		{
			if (grammar._terminals.find(ch) != grammar._terminals.end()) {
				// TODO
				throw std::runtime_error("Terminal already exists in grammar");
			}
			init(grammar, identifier);
			typename std::pair<TerminalPos, bool> pos = grammar._terminals.insert(typename Terminals::value_type(ch, _handle));
			_terminalPos = pos.first;
		}
		Symbol(BasicGrammar<Ch>& grammar, const std::set<Ch>& characterSet, const SourceType& identifier = SourceType()) :
			_handle(0),
			_isAnonymous(identifier.empty()),
			_identifierPos(),
			_symbolType(CharacterSet),
			_terminalPos(),
			_characterSet(characterSet),
			_characterIntervalSet()
		{
			if (_characterSet.empty()) {
				// TODO
				throw std::runtime_error("Empty character set");
			}
			init(grammar, identifier);
		}
		Symbol(BasicGrammar<Ch>& grammar, const IntervalSet<Ch>& characterIntervalSet, const SourceType& identifier = SourceType()) :
			_handle(0),
			_isAnonymous(identifier.empty()),
			_identifierPos(),
			_symbolType(CharacterIntervalSet),
			_terminalPos(),
			_characterSet(),
			_characterIntervalSet(characterIntervalSet)
		{
			if (_characterIntervalSet.empty()) {
				// TODO
				throw std::runtime_error("Empty character interval set");
			}
			init(grammar, identifier);
		}
		Symbol(BasicGrammar<Ch>& grammar, Ch ch1, Ch ch2, const SourceType& identifier = SourceType()) :
			_handle(0),
			_isAnonymous(identifier.empty()),
			_identifierPos(),
			_symbolType(CharacterIntervalSet),
			_terminalPos(),
			_characterSet(),
			_characterIntervalSet(ch1, ch2)
		{
			if (_characterIntervalSet.empty()) {
				// TODO
				throw std::runtime_error("Empty character interval set");
			}
			init(grammar, identifier);
		}

		inline SymbolHandle handle() const
		{
			return _handle;
		}
		inline bool isAnonymous() const
		{
			return _isAnonymous;
		}
		inline SourceType identifier() const
		{
			return (_isAnonymous) ? SourceType() : (*_identifierPos).first;
		}
		inline bool isTerminal() const
		{
			return _symbolType != NonTerminal;
		}
		inline bool isNonTerminal() const
		{
			return _symbolType == NonTerminal;
		}
		inline bool isCharacter() const
		{
			return _symbolType == Character;
		}
		inline bool isCharacterSet() const
		{
			return _symbolType == CharacterSet;
		}
		inline bool isCharacterIntervalSet() const
		{
			return _symbolType == CharacterIntervalSet;
		}
		inline Ch character() const
		{
			if (!isCharacter()) {
				// TODO
				throw std::runtime_error("Symbol is not a character terminal");
			}
			return (*_terminalPos).first;
		}
		inline std::set<Ch> characterSet() const
		{
			if (!isCharacterSet()) {
				// TODO
				throw std::runtime_error("Symbol is not a character set terminal");
			}
			return _characterSet;
		}
		inline IntervalSet<Ch> characterIntervalSet() const
		{
			if (!isCharacterIntervalSet()) {
				// TODO
				throw std::runtime_error("Symbol is not a character interval set terminal");
			}
			return _characterIntervalSet;
		}
		bool contains(Ch ch) const
		{
			switch (_symbolType) {
			case Character:
				return (*_terminalPos).first == ch;
			case CharacterSet:
				return _characterSet.find(ch) != _characterSet.end();
			case CharacterIntervalSet:
				return _characterIntervalSet.contains(ch);
			default:
				// TODO
				throw std::runtime_error("Symbol is not a terminal");
			}
		}
#ifdef ISL__DEBUGGING_ON
		SourceType debug() const
		{
			std::basic_ostringstream<Ch> oss;
			std::list< std::pair<Ch, Ch> > intervals = _characterIntervalSet.intervals();
			switch (_symbolType) {
			case NonTerminal:
				if (this->_isAnonymous) {
					oss << "ANONYMOUS_" << this->_handle;
				} else {
					oss << this->identifier();
				}
				break;
			case Character:
				oss << '\'' << (*_terminalPos).first << '\'';
				break;
			case CharacterSet:
				oss << '[';
				for (typename std::set<Ch>::const_iterator i = _characterSet.begin(); i != _characterSet.end(); ++i) {
					if (i != _characterSet.begin()) {
						oss << ", ";
					}
					oss << '\'' << (*i) << '\'';
				}
				oss << ']';
				break;
			case CharacterIntervalSet:
				oss << '[';
				for (typename std::list< std::pair<Ch, Ch> >::const_iterator i = intervals.begin(); i != intervals.end(); ++i) {
					if (i != intervals.begin()) {
						oss << ", ";
					}
					oss << '\'' << (*i).first << "' .. '" << (*i).second << '\'';
				}
				oss << ']';
				break;
			default:
				// TODO
				std::ostringstream oss;
				oss << "Invalid symbol type :" << _symbolType;
				throw std::runtime_error(oss.str().c_str());
			}
			return oss.str();
		}
#endif
	private:
		Symbol();

		enum SymbolType { NonTerminal, Character, CharacterSet, CharacterIntervalSet };

		void init(BasicGrammar<Ch>& grammar, const SourceType& identifier)
		{
			if (!_isAnonymous && (grammar._identifiers.find(identifier) != grammar._identifiers.end())) {
				// TODO
				throw std::runtime_error("Identifier already exists in grammar");
			}
			grammar._symbols.push_back(this);
			_handle = grammar._symbols.size();
			if (!_isAnonymous) {
				typename std::pair<IdentifierPos, bool> pos = grammar._identifiers.insert(
						typename Identifiers::value_type(identifier, _handle));
				_identifierPos = pos.first;
			}
		}

		SymbolHandle _handle;
		bool _isAnonymous;
		IdentifierPos _identifierPos;
		bool _isTerminal;
		SymbolType _symbolType;
		TerminalPos _terminalPos;
		std::set<Ch> _characterSet;
		IntervalSet<Ch> _characterIntervalSet;
	};

	typedef std::vector<Symbol *> Symbols;
	typedef typename Symbols::iterator SymbolPos;
	typedef typename Symbols::const_iterator ConstSymbolPos;
	
	class Composition
	{
	public:
		Composition(BasicGrammar<Ch>& grammar, SymbolHandle lhsHandle) :
			_grammar(grammar),
			_populated(false),
			_lhsHandle(lhsHandle),
			_rhsItems()
		{
			if (_grammar.symbol(lhsHandle).isTerminal()) {
				// TODO
				throw std::runtime_error("Left hand side of the composition could not be a terminal");
			}
		}

		void addToRhs(const SymbolHandleList& symbolHandles, const Quantifier& quantifier = Quantifier::onlyOne())
		{
			if (_populated) {
				// TODO
				throw std::runtime_error("Composition has been already populated");
			}
			if (symbolHandles.size() <= 0) {
				// TODO
				throw std::runtime_error("Empty symbolHandles list to add to composition");
			}
			for (SymbolHandleList::const_iterator i = symbolHandles.begin(); i != symbolHandles.end(); ++i) {
				if (!_grammar.isValidSymbolHandle(*i)) {
					// TODO
					throw std::runtime_error("Invalid symbol handle to add to composition");
				}
			}
			_rhsItems.push_back(RhsItem(symbolHandles, quantifier));
		}
		void populate()
		{
			if (_populated) {
				// TODO
				throw std::runtime_error("Composition has been already populated");
			}
			SymbolHandleList productionRhsHandles;
			for (typename RhsItems::iterator i = _rhsItems.begin(); i != _rhsItems.end(); ++i) {
				if ((*i).second.min() == 1 && (*i).second.max() == 1) {
					// No quantifier -> just add current symbols
					std::copy((*i).first.begin(), (*i).first.end(), std::back_inserter(productionRhsHandles));
				} else if ((*i).second.max() == Quantifier::infinity()) {
					SymbolHandle anonHandle = _grammar.newNonTerminal();
					productionRhsHandles.push_back(anonHandle);
					if ((*i).second.min() == 0) {
						// D:	a cur* c	=>	D:		a [ANONYMOUS] c
						//				[ANONYMOUS]:	// empty
						//				[ANONYMOUS]:	cur [ANONYMOUS]
						_grammar.newProduction(anonHandle, SymbolHandleList());
						SymbolHandleList anonRhsHandles((*i).first);
						anonRhsHandles.push_back(anonHandle);
						_grammar.newProduction(anonHandle, anonRhsHandles);
					} else if ((*i).second.min() == 1) {
						// D:	a cur+ c	=>	D:		a [ANONYMOUS] c
						//				[ANONYMOUS]: 	cur
						//				[ANONYMOUS]:	cur [ANONYMOUS]
						SymbolHandleList anonRhsHandles((*i).first);
						_grammar.newProduction(anonHandle, anonRhsHandles);
						anonRhsHandles.push_back(anonHandle);
						_grammar.newProduction(anonHandle, anonRhsHandles);
					} else {
						// D:	a cur{n,} c	=>	D:		a [ANONYMOUS] c
						//				[ANONYMOUS]:	cur .. cur (n - 1 times) [ANONYMOUS_1]
						//				[ANONYMOUS_1]:	cur
						//				[ANONYMOUS_1]:	cur [ANONYMOUS_1]
						SymbolHandleList anonRhsHandles((*i).first);
						for (unsigned int j = 2; j < (*i).second.min(); ++j) {
							std::copy((*i).first.begin(), (*i).first.end(), std::back_inserter(anonRhsHandles));
						}
						SymbolHandle anon1Handle = _grammar.newNonTerminal();
						anonRhsHandles.push_back(anon1Handle);
						_grammar.newProduction(anonHandle, anonRhsHandles);
						SymbolHandleList anon1RhsHandles((*i).first);
						_grammar.newProduction(anon1Handle, anon1RhsHandles);
						anon1RhsHandles.push_back(anon1Handle);
						_grammar.newProduction(anon1Handle, anon1RhsHandles);
					}
				} else {
					// D:	a cur{m,n} c	=>	D:		a [ANONYMOUS] c
					//				[ANONYMOUS]:	cur .. cur (m times)
					//				[ANONYMOUS]:	cur .. cur (m + 1 times)
					//				...
					//				[ANONYMOUS]	cur .. cur (n times)
					SymbolHandle anonHandle = _grammar.newNonTerminal();
					productionRhsHandles.push_back(anonHandle);
					SymbolHandleList anonRhsHandles;
					for (unsigned int j = 0; j < (*i).second.min(); ++j) {
						std::copy((*i).first.begin(), (*i).first.end(), std::back_inserter(anonRhsHandles));
					}
					_grammar.newProduction(anonHandle, anonRhsHandles);
					for (unsigned int j = (*i).second.min(); j < (*i).second.max(); ++j) {
						std::copy((*i).first.begin(), (*i).first.end(), std::back_inserter(anonRhsHandles));
						_grammar.newProduction(anonHandle, anonRhsHandles);
					}
				}
			}
			_grammar.newProduction(_lhsHandle, productionRhsHandles);
			_populated = true;
		}
#ifdef ISL__DEBUGGING_ON
		SourceType debug() const
		{
			std::basic_ostringstream<Ch> oss;
			oss << _grammar.symbol(_lhsHandle).identifier() << ':';
			for (typename RhsItems::const_iterator i = _rhsItems.begin(); i != _rhsItems.end(); ++i) {
				if (((*i).second.min() != 1 || (*i).second.max() != 1) && (*i).first.size() > 1) {
					oss << " (";
				}
				for (typename SymbolHandleList::const_iterator j = (*i).first.begin(); j != (*i).first.end(); ++j) {
					oss << ' ' << _grammar.symbol(*j).debug();
				}
				if ((*i).second.min() != 1 || (*i).second.max() != 1) {
					if ((*i).first.size() > 1) {
						oss << " )";
					}
					oss << (*i).second.template debug<Ch>();
				}
			}
			oss << ((_populated) ? " (populated)" : " (not populated)");
			return oss.str();
		}
#endif
	private:
		Composition();

		typedef std::pair<SymbolHandleList, Quantifier> RhsItem;
		typedef std::list<RhsItem> RhsItems;

		BasicGrammar<Ch>& _grammar;
		bool _populated;
		SymbolHandle _lhsHandle;
		RhsItems _rhsItems;
	};

	typedef std::vector<Composition *> Compositions;
	typedef typename Compositions::iterator CompositionPos;
	typedef typename Compositions::const_iterator ConstCompositionPos;

	typedef unsigned int ProductionHandle;
	typedef std::vector<SymbolHandle> ProductionRhs;
	typedef std::multimap<SymbolHandle, ProductionRhs> Productions;
	typedef typename Productions::iterator ProductionPos;
	typedef typename Productions::const_iterator ConstProductionPos;

	inline bool isValidSymbolHandle(SymbolHandle handle) const
	{
		return handle > 0 && handle <= _symbols.size();
	}
	inline const Symbol& symbol(SymbolHandle handle) const
	{
		if (!isValidSymbolHandle(handle)) {
			// TODO
			throw std::runtime_error("Invalid symbol handle");
		}
		return *(_symbols[handle - 1]);
	}
	inline bool isValidCompositionHandle(CompositionHandle handle) const
	{
		return handle > 0 && handle <= _compositions.size();
	}
	bool productionExists(SymbolHandle lhsHandle, const ProductionRhs& rhsHandles) const
	{
		std::pair<ConstProductionPos, ConstProductionPos> range = _productions.equal_range(lhsHandle);
		for (ConstProductionPos i = range.first; i != range.second; ++i) {
			if ((*i).second.size() != rhsHandles.size()) {
				continue;
			}
			if (std::equal((*i).second.begin(), (*i).second.end(), rhsHandles.begin())) {
				return true;
			}
		}
		return false;
	}

	Symbols _symbols;
	Identifiers _identifiers;
	Terminals _terminals;
	Compositions _compositions;
	Productions _productions;
	SymbolHandle _startSymbolHandle;

	friend class BasicParser<Ch>;
};

/*template <> BasicGrammar<char>::SymbolName BasicGrammar<char>::generateAnonSymbolName() const
{
	return "noname";
}

template <> BasicGrammar<wchar_t>::SymbolName BasicGrammar<wchar_t>::generateAnonSymbolName() const
{
	return L"noname";
}*/

typedef BasicGrammar<char> Grammar;
typedef BasicGrammar<wchar_t> WGrammar;

} // namespace isl

#endif
