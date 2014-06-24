#ifndef PRECOMP_THREAD_HPP
#define PRECOMP_THREAD_HPP

#define AMBIENT_OCCLUSION_GRADIENTS

#include "blocks.hpp"
#include "lighttable.hpp"
#include "precompiler.hpp"
#include "renderconst.hpp"
#include "vertex_block.hpp"

namespace cppcraft
{
	class PrecompThreadData;
	
	class PrecompThread
	{
	public:
		PrecompThread();
		~PrecompThread();
		
		PrecompThreadData* ptd;
		
		// stage 2, generating mesh
		void precompile(Precomp& pc);
		// stage 3, computing AO
		void ambientOcclusion(Precomp& pc);
		
		// ao gradient program, adding corner shadows to a completed mesh
		void ambientOcclusionGradients(Sector& sector, vertex_t* datadump, int vertexCount);
		// mesh optimizers
		void optimizeMesh(Precomp& pc, int shaderline, int txsize);
		void optimizeShadedMesh(Precomp& pc, int shaderline);
	};
	
}

#endif
