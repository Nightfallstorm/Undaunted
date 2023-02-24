namespace Undaunted
{
#ifndef RefListdef
#define RefListdef
	class Ref {
	public:
		RE::TESObjectREFR* objectRef;
	};

	class RefList {
	public:
		Ref* data;
		std::uint32_t length;
		RefList* AddItem(Ref item);
		RefList* SwapItem(int first, int second);
	};

#endif
}
