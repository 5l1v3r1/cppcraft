#pragma once

#include "object.hpp"
#include <list>
#include <stdio.h>

namespace terragen
{
	class ObjectQueue
	{
	public:
		static void add(const std::vector<GenObject>& queue)
		{
			for (auto& obj : queue)
				get().objects.push_back(obj);
		}
		
		static void run()
		{
			get().run_internal();
		}
		static ObjectQueue& get();
		
	private:
		void run_internal();
		
		std::list<GenObject> objects;
	};
}