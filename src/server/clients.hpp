#pragma once

#include "client.hpp"
#include <list>

namespace server
{
	class Clients
	{
		std::list<Client> clients;
	};
}
