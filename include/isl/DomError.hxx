#ifndef ISL__DOM_ERROR__HXX
#define ISL__DOM_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class DomError : public AbstractError
{
public:
	enum Code {
		IndexSizeErr = 1,
		DomStringSizeErr = 2,
		HierarchyRequestErr = 3,
		WrongDocumentErr = 4,
		InvalidCharacterErr = 5,
		NoDataAllowedErr = 6,
		NoModificationAllowedErr = 7,
		NotFoundErr = 8,
		NotSupportedErr = 9,
		InUseAttributeErr = 10,
		InvalidStateErr = 11,
		SyntaxErr = 12,
		InvalidModificationErr = 13,
		NamespaceErr = 14,
		InvalidAccessErr = 15,
		// Extra error codes:
		InvalidNodeType = 1024,
		MethodNotImplemented = 1025						// Remove it when implementation would be completed
	};

	DomError(Code code, SOURCE_LOCATION_ARGS_DECLARATION);

	Code code() const;

	virtual AbstractError * clone() const;
private:
	static std::wstring codeToText(Code code);

	Code _code;
};

} // namespace isl

#endif

