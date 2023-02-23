#include "SKSELink.h"

namespace Undaunted
{
#ifndef FormRefListdef
#define FormRefListdef
	class FormRef {
	public:
		RE::FormType type;
		std::uint32_t formId;
		RE::NiPoint3 pos;
		RE::NiPoint3 rot;
		int scale;
	};

	class FormRefList {
	public:
		FormRef* data;
		int length;
		FormRefList* AddItem(FormRef item);
	};
#endif
}
