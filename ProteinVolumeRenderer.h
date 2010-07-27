/*
 * ProteinVolumeRenderer.h
 *
 * Copyright (C) 2008 by Universitaet Stuttgart (VIS). 
 * Alle Rechte vorbehalten.
 */

#ifndef MEGAMOLCORE_PROTEINVOLRENDERER_H_INCLUDED
#define MEGAMOLCORE_PROTEINVOLRENDERER_H_INCLUDED
#if (defined(_MSC_VER) && (_MSC_VER > 1000))
#pragma once
#endif /* (defined(_MSC_VER) && (_MSC_VER > 1000)) */

#include "slicing.h"
#include "CallProteinData.h"
#include "CallVolumeData.h"
#include "MolecularDataCall.h"
#include "CallFrame.h"
#include "param/ParamSlot.h"
#include "CallerSlot.h"
#include "view/Renderer3DModule.h"
#include "view/CallRender3D.h"
#include "vislib/GLSLShader.h"
#include "vislib/SimpleFont.h"
#include "vislib/FramebufferObject.h"
#include <vector>

#define CHECK_FOR_OGL_ERROR() do { GLenum err; err = glGetError();if (err != GL_NO_ERROR) { fprintf(stderr, "%s(%d) glError: %s\n", __FILE__, __LINE__, gluErrorString(err)); } } while(0)

namespace megamol {
namespace protein {

	/**
	 * Protein Renderer class
	 */
	class ProteinVolumeRenderer : public megamol::core::view::Renderer3DModule
	{
	public:
		/**
		 * Answer the name of this module.
		 *
		 * @return The name of this module.
		 */
		static const char *ClassName(void) {
			return "ProteinVolumeRenderer";
		}

		/**
		 * Answer a human readable description of this module.
		 *
		 * @return A human readable description of this module.
		 */
		static const char *Description(void) {
			return "Offers protein volume renderings.";
		}

		/**
		 * Answers whether this module is available on the current system.
		 *
		 * @return 'true' if the module is available, 'false' otherwise.
		 */
		static bool IsAvailable(void) {
			return true;
		}

		/** Ctor. */
		ProteinVolumeRenderer(void);

		/** Dtor. */
		virtual ~ProteinVolumeRenderer(void);
		
		enum ColoringMode {
			ELEMENT   = 0,
			AMINOACID = 1,
			STRUCTURE = 2,
			VALUE     = 3,
			CHAIN_ID  = 4,
			RAINBOW   = 5,
			CHARGE    = 6
		};

	   /**********************************************************************
		 * 'get'-functions
	    **********************************************************************/

		/** Get the color of a certain atom of the protein. */
		const unsigned char * GetProteinAtomColor( unsigned int idx) { return &this->protAtomColorTable[idx*3]; };

	   /**********************************************************************
		 * 'set'-functions
	    **********************************************************************/

		/** Set current coloring mode */
        inline void SetColoringMode( ColoringMode cm) { currentColoringMode = cm; };

	protected:
		
		/**
		 * Implementation of 'Create'.
		 *
		 * @return 'true' on success, 'false' otherwise.
		 */
		virtual bool create(void);
		
		/**
		 * Implementation of 'release'.
		 */
		virtual void release(void);

	private:

		/**********************************************************************
		 * 'render'-functions
		 **********************************************************************/
		
        /**
         * The get capabilities callback. The module should set the members
         * of 'call' to tell the caller its capabilities.
         *
         * @param call The calling call.
         *
         * @return The return value of the function.
         */
        virtual bool GetCapabilities( megamol::core::Call& call);

        /**
         * The get extents callback. The module should set the members of
         * 'call' to tell the caller the extents of its data (bounding boxes
         * and times).
         *
         * @param call The calling call.
         *
         * @return The return value of the function.
         */
        virtual bool GetExtents( megamol::core::Call& call);

		/**
		* The Open GL Render callback.
		*
		* @param call The calling call.
		* @return The return value of the function.
		*/
		virtual bool Render( megamol::core::Call& call);
		
