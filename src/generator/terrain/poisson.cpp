#include "poisson.hpp"
#undef NDEBUG
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <glm/gtc/random.hpp>
#include <glm/geometric.hpp>

namespace terragen
{
  template <typename T>
  static inline void
  remove_at(std::vector<T>& v, typename std::vector<T>::size_type n)
  {
      std::swap(v[n], v.back());
      v.pop_back();
  }

  struct point_t {
    glm::vec2 pos;
    bool valid = false;
  };

  struct grid_t
  {
    grid_t(int W, int H, float cellsize)
      : w(W), h(H), cell_size(cellsize), points(w * h) {
        for (auto& p : points) p.valid = false;
      }

    bool in_neighborhood(const glm::vec2 p, const float dist)
    {
      const int x = (int) (p.x / cell_size);
      const int y = (int) (p.y / cell_size);
      const int x_min = std::max(0,   x - 2);
      const int x_max = std::min(w-1, x + 2);
      const int y_min = std::max(0,   y - 2);
      const int y_max = std::min(h-1, y + 2);

      for (int dy = y_min; dy <= y_max; dy++)
  		for (int dx = x_min; dx <= x_max; dx++)
  		{
        auto& cell = get(dx, dy);
				if (cell.valid && glm::distance(p, cell.pos) < dist) return true;
      }
      return false;
    }

    point_t& get(int x, int y) noexcept
    {
      return points.at(x + y * w);
    }
    void set(const glm::vec2 p) {
      auto& cell = get((int) (p.x / cell_size), (int) (p.y / cell_size));
      assert(cell.valid == false);
      cell = point_t{p, true};
    }

    const int w;
    const int h;
    const float cell_size;
    std::vector<point_t> points;
  };

  inline glm::vec2 point_around(const glm::vec2 origin, const float min_dist)
  {
    auto p = glm::diskRand(min_dist);
    return origin + p + glm::normalize(p) * min_dist;
  }

  Poisson::points_t Poisson::generate(
        int w, int h, const float min_dist, const uint32_t total)
  {
    const float cellSize = min_dist / sqrtf(2.0f);
    const uint32_t gridW = std::ceil(w / cellSize);
    const uint32_t gridH = std::ceil(h / cellSize);
    printf("Grid size: %u, %u  Cell: %f\n", gridW, gridH, cellSize);
    // create grid
    grid_t grid(gridW, gridH, cellSize);
    // create first point
    auto first = glm::linearRand(glm::vec2(0.0f), glm::vec2(w, h));
    grid.set(first);

    points_t m_points;
    m_points.reserve(total);
    m_points.push_back(first);
    int iterations = 0;

    while (m_points.size() < total)
    {
      // generate new random points around already existing ones
      for (auto& origin : m_points)
      {
        const glm::vec2 p = point_around(origin, min_dist);
        if (p.x >= 0 && p.x < w && p.y >= 0 && p.y < h)
        if (not grid.in_neighborhood(p, min_dist)) {
          grid.set(p);
          m_points.push_back(p);
          break;
        }
      }
      if (++iterations > 999) {
        printf("PoissonDisc: Max iterations exceeded\n");
        break;
      }
    }
    return m_points;
  }

  FastPlacement::FastPlacement(const size_t size, const float rad, const int total)
    : m_array(size * size), m_size(size)
  {
    auto points = Poisson::generate(size, size, rad, total);

    for (auto& p : points) {
      this->set(p.x, p.y, true);
    }

    /*
    for (int x = 0; x < size; x++)
    for (int z = 0; z < size; z++)
    {
      printf("%d ", get(x, z));
      if (z == size-1) printf("\n");
    }*/
  }

  void Poisson::init()
  {
    /*
    FILE* f = fopen("distribution256.data", "r");
    assert(f);
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    printf("SIZE: %d  vs ARRAY: %zu\n", size, m_array.size());
    assert(sizeof(m_array) == size);

    rewind(f);
    int res = fread(m_array.data(), 1, size, f);
    assert(res == size);
    fclose(f);
    */

  }
}
