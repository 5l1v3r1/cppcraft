#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <library/math/vector.hpp>
#include <library/timing/timer.hpp>
#include <liblattice/coordinates.h>
#include "blocks.hpp"
#include "world.hpp"
#include "flatlands.hpp"
#include "sector.hpp"
#include <deque>
#include <mutex>
#include <thread>

//#define TEST_MODEL

struct lattice_bump;
struct lattice_message;

namespace cppcraft
{
	class Sector;
	class WorldManager;
	
	class PackCoord
	{
	public:
		PackCoord() {}
		// packed precision coordinates (8 bit fractions)
		PackCoord(double x, double y, double z);
		// packed integral coordinates (0 bit fractions)
		PackCoord(int bx, int by, int bz);
		PackCoord(Sector* sector, int bx, int by, int bz);
		
		w_coord wc;
		b_coord bc;
		bool valid;
	};
	class UnpackCoord
	{
	public:
		// unpack integral coordinates (0 bit fractions)
		UnpackCoord(w_coord& wc, b_coord& bc);
		
		int bx, by, bz;
		bool valid;
	};
	class UnpackCoordF
	{
	public:
		// unpack precision coordinates (8 bit fractions)
		UnpackCoordF(w_coord& wc, b_coord& bc);
		
		w_coord wc;
		library::vec3 bc;
	};
	
	class NetworkBlock
	{
	public:
		enum btype_t
		{
			BADD,
			BREM,
			BSET
		};
		
		NetworkBlock() {}
		NetworkBlock(int bx, int by, int bz, const Block& block, btype_t type);
		
		btype_t type;
		Block block;
		w_coord wc;
		b_coord bc;
	};
	
        class NetworkFlatland
        {
        public:
                NetworkFlatland() {}

                f_coord fc;
                FlatlandSector::flatland_t fdata[Sector::BLOCKS_XZ][Sector::BLOCKS_XZ];
        };

        class NetworkSector
        {
        public:
                NetworkSector() {}

                w_coord wc;
                Sector::sectorblock_t sector;
        };

        class NetworkEmptySector
        {
        public:
                NetworkEmptySector() {}

                w_coord wc;
        };

	class NetworkThreadTransfer
	{
	public:
		bool pmoved, protated;
		// true if the last package was sent as player having moved
		bool psentmoved;
		// player coordinates
		PackCoord pcoord;
		// player rotation
		library::vec2 prot;
		
		std::deque<NetworkBlock> incoming;
		std::deque<NetworkBlock> outgoing;
		
                std::deque<NetworkFlatland> incoming_flatlands;
                std::deque<NetworkSector> incoming_sectors;
                std::deque<NetworkEmptySector> incoming_emptysectors;

		std::string outgoingChat;
		
		bool  updateSun = false;
		float newAngle;
	};
	
	class Network
	{
	public:
		enum direction_t
		{
			INCOMING,
			OUTGOING
		};
		
		void init(WorldManager& worldman);
		void stop();
		
		void handleNetworking();
		
		void addBlock(direction_t, const NetworkBlock&);
                void addFlatland(const NetworkFlatland&);
                void addSector(const NetworkSector&);
                void addEmptySector(const NetworkEmptySector&);
		void sendChat(const std::string& text);
		
		inline const std::string& getNickname() const
		{
			return this->nickname;
		}
		
		inline bool isConnected() const
		{
			return connected;
		}
		
	private:
		bool connect(WorldManager& worldman);
		
		void mainLoop(WorldManager& worldman);
		void handleTransfer();
		
		bool running;
		bool connected;
		std::mutex  mtx;
		std::thread networkThread;
		// timing for network updates
		library::Timer timer;
		// all the stuff we want to copy between the threads now and then, 
		// just to have it somewhere visible
		NetworkThreadTransfer ntt;
		
		std::string nickname;
		
		friend void bumpError(lattice_bump* bump);
		friend void message(lattice_message* mp);
	};
	extern Network network;
}

#endif // NETWORK_HPP
