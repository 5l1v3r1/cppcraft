#ifndef ITEMS_HPP
#define ITEMS_HPP

/**
 * 
 * 
 * 
**/

#include <string>

namespace cppcraft
{
	class Block;
	const int MAX_UNIQUE_ITEMS = 256;
	
	#define IT_NONE         0
	
	#define IT_WOODPICK     1
	#define IT_STONEPICK    2
	#define IT_IRONPICK     3
	#define IT_GOLDPICK     4
	#define IT_DIAMPICK     5
	
	#define IT_WOODSWORD   10
	#define IT_STONESWORD  11
	#define IT_IRONSWORD   12
	#define IT_GOLDSWORD   13
	#define IT_DIAMSWORD   14
	
	#define IT_WOODSHOVEL  20
	#define IT_STONESHOVEL 21
	#define IT_IRONSHOVEL  22
	#define IT_GOLDSHOVEL  23
	#define IT_DIAMSHOVEL  24
	
	#define IT_WOODAXE     30
	#define IT_STONEAXE    31
	#define IT_IRONAXE     32
	#define IT_GOLDAXE     33
	#define IT_DIAMAXE     34
	
	#define IT_WOODSCYTHE  40
	
	#define IT_COAL        50
	#define IT_IRON        51
	#define IT_GOLD        52
	#define IT_DIAMOND     53
	#define IT_REDSTONE    54
	#define IT_MAGIC       55
	
	#define IT_STICK      100
	
	#define IT_PORTALGUN  190
	
	#define IT_TORCH      200
	
	#define IT_CREATORGUN 255
	
	typedef unsigned short item_t;
	
	typedef enum
	{
		ITT_BLOCK,
		ITT_ITEM,
		ITT_UNKNOWN
		
	} itemtype_t;
	
	class InventoryItem
	{
	private:
		item_t id;              // identifies this item in combination with type, if id is 0 the item is always (no item)
		unsigned short count;   // if count is 0 this item is (no item), otherwise id will be read
		unsigned char special;  // special property of an item, often the same as block special property
		unsigned char type;     // type of inventory item, such as block, item or even particle
		unsigned char health;   // the health of the item, which when reaches 0 destroys the item
		
	public:
		static const int MAX_STACK = 64;
		
		InventoryItem();
		InventoryItem(item_t itemID, unsigned short icount);
		InventoryItem(item_t itemID, itemtype_t itype, unsigned short icount);
		
		inline void setID(item_t id) noexcept
		{
			this->id = id;
		}
		inline item_t getID() const noexcept
		{
			return this->id;
		}
		
		inline void setCount(unsigned short count) noexcept
		{
			this->count = count;
		}
		inline unsigned short getCount() const noexcept
		{
			return this->count;
		}
		
		inline void setSpecial(unsigned char special) noexcept
		{
			this->special = special;
		}
		inline unsigned char getSpecial() const noexcept
		{
			return this->special;
		}
		
		inline void setType(unsigned char type) noexcept
		{
			this->type = type;
		}
		inline unsigned char getType() const noexcept
		{
			return this->type;
		}
		
		inline void setHealth(unsigned char health) noexcept
		{
			this->health = health;
		}
		inline unsigned char getHealth() const noexcept
		{
			return this->health;
		}
		
		std::string getName() const;
		
		bool isAlive() const noexcept;
		bool isItem() const noexcept;
		bool isToolItem() const noexcept;
		bool isBlock() const noexcept;
		
		// returns the appropriate tile ID
		// regardless of item type
		int tileByID() const;
	};
	
	class ItemsClass
	{
	public:
		int itemSize;
		int itemsX, itemsY;
		
		void init();
		
		int getMiningTime(const Block& block, const InventoryItem& item) const;
		
		std::string getName(item_t id);
		int tileByID(item_t id);
	};
	extern ItemsClass items;
}

#endif
