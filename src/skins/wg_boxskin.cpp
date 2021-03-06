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

#include <wg_boxskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>

namespace wg
{

	using namespace Util;

	const char BoxSkin::CLASSNAME[] = {"BoxSkin"};

	//____ create() _______________________________________________________________

	BoxSkin_p BoxSkin::create()
	{
		return BoxSkin_p(new BoxSkin());
	}

	BoxSkin_p BoxSkin::create(Border frame, Color fillColor, Color frameColor )
	{
		return BoxSkin_p(new BoxSkin(frame, fillColor, frameColor));
	}

	BoxSkin_p BoxSkin::create(Border frame, std::initializer_list< std::tuple<State, Color, Color> > stateColors )
	{
		BoxSkin_p p = new BoxSkin();

		p->setFrame(frame);
		p->setColors(stateColors);

		return p;
	}


	//____ Constructor ____________________________________________________________

	BoxSkin::BoxSkin()
	{
		m_stateColorMask = 1;

		for( int i = 0 ; i < StateEnum_Nb ; i++ )
		{
			m_fillColor[i] = Color::White;
			m_frameColor[i] = Color::White;
		}

		m_bOpaque = true;
	}

	BoxSkin::BoxSkin(Border frame, Color fillColor, Color frameColor )
	{
		m_frame = frame;
		setColors(fillColor, frameColor);
	}

	//____ isInstanceOf() _________________________________________________________

	bool BoxSkin::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;

		return ExtendedSkin::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * BoxSkin::className( void ) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	BoxSkin_p BoxSkin::cast( Object * pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return BoxSkin_p( static_cast<BoxSkin*>(pObject) );

		return 0;
	}

	//____ setBlendMode() _____________________________________________________

	void BoxSkin::setBlendMode(BlendMode mode)
	{
		m_blendMode = mode;
		_updateOpaqueFlag();
	}

	//____ setFrame() ____________________________________________________

	void BoxSkin::setFrame(Border frame)
	{
		bool hadFrame = (m_frame.width() + m_frame.height() > 0);
		bool hasFrame = (frame.width() + frame.height() > 0);

		m_frame = frame;

		if (hadFrame != hasFrame)
			_updateOpaqueFlag();
	}

	//____ setColors() ________________________________________________________

	void BoxSkin::setColors(Color fill, Color frame)
	{
		m_stateColorMask = 1;

		for (int i = 0; i < StateEnum_Nb; i++)
		{
			m_fillColor[i] = fill;
			m_frameColor[i] = frame;
		}

		bool hasFrame = (m_frame.width() + m_frame.height() > 0);
		if (fill.a == 255 && (!hasFrame || frame.a == 255))
			m_bOpaque = true;
		else
			m_bOpaque = false;
	}

	void BoxSkin::setColors(State state, Color fill, Color frame)
	{
		int i = _stateToIndex(state);

		m_stateColorMask.setBit(i);

		m_fillColor[i] = fill;
		m_frameColor[i] = frame;

		_updateUnsetColors();
		_updateOpaqueFlag();
	}

	void BoxSkin::setColors(std::initializer_list< std::tuple<State, Color, Color> > stateColors)
	{
		for (auto& state : stateColors)
		{
			int i = _stateToIndex(std::get<0>(state));
			m_stateColorMask.setBit(i);
			m_fillColor[i] = std::get<1>(state);
			m_frameColor[i] = std::get<2>(state);
		}

		_updateUnsetColors();
		_updateOpaqueFlag();
	}

	//____ colors() ______________________________________________________

	std::tuple<Color, Color> BoxSkin::colors(State state) const
	{
		int i = _stateToIndex(state);
		return std::make_tuple(m_fillColor[i], m_frameColor[i]);
	}

	//____ render() _______________________________________________________________

	void BoxSkin::render( GfxDevice * pDevice, const Rect& _canvas, State state ) const
	{
		//TODO: Optimize! Clip patches against canvas first.

		BlendMode	oldBlendMode = pDevice->blendMode();

		if (m_blendMode != oldBlendMode )
			pDevice->setBlendMode(m_blendMode);

		int i = _stateToIndex(state);
		if( m_frame.width() + m_frame.height() == 0 || m_frameColor[i] == m_fillColor[i] )
		{
			pDevice->fill( _canvas, m_fillColor[i] );
		}
		else
		{
			Rect top( _canvas.x, _canvas.y, _canvas.w, m_frame.top );
			Rect left( _canvas.x, _canvas.y+m_frame.top, m_frame.left, _canvas.h - m_frame.height() );
			Rect right( _canvas.x + _canvas.w - m_frame.right, _canvas.y+m_frame.top, m_frame.right, _canvas.h - m_frame.height() );
			Rect bottom( _canvas.x, _canvas.y + _canvas.h - m_frame.bottom, _canvas.w, m_frame.bottom );
			Rect center( _canvas - m_frame );

			pDevice->fill( top, m_frameColor[i] );
			pDevice->fill( left, m_frameColor[i] );
			pDevice->fill( right, m_frameColor[i] );
			pDevice->fill( bottom, m_frameColor[i] );

			if( center.w > 0 || center.h > 0 )
				pDevice->fill( center, m_fillColor[i] );
		}

		if (m_blendMode != oldBlendMode)
			pDevice->setBlendMode(oldBlendMode);
	}

