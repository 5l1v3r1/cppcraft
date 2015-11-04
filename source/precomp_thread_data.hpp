#include "renderconst.hpp"

#include "blocks.hpp"
#include "vertex_block.hpp"

namespace cppcraft
{
	class FlatlandSector;
	class Sector;
	
	class PrecompThreadData
	{
	public:
		~PrecompThreadData();
		
		int pipelineSize[RenderConst::MAX_UNIQUE_SHADERS];
		// working buffers
		vertex_t* databuffer[RenderConst::MAX_UNIQUE_SHADERS];
		int       vertices[RenderConst::MAX_UNIQUE_SHADERS];
		
		// all the blocks
		bordered_sector_t* sector;
		
		// resolve (x, y, z) to vertex lighting
		uint16_t getLight(int x, int y, int z);
		
		// blocks counter, for exiting early when all blocks are processed
		int blocks;
		// vertex data pointer
		vertex_t* indic;
		// last block id to be processed
		block_t lastid;
		
		// selected shader type (determined by block id)
		int shaderLine;
		
		// special properties of big tiles
		bool repeat_y;
		static const int repeat_factor;
		int bigTextures;
		
		// flatlands biome color index
		int colorIndex;
		
		// vertex biome color index
		int lastclid;
		// resulting vertex biome colors
		unsigned int fbiome[4];
		// cheap CRC, to prevent calculating the same color position twice
		int fbicrc;
		
		// processes a Block, outputs a mesh w/lighting
		void process_block(const Block& currentBlock, int bx, int by, int bz);
		
		// vertex emitters for each side of a cube
		int emitCube(const Block& currentBlock, int bx, int by, int bz, int model, block_t facing);
		
		void emitCubeVertexPZ(int model, block_t id, int bx, int by, block_t block_facing);
		void emitCubeVertexNZ(int model, block_t id, int bx, int by, block_t block_facing);
		
		void emitCubeVertexPY(int model, block_t id, int bx, int by, int bz);
		void emitCubeVertexNY(int model, block_t id, int bx, int by, int bz);
		
		void emitCubeVertexPX(int model, block_t id, int by, int bz, block_t block_facing);
		void emitCubeVertexNX(int model, block_t id, int by, int bz, block_t block_facing);
		
		// sloped leafs
		int emitSloped(block_t id, int bx, int by, int bz, block_t model, block_t facing);
		
		// ladders
		int emitLadder(block_t id, int bx, int by, int bz, block_t block_facing);
		// poles
		int emitPole(block_t id, int bx, int by, int bz, block_t model);
		// stairs
		int emitStair(const Block& currentBlock, int bx, int by, int bz, block_t sides);
		// fences
		int emitFence(const Block& currentBlock, int bx, int by, int bz, block_t sides);
		// doors
		int emitDoor(const Block& currentBlock, int bx, int by, int bz, block_t sides);
		// lanterns
		int emitLantern(block_t id, int bx, int by, int bz);
		// crosses
		int emitCross(block_t id, int bx, int by, int bz);
		
		// light raytracer & emission functions for each side of a cube
		
		uint16_t smoothLight(int x1, int y1, int z1,  int x2, int y2, int z2,  int x3, int y3, int z3,  int x4, int y4, int z4);
		
		void applyFaceLighting_PZ(int bx, int by, int bz);
		void applyFaceLighting_NZ(int bx, int by, int bz);
		
		void applyFaceLighting_PY(int bx, int by, int bz);
		void applyFaceLighting_NY(int bx, int by, int bz);
		
		void applyFaceLighting_PX(int bx, int by, int bz);
		void applyFaceLighting_NX(int bx, int by, int bz);
		
		void applyFaceLightingAll(int bx, int by, int bz);
	};

}