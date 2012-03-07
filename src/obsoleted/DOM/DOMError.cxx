#include <isl/DOMError.hxx>
#include <sstream>

namespace isl
{

DOMError::DOMError(Code code, SOURCE_LOCATION_ARGS_DECLARATION) :
	AbstractError(codeToText(code), SOURCE_LOCATION_ARGS_PASSTHRU),
	_code(code)
{}

DOMError::Code DOMError::code() const
{
	return _code;
}

AbstractError * DOMError::clone() const
{
	return new DOMError(*this);
}

std::wstring DOMError::codeToText(DOMError::Code code)
{
	switch (code) {
		case INDEX_SIZE_ERR:
			return L"Index or size is negative, or greater than the allowed value";
		case DOMSTRING_SIZE_ERR:
			return L"Specified range of text does not fit into a DOMString";
		case HIERARCHY_REQUEST_ERR:
			return L"Node is inserted somewhere it doesn't belong";
		case WRONG_DOCUMENT_ERR:
			return L"Node is used in a different document than the one that created it (that doesn't support it)";
		case INVALID_CHARACTER_ERR:
			return L"Invalid or illegal character is specified";
		case NO_DATA_ALLOWED_ERR:
			return L"Data is specified for a node which does not support data";
		case NO_MODIFICATION_ALLOWED_ERR:
			return L"Attempt is made to modify an object where modifications are not allowed";
		case NOT_FOUND_ERR:
			return L"Attempt is made to reference a node in a context where it does not exist";
		case NOT_SUPPORTED_ERR:
			return L"The implementation does not support the requested type of object or operation";
		case INUSE_ATTRIBUTE_ERR:
			return L"Attempt is made to add an attribute that is already in use elsewhere";
		case INVALID_STATE_ERR:
			return L"Attempt is made to use an object that is not, or is no longer, usable";
		case SYNTAX_ERR:
			return L"Invalid or illegal string is specified";
		case INVALID_MODIFICATION_ERR:
			return L"Attempt is made to modify the type of the underlying object";
		case NAMESPACE_ERR:
			return L"Attempt is made to create or change an object in a way which is incorrect with regard to namespaces";
		case INVALID_ACCESS_ERR:
			return L"Parameter or an operation is not supported by the underlying object";
		default:
			std::wostringstream sstr;
			sstr << "Unknown DOM-exception with code = " << code;
			return sstr.str();
	}
}

} // namespace isl

