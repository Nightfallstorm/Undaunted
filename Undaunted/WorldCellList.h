#include "SKSELink.h"

namespace Undaunted
{
#ifndef WorldCellListdef
#define WorldCellListdef
	class WorldCell {
	public:
		RE::TESWorldSpace* world;
		RE::TESObjectCELL* cell;
	};

	class WorldCellList {
	public:
		WorldCell* data;
		std::uint32_t length;
		WorldCellList* AddItem(WorldCell item);
	};
#endif
}
