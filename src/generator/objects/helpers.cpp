#include "helpers.hpp"

#include "../../spiders.hpp"
#include "../random.hpp"
#include <cmath>

using namespace cppcraft;

namespace terragen
{
	int ofacing(int dx, int dz)
	{	// +z = 0 (front), -z = 1 (back), +x = 2 (right), -x = 3 (left)
		if (std::abs(dx) > std::abs(dz))
		{
			return (dx > 0) ? 2 : 3;
		} else {
			return (dz > 0) ? 0 : 1;
		}
	}
	int ofacingNeg(int dx, int dz)
	{	// note: this function is OPPOSITE and only to be used with negated blocks (slopes)
		if (std::abs(dx) > std::abs(dz))
		{
			return (dx > 0) ? 3 : 2;
		} else {
			return (dz > 0) ? 1 : 0;
		}
	}

	// checks if there is a platform for placing something somewhere
	// check in a square that there is _AIR from y and above
	// and check that there is solid ground at y-1
	bool coretest(int x, int y, int z, int ground_rad, int air_rad, int height)
	{
		int maxrad = ground_rad * ground_rad;
		for (int dx = -ground_rad; dx <= ground_rad; dx++)
		for (int dz = -ground_rad; dz <= ground_rad; dz++)
		{
			if (dx*dx + dz*dz <= maxrad)
			{
				// ground test: exit when AIR
				Block block = Spiders::getBlock(x + dx, y-1, z + dz);
				if (block.isTransparent()) return false;
			}
		}

		for (int dy = 0; dy < height; dy++)
		{
			maxrad = air_rad * air_rad;
			for (int dx = -air_rad; dx <= air_rad; dx++)
			for (int dz = -air_rad; dz <= air_rad; dz++)
			{
				if (dx*dx + dz*dz <= maxrad)
				{
					// air test: exit when not AIR or fluid
					Block block = Spiders::getBlock(x + dx, y + dy, z + dz);
					if (block.overwriteable() == false) return false;
				}
			}
		}
		return true;
	}

	void downSpider(int x, int y, int z, block_t id, int tries)
	{
		Block currentBlock = Spiders::getBlock(x, y, z);

		// air, crosses, water
		if (currentBlock.isTransparent())
		{
			if (tries--) downSpider(x, y-1, z, id, tries);
			Spiders::setBlock(x, y, z, Block(id));
		}
	}

	void ocircleXZroots(int x, int y, int z, int radius, block_t id)
	{
		int dx, dz, r = radius*radius;
		for (dx = -radius; dx <= radius; dx++)
		for (dz = -radius; dz <= radius; dz++)
		{
			if (dx*dx + dz*dz <= r)
			{
				Spiders::setBlock(x+dx, y, z+dz, Block(id));
				downSpider(x+dx, y-1, z+dz, id, 6);
			}
		}
	}

	void ocircleXZ(int x, int y, int z, int radius, block_t id)
	{
		const int maxrad = radius*radius;
		for (int dx = -radius; dx <= radius; dx++)
		for (int dz = -radius; dz <= radius; dz++)
		{
			if (dx*dx + dz*dz <= maxrad)
				Spiders::setBlock(x+dx, y, z+dz, Block(id));
		}
	}
	void ocircleXZstencil(int gx, int gy, int gz, int rad, block_t id, float chance)
	{
		const int maxrad = rad * rad;
		for (int x = -rad; x <= rad; x++)
		for (int z = -rad; z <= rad; z++)
		{
			int dist_sq = x*x + z*z;
			if (dist_sq <= maxrad)
			{
				if (randf(gx+x, gy, gz+z) < chance)
				{
					Spiders::setBlock(gx + x, gy, gz + z, Block(id));
				}
			}
		}
	}

	void oellipsoidXZ(int x, int y, int z, int radius, float radx, float radz, block_t id)
	{
		int dx, dz;
		float r1 = radius*radx; r1 *= r1;
		float r2 = radius*radz; r2 *= r2;

		for (dx = -radius; dx <= radius; dx++)
		for (dz = -radius; dz <= radius; dz++)
			if (dx*dx / r1 + dz*dz / r2 <= 1)
				Spiders::setBlock(x+dx, y, z+dz, Block(id));

	}
	void oellipsoidXY(int x, int y, int z, int radius, float radx, float rady, float stencil, block_t id)
	{
		int dx, dy;
		float r1 = radius*radx; r1 *= r1;
		float r2 = radius*rady; r2 *= r2;
		float r;

		for (dx = -radius; dx <= radius; dx++)
		for (dy = -radius; dy <= radius; dy++) {
			r = dx*dx / r1 + dy*dy / r2;
			if (r <= 1) {
				if (r < 0.65)
					Spiders::setBlock(x+dx, y+dy, z, Block(id));
				else if (randf(x+dx, y+dy, z+613) < stencil)
				// always passes with stencil = 1.0, never passes with 0.0
					Spiders::setBlock(x+dx, y+dy, z, Block(id));
			}
		}
	}

	void obell(int x, int y, int z, block_t id, int lower, int height, int radius, int inner_rad, int midlevel, float midstrength, float understrength, float stencilchance)
	{
		float radf, lradf, midd, dr;
		int dx, dy, dz, radxz;
		int r, l;

		for (dy = lower; dy <= height; dy++)
		{
			midd = 1.0 - std::abs(dy - midlevel) / (height-midlevel);
			midd *= midstrength;
			if (dy < 0) midd = -dy * understrength;

			r = powf(radius - midd, 2.0);
			l = powf(inner_rad - midd, 2.0);

			dr = (float)dy / (float)height * (float)radius;
			radf = r - dr*dr;
			lradf = l - dr*dr;

			for (dx = -radius; dx <= radius; dx++)
			for (dz = -radius; dz <= radius; dz++)
			{
				radxz = dx*dx + dz*dz;
				if (radxz >= lradf && radxz <= radf)
				{
					if (stencilchance < 1.0 && radxz >= radf-8)
					{
						if (randf(x+dx, y+dy, z+dz) < stencilchance)
							Spiders::setBlock(x+dx, y+dy, z+dz, Block(id));
					}
					else
					{
						Spiders::setBlock(x+dx, y+dy, z+dz, Block(id));
					}

				} // rad
			}

		}

	}
}
