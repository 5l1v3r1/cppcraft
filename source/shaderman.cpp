#include "shaderman.hpp"

#include <library/log.hpp>
#include <library/config.hpp>
#include <library/math/matrix.hpp>
#include <library/opengl/window.hpp>
#include "camera.hpp"
#include "gameconf.hpp"
#include "renderconst.hpp"
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

using namespace library;

namespace cppcraft
{
	Shaderman shaderman;
	
	template <typename R>
	std::string toString(R x)
	{
		return static_cast<std::ostringstream*>( &(std::ostringstream() << x) )->str();
	}
	
	std::string tokenizer(std::string text)
	{
		if (text == "const int TX_REPEAT")
		{
			text += " = " + toString(RenderConst::TX_REPEAT) + ";";
		}
		else if (text == "const int TX_SOLID")
		{
			text += " = " + toString(RenderConst::TX_SOLID) + ";";
		}
		else if (text == "const int TX_2SIDED")
		{
			text += " = " + toString(RenderConst::TX_2SIDED) + ";";
		}
		else if (text == "const int TX_CROSS")
		{
			text += " = " + toString(RenderConst::TX_CROSS) + ";";
		}
		else if (text == "const int TX_WATER")
		{
			text += " = " + toString(RenderConst::TX_WATER) + ";";
		}
		else if (text == "const float VERTEX_SCALE")
		{
			text += " = " + toString(RenderConst::VERTEX_SCALE) + ";";
		}
		else if (text == "const float VERTEX_SCALE_INV")
		{
			text += " = " + toString(1.0 / RenderConst::VERTEX_SCALE) + ";";
		}
		else if (text == "const float ZNEAR")
		{
			text += " = " + toString(camera.getZNear()) + ";";
		}
		else if (text == "const float ZFAR")
		{
			text += " = " + toString(camera.getZFar()) + ";";
		}
		// settings
		else if (text == "#define POSTPROCESS")
		{
			text = (config.get("postprocess", true)) ? text : "";
		}
		else if (text == "#define NOISE_CLOUDS")
		{
			text = (config.get("hq_clouds", false)) ? text : "";
		}
		else if (text == "#define REFLECTIONS")
		{
			text = (gameconf.reflections) ? text : "";
		}
		
		return text;
	}
	
