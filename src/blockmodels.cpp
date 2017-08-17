#include "blockmodels.hpp"

#include <library/log.hpp>
#include <cstring>

using namespace library;

namespace cppcraft
{
	BlockModels blockmodels;
	
	void BlockModels::init()
	{
		// vertex calculations
		// builfing mesh objects
		logger << Log::INFO << "Precomputing vertex data" << Log::ENDL;
		
		// initialize cube meshes
		initCubes();
		// initialize sloped leafs
		initSlopedLeafs();
		// initialize ladder meshes
		initLadders();
		// initialize stairs
		initStairs();
		// initialize poles (y, x, z)
		initPoles();
		// initialize fence (x, z)
		initFences();
		// initialize doors
		initDoors();
		// lantern model
		initLanterns();
		// initialize cross meshes
		initCrosses();
		// initialize playermodel meshes
		initPlayerMeshes();
	}
	
	Mesh::Mesh()
	{
		this->meshdata = nullptr;
		this->vertices = 0;
		this->vsize = 0;
	}
	Mesh::Mesh(const Mesh& bm)
	{
		this->meshdata = bm.meshdata;
		this->vertices = bm.vertices;
		this->vsize = bm.vsize;
	}
	void Mesh::dispose()
	{
		if (meshdata) free (meshdata);
	}
	
	BlockMesh::BlockMesh(int verts)
	{
		this->vertices = verts;
		this->vsize = sizeof(vertex_t);
		this->meshdata = malloc(vertices * vsize);
	}
	
	SelectionMesh::SelectionMesh(int verts)
	{
		this->vertices = verts;
		this->vsize = sizeof(selection_vertex_t);
		this->meshdata = malloc(vertices * vsize);
	}
	
	PlayerMesh::PlayerMesh(int verts)
	{
		this->vertices = verts;
		this->vsize = sizeof(player_vertex_t);
		this->meshdata = malloc(vertices * vsize);
	}
	
	MeshContainer::~MeshContainer()
	{
		for (size_t i = 0; i < meshes.size(); i++)
			meshes[i].dispose();
	}
	
	void MeshContainer::add(const Mesh& bm)
	{
		meshes.push_back(bm);
	}
	
	vertex_t& MeshContainer::get(int meshid, int vertex)
	{
		return ((vertex_t*) meshes[meshid].getData())[vertex];
	}
	
	int MeshContainer::copyTo(int meshid, void* dest) const
	{
		memcpy(dest,	// destination
			meshes[meshid].getData(), // source
			meshes[meshid].getTotalBytes() // size
		);
		// return the number of vertices we just copied
		return meshes[meshid].getVertices();
	}
	
	int MeshContainer::copyAll(void* dest) const
	{
		int vertices = 0;
		int tbytes = 0;
		for (size_t i = 0; i < meshes.size(); i++)
		{
			memcpy((char*) dest + tbytes, // destination
				meshes[i].getData(), // source
				meshes[i].getTotalBytes() // size
			);
			vertices += meshes[i].getVertices();
			tbytes += meshes[i].getTotalBytes();
		}
		// return the number of vertices we just copied
		return vertices;
	}
	
	void BlockModels::extrude(selection_vertex_t& select, float dist, int face)
	{
		switch (face)
		{
		case 0: // +z
			select.z += dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			break;
		case 1: // -z
			select.z -= dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			break;
		case 2: // +y
			select.y += dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		case 3: // -y
			select.y -= dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		case 4: // +x
			select.x += dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		case 5: // -x
			select.x -= dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		}
	}
	
}
