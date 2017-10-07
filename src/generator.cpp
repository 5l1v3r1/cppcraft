#include "generator.hpp"

#include <library/config.hpp>
#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "chunks.hpp"
#include "minimap.hpp"
#include "player.hpp"
#include "precompq.hpp"
#include "lighting.hpp"
#include "sectors.hpp"
#include "threadpool.hpp"
#include "world.hpp"

// load compressor last
#include "compressor.hpp"
#include "generator/terragen.hpp"
#include "generator/objectq.hpp"
#include <algorithm>
#include <cstring>
#include <deque>

using namespace library;
using namespace ThreadPool;

namespace cppcraft
{
	unsigned int g_fres[Chunks::CHUNK_SIZE][Chunks::CHUNK_SIZE];
	unsigned int g_compres[Chunks::CHUNK_SIZE][Chunks::CHUNK_SIZE];

	static std::vector<Sector*> queue;
	// multi-threaded shits, mutex for the finished queue
	// and the list of containers of finished jobs (gendata_t)
	static std::mutex mtx_genq;
	static std::deque<terragen::gendata_t*> finished;

	class GeneratorJob
		: public TPool::TJob
	{
	public:
		GeneratorJob() : TPool::TJob() {}

		void run(void* data)
		{
			terragen::gendata_t* gdata = (terragen::gendata_t*) data;
			terragen::Generator::run(gdata);

			// re-add the data back to the finished queue
			mtx_genq.lock();
			finished.push_back(gdata);
			mtx_genq.unlock();
		}
	};

	void Generator::init()
	{
		/// our esteemed generator ///
		terragen::Generator::init();
		// load all block data in view
		for (int x = 0; x < sectors.getXZ(); x++)
		for (int z = 0; z < sectors.getXZ(); z++)
		{
			// during loading, some additional sectors
			// may have been loaded through files so, we
			// check if the sector isnt already generated
			if (sectors(x, z).generated() == false)
				Generator::add(sectors(x, z));
		}
	}

	void Generator::add(Sector& sector)
	{
		if (sector.generating())
			return;

		queue.push_back(&sector);
	}

	int Generator::size()
	{
		return queue.size();
	}

	// the queue of vectors that needs terrain (blocks)
	// we will be using this comparison function to sort
	// the sectors by distance from center
	bool GenerationOrder(Sector* s1, Sector* s2)
	{
		const int center = sectors.getXZ() / 2;
		const int dx1 = s1->getX() - center;
		const int dz1 = s1->getZ() - center;

		const int dx2 = s2->getX() - center;
		const int dz2 = s2->getZ() - center;

		// euclidian:
		//return (dx1*dx1 + dz1*dz1) > (dx2*dx2 + dz2*dz2);
		// manhattan:
		return std::abs(dx1) + std::abs(dz1) > std::abs(dx2) + std::abs(dz2);
	}

	void Generator::run()
	{
		// sort by distance from center (radius)
		std::sort(queue.begin(), queue.end(), GenerationOrder);

		// queue from the top of the vector
		while (!queue.empty() && AsyncPool::available())
		{
			// because its a vector internally, we pop from the back
			Sector* sect = queue.back();
			queue.pop_back();

			if (sect->generating())
			{
				printf("Generator: Already generating (%d, %d)\n", sect->getX(), sect->getZ());
				continue;
			}

			//printf("Generating sector (%d, %d)\n",
			//	sect->getX(), sect->getZ());

			// schedule terrain generator for sector
			sect->add_genflag(Sector::GENERATING);

			// create immutable job data
			terragen::gendata_t* gdata =
				new terragen::gendata_t(sect->getWX(), sect->getWZ());

			// execute the generator job in background, delete job after its done
			AsyncPool::sched(new GeneratorJob, gdata, true);
		}

		// finished generator jobs
		mtx_genq.lock();
		while (!finished.empty())
		{
			// retrieve from queue
			terragen::gendata_t* gdata = finished.front();
			finished.pop_front();
			mtx_genq.unlock();

			int x = gdata->wx - world.getWX();
			int z = gdata->wz - world.getWZ();
			// check that the generated data is still
			// within our grid:
			if (x >= 0 && x < sectors.getXZ() &&
					z >= 0 && z < sectors.getXZ())
			{
				// resultant sector
				Sector& dest = sectors(x, z);
				//printf("Sector was generated: (%d, %d)\n", dest.getX(), dest.getZ());

				// swap out generated blocks
				dest.assignBlocks(gdata->unassignBlocks());
				// and, swap out the flatland data
				dest.flat().assign(gdata->flatl.unassign());
				// toggle sector generated flag, as well as removing generating flag
				dest.meshgen      = 0; // make sure its added to meshgen
				dest.gen_flags    = Sector::GENERATED;
				// we just generated this, it couldnt possibly have any objects scheduled for it? could it?
				dest.objects      = gdata->objects.size();
				dest.atmospherics = false;

				// add all the objects from this sector to object queue
				if (!gdata->objects.empty())
					terragen::ObjectQueue::add(gdata->objects);

				int worldX = sectors(0, 0).getWX() * BLOCKS_XZ;
				int worldZ = sectors(0, 0).getWZ() * BLOCKS_XZ;

				for (terragen::GenObject& obj : gdata->objects)
				{
					int sectX = (obj.x - worldX) / BLOCKS_XZ;
					int sectZ = (obj.z - worldZ) / BLOCKS_XZ;

					if (sectX != dest.getX() || sectZ != dest.getZ())
						assert(0);
				}

				// add it to the minimap!!!
				minimap.addSector(dest);

				// now that its been generated, let's meshmerize it
				precompq.add(dest, 0xFF);
			}
			else
			{
			#ifdef DEBUG
				printf("INVALID sector was generated: (%d, %d)\n", x, z);
			#endif
			}

			// delete the job!
			delete gdata;
			// allow more jobs in the async pool
			AsyncPool::release();

			mtx_genq.lock();
		}
		mtx_genq.unlock();
	}

