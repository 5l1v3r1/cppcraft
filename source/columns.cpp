#include "columns.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include "camera.hpp"
#include "renderconst.hpp"
#include "precompiler.hpp"
#include "vertex_block.hpp"
#include <cstring>
#include <csignal>
#define DEBUG

using namespace library;

namespace cppcraft
{
	const float Column::COLUMN_DEPRESSION = 8.0f;
	
	// all the columns you'll ever need
	Columns columns;
	
	void Columns::init()
	{
		logger << Log::INFO << "* Initializing columns" << Log::ENDL;
		
		//////////////////////
		// allocate columns //
		//////////////////////
		int num_columns = sectors.getXZ() * sectors.getXZ() * this->height;
		this->columns = new Column[num_columns]();
		
		//////////////////////////////////////////////////////////////
		// determine if above water and allocate metadata container //
		//////////////////////////////////////////////////////////////
		for (int i = 0; i < num_columns; i++)
		{
			int y = i % this->height;
			// the column is above water if the first sector is >= water level
			columns[i].aboveWater = 
				(y * BLOCKS_Y >= RenderConst::WATER_LEVEL);
		}
		
		assert(sizeof(vertex_t) == 24);
	}
	Columns::Columns()
	{
		this->height = 1;
	}
	Columns::~Columns()
	{
		int num_columns = sectors.getXZ() * sectors.getXZ() * this->height;
		for (int i = 0; i < num_columns; i++)
			columns[i].reset();
		
		delete[] columns;
	}
	
	Column::Column()
	{
		// initialize VAO to 0, signifying a column without valid GL resources
		this->vao = 0;
		// set initial flags
		this->renderable = false;
		this->hasdata = false;
	}
	
	void Column::reset()
	{
		renderable = false;
	}
	
	void Column::compile(int x, int y, int z, Precomp* pc)
	{
		///////////////////////////////////
		// generate resources for column //
		///////////////////////////////////
		
		bool updateAttribs = false;
		
		if (this->vao == 0)
		{
			// occlusion culling
			glGenQueries(RenderConst::MAX_UNIQUE_SHADERS, this->occlusion);
			// vertex array object
			glGenVertexArrays(1, &this->vao);
			// vertex and index buffer object
			glGenBuffers(1, &this->vbo);
			updateAttribs = true;
		}
		
		int vertices = 0;
		int indices = 0;
		
		for (int n = 0; n < RenderConst::MAX_UNIQUE_SHADERS; n++)
		{
			vertices += pc->vertices[n];
			//indices  += pc->indices[n];
			
			//this->indices[n]     = pc->indices[n];
			//this->indexoffset[n] = indices;
			this->vertices[n]     = pc->vertices[n];
			this->bufferoffset[n] = pc->bufferoffset[n];
			
			//indices += pc->indices[n];
		}
		
		// bind vao
		glBindVertexArray(this->vao);
		
		// bind vbo and upload vertex data
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		//glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertices * sizeof(vertex_t), pc->datadump, GL_STATIC_DRAW);
		
		// bind ibo and upload index data
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices * sizeof(indice_t), pc->indidump, GL_STATIC_DRAW);
		
		if (updateAttribs)
		{
		// attribute pointers
		const vertex_t* vrt = nullptr;
		glVertexAttribPointer(0, 3, GL_SHORT,		  GL_FALSE, sizeof(vertex_t), &vrt->x); // vertex
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_BYTE,		  GL_TRUE,  sizeof(vertex_t), &vrt->nx); // normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 4, GL_SHORT,		  GL_FALSE, sizeof(vertex_t), &vrt->u); // texture
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), &vrt->biome); // biome color
		glEnableVertexAttribArray(3);
		//glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), &vrt->c);  // shadow and brightness
		//glEnableVertexAttribArray(4);
		//glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), (char*) (&vrt->c) + 4); // torchlight color
		//glEnableVertexAttribArray(5);
		}
		
		// check for errors
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Column::upload(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Column::upload(): OpenGL state error");
		}
		#endif
		
		if (camera.getFrustum().column(x * BLOCKS_XZ + BLOCKS_XZ / 2,
									   z * BLOCKS_XZ + BLOCKS_XZ / 2,
									   0,  BLOCKS_Y,   BLOCKS_XZ / 2))
		{
			// update render list
			camera.needsupd = true;
			
			if (this->renderable == false)
			{
				// make sure it is added to renderq
				camera.recalc = true;
				this->pos.y = -COLUMN_DEPRESSION;
			}
		}
		
		// set as renderable, 
		this->renderable = true;
		// the vbo has data stored in gpu
		this->hasdata = true;
		
		// reset occluded state
		for (size_t i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			this->occluded[i] = 0;
	}
	
}
