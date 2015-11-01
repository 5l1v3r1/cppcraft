#include "compressor.hpp"

#include <library/log.hpp>
#include <library/compression/lzo.hpp>
#include "compressor_rle.hpp"
#include "sectors.hpp"

#include <cstring>

using namespace library;

namespace cppcraft
{
	library::LZO compressor;
	lzo_bytep compressor_databuffer;
	
	struct compressed_datalength_t
	{
		unsigned short lzoSize;
		//unsigned short sectors;
	};
	
	void Compressor::init()
	{
		logger << Log::INFO << "* Initializing compressor" << Log::ENDL;
		
		const int compressed_column_size = Flatland::FLATLAND_SIZE + sizeof(Sector::sectorblock_t);
		
		// initialize LZO
		compressor.init(compressed_column_size);
		
		// allocate towering buffer
		compressor_databuffer = (lzo_bytep) malloc(compressed_column_size * sizeof(lzo_byte));
		if (compressor_databuffer == nullptr)
			throw std::string("Chunks:compressorInit(): Compressor databuffer was null");
	}
	
	void Compressor::load(std::ifstream& File, int PL, int x, int z)
	{
		// read datalength
		compressed_datalength_t datalength;
		
		File.seekg(PL);
		File.read( (char*) &datalength, sizeof(compressed_datalength_t) );
		
		if (datalength.lzoSize == 0)
		{
			// clear sector at (x, z)
			sectors(x, z).clear();
			
			// exit early
			return;
		}
		
		// go past first struct
		File.seekg(PL+sizeof(compressed_datalength_t));
		// read entire compressed block
		File.read((char*) compressor_databuffer, datalength.lzoSize);
		
		// decompress data
		if (compressor.decompress2a(compressor_databuffer, datalength.lzoSize) == false)
		{
			logger << Log::ERR << "Compressor::decompress(): Failed to decompress data" << Log::ENDL;
			throw std::string("Compressor::decompress(): Failed to decompress data");
		}
		
		lzo_bytep cpos = compressor.getData();
		
		// copy over flatland struct
		memcpy (sectors.flatland(x, z).fdata, cpos, Flatland::FLATLAND_SIZE);
		
		// move to first sectorblock
		cpos += Flatland::FLATLAND_SIZE;
		
		Sector& base = sectors(x, z);
		
		// check if any blocks are present
		if (rle.hasBlocks(cpos))
		{
			// decompress directly onto sectors sectorblock
			rle.decompress(cpos, base.getBlocks());
			
			// set sector flags (based on sectorblock flags)
			// set sector-has-data flag
			//base.generated = true;
			// flag sector for mesh assembly (next stage in pipeline)
			//base.meshgen = Sector::MESHGEN_ALL;
		}
		else
		{
			// had no blocks, but we can't null it since its below terrain
			base.clear();
		}
		
		// go to next RLE compressed sector
		//cpos += rle.getSize();
		
	} // loadCompressedColumn
	
}
