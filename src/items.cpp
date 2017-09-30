#include "items.hpp"

#include <library/config.hpp>
#include <library/log.hpp>
#include "block.hpp"
#include "gameconf.hpp"

using namespace library;

namespace cppcraft
{
	ItemsClass items;

	std::string Item::getName() const
	{
		// regular blocks (as items)
		if (this->getType() == ITT_BLOCK)
			return Block(this->getID(), this->getSpecial()).getName();

		return items.getName(this->getID());
	}

	int Item::getTextureTileID() const
	{
		// regular blocks (as items)
		if (this->getType() == ITT_BLOCK)
			return toBlock().getTexture(0);

		return items.tileByID(this->getID());

	}

	void ItemsClass::init()
	{
		// initialing items
		logger << Log::INFO << "* Initializing items" << Log::ENDL;

		// items texture size
		this->itemSize = config.get("items.size", 32);

	}

	int ItemsClass::tileByID(item_t id)
	{

		// (inventory) items
		switch (id)
		{
		case IT_WOODPICK:
		case IT_STONEPICK:
		case IT_IRONPICK:
		case IT_GOLDPICK:
		case IT_DIAMPICK:
			return (id - IT_WOODPICK) + 1 * items.itemsX;

		case IT_WOODSWORD:
		case IT_STONESWORD:
		case IT_IRONSWORD:
		case IT_GOLDSWORD:
		case IT_DIAMSWORD:
			return (id - IT_WOODSWORD) + 0 * items.itemsX;

		case IT_WOODSHOVEL:
		case IT_STONESHOVEL:
		case IT_IRONSHOVEL:
		case IT_GOLDSHOVEL:
		case IT_DIAMSHOVEL:
			return (id - IT_WOODSHOVEL) + 2 * items.itemsX;

		case IT_WOODAXE:
		case IT_STONEAXE:
		case IT_IRONAXE:
		case IT_GOLDAXE:
		case IT_DIAMAXE:
			return (id - IT_WOODAXE) + 3 * items.itemsX;

		case IT_WOODSCYTHE:
			return 0 * items.itemsX + 7;

		case IT_PORTALGUN:
			return 7 * items.itemsX + 0;

		case IT_CREATORGUN:
			return 7 * items.itemsX + 1;

		case IT_COAL:
			return 5 * items.itemsX + 0;
		case IT_IRON:
			return 5 * items.itemsX + 1;
		case IT_GOLD:
			return 5 * items.itemsX + 2;
		case IT_STICK:
			return 4 * items.itemsX + 0;
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

	int ItemsClass::getMiningTime(const Block& block, const Item& helditem) const
	{
		(void) block;
		(void) helditem;

		// default value
		return 8;
	}

}
