#include "render_scene.hpp"

#include <library/log.hpp>
#include <library/opengl/fbo.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/texture_buffer.hpp>
#include "columns.hpp"
#include "drawq.hpp"
#include "camera.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "renderman.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "tiles.hpp"
#include "textureman.hpp"
#include "vertex_block.hpp"
#include <cmath>
#include <glm/gtx/transform.hpp>
#include <library/math/matrix.hpp>

using namespace library;

namespace cppcraft
{
	static const double PI = 4 * atan(1);
  static std::unique_ptr<BufferTexture> m_buffer_texture = nullptr;
  // setup the scene buffer texture array
  static std::unique_ptr<glm::vec3[]> m_bt_data = nullptr;

	void SceneRenderer::initTerrain()
	{
		// initialize column drawing queue
		drawq.init();

    // column translation array
    m_bt_data = std::make_unique<glm::vec3[]> (columns.size());
    m_buffer_texture.reset(new BufferTexture(0, GL_RGB32F));
	}

	void SceneRenderer::recalculateFrustum()
	{
		glm::vec3 look = player.getLookVector();

		// recalculate view & mvp matrix
		// view matrix (rotation + translation)
		camera.setTranslation(-playerPos.x, -playerPos.y, -playerPos.z);

		recalculateFrustum(camera, drawq, look);

		if (gameconf.reflections)
		{
			// set reflection camera view
			glm::mat4 matref = camera.getViewMatrix();
			matref *= glm::translate(glm::vec3(0.f, WATERLEVEL * 2.0f, 0.f));
			matref *= glm::scale(glm::vec3(1.0f, -1.0f, 1.0f));

			reflectionCamera.setRotationMatrix(extractRotation(matref));
			reflectionCamera.setViewMatrix(matref);

			if (gameconf.reflectTerrain)
			{
				look.y = -look.y;
				recalculateFrustum(reflectionCamera, reflectionq, look);
			}
		}
	}

