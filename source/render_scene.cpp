#include "render_scene.hpp"

#include "library/config.hpp"
#include "library/log.hpp"
#include "library/opengl/opengl.hpp"
#include "library/opengl/window.hpp"

#include "frustum.hpp"
#include "minimap.hpp"
#include "particles.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "renderman.hpp"
#include "render_sky.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "threading.hpp"
#include "world.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	void SceneRenderer::init(Renderer& renderer)
	{
		// initialize terrain renderer
		initTerrain();
		
		// initialize sky renderer
		skyrenderer.init();
		
		// initialize minimap
		minimap.init();
	}
	
	// render normal scene
	void SceneRenderer::render(Renderer& renderer, WorldManager& worldman)
	{
		bool frustumRecalc = false;
		
		// render each blah, because of blah-bah
		if (mtx.sectorseam.try_lock())
		{
			mtx.playermove.lock();
			{
				//------------------------------------//
				//          player snapshot           //
				//------------------------------------//
				this->playerX = player.snapX;
				this->playerY = player.snapY;
				this->playerZ = player.snapZ;
				this->playerMoved = player.JustMoved;
				
				this->playerSectorX = playerX / 16; //Sector::BLOCKS_XZ;
				this->playerSectorZ = playerZ / 16; //Sector::BLOCKS_XZ;
				
				frustumRecalc = frustum.recalc;
				frustum.recalc = false;
			}
			mtx.playermove.unlock();
			
			if (frustumRecalc)
			{
				/// update matview matrix using player snapshot ///
				frustum.setWorldOffset(playerX, playerZ);
			}
			
			/// world coordinate snapshots ///
			snapWX = world.getWX();
			snapWZ = world.getWZ();
			
			/// update minimap ///
			minimap.update();
			
			/// update particles ///
			particleSystem.renderUpdate();
			
			/// camera deviations ///
			if (cameraDeviation(renderer.frametick, renderer.dtime))
			{
				frustumRecalc = true;
			}
			
			// update frustum rotation matrix
			if (frustum.rotated)
			{
				frustum.setRotation(player.xrotrad, player.yrotrad, 0.0);
			}
			
			//---------------------------------//
			// Start of frustum recalculations //
			//---------------------------------//
			
			// frustum.ref: run occlusion tests
			// - is never automatically disabled
			// frustum.needsupd = gather occlusion data
			// - once an update has happened, it is disabled
			
			if (frustumRecalc)
			{
				// -= recalculate frustum =-
				
				// do as little as possible this frame
				frustum.needsupd = 1;
				// frustum was updated
				//frustum.updated = true;
				frustum.ref = true;
				// process columns & modify occlusion
				recalculateFrustum();
			}
			else
			{
				// not recalculating frustum
				
				// if last frame was an occlusion test
				if (frustum.ref)
				{
					// gather occlusion data
					frustum.needsupd = 2;
					// disable full refresh
					frustum.ref = false;
				}
				// if signal for refresh
				else if (frustum.needsupd == 1)
				{
					// run occlusion test
					frustum.ref = true;
					frustum.needsupd = 0;
				}
			}
			mtx.sectorseam.unlock();
		}
		
		// render queue needed update
		if (frustum.needsupd)
		{
			// compress rendering queue to minimal size by occlusion culling
			compressRenderingQueue();
		}
		
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		// render sky
		skyrenderer.render(*this, renderer.dtime);
		
		// copy sky to texture (skybuffer)
		textureman.bind(5, Textureman::T_SKYBUFFER);
		textureman.copyScreen(renderer.gamescr, Textureman::T_SKYBUFFER);
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		
		glEnable(GL_MULTISAMPLE_ARB);
		
		renderScene(renderer);
		
		glDisable(GL_MULTISAMPLE_ARB);
		
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		
		
		// render clouds
		skyrenderer.renderClouds(*this, renderer.frametick);
		
		// render particles
		particleSystem.render(snapWX, snapWZ);
		
	} // render scene
	
}