		/**
         * Volume rendering using protein data.
		*/
        bool RenderProteinData( megamol::core::view::CallRender3D *call, CallProteinData *protein);
		
		/**
         * Volume rendering using volume data.
		*/
        bool RenderVolumeData( megamol::core::view::CallRender3D *call, CallVolumeData *volume);
		
		/**
         * Volume rendering using molecular data.
		*/
		bool RenderMolecularData( megamol::core::view::CallRender3D *call, MolecularDataCall *mol);
		
		/**
         * Refresh all parameters.
		*/
        void ParameterRefresh();
		
		/**
		* Draw label for current loaded RMS frame.
		*
		* @param call Ths calling CallFrame.
		*/
		void DrawLabel(unsigned int frameID);
		
		/**
		* The CallFrame callback.
		*
		* @param call The calling call.
		* @return The return value of the function.
		*/
		bool ProcessFrameRequest( megamol::core::Call& call);

		/** fill amino acid color table */
		void FillAminoAcidColorTable(void);
		
		/**
		 * Creates a rainbow color table with 'num' entries.
		*
		 * @param num The number of color entries.
		*/
		void MakeRainbowColorTable( unsigned int num);
		
		/**
		 * Make color table for all atoms acoording to the current coloring mode.
		 * The color table is only computed if it is empty or if the recomputation 
		 * is forced by parameter.
		*
		* @param prot The data interface.
		 * @param forceRecompute Force recomputation of the color table.
		*/
		void MakeColorTable( const CallProteinData *prot, bool forceRecompute = false);
		
		/**
         * Create a volume containing all protein atoms.
		*
		* @param prot The data interface.
		*/
        void UpdateVolumeTexture( const CallProteinData *protein);
	
		/** 
         * Create a volume containing all molecule atoms.
		 *
		 * @param mol The data interface.
		*/
		void UpdateVolumeTexture( MolecularDataCall *mol);
		
		/**
         * Draw the volume.
		 *
		 * @param boundingbox The bounding box.
		 */
        void RenderVolume( vislib::math::Cuboid<float> boundingbox);
		
		/**
         * Write the parameters of the ray to the textures.
		 *
		 * @param boundingbox The bounding box.
		 */
        void RayParamTextures( vislib::math::Cuboid<float> boundingbox);

        /**
         * Create a volume containing the voxel map.
		 *
		 * @param volume The data interface.
         */
        void UpdateVolumeTexture( const CallVolumeData *volume);

        /**
         * Draw the volume.
		 *
		 * @param volume The data interface.
         */
        void RenderVolume( const CallVolumeData *volume);

        /**
         * Write the parameters of the ray to the textures.
		 *
		 * @param volume The data interface.
         */
        void RayParamTextures( const CallVolumeData *volume);

        /**
         * Draw the bounding box of the protein.
		 *
		 * @paramboundingbox The bounding box.
         */
        void DrawBoundingBoxTranslated( vislib::math::Cuboid<float> boundingbox);

        /**
         * Draw the bounding box of the protein around the origin.
		 *
		 * @param boundingbox The bounding box.
         */
        void DrawBoundingBox( vislib::math::Cuboid<float> boundingbox);

        /**
         * Draw the clipped polygon for correct clip plane rendering.
         *
         * @param boundingbox The bounding box.
         */
        void drawClippedPolygon( vislib::math::Cuboid<float> boundingbox);

		/**********************************************************************
		 * variables
		 **********************************************************************/
		
		// caller slot
		megamol::core::CallerSlot protDataCallerSlot;
		// callee slot
		megamol::core::CalleeSlot callFrameCalleeSlot;
		// caller slot
		megamol::core::CallerSlot protRendererCallerSlot;
		
		// 'true' if there is rms data to be rendered
		bool renderRMSData;
		
		// label with id of current loaded frame
		vislib::graphics::AbstractFont *frameLabel;
		
