#include "network.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include "chat.hpp"
#include "gameconf.hpp"
#include "netplayers.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "sectors.hpp"
#include "flatlands.hpp"
#include "spiders.hpp"
#include "sun.hpp"
#include "world.hpp"
#include <cstring>
#include <cmath>

static const double PI2 = 4 * atan(1) * 2;

using namespace library;

extern "C"
{
	#include <liblattice/liblattice.h>
}

namespace cppcraft
{
	Network network;
	
	void Network::init(WorldManager& worldman)
	{
		this->connected = false;
		if (config.get("net.connect", false))
		{
			this->running = true;
			this->networkThread = std::thread(&Network::mainLoop, this, std::ref(worldman));
		}
		
		#ifdef TEST_MODEL
			/// testing ///
			netplayers.createTestPlayer();
		#endif
		
		timer.restart();
	}
	
	void Network::mainLoop(WorldManager& worldman)
	{
		if (connect(worldman) == false)
		{
			logger << Log::ERR << "Network::mainLoop(): Connection failed" << Log::ENDL;
			return;
		}
		logger << Log::INFO << "Connected to network" << Log::ENDL;
		chatbox.add("[!]", "Connected to network", Chatbox::L_INFO);
		
		lattice_flush();
		
		timeval waitTime;
		
		// lattice processing mainloop
		while (this->running)
		{
			waitTime.tv_sec  = 0;
			waitTime.tv_usec = 5000;
			lattice_select(&waitTime);
			lattice_process();
			lattice_flush();
			
			// transfer stuff from client to network
			// handles doodlemcgees, performs actions and handles magic
			handleTransfer();
		}
		
		// try to exit normally
		if (this->connected)
		{
			c_quit("Normal exit");
			lattice_flush();
		}
	}
	
	/// incoming blocks ///
	void blockAdded(lattice_badd* badd)
	{
		NetworkBlock block;
		block.type = NetworkBlock::BADD;
		block.wc = badd->wcoord;
		block.bc = badd->bcoord;
		block.block = Block(blockid(badd->block), blockbf(badd->block));
		
		network.addBlock(Network::INCOMING, block);
	}
	void blockAdded(lattice_bset* bset)
	{
		NetworkBlock block;
		block.type = NetworkBlock::BSET;
		block.wc = bset->wcoord;
		block.bc = bset->bcoord;
		block.block = Block(blockid(bset->block), blockbf(bset->block));
		
		network.addBlock(Network::INCOMING, block);
	}
	void blockAdded(lattice_brem* brem)
	{
		NetworkBlock block;
		block.type = NetworkBlock::BREM;
		block.wc = brem->wcoord;
		block.bc = brem->bcoord;
		
		network.addBlock(Network::INCOMING, block);
	}

	void flatlandAdded(lattice_flatland* fl)
	{
		NetworkFlatland flatland;
		flatland.fc = fl->fcoord;
		memcpy(&flatland.fdata, &fl->fdata, sizeof(flatland.fdata));

		network.addFlatland(flatland);
	}

	void sectorAdded(lattice_sector* s)
	{
		NetworkSector sector;
		sector.wc = s->wcoord;
		memcpy(&sector.sector, &s->s, sizeof(sector.sector));

		network.addSector(sector);
	}

	void emptysectorAdded(lattice_emptysector* s)
	{
		NetworkEmptySector sector;
		sector.wc = s->wcoord;

		network.addEmptySector(sector);
	}

	void userAdded(NetPlayer::userid_t userid, lattice_user* user)
	{
		UnpackCoordF coord(user->wpos, user->bpos);
		// create netplayer
		NetPlayer nplayer(userid, user->nickname, user->model, user->usercolor, coord.wc, coord.bc);
		nplayer.setRotation(user->hrot.xrot, user->hrot.yrot);
		// add to active netplayers list
		netplayers.add(nplayer);
		
		logger << Log::INFO << "User joined: " << user->nickname << Log::ENDL;
	}
	void userQuits(NetPlayer::userid_t userid, lattice_quit* msg)
	{
		NetPlayer* np = netplayers.playerByUID(userid);
		if (np)
		{
			logger << Log::INFO << "User quit: " << np->getName() << " (" << &msg->desc << ")" << Log::ENDL;
			if (!netplayers.remove(userid))
			{
				logger << Log::INFO << "User quit error: user could not be removed" << Log::ENDL;
			}
		}
		else
		{
			logger << Log::INFO << "User quit error: uid not found " << userid << Log::ENDL;
		}
	}
	void userChat(NetPlayer::userid_t userid, const char* text)
	{
		NetPlayer* np = netplayers.playerByUID(userid);
		if (np)
		{
			logger << Log::INFO << "CHAT  " << np->getName() << ": " << text << Log::ENDL;
			chatbox.add(np->getName(), text, Chatbox::L_CHAT);
		}
	}
	void userChat(const char* nick, const char* text)
	{
		logger << Log::INFO << "CHAT  " << nick << ": " << text << Log::ENDL;
		chatbox.add(nick, text, Chatbox::L_CHAT);
	}
	
