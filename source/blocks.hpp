#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <string>

/////////////////////////////////////////////////////////////////
//// THIS CLASS NEEDS TO BE REBUILT INTO LESS ID MORE OBJECT ////
/////////////////////////////////////////////////////////////////

namespace cppcraft
{
	typedef uint16_t block_t;
	
	class Sector;
	class bordered_sector_t;
	
	#pragma pack(push, 2)
	class Block
	{
	public:
		typedef cppcraft::block_t block_t;
		typedef uint8_t  bfield_t;
		typedef uint8_t  light_t;
		
		// DOES NOTHING on default constructor
		Block() {}
		// constructor taking block id as parameter
		Block(block_t id)
		{
			this->id = id;
			this->bitfield = 0;
		}
		// semi-complete constructor
		Block(block_t id, bfield_t bitfield)
		{
			this->id = id;
			this->bitfield = bitfield;
		}
		// complete whole constructor
		Block(block_t id, bfield_t facing, bfield_t extra)
		{
			this->id = id;
			this->bitfield = facing | (extra << 3);
		}
		
		// sets/gets the block ID for this block
		void setID(block_t id)
		{
			// remove id part
			this->id = id;
		}
		block_t getID() const
		{
			return this->id;
		}
		// sets/gets block facing bits
		void setFacing(bfield_t facing)
		{
			// mask out old facing
			this->bitfield &= 0xF8;
			// mask in new facing
			this->bitfield |= facing;
		}
		bfield_t getFacing() const
		{
			return this->bitfield & 0x7;
		}
		// set/get special bits for this block
		void setExtra(bfield_t extra)
		{
			// remove special part
			this->bitfield &= 0x7;
			// add new special part
			this->bitfield |= (extra << 3);
		}
		bfield_t getExtra() const
		{
			return this->bitfield >> 3;
		}
		
		// set/get whole bitfield part
		void setBitfield(bfield_t bitf)
		{
			this->bitfield = bitf;
		}
		bfield_t getBitfield()
		{
			return this->bitfield;
		}
		
		// returns the whole block as 32bits of data
		inline uint32_t getWhole() const
		{
			return *(uint32_t*) this;
		}
		
		inline light_t getSkyLight() const
		{
			return this->light & 0x0F;
		}
		inline void setSkyLight(light_t level)
		{
			this->light &= 0xF0;
			this->light |= level;
		}
		inline light_t getBlockLight() const
		{
			return this->light >> 8;
		}
		inline void setBlockLight(light_t level)
		{
			this->light &= 0x0F;
			this->light |= level << 8;
		}
		
		// human readable name of a block
		std::string getName() const;
		
		// run visibility tests, revealing visible sides of this cube by comparing neighbors
		unsigned short visibleFaces(bordered_sector_t& bsb, int bx, int by, int bz) const;
		// run visibility tests using spiders
		unsigned short visibleFaces(Sector& sector, int bx, int by, int bz) const;
		
		// converts sides (0, 1, 2, 3) to cube faces (0, 1, 4, 5)
		static int facingToCube(block_t facing);
		// and vice versa
		static int cubeToFacing(block_t facing);
		
		static short bigFaceById(block_t id, block_t face, block_t facing);
		static short cubeFaceById(block_t id, block_t face, block_t facing);
		static bool blockHitbox3D(const Block& b, float dx, float dy, float dz);
		static bool physicalHitbox3D(const Block& b, float dx, float dy, float dz);
		static bool selectionHitbox3D(const Block& b, float dx, float dy, float dz);
		
		static int blockModel(block_t id);
		
		// isType
		static bool isAirOrCross(block_t id);
		static bool isFallingBlock(block_t id);
		
		// tests
		static block_t gravityTest(block_t id);
		static block_t movementTest(block_t id);
		static block_t forwardMovement(block_t id);
		static bool blockPlacement(block_t id);
		static bool blockPlacementAllowed(block_t id);
		static bool BlockPlacementTesting(block_t id);
		
		// conversions
		static block_t fluidToAir(block_t id);
		static block_t ladderToAir(block_t id);
		static block_t halfblockToAir(block_t id);
		static block_t fluidAndCrossToAir(block_t id);
		
