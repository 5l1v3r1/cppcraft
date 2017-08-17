#pragma once

#include "terrain.hpp"
#include <map>
#include <vector>

namespace terragen
{
	class Terrains
	{
	public:
		void init();
		
		// calls specified terrain function
		inline float get(int id, const glm::vec2& p)
		{
			return tvec[id].func2d(p);
		}
		inline float get(int id, const glm::vec3& p, float hvalue)
		{
			return tvec[id].func3d(p, hvalue);
		}
		
		Terrain& operator[] (int id)
		{
			return tvec[id];
		}
		uint16_t operator[] (const std::string& name)
		{
			return names[name];
		}
		
		template <typename... Args>
		int add(const std::string& name, Args&&... args)
		{
			size_t index = tvec.size();
			tvec.emplace_back(args...);
			names[name] = index;
			return index;
		}
		
		size_t size() const
		{
			return tvec.size();
		}
		
	private:
		std::vector<Terrain> tvec;
		std::map<std::string, uint16_t> names;
	};
	extern Terrains terrains;
}
