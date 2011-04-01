#ifndef ISL__NAME_LIST_RELEASER__HXX
#define ISL__NAME_LIST_RELEASER__HXX

#include <stdlib.h>
#include <dirent.h>

namespace isl
{

/*------------------------------------------------------------------------------
   Helper class to automatically release namelist parameter of the scandir(3) call
------------------------------------------------------------------------------*/

class NameListReleaser
{
public:
	NameListReleaser(::dirent ** nameList, unsigned int nameCount) :
		_nameList(nameList),
		_nameCount(nameCount)
	{}
	~NameListReleaser()
	{
		for (unsigned int i = 0; i < _nameCount; ++i) {
			free(_nameList[i]);
		}
		free(_nameList);
	}
private:
	NameListReleaser();
	NameListReleaser(const NameListReleaser&);

	NameListReleaser operator=(const NameListReleaser&);

	::dirent ** _nameList;
	unsigned int _nameCount;
};

} // namespace isl

#endif
