#pragma once

#include <string>

namespace server
{
	class Client
	{
	public:
		std::string name;
		
		// entity id
		int id;
		
		Entity& get()
		{
			return entities[id];
		}
		
	};
}