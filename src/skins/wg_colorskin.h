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
#ifndef WG_COLORSKIN_DOT_H
#define WG_COLORSKIN_DOT_H
#pragma once

#include <wg_extendedskin.h>
#include <wg_color.h>

#include <initializer_list>
#include <utility>

namespace wg
{

	class ColorSkin;
	typedef	StrongPtr<ColorSkin>	ColorSkin_p;

	class ColorSkin : public ExtendedSkin
	{
	public:
		//.____ Creation __________________________________________

		static ColorSkin_p	create();
		static ColorSkin_p 	create(Color color );
		static ColorSkin_p	create(std::initializer_list< std::tuple<State,Color> > stateColors );

		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static ColorSkin_p	cast( Object * pObject );

		//.____ Rendering ________________________________________________

		void		render( GfxDevice * pDevice, const Rect& _canvas, State state ) const;

		//.____ Appearance _________________________________________________

		void		setBlendMode(BlendMode mode);
		BlendMode	blendMode() const { return m_blendMode; }


		void		setColor(Color fill);
		void		setColor(State state, Color fill);
		void		setColor(std::initializer_list< std::tuple<State, Color> > stateColors);
		Color		color(State state) const;


		//.____ Misc ____________________________________________________

		bool		markTest( const Coord& ofs, const Rect& canvas, State state, int opacityTreshold ) const;

		bool		isOpaque() const;
		bool		isOpaque( State state ) const;
		bool		isOpaque( const Rect& rect, const Size& canvasSize, State state ) const;

		bool		isStateIdentical( State state, State comparedTo ) const;

	private:
		ColorSkin();
		ColorSkin(Color color);
		~ColorSkin() {};

		void	_updateOpaqueFlag();
		void	_updateUnsetColors();

		bool		m_bOpaque;
		BlendMode	m_blendMode = BlendMode::Blend;

		Bitmask<uint32_t>	m_stateColorMask = 1;

		Color		m_color[StateEnum_Nb];
	};


} // namespace wg
#endif //WG_COLORSKIN_DOT_H


