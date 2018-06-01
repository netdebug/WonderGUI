/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG3_SURFACEFACTORY_DOT_H
#define	WG3_SURFACEFACTORY_DOT_H
#pragma once
//==============================================================================

#include <wg3_surface.h>
#include <wg3_blob.h>


namespace wg 
{
	
	
	class SurfaceFactory;
	typedef	StrongPtr<SurfaceFactory>	SurfaceFactory_p;
	typedef	WeakPtr<SurfaceFactory>	SurfaceFactory_wp;
	
	
	//____ SurfaceFactory _______________________________________________________
	/**
	 * @brief Factory class for creating surfaces.
	 *
	 * SurfaceFactory is the base class for all surface factories. The surface factories
	 * are used by WonderGUI components that needs to dynamically create surfaces as
	 * part of their operation, like FreeTypeFont.
	 *
	 **/
	class SurfaceFactory : public Object
	{
	public:
		//.____ Identification __________________________________________

		bool					isInstanceOf( const char * pClassName ) const;
		const char *			className( void ) const;
		static const char		CLASSNAME[];
		static SurfaceFactory_p	cast( Object * pObject );

		//.____ Geometry _________________________________________________

		virtual Size		maxSize() const = 0;

		//.____ Misc _______________________________________________________

		virtual Surface_p	createSurface( Size size, PixelType type = PixelType::BGRA_8, int hint = SurfaceHint::Static ) const = 0;
        virtual Surface_p	createSurface( Size size, PixelType type, Blob * pBlob, int pitch, int hint = SurfaceHint::Static ) const = 0;
        virtual Surface_p	createSurface( Size size, PixelType type, uint8_t * pPixels, int pitch, const PixelFormat * pPixelFormat = 0, int hint = SurfaceHint::Static ) const = 0;
        virtual Surface_p	createSurface( Surface * pOther, int hint = SurfaceHint::Static ) const = 0;

	protected:
		virtual ~SurfaceFactory() {}
	};
	
	
	//==============================================================================

} // namespace wg
#endif // WG_SURFACE_DOT_H