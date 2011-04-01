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
		_startSymbolHandle(0),
		_startSymbolSet(false)
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
		_startSymbolHandle = 0,
		_startSymbolSet = false;
	}
	SymbolHandle newNonTerminal(const SourceType& identifier = SourceType())
	{
		SymbolHandle handle = _symbols.size();
		NonTerminal * ptr = new NonTerminal(*this, handle, identifier);
		_symbols.push_back(ptr);
		return handle;
	}
	SymbolHandle newTerminal(Ch ch, const SourceType& identifier = SourceType())
	{
		SymbolHandle handle = _symbols.size();
		Terminal * ptr = new Terminal(*this, handle, ch, identifier);
		_symbols.push_back(ptr);
		return handle;
	}
	SymbolHandle newTerminal(const std::set<Ch>& charSet, const SourceType& identifier = SourceType())
	{
		SymbolHandle handle = _symbols.size();
		Terminal * ptr = new Terminal(*this, handle, charSet, identifier);
		_symbols.push_back(ptr);
		return handle;
	}
	SymbolHandle newTerminal(const IntervalSet<Ch>& intervalSet, const SourceType& identifier = SourceType())
	{
		SymbolHandle handle = _symbols.size();
		Terminal * ptr = new Terminal(*this, handle, intervalSet, identifier);
		_symbols.push_back(ptr);
		return handle;
	}
	SymbolHandle newTerminal(Ch ch1, Ch ch2, const SourceType& identifier = SourceType())
	{
		SymbolHandle handle = _symbols.size();
		Terminal * ptr = new Terminal(*this, handle, ch1, ch2, identifier);
		_symbols.push_back(ptr);
		return handle;
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
		_startSymbolSet = true;
	}
	CompositionHandle newComposition(SymbolHandle lhsHandle)
	{
		_compositions.push_back(new Composition(*this, lhsHandle));
		return _compositions.size() - 1;
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
		_compositions[compositionHandle]->addToRhs(symbolHandles, quantifier);
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
		_compositions[compositionHandle]->populate();
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
			if (_startSymbolSet && ((*i)->handle() == _startSymbolHandle)) {
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
			oss << _symbols[(*i).first]->debug() << ":";
			bool isPrintingString = false;
			for (typename ProductionRhs::const_iterator j = (*i).second.begin(); j != (*i).second.end(); ++j) {
				if (isPrintingString) {
					if (!_symbols[*j]->isTerminal() ||
							(dynamic_cast<Terminal *>(_symbols[*j])->storageType() == Terminal::CharSetStorageType) ||
							(dynamic_cast<Terminal *>(_symbols[*j])->storageType() == Terminal::IntervalSetStorageType)) {
						oss << "' ";
					}
				} else {
					if (_symbols[*j]->isTerminal() &&
							(dynamic_cast<Terminal *>(_symbols[*j])->storageType() == Terminal::CharStorageType)) {
						oss << " '";
					} else {
						oss << ' ';
					}
				}
				if (_symbols[*j]->isTerminal() && (dynamic_cast<Terminal *>(_symbols[*j])->storageType() == Terminal::CharStorageType)) {
					oss << dynamic_cast<Terminal *>(_symbols[*j])->character();
					isPrintingString = true;
				} else {
					isPrintingString = false;
					oss << _symbols[*j]->debug();
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

	class AbstractSymbol
	{
	public:
		AbstractSymbol(BasicGrammar<Ch>& grammar, SymbolHandle handle, const SourceType& identifier) :
			_handle(handle),
			_anonymous(identifier.empty()),
			_identifierPos()
		{
			if (!_anonymous) {
				if (grammar._identifiers.find(identifier) != grammar._identifiers.end()) {
					// TODO
					throw std::runtime_error("Identifier already exists in grammar");
				}
				typename std::pair<IdentifierPos, bool> pos = grammar._identifiers.insert(
						typename Identifiers::value_type(identifier, handle));
				_identifierPos = pos.first;
			}
		}
		virtual ~AbstractSymbol()
		{}

		virtual bool isTerminal() const = 0;
#ifdef ISL__DEBUGGING_ON
		virtual SourceType debug() const = 0;
#endif

		inline SymbolHandle handle() const
		{
			return _handle;
		}
		inline bool anonymous() const
		{
			return _anonymous;
		}
		inline SourceType identifier() const
		{
			return (_anonymous) ? SourceType() : (*_identifierPos).first;
		}
	protected:
		SymbolHandle _handle;
		bool _anonymous;
		IdentifierPos _identifierPos;
	private:
		AbstractSymbol();
	};

	class NonTerminal : public AbstractSymbol
	{
	public:
		NonTerminal(BasicGrammar<Ch>& grammar, SymbolHandle handle, const SourceType& identifier = SourceType()) :
			AbstractSymbol(grammar, handle, identifier)
		{}

		virtual bool isTerminal() const
		{
			return false;
		}
#ifdef ISL__DEBUGGING_ON
		virtual SourceType debug() const
		{
			std::basic_ostringstream<Ch> oss;
			if (this->_anonymous) {
				oss << "ANONYMOUS_" << this->_handle;
			} else {
				oss << this->identifier();
			}
			return oss.str();
		}
#endif
	private:
		NonTerminal();
	};

	class Terminal : public AbstractSymbol
	{
	public:
		Terminal(BasicGrammar<Ch>& grammar, SymbolHandle handle, Ch ch, const SourceType& identifier = SourceType()) :
			AbstractSymbol(grammar, handle, identifier),
			_storageType(CharStorageType),
			_terminalPos(),
			_charSet(),
			_intervalSet()
		{
			if (grammar._terminals.find(ch) != grammar._terminals.end()) {
				// TODO
				throw std::runtime_error("Terminal already exists in grammar");
			}
			typename std::pair<TerminalPos, bool> pos = grammar._terminals.insert(typename Terminals::value_type(ch, handle));
			_terminalPos = pos.first;
		}
		Terminal(BasicGrammar<Ch>& grammar, SymbolHandle handle, const std::set<Ch>& charSet, const SourceType& identifier = SourceType()) :
			AbstractSymbol(grammar, handle, identifier),
			_storageType(CharSetStorageType),
			_terminalPos(),
			_charSet(charSet),
			_intervalSet()
		{}
		Terminal(BasicGrammar<Ch>& grammar, SymbolHandle handle, const IntervalSet<Ch>& intervalSet, const SourceType& identifier = SourceType()) :
			AbstractSymbol(grammar, handle, identifier),
			_storageType(IntervalSetStorageType),
			_terminalPos(),
			_charSet(),
			_intervalSet(intervalSet)
		{}
		Terminal(BasicGrammar<Ch>& grammar, SymbolHandle handle, Ch ch1, Ch ch2, const SourceType& identifier = SourceType()) :
			AbstractSymbol(grammar, handle, identifier),
			_storageType(IntervalSetStorageType),
			_terminalPos(),
			_charSet(),
			_intervalSet(ch1, ch2)
		{}

		enum StorageType { CharStorageType, CharSetStorageType, IntervalSetStorageType };

		virtual bool isTerminal() const
		{
			return true;
		}
#ifdef ISL__DEBUGGING_ON
		virtual SourceType debug() const
		{
			// TODO
			std::basic_ostringstream<Ch> oss;
			std::list< std::pair<Ch, Ch> > intervals = _intervalSet.intervals();
			switch (_storageType) {
			case CharStorageType:
				oss << '\'' << (*_terminalPos).first << '\'';
				break;
			case CharSetStorageType:
				oss << '[';
				for (typename std::set<Ch>::const_iterator i = _charSet.begin(); i != _charSet.end(); ++i) {
					if (i != _charSet.begin()) {
						oss << ", ";
					}
					oss << '\'' << (*i) << '\'';
				}
				oss << ']';
				break;
			case IntervalSetStorageType:
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
				oss << "Invalid terminal symbol storage type :" << _storageType;
				throw std::runtime_error(oss.str().c_str());
			}
			return oss.str();
		}
#endif

		bool contains(Ch ch) const
		{
			switch (_storageType) {
			case CharStorageType:
				return (*_terminalPos).first == ch;
			case CharSetStorageType:
				return _charSet.find(ch) != _charSet.end();
			case IntervalSetStorageType:
				return _intervalSet.contains(ch);
			default:
				// TODO
				std::ostringstream oss;
				oss << "Invalid terminal symbol storage type: " << _storageType;
				throw std::runtime_error(oss.str().c_str());
			}
		}
		Ch character() const
		{
			if (_storageType != CharStorageType) {
				// TODO
				std::ostringstream oss;
				oss << "Can not access terminal's character. Incompatible storage type: " << _storageType;
				throw std::runtime_error(oss.str().c_str());
			}
			return (*_terminalPos).first;
		}
		inline StorageType storageType() const
		{
			return _storageType;
		}
	private:
		Terminal();

		StorageType _storageType;
		TerminalPos _terminalPos;
		std::set<Ch> _charSet;
		IntervalSet<Ch> _intervalSet;
	};

	typedef std::vector<AbstractSymbol *> Symbols;
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
			if (!_grammar.isValidSymbolHandle(lhsHandle)) {
				// TODO
				throw std::runtime_error("Invalid symbol handle");
			}
			if (_grammar._symbols[lhsHandle]->isTerminal()) {
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
			oss << _grammar._symbols[_lhsHandle]->identifier() << ':';
			for (typename RhsItems::const_iterator i = _rhsItems.begin(); i != _rhsItems.end(); ++i) {
				if (((*i).second.min() != 1 || (*i).second.max() != 1) && (*i).first.size() > 1) {
					oss << " (";
				}
				for (typename SymbolHandleList::const_iterator j = (*i).first.begin(); j != (*i).first.end(); ++j) {
					oss << ' ' << _grammar._symbols[(*j)]->debug();
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
		return handle < _symbols.size();
	}
	inline bool isValidCompositionHandle(CompositionHandle handle) const
	{
		return handle < _compositions.size();
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
	bool _startSymbolSet;

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