	void Generator::loadSector(Sector& sector, std::ifstream& file, unsigned int PL)
	{
		// start by setting sector as not having been generated yet
		//sector.generated = false;
		sector.atmospherics = false;

		// load sector, which can return a not-generated sector
		chunks.loadSector(sector, file, PL);

		// if the sector still not generated, we need to invoke the terrain generator
		if (sector.generated() == false)
		{
			/// invoke terrain generator here ///
		}
	}

	/**
	 * Generator will truncate sector(x, z) down to the nearest chunk,
	 * and attempt to load as many sectors from this chunk as possible in one go.
	 * Will first attempt to decompress a compressed column, then replace with modified sectors as they appear
	**/
	bool Generator::generate(Sector& sector, Timer* timer, double timeOut)
	{
		if (sector.generated())
		{
			logger << Log::WARN << "Generator::generate(): sector content already generated" << Log::ENDL;
			return false;
		}

		// open this chunks .compressed file
		std::string sectorString = world.worldFolder() + "/" + chunks.getSectorString(sector);

		std::ifstream cf(sectorString + ".compressed", std::ios::in | std::ios::binary);
		bool cf_open = false;

		if (cf.good())
		{
			cf.seekg(sizeof(int));
			cf.read( (char*) g_compres, sizeof(g_compres) );
			cf_open = true;
		}

		// open this chunks (raw) .chunk file
		std::ifstream ff(sectorString + ".chunk", std::ios::in | std::ios::binary);
		bool ff_open = false;

		if (ff.good())
		{
			ff.seekg(sizeof(int));
			ff.read( (char*) g_fres, sizeof(g_fres) );
			ff_open = true;
		}

		// NOTE NOTE NOTE NOTE NOTE //
		//   NO EARLY EXITS HERE    //
		// NOTE NOTE NOTE NOTE NOTE //

		int dx = (sector.getX() + world.getWX()) & (Chunks::CHUNK_SIZE-1);
		int dz = (sector.getZ() + world.getWZ()) & (Chunks::CHUNK_SIZE-1);

		int x1 = sector.getX() - dx;
		int x2 = x1 + Chunks::CHUNK_SIZE;

		if (x1 < 0) x1 = 0;            // CLAMP AFTER x2 IS SET!!!
		if (x2 > sectors.getXZ()) x2 = sectors.getXZ();

		int z1 = sector.getZ() - dz;
		int z2 = z1 + Chunks::CHUNK_SIZE;

		if (z1 < 0) z1 = 0;            // CLAMP AFTER z2 IS SET!!!
		if (z2 > sectors.getXZ()) z2 = sectors.getXZ();

		bool minimapUpdated = false;

		for (int x = x1; x < x2; x++)
		{
			for (int z = z1; z < z2; z++)
			{
				// bottom / first sector in column
				Sector& sector = sectors(x, z);

				//-------------------------------------------------//
				// load only sectors that are flagged as 'unknown' //
				//-------------------------------------------------//

				if (sector.generated() == false)
				{
					// find sectors internal chunk position
					dx = (x + world.getWX()) & (Chunks::CHUNK_SIZE-1);
					dz = (z + world.getWZ()) & (Chunks::CHUNK_SIZE-1);

					// if compressed file is open, and there was an entry
					if (cf_open && (g_compres[dx][dz] != 0))
					{
						// read entire compressed column
						// compressed column also contains the flatland(x, z) for this area
						Compressor::load(cf, g_compres[dx][dz], x, z);

						// update minimap (colors)
						minimap.addSector(sector);
						minimapUpdated = true;
					}
					else
					{
						// reset flatlands
						//flatlands(x, z).reset();

						// clean out sector
						sector.clear();
					}

					if (ff_open)
					{
						// load single-file, if this sector has an entry
						if (g_fres[dz][dx])
						{
							// load sector using loadSectorEx method
							loadSector(sector, ff, g_fres[dz][dx]);
						}

					} // ff is open

				} // sector in unknown-state

			} // z
		} // x

		if (minimapUpdated)
			minimap.setUpdated();

		if (timer)
			return (timer->getTime() > timeOut);

		// time did not run out
		return false;

	} // generate()

}
