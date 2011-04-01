#include <isl/DomCharacterData.hxx>
#include <isl/DomError.hxx>
#include <isl/Exception.hxx>

namespace isl
{

DomCharacterData::DomCharacterData(const DomString& data, NodeType nodeType, DomDocument * ownerDocument, bool isReadOnly) :
	DomNode(nodeType, ownerDocument, isReadOnly),
	_data(data)
{}

DomString DomCharacterData::data() const
{
	return _data;
}

unsigned int DomCharacterData::length() const
{
	return _data.length();
}

void DomCharacterData::setData(const DomString& newData)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	_data = newData;
}

DomString DomCharacterData::substringData(unsigned int offset, unsigned int count) const
{
	if (offset >= _data.length()) {
		throw Exception(DomError(DomError::IndexSizeErr, SOURCE_LOCATION_ARGS));
	}
	return _data.substr(offset, count);
}

void DomCharacterData::appendData(const DomString& arg)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	_data.append(arg);
}

void DomCharacterData::insertData(unsigned int offset, const DomString& arg)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (offset >= _data.length()) {
		throw Exception(DomError(DomError::IndexSizeErr, SOURCE_LOCATION_ARGS));
	}
	_data.insert(offset, arg);
}

void DomCharacterData::deleteData(unsigned int offset, unsigned int count)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (offset >= _data.length()) {
		throw Exception(DomError(DomError::IndexSizeErr, SOURCE_LOCATION_ARGS));
	}
	_data.erase(offset, count);
}

void DomCharacterData::replaceData(unsigned int offset, unsigned int count, const DomString& arg)
{
	if (isReadOnly()) {
		throw Exception(DomError(DomError::NoModificationAllowedErr, SOURCE_LOCATION_ARGS));
	}
	if (offset >= _data.length()) {
		throw Exception(DomError(DomError::IndexSizeErr, SOURCE_LOCATION_ARGS));
	}
	_data.replace(offset, count, arg);
}

} // namespace isl

