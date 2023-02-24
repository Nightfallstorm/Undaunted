#include "SKSELink.h"
#include "FormRefList.h"

namespace Undaunted
{
#ifndef RiftListdef
#define RiftListdef
	class RiftRef {
	public:
		FormRefList reflist;
	};

	class RiftList {
	public:
		RiftRef* data;
		int length;
		RiftList* AddItem(RiftRef item);
	};
#endif
}
