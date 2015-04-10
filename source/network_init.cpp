#include "network.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include "chat.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include "world.hpp"
#include "worldmanager.hpp"
#include <cstring>
#include <cmath>

using namespace library;

extern "C"
{
	#include <liblattice/liblattice.h>
}

namespace cppcraft
{
	void message(lattice_message* mp);
	
	bool Network::connect(WorldManager& worldman)
	{
		lattice_init(-1, message);
		
		std::string  hostn = config.get("net.host", "127.0.0.1");
		unsigned int port  = config.get("net.port", 9666);
		
		std::string uname  = config.get("net.user", "guest");
		std::string upass  = config.get("net.pass", "guest");
		
                int sectors_xz = config.get("world.viewdist", 48);

                sectors_xz -= 16;
                if (sectors_xz < 8) sectors_xz = 8;

		/// CONNECTING TO AUTH SERVER ///
		std::stringstream ss;
		ss << "Connecting to " << hostn << ":" << port;
		
		logger << Log::INFO << ss.str() << Log::ENDL;
		chatbox.add("[!]", ss.str(), Chatbox::L_INFO);
		
		// temporary nickname
		this->nickname = uname;
		
		int result = authserver_login(uname.c_str(), upass.c_str(), hostn.c_str(), port, sectors_xz);
		if (result < 0)
		{
			std::string authFailed = std::string("Authentication failed: ") + authserver_errorstring(result);
			logger << Log::WARN << authFailed << Log::ENDL;
			chatbox.add("[!]", authFailed, Chatbox::L_INFO);
			return false;
		}
		
		lattice_player_t lplayer;
		
		result = lattice_getplayer(&lplayer);
		if (result < 0)
		{
			logger << Log::WARN << "Retrieving player struct failed: " << result << Log::ENDL;
			return false;
		}
		
		UnpackCoordF playerCoords(lplayer.wpos, lplayer.bpos);
		// teleport to player coordinates
		World::world_t wxyz(playerCoords.wc.x, playerCoords.wc.y, playerCoords.wc.z);
		worldman.teleport(wxyz, playerCoords.bc);
		// ------------------------------
		
		this->nickname = std::string(lplayer.nickname);
		return true;
	}
	
	void Network::stop()
	{
		if (this->running)
		{
			this->running = false;
			this->networkThread.join();
		}
	}
	
}