		// camera information
		vislib::SmartPtr<vislib::graphics::CameraParameters> cameraInfo;
        // scaling factor for the scene
        float scale;
        // translation of the scene
        vislib::math::Vector<float, 3> translation;
		
		megamol::core::param::ParamSlot coloringModeParam;
		// parameters for the volume rendering
		megamol::core::param::ParamSlot volIsoValue1Param;
		megamol::core::param::ParamSlot volIsoValue2Param;
		megamol::core::param::ParamSlot volFilterRadiusParam;
		megamol::core::param::ParamSlot volDensityScaleParam;
		megamol::core::param::ParamSlot volIsoOpacityParam;
        megamol::core::param::ParamSlot volClipPlaneFlagParam;
        megamol::core::param::ParamSlot volClipPlane0NormParam;
        megamol::core::param::ParamSlot volClipPlane0DistParam;
        megamol::core::param::ParamSlot volClipPlaneOpacityParam;


		// shader for the spheres (raycasting view)
		vislib::graphics::gl::GLSLShader sphereShader;
		// shader for the cylinders (raycasting view)
		vislib::graphics::gl::GLSLShader cylinderShader;
		// shader for the clipped spheres (raycasting view)
		vislib::graphics::gl::GLSLShader clippedSphereShader;
        // shader for volume texture generation
        vislib::graphics::gl::GLSLShader updateVolumeShader;
        // shader for volume rendering
        vislib::graphics::gl::GLSLShader volumeShader;
        vislib::graphics::gl::GLSLShader dualIsosurfaceShader;
        vislib::graphics::gl::GLSLShader volRayStartShader;
        vislib::graphics::gl::GLSLShader volRayStartEyeShader;
        vislib::graphics::gl::GLSLShader volRayLengthShader;
		
		// current coloring mode
		ColoringMode currentColoringMode;
		
		// attribute locations for GLSL-Shader
		GLint attribLocInParams;
		GLint attribLocQuatC;
		GLint attribLocColor1;
		GLint attribLocColor2;
		
		// color table for amino acids
		vislib::Array<vislib::math::Vector<unsigned char, 3> > aminoAcidColorTable;
		// color palette vector: stores the color for chains
		std::vector<vislib::math::Vector<float,3> > rainbowColors;
		// color table for protein atoms
		vislib::Array<unsigned char> protAtomColorTable;
		
		// the Id of the current frame (for dynamic data)
		unsigned int currentFrameId;

        // the number of protein atoms
        unsigned int atomCount;

        // FBO for rendering the protein
        vislib::graphics::gl::FramebufferObject proteinFBO;

        // volume texture
        GLuint volumeTex;
        unsigned int volumeSize;
        // FBO for volume generation
        GLuint volFBO;
        // volume parameters
        float volFilterRadius;
        float volDensityScale;
        float volScale[3];
        float volScaleInv[3];
        // width and height of view
        unsigned int width, height;
        // current width and height of textures used for ray casting
        unsigned int volRayTexWidth, volRayTexHeight;
        // volume ray casting textures
        GLuint volRayStartTex;
        GLuint volRayLengthTex;
        GLuint volRayDistTex;

        // render the volume as isosurface
        bool renderIsometric;
        // the average density value of the volume
        float meanDensityValue;
        // the first iso value
        float isoValue1;
        // the second iso value
        float isoValue2;
		// the opacity of the isosurface
		float volIsoOpacity;

        vislib::math::Vector<float, 3> protrenTranslate;
        float protrenScale;

        // flag wether clipping planes are enabled
        bool volClipPlaneFlag;
        // the array of clipping planes
        vislib::Array<vislib::math::Vector<double, 4> > volClipPlane;
        // view aligned slicing
        ViewSlicing slices;
		// the opacity of the clipping plane
		float volClipPlaneOpacity;

	};


} /* end namespace protein */
} /* end namespace megamol */

#endif // MEGAMOLCORE_PROTEINVOLRENDERER_H_INCLUDED