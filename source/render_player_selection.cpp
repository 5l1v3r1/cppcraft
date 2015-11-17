#include "render_player_selection.hpp"

#include <library/log.hpp>
#include <library/math/vector.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include "blocks.hpp"
#include "blockmodels.hpp"
#include "vertex_block.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "player_actions.hpp"
#include "player_logic.hpp"
#include "renderconst.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "threading.hpp"

using namespace library;

namespace cppcraft
{
	class PlayerSelection
	{
		VAO vao;
		bool renderable;
		
	public:
		void render();
	};
	PlayerSelection playerSelection;
	
	void renderPlayerSelection()
	{
		playerSelection.render();
	}
	
	block_t PlayerLogic::determineSelectionFacing(Block& block, glm::vec3& ray, glm::vec3& fracs, float stepSize)
	{
		// determine selection-box facing value
		if (block.isLadder())
		{
			return 10;
		}
		/*else if (id == _WOODPOLE)
		{
			if (fracs.y >= 1.0 - stepSize)
			{
				return 2; // top of pole
			}
			else return 9; // select entire pole
		}*/
		else if (block.isCross())
		{
			return 6; // crosses box
		}
		else
		{
			// determine best-guess facing value for cube & halfblock
			
			float minv = fracs.x;
			if (minv > fracs.y) minv = fracs.y;
			if (minv > fracs.z) minv = fracs.z;
			
			if (fracs.x == minv)
			{
				if (fracs.x <= stepSize)
					return 5; // left side of a cube
			}
			else if (fracs.y == minv)
			{
				if (fracs.y <= stepSize)
					return 3; // bottom of a cube
			}
			else
			{
				if (fracs.z <= stepSize)
					return 1; // backside of cube
			}
			
			float maxv = fracs.x;
			if (maxv < fracs.y) maxv = fracs.y;
			if (maxv < fracs.z) maxv = fracs.z;
			
			if (fracs.x == maxv)
			{
				if (fracs.x >= 1.0 - stepSize)
					return 4; // right side of a cube
			}
			else if (fracs.y == maxv)
			{
				if (fracs.y >= 1.0 - stepSize)
					return 2; // top of a cube
			}
			if (fracs.z >= 1.0 - stepSize)
				return 0; // front of cube
			
			// since we didn't find any hits yet, it could be a halfblock
			// or a lowblock, which are at 1/2 and 1/8 respectively in height
			if (block.isHalfblock() || block.isLowblock())
				return 2; // half/low-block top
		}
		// no facing detected, return "something"
		return 0;
		
	} // selection box result
	
	void PlayerSelection::render()
	{
		// determine selection
		mtx.playerselection.lock();
		
		// exit if we have no selection
		if (plogic.hasSelection() == false)
		{
			mtx.playerselection.unlock();
			return;
		}
		
		int selection = plogic.selection.facing;
		int vx = (int)plogic.selection.pos.x;
		int vy = (int)plogic.selection.pos.y;
		int vz = (int)plogic.selection.pos.z;
		
		int model  = 0; //Block::blockModel(plogic.selection.block.getID());
		int bits = plogic.selection.block.getBits();
		
		bool updated = plogic.selection.updated;
		plogic.selection.updated = false;
		
		mtx.playerselection.unlock();
		
		if (updated)
		{
			// build vertices
			int vertices = 0;
			selection_vertex_t* vertexData = nullptr;
			
			// determine selection mesh
			if (selection < 6)
			{
				// we are to render some cube model, specifically only 1 quad
				vertices = 4;
				vertexData = new selection_vertex_t[vertices];
				// regular cube face
				if (vertices != blockmodels.selectionCube[model].copyTo(selection, vertexData))
					throw std::string("PlayerSelection::render(): selection == 6 vertex count mismatch");
			}
			else if (selection == 6) // cross selection box
			{
				vertices = 24;
				vertexData = new selection_vertex_t[vertices];
				// cross selection box
				if (vertices != blockmodels.selecionCross.copyTo(0, vertexData))
					throw std::string("PlayerSelection::render(): selection == 6 vertex count mismatch");
			}
			else if (selection == 9) // pole selection box
			{
				vertices = 24;
				vertexData = new selection_vertex_t[vertices];
				// cross selection box
				if (vertices != blockmodels.selectionPole.copyTo(0, vertexData))
					throw std::string("PlayerSelection::render(): selection == 9 vertex count mismatch");
			}
			else if (selection == 10) // ladders selection box
			{
				vertices = 24;
				vertexData = new selection_vertex_t[vertices];
				// ladder selection box
				if (vertices != blockmodels.selectionLadder.copyTo(bits, vertexData))
					throw std::string("PlayerSelection::render(): selection == 10 vertex count mismatch");
			}
			
			
			// upload only if renderable
			renderable = (vertices != 0);
			
			if (renderable)
			{
				// upload selection rendering data
				vao.begin(sizeof(selection_vertex_t), vertices, vertexData);
				vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(selection_vertex_t, x));
				vao.attrib(1, 2, GL_FLOAT, GL_FALSE, offsetof(selection_vertex_t, u));
				vao.end();
				
				// cleanup
				delete[] vertexData;
			}
		}
		
		if (renderable == false) return;
		
		// enable blending
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		// --> fixes sun visible through selection
		glColorMask(1, 1, 1, 0);
		
		/// render player selection ///
		Shader* shd = nullptr;
		
		// mining
		if (paction.getAction() == PlayerActions::PA_Mineblock)
		{
			textureman.bind(0, Textureman::T_MINING);
			
			// selection shader
			shd = &shaderman[Shaderman::SELECTION_MINING];
			shd->bind();
			
			// mine tile id
			int tileID = paction.getMiningLevel() * 9.0;
			shd->sendFloat("miningTile", tileID);
		}
		else
		{
			textureman.bind(0, Textureman::T_SELECTION);
			
			// normal selection shader
			shd = &shaderman[Shaderman::SELECTION];
			shd->bind();
		}
		
		shd->sendMatrix("matmvp", camera.getMVP());
		// position in space
		if (updated)
			shd->sendVec3("vtrans", glm::vec3(vx, vy, vz));
		
		// render quad(s)
		vao.render(GL_QUADS);
		
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		
		glColorMask(1, 1, 1, 1);
	}
}