	void userMoved(NetPlayer::userid_t userid, lattice_p* movement)
	{
		NetPlayer* np = netplayers.playerByUID(userid);
		if (np)
		{
			if (movement)
			{
				UnpackCoordF coord(movement->wcoord, movement->bcoord);
				np->moveTo(coord.wc, coord.bc);
			}
			else np->stopMoving();
		}
	}
	void userRotated(NetPlayer::userid_t userid, lattice_pr* rotated)
	{
		NetPlayer* np = netplayers.playerByUID(userid);
		if (np)
		{
			np->setRotation(rotated->rot.xrot, rotated->rot.yrot);
		}
	}
	
	void networkClosing(lattice_closing* lclose)
	{
		logger << Log::WARN << "Server about to disconnect: " << lclose->desc << Log::ENDL;
		chatbox.add("[!]", "Disconnecting: " + std::string(lclose->desc), Chatbox::L_SERVER);
	}
	void bumpError(lattice_bump* bump)
	{
		PackCoord& pc = network.ntt.pcoord;
		logger << Log::INFO << "Player: (" << pc.wc.x << "," << pc.wc.y << "," << pc.wc.z << ") bxyz (" << pc.bc.x << "," << pc.bc.y << "," << pc.bc.z << ")" << Log::ENDL;
		logger << Log::INFO << "Bump: (" << bump->bad_wcoord.x << "," << bump->bad_wcoord.y << "," << bump->bad_wcoord.z << ") bxyz (" << bump->bad_bcoord.x << "," << bump->bad_bcoord.y << "," << bump->bad_bcoord.z << ")" << Log::ENDL;
	}
	
	void message(lattice_message* mp)
	{
		/*
		#define T_P       1
		#define T_QUIT    2
		#define T_PC      3
		#define T_PR      4
		#define T_PH      5
		#define T_CHAT    6
		#define T_ACTION  7
		#define T_S       8
		#define T_SC      9
		#define T_BO      10
		#define T_MO      11
		#define T_BADD    12
		#define T_BSET    13
		#define T_BREM    14
		#define T_PMINE   15
		#define T_SCHAT   16
		#define T_LUSERS  17
		#define T_LOG     18
		#define T_SATSTEP 19
		#define T_SAT     20
		#define T_FADE    21
		#define T_USER    22
		#define T_SERVER  23
		*/
		switch (mp->type)
		{
		case T_BUMP:
			bumpError((lattice_bump*) mp->args);
			break;
		case T_CONNECTED:
			logger << Log::INFO << "Connected to server" << Log::ENDL;
			network.connected = true;
			break;
		case T_CLOSING:
			networkClosing((lattice_closing*) mp->args);
			break;
			
		case T_DISCONNECTED:
			logger << Log::WARN << "Disconnected from server" << Log::ENDL;
			network.connected = false;
			break;
			
		case T_SAT:
			if (true)
			{
				double angle = ((double*) mp->args)[0];
				logger << Log::INFO << "SAT angle: " << angle << Log::ENDL;
				network.ntt.newAngle  = angle;
				network.ntt.updateSun = true;
			}
			break;
			
		case T_SATSTEP:
			if (true)
			{
				lattice_satstep& sats = ((lattice_satstep*) mp->args)[0];
				logger << Log::INFO << "SAT step: " << sats.satstep << " SAT: " << sats.sat << Log::ENDL;
				thesun.setStep(sats.satstep);
				network.ntt.newAngle  = sats.sat;
				network.ntt.updateSun = true;
			}
			break;
			
		case T_P:
			userMoved(mp->fromuid, (lattice_p*) mp->args);
			break;
		case T_PR:
			userRotated(mp->fromuid, (lattice_pr*) mp->args);
			break;
			
		case T_BADD:
			blockAdded((lattice_badd*) mp->args);
			break;
		case T_BSET:
			blockAdded((lattice_bset*) mp->args);
			break;
		case T_BREM:
			blockAdded((lattice_brem*) mp->args);
			break;
		case T_FLATLAND:
			flatlandAdded((lattice_flatland*) mp->args);
			break;
		case T_SECTOR:
			sectorAdded((lattice_sector*) mp->args);
			break;
		case T_EMPTYSECTOR:
			emptysectorAdded((lattice_emptysector*) mp->args);
			break;
		case T_LOG:
			logger << Log::INFO << "SERVER  " << ((const char*) mp->args) << Log::ENDL;
			chatbox.add("SERVER", (const char*) mp->args, Chatbox::L_SERVER);
			break;
			
		case T_SCHAT:
			userChat(((lattice_schat*)mp->args)->nickname, ((lattice_schat*)mp->args)->schat_text);
			break;
		case T_CHAT:
			userChat(mp->fromuid, (const char*) mp->args);
			break;
			
		case T_USER:
			userAdded(mp->fromuid, (lattice_user*) mp->args);
			break;
		case T_FADE:
		case T_QUIT:
			userQuits(mp->fromuid, (lattice_quit*) mp->args);
			break;
			
		default:
			logger << Log::INFO << "Unimplemented message type: " << mp->type << Log::ENDL;
		}
		
		return;
	}
	
