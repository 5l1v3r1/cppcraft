#ifndef SECTOR_HPP
#define SECTOR_HPP

#include "blocks.hpp"

namespace cppcraft
{
	#pragma pack(push, 4)
	class Sector
	{
	public:
		// sector size constants
		static const int BLOCKS_XZ = 16;
		static const int BLOCKS_Y  =  8;
		// sector bitshift size constants
		static const int BLOCKS_XZ_SH = 4;
		static const int BLOCKS_Y_SH  = 3;
		// bitmask for a fully hidden sector
		static const int MAX_HARDSOLID = 63;
		
		// sector-block, a 3D volume of blocks
		#pragma pack(push, 2)
		typedef struct sectorblock_t
		{
			sectorblock_t() {}
			
			Block b[BLOCKS_XZ][BLOCKS_XZ][BLOCKS_Y] __attribute__((aligned(2)));
			short blocks;
			short lights;
			unsigned char hardsolid;
			unsigned char special;
			unsigned short version;
			
		} sectorblock_t;
		#pragma pack(pop)
		
		enum progress_t
		{
			PROG_NEEDGEN,
			PROG_NEEDRECOMP,
			PROG_RECOMPILE,
			PROG_RECOMPILING,
			PROG_NEEDAO,
			PROG_AO,
			PROG_NEEDCOMPILE,
			PROG_COMPILED
		};
		
		enum sectorcontents_t
		{
			CONT_NULLSECTOR,
			CONT_UNKNOWN,
			CONT_SAVEDATA
		};
		
		Sector() {}
		// creates a sector with location (x, y, z)
		Sector(int x, int y, int z);
		~Sector();
		
		inline int getX() const
		{
			return (this - this->y)->x;
		}
		inline int getY() const
		{
			return this->y;
		}
		inline int getZ() const
		{
			return (this - this->y)->z;
		}
		
		// returns true if the sector has been assigned a sectorblock
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
		inline unsigned char solidFlags() const
		{
			return this->blockpt->hardsolid;
		}
		
		// returns true if there is a point in trying to recompile this sector
		inline bool isUpdateable() const
		{
			return (contents == CONT_SAVEDATA && culled == false);
		}
		
		void createBlocks();
		void smartAssignBlocks(bool needBlocks);
		void clear();      // clears everything! and then some!
		void invalidate(); // sector needs to be generated
		
		// returns reference to a Block at (x, y, z)
		inline Block& operator() (int x, int y, int z)
		{
			return this->blockpt->b[x][z][y];
		}
		
		// distance to another sector (in block units)
		float distanceTo(const Sector& sector, int bx, int by, int bz) const;
		
		// torchlight related
		int countLights(); // recounts lights internally AND returns that count
		
		inline Sector* getBaseSector()
		{
			return this - this->y;
		}
		
	private:
		/// data members
		// data/blocks
		sectorblock_t* blockpt;
		// special datablock
		//specialptr special;
		
		// grid position
		short x, y, z;
		
	public:
		// flags
		bool render;   // renderable
		//! See: progress_t
		char progress;
		//! See: sectorcontents_t
		char contents;
		//! 0 = not computed, 1 = no lights, 2 = found lights
		char hasLight;
		bool culled;   // surrounded by solids
		bool hasWork;
		
		friend class Chunks;
		friend class Compressor;
		friend class SectorContainer;
		friend class Seamless;
		friend class Spiders;
		friend class Network;
	};
	#pragma pack(pop)
}

#endif
