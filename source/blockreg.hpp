#pragma once

namespace cppcraft
{
	class BlockData
	{
	public:
		// function for generating block mesh
		// std::function<void(std::vector<> mesh&, Block&, int x, int y, int z)> mesh;
		
		// tick function
		typedef void (*tick_func_t) ();
		tick_func_t tick_function = nullptr;
		
		// fully opaque blocks are common, and cover all their sides
		bool opaque;
		
		// function for generating vertex color for block
		//std::function<uint32_t(int x, int y, int z, terrain_t& weights)> getVertexColor;
		
	};
	
	class BlockDB
	{
		// name to id conversion
		// example: cppcraft:dirt
		std::map<std::string, int> names;
		
		// vector of registered blocks
		// the ID of a block is its index into this vector
		std::vector<BlockData> blocks;
	};
	BlockDB blockRegistry;
}
