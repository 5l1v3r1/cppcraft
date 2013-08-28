#include "items.hpp"

namespace cppcraft
{
	ItemsClass items;
	
	InventoryItem::InventoryItem()
	{
		id = IT_NONE;
		type = ITT_BLOCK;
		count = special = health = 0;
	}
	
	InventoryItem::InventoryItem(item_t itemID, unsigned short icount)
	{
		id      = itemID;
		count   = icount;
		type    = ITT_BLOCK;
		special = 0;
		health  = 0;
	}
	
	InventoryItem::InventoryItem(item_t itemID, itemtype_t itype, unsigned short icount)
	{
		id      = itemID;
		type    = itype;
		count   = icount;
		special = 0;
		health  = 0;
	}
	
	bool InventoryItem::isActionItem() const
	{
		return (this->type == ITT_ITEM);
	}
	
	int ItemsClass::tileByID(int id)
	{
		switch(id)
		{
			case IT_WOODPICK:
			case IT_STONEPICK:
			case IT_IRONPICK:
			case IT_GOLDPICK:
			case IT_DIAMPICK:
				return (id - IT_WOODPICK) + 1 * itemsX;
				
			case IT_WOODSWORD:
			case IT_STONESWORD:
			case IT_IRONSWORD:
			case IT_GOLDSWORD:
			case IT_DIAMSWORD:
				return (id - IT_WOODSWORD) + 0 * itemsX;
				
			case IT_WOODSHOVEL:
			case IT_STONESHOVEL:
			case IT_IRONSHOVEL:
			case IT_GOLDSHOVEL:
			case IT_DIAMSHOVEL:
				return (id - IT_WOODSHOVEL) + 2 * itemsX;
				
			case IT_WOODAXE:
			case IT_STONEAXE:
			case IT_IRONAXE:
			case IT_GOLDAXE:
			case IT_DIAMAXE:
				return (id - IT_WOODAXE) + 3 * itemsX;
				
			case IT_WOODSCYTHE:
				return 0 * itemsX + 7;
				
			case IT_PORTALGUN:
				return 7 * itemsX + 0;
				
			case IT_CREATORGUN:
				return 7 * itemsX + 1;
				
			case IT_COAL:
				return 5 * itemsX + 0;
			case IT_IRON:
				return 5 * itemsX + 1;
			case IT_GOLD:
				return 5 * itemsX + 2;
			case IT_STICK:
				return 4 * itemsX + 0;
		}
		
		return 255;
	} // tileById
	
	std::string itemMaterialType(item_t baseid)
	{
		switch(baseid)
		{
			case 0: return "Wooden";
			case 1: return "Stone";
			case 2: return "Iron";
			case 3: return "Golden";
			case 4: return "Diamond";
		}
		return "itemMaterialType: FAIL";
	}

	
	std::string ItemsClass::getName(item_t id)
	{
		switch(id)
		{
			case IT_NONE:
				return "No item";
				
			case IT_WOODPICK:
			case IT_STONEPICK:
			case IT_IRONPICK:
			case IT_GOLDPICK:
			case IT_DIAMPICK:
				return itemMaterialType(id - IT_WOODPICK) + " pickaxe";
				
			case IT_WOODSWORD:
			case IT_STONESWORD:
			case IT_IRONSWORD:
			case IT_GOLDSWORD:
			case IT_DIAMSWORD:
				return itemMaterialType(id - IT_WOODSWORD) + " sword";
				
			case IT_WOODSHOVEL:
			case IT_STONESHOVEL:
			case IT_IRONSHOVEL:
			case IT_GOLDSHOVEL:
			case IT_DIAMSHOVEL:
				return itemMaterialType(id - IT_WOODSHOVEL) + " shovel";
				
			case IT_WOODAXE:
			case IT_STONEAXE:
			case IT_IRONAXE:
			case IT_GOLDAXE:
			case IT_DIAMAXE:
				return itemMaterialType(id - IT_WOODAXE) + " axe";
				
			case IT_WOODSCYTHE:
				return itemMaterialType(id - IT_WOODSCYTHE) + " hoe";
				
			case IT_COAL:
				return "Coal";
			case IT_IRON:
				return "Iron bar";
			case IT_GOLD:
				return "Gold bar";
			case IT_DIAMOND:
				return "Diamond bar";
			case IT_REDSTONE:
				return "Redstone";
			case IT_MAGIC:
				return "Magic stone";
				
			case IT_STICK:
				return "Wooden stick";
				
			case IT_PORTALGUN:
				return "Portal gun";
				
			case IT_TORCH:
				return "Torch";
				
			case IT_CREATORGUN:
				return "Someones gun";
				
			default:
				return "Unknown item "; //(" & id & ")";
		}
		
	} // ItemsClass::getName()

}
