#include "render_scene.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/fbo.hpp>
#include "columns.hpp"
#include "drawq.hpp"
#include "camera.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "renderman.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "textureman.hpp"
#include "torchlight.hpp"
#include "vertex_block.hpp"
#include <cmath>
#define DEBUG

using namespace library;

namespace cppcraft
{
	static const double PI = 4 * atan(1);
	GLuint renderVAO;
	
	void SceneRenderer::initTerrain()
	{
		// initialize flatland 3d textures
		//flatlands.initTextures();
		
		// initialize column drawing queue
		drawq.init();
		
		glGenVertexArrays(1, &renderVAO);
	}
	
	void SceneRenderer::recalculateFrustum()
	{
		vec3 look = player.getLookVector();
		
		// recalculate view & mvp matrix
		// view matrix (rotation + translation)
		camera.setTranslation(-playerX, -playerY, -playerZ);
		
		recalculateFrustum(camera, drawq, look);
		
		if (gameconf.reflections)
		{
			// set reflection camera view
			mat4 matref = camera.getViewMatrix();
			matref.translate(0, RenderConst::WATER_LEVEL * 2, 0);
			matref *= mat4(1.0, -1.0, 1.0);
			
			reflectionCamera.setRotationMatrix(matref.rotation());
			reflectionCamera.setViewMatrix(matref);
			
			if (gameconf.reflectTerrain)
			{
				look.y = -look.y;
				recalculateFrustum(reflectionCamera, reflectionq, look);
			}
		}
	}
	
	void SceneRenderer::recalculateFrustum(cppcraft::Camera& camera, DrawQueue& drawq, const vec3& look)
	{
		// recalculate camera frustum
		camera.calculateFrustum();
		
		static const int safety_border = 2;
		#define visibility_border  DrawQueue::visibility_border
		
		static const float half_fov = 0.65; // sin(30 * degToRad) = +/- 0.5
		
		// major direction scheme
		int xstp = 1;
		int x0, x1;
		if (look.x >= 0.0f)
		{
			if (look.x > half_fov)
				x0 = playerSectorX - safety_border;
			else
				x0 = playerSectorX - camera.cameraViewSectors;
			
			x1 = playerSectorX + camera.cameraViewSectors;
			
			if (x0 < visibility_border)
				x0 = visibility_border;
			if (x1 >= sectors.getXZ() - visibility_border)
				x1 = sectors.getXZ()-1-visibility_border;
		}
		else
		{
			x1 = playerSectorX - camera.cameraViewSectors;
			
			if (look.x < -half_fov)
				x0 = playerSectorX + safety_border;
			else
				x0 = playerSectorX + camera.cameraViewSectors;
			
			if (x1 < visibility_border)
				x1 = visibility_border;
			if (x0 >= sectors.getXZ()-visibility_border)
				x0 = sectors.getXZ()-1-visibility_border;
			
			xstp = -1;
		}
		
		int zstp = 1;
		int z0, z1;
		if (look.z >= 0.0f)
		{
			if (look.z > half_fov)
				z0 = playerSectorZ - safety_border;
			else
				z0 = playerSectorZ - camera.cameraViewSectors;
			
			z1 = playerSectorZ + camera.cameraViewSectors;
			
			if (z0 < visibility_border)
				z0 = visibility_border;
			if (z1 >= sectors.getXZ()-visibility_border)
				z1 = sectors.getXZ()-1-visibility_border;
		}
		else
		{
			z1 = playerSectorZ - camera.cameraViewSectors;
			
			if (look.z < -half_fov)
				z0 = playerSectorZ + safety_border;
			else
				z0 = playerSectorZ + camera.cameraViewSectors;
			
			if (z1 < visibility_border)
				z1 = visibility_border;
			if (z0 >= sectors.getXZ()-visibility_border)
				z0 = sectors.getXZ()-1-visibility_border;
			
			zstp = -1;
		}
		
		int ystp = (look.y < 0.0f) ? -1 : 1;
		
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
			majority = 2; // +z
		else
			majority = 3; // -z
		
		// reset drawqing queue
		drawq.reset();
		
		// set frustum culling settings
		DrawQueue::rendergrid_t rg;
		rg.xstp = xstp;
		rg.ystp = ystp;
		rg.zstp = zstp;
		rg.majority = majority;
		rg.playerY  = playerY;
		rg.frustum = &camera.getFrustum();
		rg.gridSize = camera.getGridsize();
		
		// start at roomsize 1, avoiding "everything"
		drawq.uniformGrid(rg, x0, x1, z0, z1, 1);
	}
	
