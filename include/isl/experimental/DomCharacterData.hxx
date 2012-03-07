#ifndef ISL__DOM_CHARACTER_DATA__HXX
#define ISL__DOM_CHARACTER_DATA__HXX

#include <isl/DomNode.hxx>

namespace isl
{

class DomCharacterData : public DomNode
{
public:
	DomString data() const;
	unsigned int length() const;
	void setData(const DomString& newData);
	DomString substringData(unsigned int offset, unsigned int count) const;
	void appendData(const DomString& arg);
	void insertData(unsigned int offset, const DomString& arg);
	void deleteData(unsigned int offset, unsigned int count);
	void replaceData(unsigned int offset, unsigned int count, const DomString& arg);
protected:
	DomCharacterData(const DomString& data, NodeType nodeType, DomDocument * ownerDocument, bool isReadOnly = false);
private:
	DomCharacterData();
	DomCharacterData(const DomCharacterData&);

	DomCharacterData& operator=(const DomCharacterData&);

	DomString _data;
};

} // namespace isl

#endif

