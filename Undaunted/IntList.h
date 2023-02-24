namespace Undaunted
{
#ifndef Intlistdef
#define Intlistdef
	class IntList {
	public:
		std::uint32_t* data;
		std::uint32_t length;
		IntList* AddItem(std::uint32_t item);
	};

#endif
}