	void SceneRenderer::compressRenderingQueue()
	{
		if (camera.needsupd == 0) return;
		// reset after 2nd run
		else if (camera.needsupd == 2) camera.needsupd = 0;
		
		Column* cv;    // queue: shaderline --> mesh index
		GLuint  occlusion_result;
		
		// update and compress the draw queue
		// by counting visible entries for each shader line, and re-adding as we go
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			// number of columns in each draw queue
			int count = drawq[i].count();
			// clear, which does only 1 thing: set count = 0
			drawq[i].clear();
			
			// loop through this shader line
			for (int j = 0; j < count; j++)
			{
				cv = drawq[i].get(j);
				
				// get/set occlusion status
				if (cv->occluded[i] == 1)
				{
					glGetQueryObjectuiv(cv->occlusion[i], GL_QUERY_RESULT_AVAILABLE, &occlusion_result);
					
					if (occlusion_result)
					{
						// get result immediately
						glGetQueryObjectuiv(cv->occlusion[i], GL_QUERY_RESULT, &occlusion_result);
						
						if (occlusion_result)
						{
							// add since there was at least 1 sample visible
							cv->occluded[i] = 2;
						}
						else cv->occluded[i] = 4;
					}
					else
					{
						// we need to update again :(
						camera.needsupd = 2;
					}
				}
				
				// finally, as long as not completely occluded/discarded
				if (cv->occluded[i] != 4)
				{
					// add to new position, effectively compressing
					// and linearizing queue internally
					drawq[i].add(cv);
				}
				
			}
			
		} // next shaderline
		
	} // sort render queue
	
	void renderColumn(Column* cv, int i, vec3& position, GLint loc_vtrans)
	{
		// make sure we don't resend same position again
		// around 10k+ skips per second with axis=64
		if (position.x != cv->pos.x || 
			position.y != cv->pos.y || 
			position.z != cv->pos.z)
		{
			// set new position
			position = cv->pos;
			// send new position to shader
			glUniform3fv(loc_vtrans, 1, &position.x);
			
			// cool effect
			if (cv->pos.y < 0.0f)
			{
				cv->pos.y += 0.5f;
				if (cv->pos.y > 0.0f) cv->pos.y = 0.0f;
			}
		}
		glBindVertexArray(cv->vao);
		//glDrawElements(GL_TRIANGLES, cv->indices[i], GL_UNSIGNED_SHORT, (GLvoid*) (intptr_t) cv->indexoffset[i]);
		glDrawArrays(GL_QUADS, cv->bufferoffset[i], cv->vertices[i]);
	}
	
	void renderColumnSet(int i, vec3& position, GLint loc_vtrans)
	{
		if (camera.needsupd)
		{
			// render and count visible samples
			for (int j = 0; j < drawq[i].count(); j++)
			{
				Column* cv = drawq[i].get(j);
				
				switch (cv->occluded[i])
				{
				case 0:
					// start counting samples passed
					glBeginQuery(GL_ANY_SAMPLES_PASSED, cv->occlusion[i]);
					
					renderColumn(cv, i, position, loc_vtrans);
					
					// end counting
					glEndQuery(GL_ANY_SAMPLES_PASSED);
					// set this as having been sampled
					cv->occluded[i] = 1;
					break;
				case 1:
				case 2:
					renderColumn(cv, i, position, loc_vtrans);
					break;
				default: //case 3:
					cv->occluded[i] = 0;
				}
			}
		}
		else
		{
			// direct render
			for (int j = 0; j < drawq[i].count(); j++)
			{
				Column* cv = drawq[i].get(j);
				renderColumn(cv, i, position, loc_vtrans);
			}
		}
	}
	
	void handleSceneUniforms(
			double frameCounter, 
			Shader& shd, 
			GLint& location, 
			GLint& loc_vtrans, 
			vec3& position,
			cppcraft::Camera& camera
		)
	{
		// bind appropriate shader
		shd.bind();
		
		// camera updates
		if (camera.ref)
		{
			// modelview matrix
			shd.sendMatrix("matview", camera.getViewMatrix());
			// mvp matrix
			shd.sendMatrix("matmvp", camera.getMVP());
		}
		
		/*static int lastref = 9;
		
		if (lastref != camera.ref)
		{
			lastref = camera.ref;
			logger << Log::INFO << camera.ref << Log::ENDL;
		}*/
		
		// common stuff
		shd.sendVec3 ("lightVector", thesun.getRealtimeAngle());
		shd.sendFloat("daylight",    thesun.getRealtimeDaylight());
		shd.sendFloat("frameCounter", frameCounter);
		
		shd.sendFloat("modulation", torchlight.getModulation(frameCounter));
		
		// translation uniform location
		loc_vtrans = shd.getUniform("vtrans");
		position.x = -1.0f; // invalidate position
		
		// texrange, because too lazy to create all shaders
		location = shd.getUniform("texrange");
	}
	
	void SceneRenderer::renderScene(Renderer& renderer, cppcraft::Camera& renderCam)
	{
		GLint loc_vtrans, location;
		vec3 position(-1.0f);
		
		textureman.bind(2, Textureman::T_SKYBOX);
		
		// bind standard shader
		handleSceneUniforms(renderer.frametick, 
							shaderman[Shaderman::STD_BLOCKS], 
							location, 
							loc_vtrans, 
							position, renderCam);
		
		// check for errors
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::renderScene(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderScene(): OpenGL state error");
		}
		#endif
		
		// render all nonwater shaders
		
		for (int i = 0; i < (int) RenderConst::TX_WATER; i++)
		{
			switch (i)
			{
			case RenderConst::TX_REPEAT: // repeatable solids (most terrain)
				
				// big repeatable textures
				textureman.bind(0, Textureman::T_BIG_DIFF);
				textureman.bind(1, Textureman::T_BIG_TONE);
				break;
				
			case RenderConst::TX_SOLID: // solid stuff (most blocks)
				
				// change to small, repeatable textures
				textureman.bind(0, Textureman::T_DIFFUSE);
				textureman.bind(1, Textureman::T_TONEMAP);
				break;
				
			case RenderConst::TX_TRANS: // culled alpha (tree leafs etc.)
				
				// disable face culling (for 2-sidedness)
				glDisable(GL_CULL_FACE);
				
				// change shader-set
				handleSceneUniforms(renderer.frametick, 
									shaderman[Shaderman::CULLED_BLOCKS], 
									location, 
									loc_vtrans, 
									position, renderCam);
				break;
				
			case RenderConst::TX_2SIDED: // 2-sided faces (torches, vines etc.)
				
				// change to small, clamped textures
				textureman[Textureman::T_TONEMAP].setWrapMode(GL_CLAMP_TO_EDGE);
				glActiveTexture(GL_TEXTURE0);
				textureman[Textureman::T_DIFFUSE].setWrapMode(GL_CLAMP_TO_EDGE);
				
				// change shader-set
				handleSceneUniforms(renderer.frametick, 
									shaderman[Shaderman::ALPHA_BLOCKS], 
									location, 
									loc_vtrans, 
									position, renderCam);
				
				// safe to increase step from this -->
				if (drawq[i].count() == 0) continue;
				// <-- safe to increase step from this
				
				// set texrange
				glUniform1i(location, i);
				break;
				
			case RenderConst::TX_CROSS:
				
				// set new texrange
				glUniform1i(location, i);
				break;
			}
			
			// render it all
			renderColumnSet(i, position, loc_vtrans);
			
		} // next shaderline
		
		// change to repeating textures
		textureman[Textureman::T_DIFFUSE].setWrapMode(GL_REPEAT);
		glActiveTexture(GL_TEXTURE1);
		textureman[Textureman::T_DIFFUSE].setWrapMode(GL_REPEAT);
	}
	
	void renderReflectedColumn(Column* cv, int i, vec3& position, GLint loc_vtrans)
	{
		if (position.x != cv->pos.x || 
			position.z != cv->pos.z)
		{
			// remember position
			position = vec3(cv->pos.x, 0.0f, cv->pos.z);
			// translate to new position
			glUniform3fv(loc_vtrans, 1, &position.x);
		}
		glBindVertexArray(cv->vao);
		//glDrawElements(GL_TRIANGLES, cv->indices[i], GL_UNSIGNED_SHORT, (GLvoid*) (intptr_t) cv->indexoffset[i]);
		glDrawArrays(GL_QUADS, cv->bufferoffset[i], cv->vertices[i]);
	} // renderReflectedColumn()
	
	void SceneRenderer::renderReflectedScene(Renderer& renderer, cppcraft::Camera& renderCam)
	{
		GLint loc_vtrans, location;
		vec3  position(-1);
		
		// bind standard shader
		handleSceneUniforms(renderer.frametick, 
							shaderman[Shaderman::BLOCKS_REFLECT], 
							location,
							loc_vtrans,
							position, renderCam);
		
		// check for errors
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::renderReflectedScene(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderReflectedScene(): OpenGL state error");
		}
		#endif
		
		// render everything above water plane
		
		for (int i = 0; i < (int) RenderConst::TX_WATER; i++)
		{
			switch (i)
			{
			case RenderConst::TX_REPEAT: // repeatable solids (most terrain)
				
				// change to big tile textures
				textureman.bind(0, Textureman::T_BIG_DIFF);
				textureman.bind(1, Textureman::T_BIG_TONE);
				break;
				
			case RenderConst::TX_SOLID: // solid stuff (most blocks)
				
				// change to normal tile textures
				textureman.bind(0, Textureman::T_DIFFUSE);
				textureman.bind(1, Textureman::T_TONEMAP);
				break;
				
			case RenderConst::TX_2SIDED: // 2-sided faces (torches, vines etc.)
				
				// disable face culling (for 2-sidedness)
				glDisable(GL_CULL_FACE);
				break;
				
			}
			
			// direct render
			for (int j = 0; j < reflectionq[i].count(); j++)
			{
				Column* cv = reflectionq[i].get(j);
				renderReflectedColumn(cv, i, position, loc_vtrans);
			}
		} // next shaderline
		
	} // renderReflectedScene()
	
	void SceneRenderer::renderSceneWater(Renderer& renderer)
	{
		GLint loc_vtrans, location;
		vec3  position(-1);
		
		// check for errors
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::renderSceneWater(): OpenGL error @ top. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderSceneWater(): OpenGL state error");
		}
		#endif
		
		// bind underwater scene
		textureman.bind(0, Textureman::T_UNDERWATERMAP);
		textureman.bind(1, Textureman::T_UNDERWDEPTH);
		
		if (this->isUnderwater())
		{
			// underwater shader-set
			handleSceneUniforms(renderer.frametick, 
								shaderman[Shaderman::BLOCKS_DEPTH], 
								location, 
								loc_vtrans, 
								position, camera);
			// cull only front water-faces inside water
			glCullFace(GL_FRONT);
		}
		else
		{
			if (gameconf.reflections)
			{
				// bind world-reflection
				textureman.bind(2, Textureman::T_REFLECTION);
			}
		}
		
		// check for errors
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::renderSceneWater(): OpenGL error @ middle. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderSceneWater(): OpenGL state error");
		}
		#endif
		
		// FIXME: need to render water running_water and lava separately instead of "accepting fully submerged"
		// as not rendering anything but depth values
		
		if (this->isUnderwater() == false)
		{
			// water shader
			for (int i = RenderConst::TX_WATER; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			{
				switch (i)
				{
				case RenderConst::TX_WATER:
					
					// safe to increase step from this -->
					if (drawq[i].count() == 0) continue;
					// <-- safe to increase step from this
					
					// water shader-set
					handleSceneUniforms(renderer.frametick, 
										shaderman[Shaderman::BLOCKS_WATER],
										location,
										loc_vtrans,
										position, camera);
					// sun view angle
					shaderman[Shaderman::BLOCKS_WATER].sendVec3 ("v_ldir", thesun.getRealtimeViewAngle());
					// update world offset
					if (camera.ref)
						shaderman[Shaderman::BLOCKS_WATER].sendVec3("worldOffset", camera.getWorldOffset());
					break;
					
				case RenderConst::TX_LAVA:
					
					// safe to increase step from this -->
					if (drawq[i].count() == 0) continue;
					// <-- safe to increase step from this
					
					textureman.bind(2, Textureman::T_MAGMA);
					// lava shader-set
					handleSceneUniforms(renderer.frametick, 
										shaderman[Shaderman::BLOCKS_LAVA],
										location,
										loc_vtrans,
										position, camera);
					// update world offset
					if (camera.ref)
						shaderman[Shaderman::BLOCKS_LAVA].sendVec3("worldOffset", camera.getWorldOffset());
					break;
				}
				
				// render it all
				renderColumnSet(i, position, loc_vtrans);
				
			} // each shader
			
		} // underwater
		else
		{
			if (this->isUnderwater() == 1)
			{
				// render water meshes
				renderColumnSet(RenderConst::TX_WATER, position, loc_vtrans);
			}
			else
			{
				// render lava meshes
				renderColumnSet(RenderConst::TX_LAVA, position, loc_vtrans);
			}
			// restore cullface setting
			glCullFace(GL_BACK);
		}
		
		// check for errors
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::renderSceneWater(): OpenGL error @ bottom. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderSceneWater(): OpenGL state error");
		}
		#endif
		
	} // renderSceneWater()
	
}
