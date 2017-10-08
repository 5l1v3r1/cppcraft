#pragma once

namespace terragen
{
	struct gendata_t;

	class PostProcess
	{
	public:
		// how much soil or sand we need to convert to stone
		static const int STONE_CONV_OVERW = 4;
		// how much soil we need for stone conversion
		static const int STONE_CONV_UNDER = 8;

		static void init();
		static void run(gendata_t* gdata);
		static void try_deposit(gendata_t* gdata, int x, int y, int z);
	};
}
