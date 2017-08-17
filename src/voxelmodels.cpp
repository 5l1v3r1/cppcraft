#include "voxelmodels.hpp"

#include "library/bitmap/bitmap.hpp"
#include "library/math/vector.hpp"
#include "library/opengl/opengl.hpp"
#include "library/voxels/voxelizer.hpp"
#include "library/voxels/voxelmodel.hpp"
#include "blocks.hpp"
#include "items.hpp"
#include "tiles.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	VoxelModels voxels;
	const float VoxelModels::VOXELMODELS_ZSCALE = 0.05;
	
	void VoxelModels::createItemModels(Bitmap& bmp)
	{
		// roll through all items
		for (int i = 0; i < VOXELITEMS_MAX; i++)
		{
			createModel(voxelItems[i], bmp, items.tileByID(i), items.itemSize);
		}
	}
	
	void VoxelModels::createBlockModels(Bitmap& bmp)
	{
		/*
		createModel(voxelBlocks[VOXB_LADDER], bmp, Block(_LADDER).getTexture(0), tiles.tileSize);
		
		createModelTall(voxelBlocks[VOXB_WOODDOOR], bmp, Block::cubeFaceById(_WOODDOOR, 0, 1), Block::cubeFaceById(_WOODDOOR, 0, 0), tiles.tileSize);
		createModelTall(voxelBlocks[VOXB_STONEDOOR], bmp, Block::cubeFaceById(_STONEDOOR, 0, 1), Block::cubeFaceById(_STONEDOOR, 0, 0), tiles.tileSize);
		
		createModel(voxelBlocks[VOXB_TORCH], bmp, Block::cubeFaceById(_TORCH, 0, 0), tiles.tileSize);
		
		createModel(voxelBlocks[VOXB_FLOWERRED], bmp, Block::cubeFaceById(_FLOWERRED, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_FLOWERYELLOW], bmp, Block::cubeFaceById(_FLOWERYELLOW, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_FLOWERROSE], bmp, Block::cubeFaceById(_FLOWERROSE, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_GRASS_LONG], bmp, Block::cubeFaceById(_GRASS_LONG, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_GRASS_SHORT], bmp, Block::cubeFaceById(_GRASS_SHORT, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_DESERTFLOWER], bmp, Block::cubeFaceById(_DESERTFLOWER, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_FLOWERREDMAGENTA], bmp, Block::cubeFaceById(_FLOWERREDMAG, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_PLANT_DRYBROWN], bmp, Block::cubeFaceById(_PLANT_DRYBROWN, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_FLOWER_SMALL], bmp, Block::cubeFaceById(_FLOWER_SMALL, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_REEDS], bmp, Block::cubeFaceById(_REEDS, 0, 0), tiles.tileSize);
		
		createModel(voxelBlocks[VOXB_BUSH_HOLLY], bmp, Block::cubeFaceById(_BUSH_HOLLY, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_BUSH2], bmp, Block::cubeFaceById(_BUSH2, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_BUSH3], bmp, Block::cubeFaceById(_BUSH3, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_BUSH4], bmp, Block::cubeFaceById(_BUSH4, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_BUSH5], bmp, Block::cubeFaceById(_BUSH5, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_BUSH_PINKROSE], bmp, Block::cubeFaceById(_BUSH_PINKROSE, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_BUSH_MRSPIKY], bmp, Block::cubeFaceById(_BUSH_MRSPIKY, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_FLOWER_CROCUS], bmp, Block::cubeFaceById(_FLOWER_CROCUS, 0, 0), tiles.tileSize);
		
		createModel(voxelBlocks[VOXB_DARKSHROOM], bmp, Block::cubeFaceById(_DARKSHROOM, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_REDSHROOM], bmp, Block::cubeFaceById(_REDSHROOM, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_BLUESHROOM], bmp, Block::cubeFaceById(_BLUESHROOM, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_WATERLILY], bmp, Block::cubeFaceById(_WATERLILY, 0, 0), tiles.tileSize);
		
		createModel(voxelBlocks[VOXB_FIRE], bmp, Block::cubeFaceById(_FIRE, 0, 0), tiles.tileSize);
		createModel(voxelBlocks[VOXB_HANGINGAPPLE], bmp, Block::cubeFaceById(_HANGINGAPPLE, 0, 0), tiles.tileSize);
		*/
	}
	
	void VoxelModels::createModel(VoxelModel*& model, Bitmap& bmp, int tileID, int tileSize)
	{
		// create temporary bitmap
		Bitmap tmp(tileSize, tileSize, 32);
		if (tmp.data() == nullptr) throw "VoxelModels::createModel(): bitmap buffer was null";
		
		// copy tile to temporary buffer
		memcpy(tmp.data(), bmp.data() + tileID * tileSize * tileSize, tileSize * tileSize * 4);
		
		glm::vec3 offset(0.0, 0.0, 0.0);
		glm::vec3 scale(1.0 / tileSize, 1.0 / tileSize, VOXELMODELS_ZSCALE);
		XModel xmod;
		
		// create model from bitmap
		xmod.extrude(tmp, offset, scale);
		
		// create model
		model = new VoxelModel();
		model->create(xmod.vertices(), xmod.data());
		
	}
	
	void VoxelModels::createModelTall(VoxelModel*& model, Bitmap& bmp, int tile_bot, int tile_top, int tileSize)
	{
		// create temporary bitmap
		Bitmap tmp(tileSize, tileSize * 2, 4);
		if (tmp.data() == nullptr) return;
		
		int size = tileSize * tileSize;
		
		// copy tiles to temporary buffer
		memcpy (tmp.data() + 0,    bmp.data() + tile_bot * size, size * 4);
		memcpy (tmp.data() + size, bmp.data() + tile_top * size, size * 4);
		
		glm::vec3 offset(0.0, 0.0, 0.0);
		glm::vec3 scale (1.0 / tileSize, 1.0 / tileSize, VOXELMODELS_ZSCALE);
		XModel xmod;
		
		// create model from bitmap
		xmod.extrude(tmp, offset, scale);
		
		// create model
		model = new VoxelModel();
		model->create(xmod.vertices(), xmod.data());
		
	}
	
	void VoxelModels::renderModel(VoxelModel* model)
	{
		if (model->isGood() == false) return;
		
		model->render();
	}
	
	void VoxelModels::renderItem(int modelid)
	{
		renderModel(voxelItems[modelid]);
	}
	
	int VoxelModels::getVoxelId(block_t id)
	{
		/*
		if (id == _LADDER)
		{
			return VOXB_LADDER;
		}
		else if (id == _WOODDOOR)
		{
			return VOXB_WOODDOOR;
		}
		else if (id == _STONEDOOR)
		{
			return VOXB_STONEDOOR;
		}
		else if (id == _TORCH)
		{
			return VOXB_TORCH;
		}
		else if (id >= _FLOWERRED && id <= _REEDS)
		{
			return id - _FLOWERRED + VOXB_FLOWERRED;
		}
		else if (id >= _BUSH_HOLLY && id <= _FLOWER_CROCUS)
		{
			return id - _BUSH_HOLLY + VOXB_BUSH_HOLLY;
		}
		else if (id >= _DARKSHROOM && id <= _BLUESHROOM)
		{
			return id - _DARKSHROOM + VOXB_DARKSHROOM;
		}
		else if (id == _WATERLILY)
		{
			return VOXB_WATERLILY;
		}
		else if (id == _FIRE)
		{
			return VOXB_FIRE;
		}
		else if (id == _HANGINGAPPLE)
		{
			return VOXB_HANGINGAPPLE;
		}*/
		
		throw std::string("getVoxelId: Invalid voxel id");
		
	} // getVoxelId
	
	void VoxelModels::renderBlock(int modelid)
	{
		modelid = getVoxelId(modelid);
		if (modelid == -1) throw std::string("VoxelModels::renderBlock(): Exigent circumstances");
		
		renderModel(voxelBlocks[modelid]);
	}
	
}