		// visibility tests
		// cube face sides
		bool visibilityComp(const Block& b, int facing) const;
		// cube face top
		bool visibilityCompTop(const Block& b) const;
		// cube face bottom
		bool visibilityCompBottom(const Block& b) const;
		
		// returns true if the block has an activation / utility function
		bool hasActivation() const;
		
		// human readable name of a block
		static std::string getBlockName(block_t id);
    
	private:
		block_t  id;
		bfield_t bitfield;
		light_t  light;
	};
	#pragma pack(pop)
	
#define MAX_UNIQUE_IDS  1023

#define AIR_END           23

#define STONE_START       24
#define STONE_END         98

#define SOIL_START       100
#define SOIL_END         159

#define SAND_START       160
#define SAND_END         199

#define HALFBLOCK_START  400
#define HALFBLOCK_END    459

#define STAIRS_START     _STONESTAIR
#define STAIRS_END       HALFBLOCK_END

#define LOWBLOCK_START   460
#define LOWBLOCK_END     503

#define ALPHA_BARRIER    496

#define TREES_START      504
#define TREES_END        549

#define SUPERTRANS_START 580
#define SUPERTRANS_END   649

#define CROSS_START      750
#define CROSS_UNDERWATER 920
#define CROSS_END        959

#define BLEND_BARRIER    960

#define _AIR           0 // air is ignored
#define _FOG           1 // fog block

#define _ADMINIUM     24 // les indestructible

#define _STONEHEAP    25 // stones
#define _STONE        26
#define _REDOCHRE     27
#define _STONEPATTERN 30

#define _COAL        40 // ores
#define _IRON        41
#define _GOLD        42
#define _REDSTONE    43
#define _GREENSTONE  44
#define _DIAMOND     45
#define _MAGICSTONE  46
#define _LIVINGSTONE 47

#define _ASPHALT     50

#define _IRONBLOCK   55
#define _GOLDBLOCK   56
#define _DIAMBLOCK   57
#define _ICECUBE     58

#define _MOLTENSTONE   62

#define _GREENSOIL     100 // earth
#define _GREENGRASS_S  101
#define _GREENGRASS    102
#define _SNOWSOIL      103
#define _SNOWGRASS_S   104
#define _SNOWGRASS     105

#define _STONEGRASS    130

#define _SANDBEACH   160
#define _SANDOCEAN   161
#define _OCEANFLOOR  162 // ocean sand
#define _DESERTSAND  163 // desert sand

#define _GRAVEL1     167 // brown gravel
#define _GRAVEL2     168 // grey gravel

#define _CLAY        195 // blue-grey clay
#define _CLAYRED     196 // red-grey clay

#define _WOODBROWN  200 // ministry of forestry
#define _WOODBJORK  201
#define _WOODPALM   202
#define _WOODPINE   203
#define _PLANK      204
#define _WORKBENCH  205 // all-in-one workbench /- plankmaster-2000 -/
#define _CHEST      206
#define _BOOKCASE   207

#define _PUMPKIN     231
#define _CACTUSBLOCK 232

#define _GIANTSHROOMCORE      250
#define _GIANTSHROOMCORETOP   251
#define _GIANTSHROOMUNDERTOP  252
#define _GIANTSHROOMTOP       253
#define _GIANTSHROOMTOPEDGE   254
#define _GIANTSHROOMTOPSPECLE 255

#define _FURNACE        300  // player tool blocks
#define _FURNACEBURNING 301

#define _BRICKWALL      306
#define _BRICKWALL2     307

#define _SPONGE         330
#define _TRAMPOLINE     333
#define _TNT			334

#define _GRADIENT_SOFT  337 // soft paletted blocks
#define _GRADIENT_HARD  338 // hard paletted blocks


///  half blocks 400 -> 459 ///

#define _HALFSTONE      400
#define _HALFSTONETILE  401
#define _HALFSTONEPLATE 402
#define _HALFBRICK      403
#define _HALFBRICK2     404

#define _HALFWOOD       410


// special case: kill only bottom / sides matching other halfs
#define _STONESTAIR     450
#define _WOODSTAIR      451
#define _BRICKSTAIR     452

///  low blocks 460 -> 503 ///

#define _LOWSTONETILE   480
#define _LOWSTONETILE2  481
#define _LOWSNOW        482
#define _LOWICE         483 // ice

///  alpha blocks 504+ ///

///  leaf 504-549 ///
#define _LEAF_LEAFS    504 // more forestry

#define _LEAF_NEEDLE   510

#define _VINES         545

// --------------------------
#define _LADDER        550

#define _LANTERN       552
#define _LANTERN_OFF   553

#define _WOODFENCE     554

#define _WOODPOLE      555

#define _WOODDOOR      560
#define _STONEDOOR     561

#define _WOODSIGN      570

#define _GLASS         580 // glass super-transparent objects

#define _CACTUS        600

#define _DIG0          639 // player stuff
#define _DIG1          640
#define _DIG2          641
#define _DIG3          642
#define _DIG4          643
#define _DIG5          644
#define _DIG6          645
#define _DIG7          646
#define _DIG8          647
#define _DIG9          648
#define _SELECTION     649

///  crosses 750->960 ///
#define _FLOWERRED       750
#define _FLOWERYELLOW    751
#define _FLOWERROSE      752
#define _GRASS_LONG      753
#define _GRASS_SHORT     754
#define _DESERTFLOWER    755
#define _FLOWERREDMAG    756
#define _PLANT_DRYBROWN  757
#define _FLOWER_SMALL    758
#define _REEDS           759

#define _BUSH_HOLLY    775
#define _BUSH2         776
#define _BUSH3         777
#define _BUSH4         778
#define _BUSH5         779
#define _BUSH_PINKROSE 780
#define _BUSH_MRSPIKY  781
#define _FLOWER_CROCUS 782

#define _DARKSHROOM    790
#define _REDSHROOM     791
#define _BLUESHROOM    792

#define _WATERLILY     796

#define _IVYLEAF       798
#define _LILYPAD       799

#define _FIRE          800

#define _HANGINGAPPLE  810

#define _TORCH         820

//  underwater crosses 920-959 //
#define _SEAWEED       920

// alphablend blocks (960+) //

#define _WATER        1000  // liquid H20
#define _LAVABLOCK    1001

#define isAir(id) (id == _AIR)
#define isStone(id) (id >= STONE_START && id <= STONE_END)
#define isDirt(id) (id >= SOIL_START && id <= SOIL_END)
#define isDirtSoil(id) (isDirt(id) && (id % 3 == 1))   // sub-element 1/3
#define isDirtGrass(id) (isDirt(id) && (id % 3 == 2))  // sub-element 2/3
#define isSnow(id) (id >= _SNOWSOIL && id <= _SNOWGRASS)
#define isSand(id) (id >= SAND_START && id <= SAND_END)
#define isGravel(id) (id >= _GRAVEL1 && id <= _CLAYRED)
#define isWood(id) (id >= _WOODBROWN && id <= _PUMPKIN)
#define isLeaf(id) (id >= TREES_START && id <= TREES_END)
#define isCross(id) (id >= CROSS_START && id <= CROSS_END)
#define isFluid(id) (id == _WATER || id == _LAVABLOCK)
#define isLight(id) (id == _TORCH || id == _MOLTENSTONE || id == _LAVABLOCK || id == _REDSTONE || id == _GREENSTONE || id == _FIRE || id == _LANTERN)

#define isStair(id) (id >= STAIRS_START && id < STAIRS_END)
#define isLowblock(id) (id >= LOWBLOCK_START && id <= LOWBLOCK_END)
#define isHalfblock(id) (id >= HALFBLOCK_START && id <= HALFBLOCK_END)

#define isPole(id) (id == _WOODPOLE)
#define isDoor(id) (id == _WOODDOOR || id == _STONEDOOR)

#define isColoredBlock(id) (id == _GIANTSHROOMTOP || id == _GIANTSHROOMTOPSPECLE || id == _GIANTSHROOMTOPEDGE)

#define mHardsolid(id) (isAir(id) || id >= HALFBLOCK_START)

}

#endif