	void Shaderman::init(WindowClass& gamescr, const Camera& camera)
	{
		logger << Log::INFO << "* Loading & processing shaders" << Log::ENDL;
		
		// "constant" data
		vec3 vecScreen(gamescr.getWidth(), gamescr.getHeight(), gamescr.getAspect());
		vec3 vecSuperScreen(gamescr.getWidth() * gameconf.supersampling, gamescr.getHeight() * gameconf.supersampling, gamescr.getAspect());
		
		// load and initialize all shaders
		std::vector<std::string> linkstage;
		
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_normal");
		linkstage.emplace_back("in_texture");
		linkstage.emplace_back("in_biome");
		linkstage.emplace_back("in_color");
		linkstage.emplace_back("in_color2");
		
		// block shaders
		for (int i = 0; i < 8; i++)
		{
			switch (i)
			{
			case 0:
				shaders[STD_BLOCKS] = Shader("shaders/standard.glsl", tokenizer, linkstage);
				break;
			case 1:
				shaders[CULLED_BLOCKS] = Shader("shaders/culledalpha.glsl", tokenizer, linkstage);
				break;
			case 2:
				shaders[ALPHA_BLOCKS] = Shader("shaders/stdalpha.glsl", tokenizer, linkstage);
				break;
			case 3:
				shaders[BLOCKS_WATER] = Shader("shaders/blocks_water.glsl", tokenizer, linkstage);
				break;
			case 4:
				shaders[BLOCKS_RUNWATER] = Shader("shaders/blocks_runwater.glsl", tokenizer, linkstage);
				break;
			case 5:
				shaders[BLOCKS_LAVA] = Shader("shaders/blocks_lava.glsl", tokenizer, linkstage);
				break;
			case 6:
				shaders[BLOCKS_REFLECT] = Shader("shaders/blocks_reflection.glsl", tokenizer, linkstage);
				break;
			case 7:
				shaders[BLOCKS_DEPTH] = Shader("shaders/blocks_depth.glsl", tokenizer, linkstage);
				break;
			}
			
			int sbase = (int)STD_BLOCKS;
			
			// projection matrix
			shaders[sbase + i].sendMatrix("matproj", camera.getProjection());
			// texture units
			shaders[sbase + i].sendInteger("diffuse", 0);
			shaders[sbase + i].sendInteger("tonemap", 1);
			shaders[sbase + i].sendInteger("skymap", 2);
		}
		
		// extra textures for water
		shaders[BLOCKS_WATER].bind();
		shaders[BLOCKS_WATER].sendInteger("underwatermap", 0);
		shaders[BLOCKS_WATER].sendInteger("depthtexture",  1);
		shaders[BLOCKS_WATER].sendInteger("reflectionmap", 2);
		// near plane half size
		shaders[BLOCKS_WATER].sendVec2("nearPlaneHalfSize", camera.getNearPlaneHalfSize());
		
		// extra textures for lava
		shaders[BLOCKS_LAVA].bind();
		shaders[BLOCKS_LAVA].sendInteger("lavatex", 2);
		
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_normal");
		linkstage.emplace_back("in_texture");
		
		shaders[PLAYERMODEL] = Shader("shaders/players.glsl", tokenizer, linkstage);
		shaders[PLAYERMODEL].sendMatrix("matproj", camera.getProjection());
		shaders[PLAYERMODEL].sendInteger("texture", 0);
		
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_texture");
		
		// player selection shaders
		shaders[SELECTION] = Shader("shaders/selection.glsl", tokenizer, linkstage);
		shaders[SELECTION].sendInteger("diffuse", 0);
		
		shaders[SELECTION_MINING] = Shader("shaders/selection_mine.glsl", tokenizer, linkstage);
		shaders[SELECTION_MINING].sendInteger("diffuse", 0);
		
		// voxel shaders
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_normal");
		linkstage.emplace_back("in_color");
		
		shaders[VOXEL] = Shader("shaders/playerhand_voxel.glsl", tokenizer, linkstage);
		
		
		// particles
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_data");
		linkstage.emplace_back("in_normdata");
		linkstage.emplace_back("in_color");
		
		shaders[PARTICLE] = Shader("shaders/particles.glsl", tokenizer, linkstage);
		shaders[PARTICLE].sendInteger("texture", 0);
		shaders[PARTICLE].sendMatrix("matproj", camera.getProjection());
		shaders[PARTICLE].sendVec2("screensize", vecScreen.xy());
		
		// atmospherics shader
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		
		shaders[ATMOSPHERE] = Shader("shaders/atmosphere.glsl", tokenizer, linkstage);
		shaders[ATMOSPHERE].sendInteger("texture", 0);
		shaders[ATMOSPHERE].sendInteger("stars",   1);
		shaders[ATMOSPHERE].sendMatrix("matproj", camera.getProjection());
		
		// sun shader
		shaders[SUN] = Shader("shaders/sun.glsl", tokenizer, linkstage);
		shaders[SUN].sendMatrix("matproj", camera.getProjection());
		shaders[SUN].sendInteger("texture", 0);
		
		// projected sun shader
		shaders[SUNPROJ] = Shader("shaders/sunproj.glsl", tokenizer, linkstage);
		shaders[SUNPROJ].sendMatrix("matproj", camera.getProjection());
		shaders[SUNPROJ].sendInteger("texture", 0);
		shaders[SUNPROJ].sendInteger("depth",   1);
		
		// moon shader
		shaders[MOON] = Shader("shaders/moon.glsl", tokenizer, linkstage);
		shaders[MOON].sendMatrix("matproj", camera.getProjection());
		shaders[MOON].sendInteger("texture", 0);
		
		// clouds shader
		shaders[CLOUDS] = Shader("shaders/clouds.glsl", tokenizer, linkstage);
		shaders[CLOUDS].sendMatrix("matproj", camera.getProjectionLong());
		shaders[CLOUDS].sendInteger("cloudstex", 0);
		
		// screenspace fog shader
		shaders[FSTERRAINFOG] = Shader("shaders/fsterrainfog.glsl", tokenizer, linkstage);
		shaders[FSTERRAINFOG].sendInteger("terrain",      0);
		shaders[FSTERRAINFOG].sendInteger("skytexture",   1);
		shaders[FSTERRAINFOG].sendInteger("normalstex",   2);
		shaders[FSTERRAINFOG].sendInteger("depthtexture", 3);
		shaders[FSTERRAINFOG].sendInteger("randomtex",    4);
		// near plane half size
		shaders[FSTERRAINFOG].sendVec2("nearPlaneHalfSize", camera.getNearPlaneHalfSize());
		// SSAO filter radius
		shaders[FSTERRAINFOG].sendVec2("screenSize", vecScreen.xy());
		
		// screenspace terrain shader
		shaders[FSTERRAIN] = Shader("shaders/fsterrain.glsl", tokenizer, linkstage);
		shaders[FSTERRAIN].sendInteger("terrain",     0);
		shaders[FSTERRAIN].sendInteger("blurtexture", 1);
		
		// supersampling (downsampler) shader
		shaders[SUPERSAMPLING] = Shader("shaders/supersample.glsl", tokenizer, linkstage);
		shaders[SUPERSAMPLING].sendInteger("colorbuffer", 0);
		shaders[SUPERSAMPLING].sendInteger("samples",     gameconf.supersampling);
		shaders[SUPERSAMPLING].sendVec2("offsets",        vec2(1.0) / vecSuperScreen.xy());
		
		// lensflare
		shaders[LENSFLARE] = Shader("shaders/lensflare.glsl", tokenizer, linkstage);
		
		shaders[LENSFLARE].sendInteger("LowBlurredSunTexture",  0);
		shaders[LENSFLARE].sendInteger("HighBlurredSunTexture", 1);
		shaders[LENSFLARE].sendInteger("DirtTexture", 2);
		
		shaders[LENSFLARE].sendFloat("Dispersal", 0.2);
		shaders[LENSFLARE].sendFloat("HaloWidth", 0.5);
		shaders[LENSFLARE].sendFloat("Intensity", 2.5);
		shaders[LENSFLARE].sendVec3("Distortion", vec3(0.95, 0.97, 1.0));
		
		// blur shaders
		shaders[BLUR] = Shader("shaders/blur.glsl", nullptr, linkstage);
		shaders[BLUR].sendInteger("blurTexture", 0);
		shaders[GAUSS] = Shader("shaders/blurGaussian.glsl", nullptr, linkstage);
		shaders[GAUSS].sendInteger("texture", 0);
		
		// screenspace postprocessing shader
		shaders[POSTPROCESS] = Shader("shaders/screenspace.glsl", tokenizer, linkstage);
		shaders[POSTPROCESS].sendInteger("terrain",   0);
		shaders[POSTPROCESS].sendInteger("lensflare", 1);
		
		// minimap shader
		shaders[MINIMAP] = Shader("shaders/minimap.glsl", tokenizer, linkstage);
		shaders[MINIMAP].sendInteger("texture", 0);
		
		// Player hand shader
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_normal");
		linkstage.emplace_back("in_texture");
		
		shaders[PLAYERHAND] = Shader("shaders/playerhand.glsl", tokenizer, linkstage);
		shaders[PLAYERHAND].sendInteger("texture", 0);
		shaders[PLAYERHAND].sendMatrix("matproj", camera.getProjection());
		
		// playerhand held-item, re-using meshobjects
		shaders[PHAND_HELDITEM] = Shader("shaders/playerhand_helditem.glsl", tokenizer, linkstage);
		shaders[PHAND_HELDITEM].sendInteger("texture", 0);
		shaders[PHAND_HELDITEM].sendMatrix("matproj", camera.getProjection());
		
		// Multi-purpose GUI shader
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_texture");
		linkstage.emplace_back("in_color");
		
		shaders[GUI] = Shader("shaders/gui.glsl", tokenizer, linkstage);
		shaders[GUI].sendInteger("texture", 0);
		
		// GUI menuitem shader
		shaders[MENUITEM] = Shader("shaders/gui_item.glsl", tokenizer, linkstage);
		shaders[MENUITEM].sendInteger("texture", 0);
		
		// compass shader
		shaders[COMPASS] = Shader("shaders/compass.glsl", tokenizer, linkstage);
		shaders[COMPASS].sendInteger("texture", 0);
		
		// color-only shader
		linkstage.clear();
		linkstage.emplace_back("in_vertex");
		linkstage.emplace_back("in_color");
		
		shaders[GUI_COLOR] = Shader("shaders/gui_color.glsl", tokenizer, linkstage);
	}
	
	Shader& Shaderman::operator[] (shaderlist_t shader)
	{
		return shaders[shader];
	}
	
}
