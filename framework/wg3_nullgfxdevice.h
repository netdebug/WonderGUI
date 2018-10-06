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

#ifndef WG3_NULLGFXDEVICE_DOT_H
#define WG3_NULLGFXDEVICE_DOT_H
#pragma once

#include <wg3_types.h>
#include <wg3_gfxdevice.h>

namespace wg 
{
	
	class Surface;
	class Rect;
	class Color;
	
	class NullGfxDevice;
	typedef	StrongPtr<NullGfxDevice>	NullGfxDevice_p;
	typedef	WeakPtr<NullGfxDevice>	NullGfxDevice_wp;
	
	class NullGfxDevice : public GfxDevice
	{
	public:
		//.____ Creation ________________________________________________

		static NullGfxDevice_p	create( Size size );
	
		//.____ Identification __________________________________________

		bool					isInstanceOf( const char * pClassName ) const;
		const char *			className( void ) const;
		static const char		CLASSNAME[];
		static NullGfxDevice_p	cast( Object * pObject );
		const char *			surfaceClassName( void ) const;

		//.____ Misc _______________________________________________________

		SurfaceFactory_p		surfaceFactory();

		//.____ Geometry _________________________________________________

		bool		setCanvas(Surface * pCanvas);

		//.____ Rendering ________________________________________________

		void	fill( const Rect& rect, const Color& col ) override;
		void	blit( Surface * src, const Rect& srcrect, Coord dest  ) override;
		void	tileBlit( Surface * src, const Rect& srcrect, const Rect& dest ) override;
	
		void	fillSubPixel( const RectF& rect, const Color& col )  override;
		void	stretchBlit( Surface * pSrc, const RectF& source, const Rect& dest ) override;
	
        void    plotPixels( int nCoords, const Coord * pCoords, const Color * pColors) override;
        void    clipPlotPixels( const Rect& clip, int nCoords, const Coord * pCoords, const Color * pColors) override;
	
		void	drawLine( Coord begin, Coord end, Color color, float thickness = 1.f ) override;
	
		void	clipDrawLine( const Rect& clip, Coord begin, Coord end, Color color, float thickness = 1.f );

		void	clipDrawHorrWave(const Rect&clip, Coord begin, int length, const WaveLine * pTopLine, const WaveLine * pBottomLine, Color front, Color back);

		void	clipDrawElipse(const Rect&clip, const RectF& canvas, float thickness, Color fill, float outlineThickness = 0, Color outlineColor = Color::Black);

	protected:
		NullGfxDevice( Size size );
		~NullGfxDevice();

		void	_drawStraightLine(Coord start, Orientation orientation, int _length, const Color& _col) override;

	};
	

} // namespace wg
#endif //WG3_NULLGFXDEVICE_DOT_H