	//____ minSize() ______________________________________________________________

	Size BoxSkin::minSize() const
	{
		Size content = ExtendedSkin::minSize();
		Size frame = m_frame.size();

		return Size( wg::max(content.w,frame.w), wg::max(content.h,frame.h) );
	}

	//____ preferredSize() ________________________________________________________

	Size BoxSkin::preferredSize() const
	{
		Size content = ExtendedSkin::preferredSize();
		Size frame = m_frame.size();

		return Size( wg::max(content.w,frame.w), wg::max(content.h,frame.h) );
	}

	//____ sizeForContent() _______________________________________________________

	Size BoxSkin::sizeForContent( const Size contentSize ) const
	{
		Size content = ExtendedSkin::sizeForContent(contentSize);
		Size frame = m_frame.size();

		return Size( wg::max(content.w,frame.w), wg::max(content.h,frame.h) );
	}

	//____ markTest() _____________________________________________________________

	bool BoxSkin::markTest( const Coord& ofs, const Rect& canvas, State state, int opacityTreshold ) const
	{
		if( !canvas.contains(ofs) )
			return false;

		int opacity;

		if( m_bOpaque )
			opacity = 255;
		else
		{
			int i = _stateToIndex(state);

			Rect center = canvas - m_frame;
			if( center.contains(ofs) )
				opacity = m_fillColor[i].a;
			else
				opacity = m_frameColor[i].a;
		}

		return ( opacity >= opacityTreshold );
	}

	//____ isOpaque() _____________________________________________________________

	bool BoxSkin::isOpaque() const
	{
		return m_bOpaque;
	}

	bool BoxSkin::isOpaque( State state ) const
	{
		int i = _stateToIndex(state);
		if( m_bOpaque || (m_fillColor[i].a == 255 && (m_frameColor[i] == 255 || (m_frame.width() + m_frame.height() == 0))) )
			return true;

		return false;
	}

	bool BoxSkin::isOpaque( const Rect& rect, const Size& canvasSize, State state ) const
	{
		if( m_bOpaque )
			return true;

		Rect center = Rect(canvasSize) - m_frame;
		int i = _stateToIndex(state);
		if( center.contains(rect) )
			return m_fillColor[i].a == 255;
		else if( !center.intersectsWith(rect) )
			return m_frameColor[i].a == 255;

		return m_fillColor[i].a == 255 && m_frameColor[i].a == 255;
	}

	//____ isStateIdentical() ____________________________________________________

	bool BoxSkin::isStateIdentical( State state, State comparedTo ) const
	{
		int i1 = _stateToIndex(state);
		int i2 = _stateToIndex(comparedTo);

		if( m_fillColor[i1] == m_fillColor[i2] && (m_frame.isEmpty() || m_frameColor[i1] == m_frameColor[i2]) &&
			ExtendedSkin::isStateIdentical(state,comparedTo) )
			return true;
		else
			return false;
	}

	//____ _updateOpaqueFlag() ____________________________________________________

	void BoxSkin::_updateOpaqueFlag()
	{
		switch (m_blendMode)
		{
			case BlendMode::Replace:
				m_bOpaque = true;
				break;

			case BlendMode::Blend:
			{
				int alpha = 0;
				int frameAlpha = 0;

				for (int i = 0; i < StateEnum_Nb; i++)
				{
					alpha += (int)m_fillColor[i].a;
					frameAlpha += (int)m_frameColor[i].a;
				}

				bool hasFrame = (m_frame.width() + m_frame.height() > 0);

				if (alpha == 255 * StateEnum_Nb && (!hasFrame || frameAlpha == 255 * StateEnum_Nb))
					m_bOpaque = true;
				else
					m_bOpaque = false;

				break;
			}

			default:
				m_bOpaque = false;
		}
	}

	//____ _updateUnsetColors() _______________________________________________

	void BoxSkin::_updateUnsetColors()
	{
		for (int i = 0; i < StateEnum_Nb; i++)
		{
			if (!m_stateColorMask.bit(i))
			{
				int bestAlternative = bestStateIndexMatch(i, m_stateColorMask);
				m_fillColor[i] = m_fillColor[bestAlternative];
				m_frameColor[i] = m_frameColor[bestAlternative];
			}
		}
	}


} // namespace wg
