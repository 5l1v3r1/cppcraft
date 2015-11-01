#include "compressor_rle.hpp"

#include <library/log.hpp>

using namespace library;

namespace cppcraft
{
	RLECompressor rle;
	
	bool RLECompressor::hasBlocks(unsigned char* data)
	{
		// set RLEHeader internally, so that further info can be retrieved later on
		// CAREFUL when removing the next line, there is a dependency in the compressor
		rh = (RLEHeader*) data;
		
		return rh->getBlocks() != 0;
	}
	
	void RLECompressor::decompress(unsigned char* data, Sector::sectorblock_t& sb)
	{
		// first treat data as a RLEHeader
		rh = (RLEHeader*) data;
		data += sizeof(RLEHeader);
		
		// get entry data
		RLEEntry* entry = (RLEEntry*) data;
		data += rh->getEntries() * sizeof(RLEEntry);
		
		// first palette (block)
		block_t* palette = (block_t*) data;
		// last entry in palette entries list (used as count)
		RLEEntry* entries = entry + rh->getEntries();
		// entry point to sector blocks
		Block* sblock = sb.b;
		
		for (; entry < entries; entry++)
		{
			block_t blockData = palette[ entry->palid ];
			while (entry->count--)
			{
				*sblock = Block(blockData); sblock++;
			}
		}
		// set sectorblock info
		sb.blocks = rh->getBlocks();
		sb.lights = rh->getLights();
		//sb.hardsolid = rh->getHardsolid();
	}
	
}
