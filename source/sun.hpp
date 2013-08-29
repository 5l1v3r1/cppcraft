#ifndef SUN_HPP
#define SUN_HPP

#include "library/math/vector.hpp"

namespace cppcraft
{
	class SunClass
	{
	private:
		float radianAngle;   // target angle
		library::vec3 angle; // target sun vector from player
		
		float realRadian;
		library::vec3 realAngle;
		float realAmbience;
		
		float renderDist;
		
	public:
		static const float SUN_DEF_ANGLE;
		
		void init(float radangle);
		void render();
		
		// permanently sets the static sun angle
		void setRadianAngle(float radians);
		
		// getters for static sun values
		const library::vec3& getAngle() const;
		float getRadianAngle() const;
		
		// daylight multiplier: 1.0 = full daylight, 0.0 = total darkness
		float daylight(float y);
		// world ambient light color
		library::vec3 ambienceColor();
		
		// integrator for realtime sun values
		void integrate(float step);
		// gettesr for realtime sun values
		const library::vec3& getRealtimeAngle() const;
		float getRealtimeRadianAngle() const;
		float getRealtimeDaylight() const;
		
		// sky renderer uses this class extensively
		friend class SkyRenderer;
		friend class Textureman;
		friend class SceneRenderer;
	};
	extern SunClass thesun;
	
}

#endif