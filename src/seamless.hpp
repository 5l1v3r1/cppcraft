#ifndef SEAMLESS_HPP
#define SEAMLESS_HPP

namespace cppcraft
{
	
	class Seamless
	{
	private:
		static bool seamlessness();
		static void preconditions();
		
	public:
		static bool run();
		
		static const int OFFSET;
	};
	
}

#endif
