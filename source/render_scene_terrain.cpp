#include "render_scene.hpp"

#include "library/log.hpp"
#include "library/opengl/opengl.hpp"
#include "columns.hpp"
#include "drawq.hpp"
#include "frustum.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "rendergrid.hpp"
#include "renderman.hpp"
#include "render_player_selection.hpp"
#include "sectors.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "textureman.hpp"
#include "torchlight.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	void SceneRenderer::initTerrain()
	{
		// initialize column drawing queue
		DrawQueue::init();
	}
	
	void SceneRenderer::recalculateFrustum()
	{
		// recalculate view & mvp matrix
		// view matrix (rotation + translation)
		frustum.setTranslation(-playerX, -playerY, -playerZ);
		
		// recalculate camera frustum
		frustum.calculateFrustum();
		
		vec3 look = player.getLookVector();
		
		const int safety_border = 1;
		const int visibility_border = RenderGrid::visibility_border;
		
		const float half_fov = 0.65; // sin(30 * degToRad) = +/- 0.5
		
		// major direction scheme
		int xstp = 1;
		int x0, x1;
		if (look.x >= 0)
		{
			if (look.x > half_fov)
				x0 = playerSectorX - safety_border;
			else
				x0 = playerSectorX - frustum.cameraViewSectors;
			
			x1 = x0 + frustum.cameraViewSectors * 2;
			
			if (x0 < visibility_border)
				x0 = visibility_border;
			if (x1 >= Sectors.getXZ() - visibility_border)
				x1 = Sectors.getXZ()-1-visibility_border;
		}
		else
		{
			x1 = playerSectorX - frustum.cameraViewSectors;
			
			if (look.x < -half_fov)
				x0 = playerSectorX + safety_border;
			else
				x0 = x1 + frustum.cameraViewSectors * 2;
			
			if (x1 < visibility_border)
				x1 = visibility_border;
			if (x0 >= Sectors.getXZ()-visibility_border)
				x0 = Sectors.getXZ()-1-visibility_border;
			
			xstp = -1;
		}
		
		int zstp = 1;
		int z0, z1;
		if (look.z >= 0)
		{
			if (look.z > half_fov)
				z0 = playerSectorZ - safety_border;
			else
				z0 = playerSectorZ - frustum.cameraViewSectors;
			
			z1 = z0 + frustum.cameraViewSectors * 2;
			
			if (z0 < visibility_border)
				z0 = visibility_border;
			if (z1 >= Sectors.getXZ()-visibility_border)
				z1 = Sectors.getXZ()-1-visibility_border;
		}
		else
		{
			z1 = playerSectorZ - frustum.cameraViewSectors;
			
			if (look.z < -half_fov)
				z0 = playerSectorZ + safety_border;
			else
				z0 = z1 + frustum.cameraViewSectors * 2;
			
			if (z1 < visibility_border)
				z1 = visibility_border;
			if (z0 >= Sectors.getXZ()-visibility_border)
				z0 = Sectors.getXZ()-1-visibility_border;
			
			zstp = -1;
		}
		
		int ystp = (look.y < 0.0) ? -1 : 1;
		
		int majority = 0;
		
		// determine iteration direction
		if (fabsf(look.x) > fabsf(look.z))
		{
			if (look.x >= 0)
				majority = 0; // +x
			else
				majority = 1; // -x
		}
		else if (look.z >= 0)
			majority = 4; // +z
		else
			majority = 5; // -z
		
		// reset drawqing queue
		DrawQueue::reset();
		
		RenderGrid::rendergrid_t rv;
		rv.xstp = xstp;
		rv.ystp = ystp;
		rv.zstp = zstp;
		rv.majority = majority;
		
		// start at roomsize 1, avoiding "everything"
		RenderGrid::uniformGrid(rv, x0, x1, z0, z1, 1);
	}
	
	void SceneRenderer::compressRenderingQueue()
	{
		// reset after 2nd run
		if (frustum.needsupd == 2) frustum.needsupd = 0;
		
		// update and compress the draw queue
		// by counting visible entries for each shader line, and re-adding as we go
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			int count = drawq[i].count();
			// clear, setting count to 0 internally
			drawq[i].clear();
			
			// loop through this shader line
			for (int j = 0; j < count; j++)
			{
				Column* cv = drawq[i].get(j);
				
				// get/set occlusion status
				if (cv->occluded[i] == 1)
				{
					GLuint occlusion_result;
					glGetQueryObjectuiv(cv->occlusion[i], GL_QUERY_RESULT_AVAILABLE, &occlusion_result);
					
					if (occlusion_result)
					{
						// get result immediately
						glGetQueryObjectuiv(cv->occlusion[i], GL_QUERY_RESULT, &occlusion_result);
						
						if (occlusion_result)
						{
							// add if any samples passed
							cv->occluded[i] = 0;
						}
						else cv->occluded[i] = 2;
					}
					else frustum.needsupd = 2; // we need to update again :(
				}
				else if (cv->occluded[i] == 3)
				{
					// uncontested from last frame --> 1
					cv->occluded[i] = 1;
				}
				
				// finally, as long as not completely occluded/discarded
				if (cv->occluded[i] != 2)
				{
					// add to new position, effectively compressing
					// and linearizing queue interally
					drawq[i].add(cv);
				}
			}
			
		} // next shaderline
		
	} // sort render queue
	
	void renderColumn(Column* cv, int i, vec3& position, GLint loc_vtrans, double dtime)
	{
		if (position.x != cv->pos.x || 
			position.y != cv->pos.y || 
			position.z != cv->pos.z)
		{
			// translate to new position
			glUniform3fv(loc_vtrans, 1, &cv->pos.x);
			
			// remember position
			position = cv->pos;
			
			// cool effect
			if (cv->pos.y < 0.0)
			{
				cv->pos.y += 0.125 * dtime;
				if (cv->pos.y > 0.0) cv->pos.y = 0.0;
			}
		}
		glBindVertexArray(cv->vao);
		glDrawArrays(GL_QUADS, cv->bufferoffset[i], cv->vertices[i]);
	}
	
	void handleSceneUniforms(
			double frameCounter, 
			Shader& shd, 
			GLint& location, 
			GLint& loc_vtrans, 
			vec3& position
		)
	{
		// bind appropriate shader
		shd.bind();
		
		// frustum updates
		if (frustum.ref)
		{
			// modelview matrix
			shd.sendMatrix("matview", frustum.getViewMatrix());
		}
		
		// translation
		loc_vtrans = shd.getUniform("vtrans");
		position = vec3(-1);
		
		// common stuff
		shd.sendVec3 ("lightVector", thesun.getRealtimeAngle());
		shd.sendFloat("daylight",    thesun.getRealtimeDaylight());
		shd.sendFloat("frameCounter", frameCounter);
		
		shd.sendFloat("modulation", torchlight.getModulation(frameCounter));
		
		// texrange
		location = shd.getUniform("texrange");
	}
	
	void SceneRenderer::renderScene(Renderer& renderer)
	{
		GLint loc_vtrans, location;
		vec3  position(-1);
		
		// skybuffer at texture slot 5
		//textureman.bind(5, Textureman::T_SKYBUFFER);
		
		// bind skybox at slot 4
		textureman.bind(4, Textureman::T_SKYBOX);
		
		// bind standard shader
		handleSceneUniforms(renderer.frametick, 
							shaderman[Shaderman::STD_BLOCKS], 
							location, 
							loc_vtrans, 
							position);
		// check for errors
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::renderScene(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderScene(): OpenGL state error");
		}
		
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			switch (i)
			{
			case RenderConst::TX_REPEAT: // repeatable solids (most terrain)
				
				// enable face culling
				glEnable(GL_CULL_FACE);
				// change to repeatable textures
				textureman.bind(0, Textureman::T_BIG_DIFF);
				textureman.bind(1, Textureman::T_BIG_TONE);
				break;
				
			case RenderConst::TX_SOLID: // solid stuff (most blocks)
				
				// change to clamped textures
				textureman.bind(0, Textureman::T_DIFFUSE);
				textureman.bind(1, Textureman::T_TONEMAP);
				break;
				
			case RenderConst::TX_TRANS: // culled alpha (tree leafs etc.)
				
				// change shader-set
				handleSceneUniforms(renderer.frametick, 
									shaderman[Shaderman::CULLED_BLOCKS], 
									location, 
									loc_vtrans, 
									position);
				break;
				
			case RenderConst::TX_2SIDED: // 2-sided faces (torches, vines etc.)
				
				// disable face culling (for 2-sidedness)
				glDisable(GL_CULL_FACE);
				
				// change shader-set
				handleSceneUniforms(renderer.frametick, 
									shaderman[Shaderman::ALPHA_BLOCKS], 
									location, 
									loc_vtrans, 
									position);
				// safe to increase step from this -->
				if (drawq[i].count() == 0)
				{
					i += 1;
					break;
				}
				// <-- safe to increase step from this
				
				// set texrange
				glUniform1i(location, i);
				break;
				
			case RenderConst::TX_CROSS:
				// set new texrange
				glUniform1i(location, i);
				
				break;
			case RenderConst::TX_WATER:
				
				// render player selection
				renderPlayerSelection();
				
				if (plogic.FullySubmerged == plogic.PS_None)
					glEnable(GL_CULL_FACE);
				
				position = vec3(-1);
				
				// copy screen as underwatermap
				textureman.bind(3, Textureman::T_UNDERWATERMAP);
				textureman.copyScreen(renderer.gamescr, Textureman::T_UNDERWATERMAP);
				
				// change shader-set
				handleSceneUniforms(renderer.frametick, 
									shaderman[Shaderman::BLOCKS_WATER], 
									location, 
									loc_vtrans, 
									position);
				// update world offset
				shaderman[Shaderman::BLOCKS_WATER].sendVec3("worldOffset", frustum.getWorldOffset());
				break;
			}
			
			if (frustum.ref)
			{
				// render and count visible samples
				for (int j = 0; j < drawq[i].count(); j++)
				{
					Column* cv = drawq[i].get(j);
					
					// start counting samples passed
					glBeginQuery(GL_ANY_SAMPLES_PASSED, cv->occlusion[i]);
					
					renderColumn(cv, i, position, loc_vtrans, renderer.dtime);
					
					// end counting
					glEndQuery(GL_ANY_SAMPLES_PASSED);
				}
			}
			else
			{
				// direct render
				for (int j = 0; j < drawq[i].count(); j++)
				{
					Column* cv = drawq[i].get(j);
					renderColumn(cv, i, position, loc_vtrans, renderer.dtime);
				}
			}
			
		} // next shaderline
		
	}
}
