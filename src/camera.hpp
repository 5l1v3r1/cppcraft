#ifndef CAMERA_FRUSTUM_HPP
#define CAMERA_FRUSTUM_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <library/opengl/camera.hpp>

namespace cppcraft
{
	class Camera : public library::Camera
	{
	private:
		int gridsize;
		// camera matrices
		glm::mat4 matproj_long; // PL
		// in-sector offset (from seamless transition)
		glm::vec3 worldOffset;

	public:
		bool recalc;   // recalculate viewing frustum
		bool rotated;  // camera was rotated, resend rotation matrices
		bool ref;      // run occlusion tests
		int  needsupd; // update render list
		int  cameraViewSectors;

		void init(library::WindowClass& wnd);

		int getGridsize() const { return this->gridsize; }
		const glm::mat4& getProjectionLong() const
		{
			return matproj_long;
		}

		inline const glm::vec3& getWorldOffset() const
		{
			return this->worldOffset;
		}
		void setWorldOffset(double posX, double posY, double posZ);

	};
	extern cppcraft::Camera camera;
	extern cppcraft::Camera reflectionCamera;
}

#endif
