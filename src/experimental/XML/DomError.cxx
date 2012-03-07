#include <isl/DomError.hxx>
#include <sstream>

namespace isl
{

DomError::DomError(Code code, SOURCE_LOCATION_ARGS_DECLARATION) :
	AbstractError(codeToText(code), SOURCE_LOCATION_ARGS_PASSTHRU),
	_code(code)
{}

DomError::Code DomError::code() const
{
	return _code;
}

AbstractError * DomError::clone() const
{
	return new DomError(*this);
}

std::wstring DomError::codeToText(DomError::Code code)
{
	switch (code) {
		case IndexSizeErr:
			return L"Index or size is negative, or greater than the allowed value (INDEX_SIZE_ERR)";
		case DomStringSizeErr:
			return L"Specified range of text does not fit into a DOMString (DOMSTRING_SIZE_ERR)";
		case HierarchyRequestErr:
			return L"Node is inserted somewhere it doesn't belong (HIERARCHY_REQUEST_ERR)";
		case WrongDocumentErr:
			return L"Node is used in a different document than the one that created it (WRONG_DOCUMENT_ERR)";
		case InvalidCharacterErr:
			return L"Invalid or illegal character is specified (INVALID_CHARACTER_ERR)";
		case NoDataAllowedErr:
			return L"Data is specified for a node which does not support data (NO_DATA_ALLOWED_ERR)";
		case NoModificationAllowedErr:
			return L"Attempt is made to modify an object where modifications are not allowed (NO_MODIFICATION_ALLOWED_ERR)";
		case NotFoundErr:
			return L"Attempt is made to reference a node in a context where it does not exist (NOT_FOUND_ERR)";
		case NotSupportedErr:
			return L"The implementation does not support the requested type of object or operation (NOT_SUPPORTED_ERR)";
		case InUseAttributeErr:
			return L"Attempt is made to add an attribute that is already in use elsewhere (INUSE_ATTRIBUTE_ERR)";
		case InvalidStateErr:
			return L"Attempt is made to use an object that is not, or is no longer, usable (INVALID_STATE_ERR)";
		case SyntaxErr:
			return L"Invalid or illegal string is specified (SYNTAX_ERR)";
		case InvalidModificationErr:
			return L"Attempt is made to modify the type of the underlying object (TYPE_MISMATCH_ERR)";
		case NamespaceErr:
			return L"Attempt is made to create or change an object in a way which is incorrect with regard to namespaces (VALIDATION_ERR)";
		case InvalidAccessErr:
			return L"Parameter or an operation is not supported by the underlying object (WRONG_DOCUMENT_ERR)";
		case InvalidNodeType:
			return L"Invalid node type";
		case MethodNotImplemented:
			return L"Method is not implemented yet. :-(";
		default:
			std::wostringstream sstr;
			sstr << L"Unknown DOM-exception with code = " << code;
			return sstr.str();
	}
}

} // namespace isl

