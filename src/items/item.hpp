#pragma once
#include "../block.hpp"
#include <string>
#include <cstdint>
// no item is always zero
#define IT_NONE  0

namespace cppcraft
{
	typedef unsigned short item_t;

  /**
   *
   *
   *
  **/
	class Item {
	public:
  	static const int BLOCK = 0;
  	static const int ITEM  = 1;

		Item(uint32_t guid, item_t id, item_t type = BLOCK, uint16_t cnt = 1)
			: m_guid(guid), m_id(id | (type << 15)), m_count(cnt) {}

    uint32_t guid() const noexcept {
      return this->m_guid;
    }

		void setID(item_t id) noexcept {
      this->m_id &= 0x8000;
			this->m_id |= id & 0x7FFF;
		}
		item_t getID() const noexcept {
			return this->m_id & 0x7FFF;
		}

    void setType(item_t type) noexcept {
      this->m_id &= 0x7FFF;
			this->m_id |= type << 15;
		}
		item_t getType() const noexcept {
			return this->m_id >> 15;
		}

		void setCount(uint16_t count) noexcept {
			this->m_count = count;
		}
		uint16_t getCount() const noexcept {
			return this->m_count;
		}

		bool isValid() const noexcept {
			return this->getID() != IT_NONE && this->getCount() > 0;
		}

	private:
    uint32_t m_guid; // global unique ID
		item_t   m_id;
		uint16_t m_count;
	};
  static_assert(sizeof(Item) == 8, "Raw items should be 64-bits");
}
