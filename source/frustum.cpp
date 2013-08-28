#include "frustum.hpp"

#include "library/config.hpp"
#include "library/log.hpp"
#include "library/opengl/window.hpp"
#include "sectors.hpp"
#include "world.hpp"

using namespace library;

namespace cppcraft
{
	// Viewing frustum object
	ViewingFrustum frustum;
	
	void ViewingFrustum::init(WindowClass& wnd)
	{
		logger << Log::INFO << "* Initializing frustum" << Log::ENDL;
		
		this->recalc   = true;
		this->rotated  = true;
		this->ref      = false;
		this->needsupd = false;
		
		if (Sectors.getXZ() >= 80)
			this->gridsize = 32;
		else if (Sectors.getXZ() >= 40)
			this->gridsize = 16;
		else
			this->gridsize = 8;
		
		// sectors view depth-range
		this->cameraViewSectors = Sectors.getXZ() / 2 - 2;
		// unit blocks depth-range
		this->FOV   = config.get("frustum.fov", 61.0f);
		this->znear = 0.1;
		this->zfar  = cameraViewSectors * Sector::BLOCKS_XZ;
		
		// projection matrices
		matproj.perspective(this->FOV, wnd.SA, this->znear, this->zfar);
		matproj_long.perspective(this->FOV, wnd.SA, this->znear, this->zfar * 1.6);
		
	}
	
	void ViewingFrustum::calculateFrustum()
	{
		this->frustum.calculate(matproj, matview);
	}
	
	void ViewingFrustum::setRotation(float rx, float ry, float rz)
	{
		matrot.rotateZYX(rx, ry, rz);
	}
	
	void ViewingFrustum::setTranslation(float tx, float ty, float tz)
	{
		// view matrix = rotation & translation
		matview = matrot;
		matview.translated(tx, ty, tz);
		
		// modelview projection matrix
		matviewproj = matproj * matview;
	}
	
	const Matrix& ViewingFrustum::getProjection() const
	{
		return matproj;
	}
	const Matrix& ViewingFrustum::getProjectionLong() const
	{
		return matproj_long;
	}
	const Matrix& ViewingFrustum::getViewMatrix() const
	{
		return matview;
	}
	const Matrix& ViewingFrustum::getRotationMatrix() const
	{
		return matrot;
	}
	const Matrix& ViewingFrustum::getMVP_Matrix() const
	{
		return matviewproj;
	}
	
	const Frustum& ViewingFrustum::camera() const
	{
		return this->frustum;
	}
	
	const vec3& ViewingFrustum::getWorldOffset() const
	{
		return this->worldOffset;
	}
	
	void ViewingFrustum::setWorldOffset(double posX, double posZ)
	{
		// calculate sectoral offsets
		int halfWorld = Sectors.getXZ() * Sector::BLOCKS_XZ / 2;
		
		int dwx = World::WORLD_STARTING_X - world.getWX();
		dwx = dwx * Sector::BLOCKS_XZ - halfWorld;
		
		int dwz = World::WORLD_STARTING_Z - world.getWZ();
		dwz = dwz * Sector::BLOCKS_XZ - halfWorld;
		
		// position.xz - center of local area (posX and posZ is NEGATIVE)
		worldOffset.x = dwx - posX;
		worldOffset.y = 0.0;
		worldOffset.z = dwz - posZ;
	}
	
}
