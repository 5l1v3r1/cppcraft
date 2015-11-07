#ifndef MINIMAP_HPP
#define MINIMAP_HPP

#include <glm/mat4x4.hpp>

namespace library
{
	class Bitmap;
	class Texture;
}

namespace cppcraft
{
	class Sector;
	
	class Minimap
	{
	public:
		Minimap();
		void init();
		void update(double px, double pz);
		void render(glm::mat4& mvp);
		
		void addSector(Sector& s);
		void roll(int x, int z);
		
		void setUpdated();
		
	private:
		library::Bitmap*  bitmap;
		library::Texture* texture;
		
		bool needs_update;
		float ofsX, ofsY;
	};
	extern Minimap minimap;
}
#endif
