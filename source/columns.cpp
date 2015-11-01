#include "columns.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include "camera.hpp"
#include "renderconst.hpp"
#include "threading.hpp"
#include "vertex_block.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	const float Column::COLUMN_DEPRESSION = 8.0f;
	
	// all the columns you'll ever need
	Columns columns;
	
	// list of metadata that add up to a complete column VBO
	vbodata_t** vboList;
	// column compiler accumulation buffers
	vertex_t*  column_vertex_dump;
	indice_t*  column_index_dump;
	
	void Columns::init()
	{
		logger << Log::INFO << "* Initializing columns" << Log::ENDL;
		
		//////////////////////
		// allocate columns //
		//////////////////////
		int num_columns = sectors.getXZ() * sectors.getXZ() * this->height;
		this->columns = 
			new Column[num_columns]();
		
		//////////////////////////////////////////////////////////////
		// determine if above water and allocate metadata container //
		//////////////////////////////////////////////////////////////
		for (int i = 0; i < num_columns; i++)
		{
			int y = i % this->height;
			// the column is above water if the first sector is >= water level
			columns[i].aboveWater = 
				(getSectorLevel(y) * Sector::BLOCKS_Y >= RenderConst::WATER_LEVEL);
			// determine column size (in sectors)
			columns[i].vbodata = new vbodata_t[getSizeInSectors(y)]();
		}
	}
	Columns::Columns()
	{
		this->height = 3;
		
		sectorLevels = new int[this->height];
		sectorLevels[0] = 0;
		sectorLevels[1] = 3;
		sectorLevels[2] = 4;
		
		sectorSizes = new int[this->height];
		sectorSizes[0] =  3;
		sectorSizes[1] =  1;
		sectorSizes[2] = 12;
		
		// NOTE ASSUMPTION: TOP COLUMN IS TALLEST
		int tallest = sectorSizes[height - 1];
		
		////////////////////////////////////////////////////////
		// allocate temporary datadumps for compiling columns //
		////////////////////////////////////////////////////////
		
		vboList = new vbodata_t*[tallest];
		
		column_vertex_dump = 
			new vertex_t[tallest * RenderConst::MAX_FACES_PER_SECTOR * 4];
		column_index_dump = 
			new indice_t[tallest * RenderConst::MAX_FACES_PER_SECTOR * 4];
		
	}
	Columns::~Columns()
	{
		int num_columns = sectors.getXZ() * sectors.getXZ() * this->height;
		for (int i = 0; i < num_columns; i++)
		{
			int y = i % this->height;
			columns[i].reset(y);
		}
		delete[] columns;
		delete[] sectorLevels;
		delete[] sectorSizes;
		// column assembler stuff
		delete[] vboList;
		delete[] column_vertex_dump;
		delete[] column_index_dump;
	}
	
	Column::Column()
	{
		// initialize VAO to 0, signifying a column without valid GL resources
		this->vao = 0;
		// set initial flags
		this->updated    = false;
		this->renderable = false;
		this->hasdata = false;
	}
	Column::~Column()
	{
		delete[] this->vbodata;
	}
	
	inline void Column::deleteData(int y)
	{
		for (int sy = columns.getSizeInSectors(y)-1; sy >= 0; sy--)
		{
			delete[] vbodata[sy].pcdata;
			vbodata[sy].pcdata = nullptr;
			delete[] vbodata[sy].indexdata;
			vbodata[sy].indexdata = nullptr;
		}
	}
	void Column::reset(int y)
	{
		deleteData(y);
		renderable = false;
		updated = false;
	}
	
	void Column::compile(int x, int y, int z)
	{
		/////////////////////////////////////////////////////////////
		// assemble entire column from vbodata section of a column //
		/////////////////////////////////////////////////////////////
		
		int vboCount = 0;
		
		for (int sy = columns.getSizeInSectors(y)-1; sy >= 0; sy--)
		{
			if (vbodata[sy].pcdata)
			{
				// remember which VBO data section
				vboList[vboCount] = &vbodata[sy];
				// renderable and consistent, add to queue
				vboCount += 1;
			}
		}
		
		// exit if this column isn't renderable at all
		if (vboCount == 0)
		{
			//logger << Log::WARN << "Column::compile(): column was not ready" << Log::ENDL;
			this->updated    = false;
			this->renderable = false;
			return;
		}
		
		///////////////////////////////////////////////////////
		// find offsets for each shader type, and total size //
		///////////////////////////////////////////////////////
		
		vertex_t* dumpVertexOffset[RenderConst::MAX_UNIQUE_SHADERS] = {nullptr};
		int totalVertices = 0; // total amount of vertices for entire column
		
		indice_t* dumpIndexOffset[RenderConst::MAX_UNIQUE_SHADERS] = {nullptr};
		int totalIndices = 0; // total amount of indices for entire column
		
		// go through all VBOs in column and find total vertices for each shader
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			int vertexCount = 0;
			int indexCount  = 0;
			
			for (int vindex = 0; vindex < vboCount; vindex++)
			{
				// count vertices from each shader path
				vertexCount += vboList[vindex]->vertices[i];
				// count indices from each shader path
				indexCount += vboList[vindex]->indices[i];
			}
			
			// base offset used later on for copying small vertex segments
			dumpVertexOffset[i]  = column_vertex_dump + totalVertices;
			totalVertices += vertexCount;
			
			this->indexoffset[i] = totalIndices * sizeof(indice_t);
			this->indices[i]     = indexCount;
			
			// base offset used later on for copying small index segments
			dumpIndexOffset[i]  = column_index_dump + totalIndices;
			totalIndices += indexCount;
		}
		
		if (totalVertices > 32000)
			logger << Log::WARN << "Many indices: " << totalIndices << Log::ENDL;
		
		////////////////////////////////////////////////////////
		// loop through each sector in column and memcpy data //
		////////////////////////////////////////////////////////
		
		// loop through all vertices in shader path
		int baseIndex = 0;
		
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			for (int vindex = 0; vindex < vboCount; vindex++)
			{
				vbodata_t& v = *vboList[vindex];
				
				// only copy where there actually are any vertices
				if (v.vertices[i])
				{
					// macro for vertex buffer offset
					#define m_vertoffset (v.pcdata + v.bufferoffset[i])
					// macro for index buffer offset
					#define m_indexoffset (v.indexdata + indexOffset)
					
					// copy vertices from each individual vbodata segment to the main vertex buffer
					memcpy( dumpVertexOffset[i], m_vertoffset, v.vertices[i] * sizeof(vertex_t) );
					// increase the vertex dump offset
					dumpVertexOffset[i] += v.vertices[i];
					
					int indexOffset = 0;
					for (int j = 0; j < i; j++) indexOffset += v.indices[j];
					
					// copy indices from each individual vbodata segment to the main index buffer
					memcpy( dumpIndexOffset[i], m_indexoffset, v.indices[i] * sizeof(indice_t) );
					
					// increase each index by using indexCount as offset
					indice_t* dumpBaseIndex = dumpIndexOffset[i];
					for (int index = 0; index < v.indices[i]; index++)
					{
						dumpBaseIndex[index] += baseIndex;
					}
					baseIndex += v.vertices[i]; // NOT indices
					
					// increase the index dump offset
					dumpIndexOffset[i] += v.indices[i];
				}
				
			} // vbodata
			
		} // shaders
		
		//logger << Log::INFO << "Total: " << totalIndices << Log::ENDL;
		
		// relieves some RAM usage
		this->deleteData(y);
		
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
			glGenBuffers(2, &this->vbo);
			//glGenBuffers(1, &this->ibo);
			
			updateAttribs = true;
		}
		
		// bind vao
		glBindVertexArray(this->vao);
		// bind vbo and upload vertex data
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBufferData(GL_ARRAY_BUFFER, totalVertices * sizeof(vertex_t), column_vertex_dump, GL_STATIC_DRAW);
		// bind ibo and upload index data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int) totalIndices * sizeof(indice_t), column_index_dump, GL_STATIC_DRAW);
		
		if (updateAttribs)
		{
		// attribute pointers
		const vertex_t* vrt = nullptr;
		glVertexAttribPointer(0, 3, GL_SHORT,		  GL_FALSE, sizeof(vertex_t), &vrt->x); // vertex
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_BYTE,		  GL_TRUE,  sizeof(vertex_t), &vrt->nx); // normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 4, GL_SHORT,		  GL_FALSE, sizeof(vertex_t), &vrt->u); // texture
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), &vrt->biome); // biome color
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), &vrt->c);  // shadow and brightness
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), (char*) (&vrt->c) + 4); // torchlight color
		glEnableVertexAttribArray(5);
		}
		
		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			throw std::string("Column::compile(): OpenGL error after ending compiler");
		}
		#endif
		
		if (camera.getFrustum().column(x * Sector::BLOCKS_XZ + Sector::BLOCKS_XZ / 2,
								z * Sector::BLOCKS_XZ + Sector::BLOCKS_XZ / 2,
								columns.getSectorLevel(y) * Sector::BLOCKS_Y,
								columns.getSizeInSectors(y) * Sector::BLOCKS_Y, 
								Sector::BLOCKS_XZ / 2))
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
		// and no longer needing update
		this->updated = false;
		// the vbo has data stored in gpu
		this->hasdata = true;
		
		// reset occluded state
		for (size_t i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			this->occluded[i] = 0;
	}
	
}
