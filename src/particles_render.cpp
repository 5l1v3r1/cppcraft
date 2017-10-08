#include "particles.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include "camera.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "sector.hpp"
#include "sun.hpp"
#include "world.hpp"
#include <glm/gtx/transform.hpp>

using namespace library;

namespace cppcraft
{
	void Particles::renderUpdate()
	{
		this->mtx.lock();

		// set snapshots
		snapRenderCount = renderCount;

		if (snapRenderCount == 0)
		{
			this->mtx.unlock();
			return;
		}
		snapWX = currentWX;
		snapWZ = currentWZ;

		if (particleSystem.updated)
		{
			particleSystem.updated = false;

			bool updateAttribs = false;
			if (vao == 0)
			{
				glGenVertexArrays(1, &vao);
				glGenBuffers(1, &vbo);
				updateAttribs = true;
			}
			glBindVertexArray(vao);

			// particles vbo
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, renderCount * sizeof(particle_vertex_t), vertices, GL_STREAM_DRAW);

			if (updateAttribs)
			{
				for (int i = 0; i < 4; i++)
					glEnableVertexAttribArray(i);

				// position
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle_vertex_t), 0);
				// unnormalized 8-bit channels
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(particle_vertex_t), (GLvoid*) 12);
				// normalized 8-bit channels (alpha, brightness, offset X, offset Y)
				glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(particle_vertex_t), (GLvoid*) 16);
				// color
				glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(particle_vertex_t), (GLvoid*) 20);
			}
		}
		this->mtx.unlock();
	}

	void Particles::render(int snapWX, int snapWZ, double timeElapsed)
	{
		// start particles shader
		// BEFORE we potentially exit because there are no particles to render
		// this is done to get the first-frame update.. considering there's
		// always particles to render, we'll just go ahead and always bind the shader

		Shader& shd = shaderman[Shaderman::PARTICLE];
		shd.bind();
		shd.sendFloat("timeElapsed", timeElapsed);

		if (camera.ref)
		{
			float tx = (this->snapWX - snapWX) * Sector::BLOCKS_XZ;
			float tz = (this->snapWZ - snapWZ) * Sector::BLOCKS_XZ;

			glm::mat4 matview = camera.getViewMatrix();
			matview *= glm::translate(glm::vec3(tx, 0.0f, tz));

			// send view matrix
			shd.sendMatrix("matview", matview);
		}

		if (vao == 0 || snapRenderCount == 0) return;

		glEnable(GL_POINT_SPRITE);

		// bind 2d array of textures/tiles
		textureman.bind(0, Textureman::T_PARTICLES);

		// bind and render particles
		glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, snapRenderCount);

		glDisable(GL_POINT_SPRITE);
	}

}
