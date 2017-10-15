/**
 *
 *
 *
**/
#include <glm/vec3.hpp>

namespace cppcraft
{
	class Camera;
	class DrawQueue;
	class Renderer;
	class SkyRenderer;
	class WorldManager;

	class SceneRenderer
	{
	public:
		SceneRenderer(Renderer& renderer);
		void render(Renderer& renderer);

		char isUnderwater() const
		{
			return underwater;
		}

	private:
		double lastTime = 0.0;

		// snapshots
		glm::vec3 snapPlayerPos;
		// used when rendering
		glm::vec3 playerPos;
		int playerSectorX, playerSectorZ;
		int snapWX = 0;
    int snapWZ = 0;
		bool playerMoved = true;
		char underwater = 0;

		// camera bobbing
		double motionTimed = 0.0;
		double lastCameraDeviation = 0.0;

		// camera bobbing computatron
		double cameraDeviation(double frameCounter, double dtime);

		// juggernauts
		void initTerrain();
		void recalculateFrustum();
		void recalculateFrustum(Camera& camera, DrawQueue& drawq, const glm::vec3& look);
		void compressRenderingQueue();
		// main scene rendering function
		void renderScene(Renderer& renderer, cppcraft::Camera& camera);
		void renderReflectedScene(Renderer& renderer, cppcraft::Camera& camera);
		void renderSceneWater(Renderer& renderer);

		friend class SkyRenderer;
		friend class GUIRenderer;
	};
}