	void SceneRenderer::recalculateFrustum(cppcraft::Camera& camera, DrawQueue& drawq, const glm::vec3& look)
	{
		// recalculate camera frustum
		camera.calculateFrustum();

		static const int safety_border = 2;
		#define VISIBILITY_BORDER  DrawQueue::VISIBILITY_BORDER

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

			if (x0 < VISIBILITY_BORDER)
				x0 = VISIBILITY_BORDER;
			if (x1 >= sectors.getXZ() - VISIBILITY_BORDER)
				x1 = sectors.getXZ()-1-VISIBILITY_BORDER;
		}
		else
		{
			x1 = playerSectorX - camera.cameraViewSectors;

			if (look.x < -half_fov)
				x0 = playerSectorX + safety_border;
			else
				x0 = playerSectorX + camera.cameraViewSectors;

			if (x1 < VISIBILITY_BORDER)
				x1 = VISIBILITY_BORDER;
			if (x0 >= sectors.getXZ()-VISIBILITY_BORDER)
				x0 = sectors.getXZ()-1-VISIBILITY_BORDER;

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

			if (z0 < VISIBILITY_BORDER)
				z0 = VISIBILITY_BORDER;
			if (z1 >= sectors.getXZ()-VISIBILITY_BORDER)
				z1 = sectors.getXZ()-1-VISIBILITY_BORDER;
		}
		else
		{
			z1 = playerSectorZ - camera.cameraViewSectors;

			if (look.z < -half_fov)
				z0 = playerSectorZ + safety_border;
			else
				z0 = playerSectorZ + camera.cameraViewSectors;

			if (z1 < VISIBILITY_BORDER)
				z1 = VISIBILITY_BORDER;
			if (z0 >= sectors.getXZ()-VISIBILITY_BORDER)
				z0 = sectors.getXZ()-1-VISIBILITY_BORDER;

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
		rg.playerY  = playerPos.y;
    rg.wdx = this->snap_delta_x();
    rg.wdz = this->snap_delta_z();
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

		// update and compress the draw queue
		// by counting visible entries for each shader line, and re-adding as we go
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
      size_t items = 0;

			// loop through this shader line
			for (auto* cv : drawq[i])
			{
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
          drawq[i][items++] = cv;
          // if the column is still rising up, let it rise
          if (cv->pos.y < 0.0) {
            cv->pos.y += 0.25f * renderer.delta_time();
            if (cv->pos.y > 0.0f) cv->pos.y = 0.0f;
          }
          // set position for column
          m_bt_data.get()[cv->index()] = cv->pos;
				}
			}
      drawq[i].resize(items);

		} // next shaderline

    // upload array of vec3
    m_buffer_texture->upload(m_bt_data.get(), columns.size() * 3 * sizeof(float));

	} // sort render queue

	void SceneRenderer::renderColumn(Column* cv, int i)
	{
		glBindVertexArray(cv->vao);
		//glDrawElements(GL_TRIANGLES, cv->indices[i], GL_UNSIGNED_SHORT, (GLvoid*) (intptr_t) cv->indexoffset[i]);
		glDrawArrays(GL_QUADS, cv->bufferoffset[i], cv->vertices[i]);
	}

	void SceneRenderer::renderColumnSet(int i)
	{
		if (camera.needsupd)
		{
			// render and count visible samples
      for(auto* cv : drawq[i])
			{
				switch (cv->occluded[i]) {
				case 0:
					// start counting samples passed
					glBeginQuery(GL_ANY_SAMPLES_PASSED, cv->occlusion[i]);

					renderColumn(cv, i);

					// end counting
					glEndQuery(GL_ANY_SAMPLES_PASSED);
					// set this as having been sampled
					cv->occluded[i] = 1;
					break;
				case 1:
				case 2:
					renderColumn(cv, i);
					break;
				default: //case 3:
					cv->occluded[i] = 0;
				}
			}
		}
		else
		{
			// direct render
			for (auto* column : drawq[i])
			{
				renderColumn(column, i);
			}
		}
	}

	void handleSceneUniforms(
			double frameCounter,
			Shader& shd,
			GLint& location,
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

		// common stuff
		shd.sendVec3 ("lightVector", thesun.getRealtimeAngle());
		shd.sendFloat("daylight",    thesun.getRealtimeDaylight());
		shd.sendFloat("frameCounter", frameCounter);

		shd.sendFloat("modulation", 1.0f); //torchlight.getModulation(frameCounter));

    // texrange, because too lazy to create all shaders
    location = shd.getUniform("texrange");
	}

	void SceneRenderer::renderScene(cppcraft::Camera& renderCam)
	{
		GLint location;

		textureman.bind(2, Textureman::T_SKYBOX);
    // translation buffer texture
    m_buffer_texture->bind(8);

		// bind standard shader
		handleSceneUniforms(renderer.time(),
							shaderman[Shaderman::STD_BLOCKS],
							location, renderCam);

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
			switch (i) {
			case RenderConst::TX_REPEAT: // repeatable solids (most terrain)

				// big repeatable textures
        tiledb.bigtiles.diff_texture().bind(0);
        tiledb.bigtiles.tone_texture().bind(1);
				break;

			case RenderConst::TX_SOLID: // solid stuff (most blocks)

				// change to small, repeatable textures
				tiledb.tiles.diff_texture().bind(0);
        tiledb.tiles.diff_texture().setWrapMode(GL_REPEAT);
        tiledb.tiles.tone_texture().bind(1);
        tiledb.tiles.tone_texture().setWrapMode(GL_REPEAT);
				break;

			case RenderConst::TX_TRANS: // see-through (tree leafs etc.)

				// disable face culling for 2-sidedness
				glDisable(GL_CULL_FACE);

				// change shader-set
				handleSceneUniforms(renderer.time(),
									shaderman[Shaderman::CULLED_BLOCKS],
									location, renderCam);
				break;

			case RenderConst::TX_2SIDED: // 2-sided faces (torches, vines etc.)

				// change to small, clamped textures
        tiledb.tiles.diff_texture().setWrapMode(GL_CLAMP_TO_EDGE);
				glActiveTexture(GL_TEXTURE0);
        tiledb.tiles.diff_texture().setWrapMode(GL_CLAMP_TO_EDGE);

				// change shader-set
				handleSceneUniforms(renderer.time(),
									shaderman[Shaderman::ALPHA_BLOCKS],
									location, renderCam);

				// safe to increase step from this -->
				if (drawq[i].size() == 0) continue;
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
			renderColumnSet(i);

		} // next shaderline
	}

	static void renderReflectedColumn(Column* cv, int i)
	{
		glBindVertexArray(cv->vao);
		//glDrawElements(GL_TRIANGLES, cv->indices[i], GL_UNSIGNED_SHORT, (GLvoid*) (intptr_t) cv->indexoffset[i]);
		glDrawArrays(GL_QUADS, cv->bufferoffset[i], cv->vertices[i]);
	} // renderReflectedColumn()

	void SceneRenderer::renderReflectedScene(cppcraft::Camera& renderCam)
	{
		GLint location;

		// bind standard shader
		handleSceneUniforms(renderer.time(),
							shaderman[Shaderman::BLOCKS_REFLECT],
							location, renderCam);

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
        tiledb.bigtiles.diff_texture().bind(0);
        tiledb.bigtiles.tone_texture().bind(1);
				break;

			case RenderConst::TX_SOLID: // solid stuff (most blocks)

				// change to normal tile textures
        tiledb.tiles.diff_texture().bind(0);
        tiledb.tiles.tone_texture().bind(1);
				break;

			case RenderConst::TX_2SIDED: // 2-sided faces (torches, vines etc.)

				// disable face culling (for 2-sidedness)
				glDisable(GL_CULL_FACE);
				break;

			}

			// direct render
      for (auto* cv : reflectionq[i])
			{
				renderReflectedColumn(cv, i);
			}
		} // next shaderline

	} // renderReflectedScene()

	void SceneRenderer::renderSceneWater()
	{
		GLint location;
    // translation buffer texture
    m_buffer_texture->bind(8);

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
			handleSceneUniforms(renderer.time(),
								shaderman[Shaderman::BLOCKS_DEPTH],
								location, camera);
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
			// fluid shaders
			for (int i = RenderConst::TX_WATER; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			{
				switch (i)
				{
				case RenderConst::TX_WATER:

					// safe to increase step from this -->
					if (drawq[i].size() == 0) continue;
					// <-- safe to increase step from this

					// water shader-set
					handleSceneUniforms(renderer.time(),
										shaderman[Shaderman::BLOCKS_WATER],
										location, camera);
					// sun view angle
					shaderman[Shaderman::BLOCKS_WATER].sendVec3 ("v_ldir", thesun.getRealtimeViewAngle());
					// update world offset
					if (camera.ref)
						shaderman[Shaderman::BLOCKS_WATER].sendVec3("worldOffset", camera.getWorldOffset());
					break;

				case RenderConst::TX_LAVA:

					// safe to increase step from this -->
					if (drawq[i].size() == 0) continue;
					// <-- safe to increase step from this

					textureman.bind(2, Textureman::T_MAGMA);
					// lava shader-set
					handleSceneUniforms(renderer.time(),
										shaderman[Shaderman::BLOCKS_LAVA],
										location, camera);
					// update world offset
					if (camera.ref)
						shaderman[Shaderman::BLOCKS_LAVA].sendVec3("worldOffset", camera.getWorldOffset());
					break;
				}

				// render it all
				renderColumnSet(i);

			} // each shader

		} // underwater
		else
		{
			if (this->isUnderwater() == 1)
			{
				// render water meshes
				renderColumnSet(RenderConst::TX_WATER);
			}
			else
			{
				// render lava meshes
				renderColumnSet(RenderConst::TX_LAVA);
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
