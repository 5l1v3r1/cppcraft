#pragma once

#include "terrain.hpp"
#include <deque>
#include <map>

namespace terragen
{
	class Terrains {
	public:
		void init();

		Terrain& operator[] (int id)
		{
			return tvec.at(id);
		}
		int operator[] (const std::string& name)
		{
			return names[name];
		}

		template <typename... Args>
		Terrain& add(const std::string& name, Args&&... args)
		{
			size_t index = tvec.size();
			tvec.emplace_back(args...);
			names[name] = index;
			return tvec.back();
		}

		size_t size() const noexcept
		{
			return tvec.size();
		}

	private:
		std::deque<Terrain> tvec;
		std::map<std::string, int> names;
	};
	extern Terrains terrains;
}