	/// from main (worldman) thread ///
	void Network::handleNetworking()
	{
		mtx.lock();
		{
			ntt.pmoved |= player.changedPosition;
			if (ntt.pmoved)   ntt.pcoord = PackCoord(player.X, player.Y, player.Z);
			ntt.protated |= player.changedRotation;
			if (ntt.protated) ntt.prot   = vec2(player.xrotrad, player.yrotrad);
			
                        // receive flatlands from network thread
                        while (ntt.incoming_flatlands.size())
                        {
                                NetworkFlatland& flatland = ntt.incoming_flatlands.front();


                                // set final world coordinates
                                int sx = flatland.fc.x - world.getWX();
                                int sz = flatland.fc.z - world.getWZ();
                                // validate position is inside our grid
                                if (sx < 0 || sx >= Sectors.getXZ() ||
                                    sz < 0 || sz >= Sectors.getXZ())
                                {
                                    ntt.incoming_flatlands.pop_front();
                                    logger << Log::INFO << "Failed Flatland: (" << flatland.fc.x << "," << flatland.fc.z << ")" << Log::ENDL;
                                    continue;
                                }

                                memcpy(&flatlands(sx, sz).fdata, &flatland.fdata, FlatlandSector::FLATLAND_SIZE);

                                ntt.incoming_flatlands.pop_front();
                        }

                        // receive sectors from network thread
                        while (ntt.incoming_sectors.size())
                        {
                                NetworkSector& sector = ntt.incoming_sectors.front();

                                // set final world coordinates
                                int sx = sector.wc.x - world.getWX();
                                int sy = sector.wc.y - world.getWY();
                                int sz = sector.wc.z - world.getWZ();
                                // validate position is inside our grid
                                if (sx < 0 || sx >= Sectors.getXZ() ||
                                    sy < 0 || sy >= Sectors.getY() ||
                                    sz < 0 || sz >= Sectors.getXZ())
                                {
                                    ntt.incoming_sectors.pop_front();
                                    logger << Log::INFO << "Failed Sector: (" << sector.wc.x << "," << sector.wc.y << "," << sector.wc.z << ")" << Log::ENDL;
                                    continue;
                                }
                                int bx = (sx << Sector::BLOCKS_XZ_SH);
                                int by = (sy << Sector::BLOCKS_Y_SH);
                                int bz = (sz << Sector::BLOCKS_XZ_SH);

                                if(Spiders::addsector(bx, by, bz, &sector.sector) == false)
                                {
                                    logger << Log::INFO << "Failed Sector: (" << sector.wc.x << "," << sector.wc.y << "," << sector.wc.z << ")" << Log::ENDL;
                                }

                                ntt.incoming_sectors.pop_front();
                        }

                        // receive emptysectors from network thread
                        while (ntt.incoming_emptysectors.size())
                        {
                                NetworkEmptySector& emptysector = ntt.incoming_emptysectors.front();

                                // set final world coordinates
                                int sx = emptysector.wc.x - world.getWX();
                                int sy = emptysector.wc.y - world.getWY();
                                int sz = emptysector.wc.z - world.getWZ();
                                // validate position is inside our grid
                                if (sx < 0 || sx >= Sectors.getXZ() ||
                                    sy < 0 || sy >= Sectors.getY() ||
                                    sz < 0 || sz >= Sectors.getXZ())
                                {
                                    ntt.incoming_emptysectors.pop_front();
                                    logger << Log::INFO << "Failed Empty Sector: (" << emptysector.wc.x << "," << emptysector.wc.y << "," << emptysector.wc.z << ")" << Log::ENDL;
                                    continue;
                                }
                                int bx = (sx << Sector::BLOCKS_XZ_SH);
                                int by = (sy << Sector::BLOCKS_Y_SH);
                                int bz = (sz << Sector::BLOCKS_XZ_SH);

                                if (Spiders::addemptysector(bx, by, bz) == false)
                                {
                                    logger << Log::INFO << "Failed Empty Sector: (" << emptysector.wc.x << "," << emptysector.wc.y << "," << emptysector.wc.z << ")" << Log::ENDL;
                                }

                                ntt.incoming_emptysectors.pop_front();
                        }

			// receive blocks from network thread
			while (ntt.incoming.size())
			{
				NetworkBlock& block = ntt.incoming.front();
				
				// unpack position
				UnpackCoord coords(block.wc, block.bc);
				
				if (coords.valid)
				{
					block_t id = block.block.getID();
					block_t bf = block.block.getData() >> 10;
					//logger << Log::INFO << "block: " << id << ", " << bf << Log::ENDL;
					
					// make world modification
					switch (block.type)
					{
						case NetworkBlock::BADD:
							Spiders::addblock(coords.bx, coords.by, coords.bz, id, bf, true);
							break;
						case NetworkBlock::BSET:
							Spiders::addblock(coords.bx, coords.by, coords.bz, id, bf, true);
							break;
						case NetworkBlock::BREM:
							Spiders::removeBlock(coords.bx, coords.by, coords.bz, true);
							break;
					}
					
				} // valid coordinates
				else
				{
					//logger << Log::WARN << "network: Block coords out of bounds" << Log::ENDL;
				}
				ntt.incoming.pop_front();
				
			} // incoming work from network
			
			if (ntt.updateSun)
			{
				ntt.updateSun = false;
				thesun.setRadianAngle(ntt.newAngle);
				Sectors.updateAll();
			}
		} // network -> main thread
		mtx.unlock();
		
		// handle players - interpolate movement and determine if renderable
		netplayers.handlePlayers();
	}
	
