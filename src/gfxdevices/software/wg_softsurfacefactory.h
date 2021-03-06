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
#ifndef WG_SURFACEFACTORY_SOFT_DOT_H
#define WG_SURFACEFACTORY_SOFT_DOT_H

#include <wg_softsurface.h>
#include <wg_surfacefactory.h>

namespace wg
{

	class SoftSurfaceFactory;
	typedef	StrongPtr<SoftSurfaceFactory>	SoftSurfaceFactory_p;
	typedef	WeakPtr<SoftSurfaceFactory>	SoftSurfaceFactory_wp;

	//____ SoftSurfaceFactory _______________________________________________________

	class SoftSurfaceFactory : public SurfaceFactory
	{
	public:
		//.____ Creation __________________________________________

		static SoftSurfaceFactory_p	create() { return SoftSurfaceFactory_p(new SoftSurfaceFactory()); }

		//.____ Identification __________________________________________

		bool							isInstanceOf( const char * pClassName ) const override;
		const char *					className( void ) const override;
		static const char				CLASSNAME[];
		static SoftSurfaceFactory_p	cast( Object * pObject );

		//.____ Geometry _________________________________________________

		Size				maxSize() const override;

		//.____ Misc _______________________________________________________

		virtual Surface_p	createSurface( Size size, PixelFormat format = PixelFormat::BGRA_8, int flags = SurfaceFlag::Static, const Color * pClut = nullptr) const override;
		virtual Surface_p	createSurface( Size size, PixelFormat format, Blob * pBlob, int pitch, int flags = SurfaceFlag::Static, const Color * pClut = nullptr) const override;
		virtual Surface_p	createSurface( Size size, PixelFormat format, uint8_t * pPixels, int pitch, const PixelDescription * pPixelDescription = 0, int flags = SurfaceFlag::Static, const Color * pClut = nullptr) const override;
		virtual Surface_p	createSurface( Surface * pOther, int flags = SurfaceFlag::Static ) const override;

	protected:
		virtual ~SoftSurfaceFactory() {}
	};

	//========================================================================================

} // namespace wg
#endif // WG_SOFTSURFACEFACTORY_DOT_H
