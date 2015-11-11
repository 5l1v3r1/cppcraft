#pragma once

namespace terragen
{
	struct gendata_t;
	
	class PostProcess
	{
	public:
		static void init();
		static void run(gendata_t* gdata);
	};
}