	/// from networking scheduler ///
	void Network::handleTransfer()
	{
		mtx.lock();
		if (this->connected)
		{
			static const double UPDATE_INTERVAL = 0.100;
			static double lastTime = 0.0;
			
			double deltat = timer.getTime();
			if (deltat > lastTime + UPDATE_INTERVAL)
			{
				lastTime = deltat;
				
				// update position & rotation
				if (ntt.pmoved)
				{
					// start moving
					c_p(ntt.pcoord.wc, ntt.pcoord.bc);
					// signal that we are currently moving
					ntt.psentmoved = true;
					// disable further updates unless they come in after this one
					ntt.pmoved     = false;
				}
				else if (ntt.psentmoved)
				{
					// stop player, but only if we sent moving before this
					c_p_empty();
					ntt.psentmoved = false;
				}
				if (ntt.protated)
				{
					head_rot rot;
					rot.xrot = ntt.prot.x / PI2 * 4096;
					rot.yrot = ntt.prot.y / PI2 * 4096;
					c_pr(rot);
					// disable further updates unless they come in after this one
					ntt.protated = false;
				}
				//logger << Log::INFO << "Send position updates at " << deltat << Log::ENDL;
			}
			
			// send blocks to remote host
			while (ntt.outgoing.size())
			{
				NetworkBlock& nb = ntt.outgoing.front();
				::block_t block;
				//block.id = nb.block.getID();
				//block.bf = nb.block.getData() >> 10;
				block = nb.block.getData();

				// make world modification
				switch (nb.type)
				{
				case NetworkBlock::BADD:
					c_badd(nb.wc, nb.bc, block);
					break;
				case NetworkBlock::BSET:
					c_bset(nb.wc, nb.bc, block);
					break;
				case NetworkBlock::BREM:
					c_brem(nb.wc, nb.bc);
					break;
				}
				// remove front block from queue
				ntt.outgoing.pop_front();
				
			} // outgoing block queue
			
			// outgoing chat
			if (ntt.outgoingChat.size())
			{
				c_schat(ntt.outgoingChat.c_str());
				ntt.outgoingChat.clear();
			}
		}
		mtx.unlock();
	}
	
	void Network::addBlock(direction_t dir, const NetworkBlock& block)
	{
		mtx.lock();
		if (dir == INCOMING)
		{
			ntt.incoming.push_front(block);
		}
		else
		{
			ntt.outgoing.push_front(block);
		}
		mtx.unlock();
	}

	void Network::addFlatland(const NetworkFlatland& fl)
	{
		mtx.lock();
		ntt.incoming_flatlands.push_front(fl);
		mtx.unlock();
	}
	void Network::addSector(const NetworkSector& s)
	{
		mtx.lock();
		ntt.incoming_sectors.push_front(s);
		mtx.unlock();
	}
	void Network::addEmptySector(const NetworkEmptySector& s)
	{
		mtx.lock();
		ntt.incoming_emptysectors.push_front(s);
		mtx.unlock();
	}
	void Network::sendChat(const std::string& text)
	{
		mtx.lock();
		ntt.outgoingChat = text;
		mtx.unlock();
		
		chatbox.add(this->nickname, text, Chatbox::L_CHAT);
	}
}
