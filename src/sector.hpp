#ifndef SECTOR_HPP
#define SECTOR_HPP

#include "common.hpp"
#include "blocks.hpp"
#include "flatland.hpp"
#include <map>

namespace cppcraft
{
	#pragma pack(push, 4)
	class Sector
	{
	public:
		// sector size constants
		static const int BLOCKS_XZ = cppcraft::BLOCKS_XZ;
		static const int BLOCKS_Y  = cppcraft::BLOCKS_Y;
		// sector bitshift size constants
		static const int BLOCKS_XZ_SH = 4;
		static const int BLOCKS_Y_SH  = 8;
		// all parts of sector needs to be rebuilt
		static const int MESHGEN_ALL = 0xFF;

		static const int GENERATED  = 0x1;
		static const int GENERATING = 0x2;

		struct sectorblock_t
		{
			sectorblock_t() {}

			Block& operator() (int x, int y, int z)
			{
				return b[x * BLOCKS_XZ * BLOCKS_Y + z * BLOCKS_Y + y];
			}

			Block b[BLOCKS_XZ * BLOCKS_XZ * BLOCKS_Y];
			short blocks;
			short lights;
			unsigned short nothing_yet;
			unsigned short checksum;
		};
		struct sectordata_t
		{
			sectordata_t() {}

			void assign(int bx, int by, int bz, void* data)
			{
				uint16_t index = Sector::to_block_index(bx, by, bz);
				this->data[index] = data;
			}
			void* get(int bx, int by, int bz)
			{
				int index = Sector::to_block_index(bx, by, bz);
				if (data.find(index) != data.end())
					return data[index];
				return nullptr;
			}
		private:
			std::map<uint16_t, void*> data;
		};

		Sector() {}
		// creates a sector with location (x, y, z)
		Sector(int x, int z);
		~Sector();

		// returns the local coordinates for this sector X and Z
		inline int getX() const
		{
			return this->x;
		}
		inline int getZ() const
		{
			return this->z;
		}
		// returns the world absolute coordinates for this sector X and Z
		int getWX() const;
		int getWZ() const;

		// returns true if the sector has been assigned blocks
		inline bool hasBlocks() const
		{
			return (this->blockpt != nullptr);
		}
		inline short blockCount() const
		{
			return this->blockpt->blocks;
		}
		inline short lightCount() const
		{
			return this->blockpt->lights;
		}

		// clears everything! and then some!
		// at least, it used to FeelsBadMan
		void clear();

		bool isUpdatingMesh() const {
			return this->meshgen != 0;
		}
		auto meshGenStage() const {
			return this->meshgen;
		}
		// returns true if the sector is surrounded by sectors
		// that are already properly generated, or on an edge
		bool isReadyForMeshgen() const;

		// update relevant parts of this sectors mesh
		void updateAllMeshes();
		void updateMeshesAt(int)
		{
			updateAllMeshes();
		}
		void updateByMask(uint8_t)
		{
			updateAllMeshes();
		}

		// returns reference to a Block at (x, y, z)
		inline const Block& operator() (int x, int y, int z) const
		{
			return blockpt->operator()(x, y, z);
		}
		inline Block& operator() (int x, int y, int z)
		{
			return blockpt->operator()(x, y, z);
		}
		inline Block& operator[] (unsigned short index)
		{
			return ((Block*) blockpt)[index];
		}
		// returns a reference to the special section, if one exists
		// otherwise, GOD HELP US ALL
		inline sectordata_t& data()
		{
			return *datasect;
		}
		// returns a reference to the flatland container, if one exists
		// otherwise, GOD HELP US ALL
		inline const Flatland& flat() const
		{
			return _flatl;
		}
		inline Flatland& flat()
		{
			return _flatl;
		}

		// distance to another sector (in block units)
		float distanceTo(const Sector& sector, int bx, int bz) const;

		// torchlight related
		// counts total lights in chunk AND returns that count
		int countLights();

		inline sectorblock_t& getBlocks()
		{
			return *blockpt;
		}
		inline void assignBlocks(sectorblock_t* blocks)
		{
			delete this->blockpt; // out with the old
			this->blockpt = blocks; // in with the new
		}

		std::string to_string() const
		{
			char buffer[32];
			int len = snprintf(buffer, 32, "(x=%d, z=%d)", this->getX(), this->getZ());
			return std::string(buffer, len);
		}

		static uint16_t to_block_index(int bx, int by, int bz)
		{
			return (bx * BLOCKS_XZ + bz) * BLOCKS_XZ + by;
		}

		inline bool generated() const
		{
			return gen_flags & 0x1;
		}
		inline bool generating() const
		{
			return gen_flags & 0x2;
		}

	private:
		// blocks
		sectorblock_t* blockpt;
		// data section
		sectordata_t* datasect;
		// 2d data (just a container!)
		Flatland _flatl;

		friend class Sectors;
		friend class Generator;
		friend class Seamstress;
		friend class Compressor;
	public:
		// 8 bits to signify which parts of sector needs update
		// when an update is needed,
		uint8_t meshgen;
		// true when the generator has generated blocks for this sector
		uint8_t gen_flags;
		// non-zero when objects are scheduled directly on this sector
		uint8_t objects;

		// we flooded this with light, or it needs flooding if the player looks at it?
		bool atmospherics;

		// grid position
		short x, z;
	};
	#pragma pack(pop)
}

#endif
