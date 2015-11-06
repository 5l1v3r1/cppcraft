#include "render_gui.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include <library/math/toolbox.hpp>
#include <library/math/vector.hpp>
#include "blockmodels.hpp"
#include "camera.hpp"
#include "menu.hpp"
#include "player.hpp"
#include "player_actions.hpp"
#include "player_logic.hpp"
#include "sun.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "voxelmodels.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	static const double PI = 4 * atan(1);
	
	class PlayerHand
	{
		VAO vao;
		VAO cubeVAO;
		vec3 lastHand;
		double lastTime;
		int lastMode;
		
		struct phand_vertex_t
		{
			float x, y, z;
			float nx, ny, nz;
			float u, v, w;
		};
		
		phand_vertex_t vertices[2][4] = 
		{
			{ // top face
				//  xyz        norm        uvw
				{ 1, 1, 1,   0, 1, 0,   0, 1.5, 9 },
				{ 1, 1, 0,   0, 1, 0,   0, 0.0, 9 },
				{ 0, 1, 0,   0, 1, 0,   1, 0.0, 9 },
				{ 0, 1, 1,   0, 1, 0,   1, 1.5, 9 }
			},
			{ // left face
				{ 0, 1, 1,  -1, 0, 0,   0, 1.5, 9 },
				{ 0, 1, 0,  -1, 0, 0,   0, 0.0, 9 },
				{ 0, 0, 0,  -1, 0, 0,   1, 0.0, 9 },
				{ 0, 0, 1,  -1, 0, 0,   1, 1.5, 9 }
			}
		};
		
		// hand scale matrix
		mat4 handScale;
		
	public:
		PlayerHand();
		void render(double frameCounter);
		void renderItem();
		void renderHandItem(vec4& shadow, vec4& torch, float modulation);
	};
	PlayerHand playerHand;
	
	void GUIRenderer::renderPlayerhand(double frameCounter)
	{
		playerHand.render(frameCounter);
	}
	
	PlayerHand::PlayerHand()
	{
		lastMode = -1;
		lastTime = 0.0;
		handScale = mat4(0.4, 0.3, 2.0);
	}
	
	void PlayerHand::render(double frameCounter)
	{
		const double DEFAULT_INTERPOLATION = 0.75;
		
		// manipulate hand
		double period = (frameCounter - lastTime) * 0.25 / PI;
		vec3 hand(0.85, -0.75, -1.50);
		int mode = 0;
		
		if (paction.getAction() == PlayerActions::PA_Mineblock)
		{
			// mining animation
			hand.x -= fabs(sin(period * 1.5) * 0.5);
			hand.y += sin(period * 3) * 0.1;
			if (false) //helditem.count)
				hand.z = -1.25 - fabs(sin(period * 1.5)) * 1.25;
			else
				hand.z = -1.25 - fabs(sin(period * 1.5)) * 1.5;
			
			mode = 1;
		}
		else if (paction.getAction() == PlayerActions::PA_Swingtool)
		{
			// swinging animation
			hand.x -= fabs(sin(period * 1.0)) * 0.75;
			hand.y = -0.5 - period * 0.1;
			
			hand.z = -1.25 - fabs(sin(period * 1.0)) * 1.2;
			mode = 2;
		}
		else if (paction.getAction() == PlayerActions::PA_Cooldown)
		{
			// cooldown animation
			hand.y += sin(period / PI * 2) * 0.1;
			if (false) //helditem.count)
				hand.z = -1.25 - fabs(sin(period)) * 1.25;
			else
				hand.z = -1.25 - fabs(sin(period)) * 1.5;
			
			if (lastMode != mode) lastTime = frameCounter - paction.cooldownTime;
			mode = 3;
		}
		else
		{
			if (plogic.FullySubmerged != PlayerLogic::PS_None)
			{
				hand.y += sin(period * 1.0) * 0.1;
				hand.z -= sin(period * 0.5) * 0.1;
				mode = 15;
			}
			else if (plogic.freefall)
			{
				float dy = clamp(-0.4, 0.4, -player.pay);
				if (plogic.Submerged != PlayerLogic::PS_None) dy = 0.0;
				
				hand.y += dy + sin(period * 1.0) * 0.1;
				hand.z -=      sin(period * 0.5) * 0.1;
				mode = 16;
			}
			else if (player.JustMoved)
			{	
				// normal animation (based on player speed)
				
				if (player.Flying)
				{
					hand.y += sin(period * 2.0) * 0.1;
					hand.z -= sin(period * 1.0) * 0.1;
					mode = 18;
				}
				else if (plogic.movestate == PMS_Sprint)
				{
					hand.y += sin(period * 3.00) * 0.1;
					hand.z -= sin(period * 1.50) * 0.1;
					mode = 19;
				}
				else
				{
					hand.y += sin(period * 2.0) * 0.1;
					hand.z -= sin(period * 1.0) * 0.1;
					mode = 20;
				}
			}
			else if (plogic.movestate == PMS_Crouch)
			{
				// resting crouch
				hand.y += sin(period * 1.0) * 0.05;
				hand.z -= sin(period * 0.5) * 0.05;
				mode = 21;
			}
			else
			{
				// resting normal
				hand.y += sin(period * 0.25) * 0.1;
				hand.z -= sin(period * 0.5) * 0.05;
				mode = 22;
			}
			
		} // player action
		
		if (lastMode == -1 || lastMode == mode)
		{
			lastHand = hand.mix(lastHand, 0.5);
			
			if (lastMode == -1)
			{
				lastMode = mode;
				lastTime = frameCounter;
			}
		}
		else
		{
			// interpolate slowly to move the hand to the new position
			lastHand = hand.mix(lastHand, DEFAULT_INTERPOLATION);
			// only set new mode once we get close enough to the new position
			if (fabsf(hand.y - lastHand.y) < 0.01 &&
				fabsf(hand.z - lastHand.z) < 0.01)
			{
				lastMode = mode;
				lastTime = frameCounter;
			}
		}
		
		// convert shadow & torchlight color to 4-vectors
		vec4 shadow = library::colorToVector(plogic.shadowColor);
		vec4 torch = library::colorToVector(plogic.torchColor);
		float modulation = 1.0f; //torchlight.getModulation(frameCounter);
		
		renderHandItem(shadow, torch, modulation);
		
		// render player hand
		Shader& shd = shaderman[Shaderman::PLAYERHAND];
		shd.bind();
		// lighting & ambience
		shd.sendVec3("lightVector", thesun.getRealtimeAngle());
		shd.sendFloat("daylight", thesun.getRealtimeDaylight());
		// player shadow & torchlight color
		shd.sendVec4("lightdata", shadow);
		shd.sendVec4("torchlight", torch);
		// torchlight modulation
		shd.sendFloat("modulation", modulation);
		
		// send rotation matrix when camera has been rotated
		if (camera.rotated)
		{
			shd.sendMatrix("matrot", camera.getRotationMatrix());
		}
		
		// view translation-matrix
		mat4 matview = mat4(vec3(lastHand.x, lastHand.y, lastHand.z));
		
		shd.sendMatrix("matview", matview * handScale);
		
		// bind player models texture
		textureman.bind(0, Textureman::T_PLAYERMODELS);
		
		// update vertex data
		if (vao.isGood() == false)
		{
			vao.begin(sizeof(phand_vertex_t), 8, &vertices[0][0]);
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(phand_vertex_t, x));
			vao.attrib(1, 3, GL_FLOAT, GL_FALSE, offsetof(phand_vertex_t, nx));
			vao.attrib(2, 3, GL_FLOAT, GL_FALSE, offsetof(phand_vertex_t, u));
			vao.end();
		}
		
		// finally, render...
		vao.render(GL_QUADS);
	}
	
	void PlayerHand::renderHandItem(vec4& shadow, vec4& torch, float modulation)
	{
		// render held item
		Item& helditem = menu.getHeldItem();
		// no item, no render
		if (helditem.isAlive() == false) return;
		
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		
		bool isVoxelBlock = helditem.isBlock() && voxels.isVoxelBlock(helditem.getID());
		
		if (helditem.isItem() || isVoxelBlock)
		{
			Shader& shd = shaderman[Shaderman::VOXEL];
			shd.bind();
			// player shadow & torchlight color
			shd.sendVec4("lightdata", shadow);
			shd.sendVec4("torchlight", torch);
			// torchlight modulation
			shd.sendFloat("modulation", modulation);
			// view matrix
			mat4 matview(1.0);
			mat4 matrot;
			if (helditem.isToolItem())
			{
				matview.translate(lastHand.x, lastHand.y - 0.1, lastHand.z + 0.1);
				matrot = rotationMatrix(0, PI / 2, PI/4);
			}
			else
			{
				if (isVoxelBlock && isDoor(helditem.getID()))
				{
					// doors are really tall
					matview.translate(lastHand.x + 0.1, lastHand.y - 0.8, lastHand.z + 0.25);
				}
				else
				{
					matview.translate(lastHand.x + 0.1, lastHand.y, lastHand.z + 0.25);
				}
				matrot = rotationMatrix(0, PI / 2, 0);
			}
			
			matview *= matrot;
			
			shd.sendMatrix("matnrot", matrot);
			shd.sendMatrix("matmvp", camera.getProjection() * matview);
			
			// rotated lighting & ambience
			shd.sendVec3("lightVector", thesun.getRealtimeAngle());
			shd.sendFloat("daylight", thesun.getRealtimeDaylight());
			// send rotation matrix when camera has been rotated
			if (camera.rotated)
			{
				shd.sendMatrix("matrot", camera.getRotationMatrix());
			}
			
			if (isVoxelBlock)
				voxels.renderBlock(helditem.getID());
			else
				voxels.renderItem(helditem.getID());
			
		}
		else
		{
			// bind blocks diffuse texture
			textureman.bind(0, Textureman::T_DIFFUSE);
			// helditem block mesh shader
			Shader& shd = shaderman[Shaderman::PHAND_HELDITEM];
			shd.bind();
			// lighting & ambience
			shd.sendVec3("lightVector", thesun.getRealtimeAngle());
			shd.sendFloat("daylight", thesun.getRealtimeDaylight());
			// player shadow & torchlight color
			shd.sendVec4("lightdata", shadow);
			shd.sendVec4("torchlight", torch);
			// torchlight modulation
			shd.sendFloat("modulation", modulation);
			
			// send rotation matrix when camera has been rotated
			if (camera.rotated)
			{
				shd.sendMatrix("matrot", camera.getRotationMatrix());
			}
			
			// view matrix
			mat4 matview;
			
			// update vertex data
			block_t id = helditem.getID();
			
			int count = 0;
			vertex_t* vertices = nullptr;
			
			if (id == _LANTERN)
			{
				// lantern model
				count = blockmodels.lanterns.totalCount();
				vertices = new vertex_t[count];
				blockmodels.lanterns.copyAll(vertices);
				
				// texture id
				for (int i = 0; i < count; i++)
					vertices[i].w = Block::cubeFaceById(helditem.getID(), (i / 24) * 2, 0);
				
				// translation & scaling
				matview = mat4(0.5);
				matview.translate(lastHand.x - 0.35, lastHand.y, lastHand.z - 0.6);
			}
			else
			{
				// normal block model
				int model = Block::blockModel(id);
				
				count = blockmodels.cubes[model].totalCount();
				vertices = new vertex_t[count];
				blockmodels.cubes[model].copyAll(vertices);
				
				// texture id
				for (int i = 0; i < count; i++)
					vertices[i].w = Block::cubeFaceById(helditem.getID(), i / 4, 3);
				
				// translation & scaling
				matview.identity();
				matview.translate(lastHand.x - 0.15, lastHand.y - 0.1, lastHand.z - 0.6);
				matview *= mat4(0.6);
			}
			
			if (count)
			{
				shd.sendMatrix("matview", matview);
				
				cubeVAO.begin(sizeof(vertex_t), count, vertices);
				cubeVAO.attrib(0, 3, GL_SHORT, GL_FALSE, offsetof(vertex_t, x));
				cubeVAO.attrib(1, 3, GL_BYTE,  GL_TRUE,  offsetof(vertex_t, nx));
				cubeVAO.attrib(2, 3, GL_SHORT, GL_FALSE, offsetof(vertex_t, u));
				cubeVAO.end();
				
				delete[] vertices;
				// finally, render...
				cubeVAO.render(GL_QUADS);
			}
			
		}
		
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
	}
	
}
