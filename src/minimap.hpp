#ifndef MINIMAP_HPP
#define MINIMAP_HPP

#include <glm/mat4x4.hpp>
#include "sector.hpp"

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

    // update minimap now
		void addSector(Sector& s);
		void roll(int x, int z);
	  void setUpdated();

    // delay-update minimap for this sector
    void sched(Sector& sector) {
      sector.add_genflag(Sector::MINIMAP);
    }

	private:
		library::Bitmap*  bitmap;
		library::Texture* texture;

		bool needs_update;
		float ofsX, ofsY;
	};
	extern Minimap minimap;
}
#endif
