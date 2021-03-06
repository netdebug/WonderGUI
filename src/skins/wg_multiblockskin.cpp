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

#include <wg_multiblockskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>

#include <assert.h>

namespace wg
{
	using namespace Util;

	const char MultiBlockSkin::CLASSNAME[] = { "MultiBlockSkin" };

	//____ create() _______________________________________________________________

	MultiBlockSkin_p MultiBlockSkin::create(Size blockSize, Border frame)
	{
		return MultiBlockSkin_p(new MultiBlockSkin(blockSize,frame));
	}


	//____ Constructor ____________________________________________________________

	MultiBlockSkin::MultiBlockSkin(Size blockSize, Border frame)
	{
		m_blockSizePoints	= blockSize;
		m_frame				= frame;
		m_bIsOpaque			= false;

		for( int i = 0 ; i < StateEnum_Nb ; i++ )
		m_bStateOpaque[i] = false;
	}

	//____ isInstanceOf() _________________________________________________________

	bool MultiBlockSkin::isInstanceOf(const char * pClassName) const
	{
		if (pClassName == CLASSNAME)
			return true;

		return ExtendedSkin::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * MultiBlockSkin::className(void) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	MultiBlockSkin_p MultiBlockSkin::cast(Object * pObject)
	{
		if (pObject && pObject->isInstanceOf(CLASSNAME))
			return MultiBlockSkin_p(static_cast<MultiBlockSkin*>(pObject));

		return 0;
	}

	//____ addLayer() _____________________________________________________________

	int MultiBlockSkin::addLayer(Surface * pSurf, Coord ofs)
	{
		// HACK!

//		m_scale = pSurf->ScaleFactor();
//		m_blockSize = (m_blockSizePoints*m_scale) / WG_SCALE_BASE;
		m_blockSize = m_blockSizePoints;

		//

		m_layers.emplace_back();
		auto& layer = m_layers.back();

		layer.blendMode = BlendMode::Blend;
		layer.pSurface = pSurf;
		layer.stateBlockMask = 1;               // Only normal state is set.

		if (pSurf->isOpaque())
			m_bIsOpaque = true;


		for (int i = 0; i < StateEnum_Nb; i++)
		{
			layer.blockOfs[i] = ofs;
			layer.tintColor[i] = Color::White;

			_updateStateOpacity(i);
		}


		return (int) m_layers.size();
	}

	int MultiBlockSkin::addLayer(Surface * pSurf, Coord blockStartOfs, Coord blockPitch, const std::initializer_list<State>& stateBlocks)
	{
		// HACK!

//		m_scale = pSurf->ScaleFactor();
//		m_blockSize = (m_blockSizePoints*m_scale) / WG_SCALE_BASE;
		m_blockSize = m_blockSizePoints;

		//

		m_layers.emplace_back();
		auto& layer = m_layers.back();

		layer.blendMode = BlendMode::Blend;
		layer.pSurface = pSurf;
		layer.stateBlockMask = 0;

		if (pSurf->isOpaque())
			m_bIsOpaque = true;

		//

		for (int i = 0; i < StateEnum_Nb; i++)
			layer.tintColor[i] = Color::White;

		//

		int ofs = 0;
		for (StateEnum state : stateBlocks)
		{
			int index = _stateToIndex(state);

			layer.stateBlockMask.setBit(index);
			layer.blockOfs[index] = blockStartOfs + blockPitch*ofs;
			ofs++;
		}

		//

		assert(layer.stateBlockMask.bit(0) == true);				// A block for state normal is required.

		// Fill in fallback states and update opacity flag

		for (int i = 0; i < StateEnum_Nb; i++)
		{
			if (!layer.stateBlockMask.bit(i))
			{
				int fallbackIndex = bestStateIndexMatch(i, layer.stateBlockMask);
				layer.blockOfs[i] = layer.blockOfs[fallbackIndex];
			}

			_updateStateOpacity(i);
		}

		//

		return (int) m_layers.size();
	}

	int MultiBlockSkin::addLayer(Surface * pSurf, const std::initializer_list<State>& stateBlocks, Orientation orientation, int spacing, Coord blockStartOfs )
	{
		Coord blockPitch;

		if (orientation == Orientation::Horizontal)
			blockPitch.x = m_blockSizePoints.w + spacing;
		else
			blockPitch.y = m_blockSizePoints.h + spacing;

		return addLayer(pSurf, blockStartOfs, blockPitch, stateBlocks);
	}



	//____ setLayerTint() _________________________________________________________

	bool MultiBlockSkin::setLayerTint(int layerIdx, Color tintColor)
	{
		auto& layer = m_layers.at(layerIdx-1);

		for (int i = 0; i < StateEnum_Nb; i++)
		{
			uint8_t		oldAlpha = layer.tintColor[i].a;

			layer.tintColor[i] = tintColor;

			if (oldAlpha != layer.tintColor[i].a)
				_updateStateOpacity(i);
		}
		return true;
	}


	bool MultiBlockSkin::setLayerTint(int layerIdx, const std::initializer_list< std::pair<State,Color> >& stateColors)
	{
		auto& layer = m_layers.at(layerIdx-1);

		// Set default color for normal state.

		layer.tintColor[0] = Color::White;

		//

		for (auto& stateColor : stateColors)
		{
			int index = _stateToIndex(stateColor.first);

			uint8_t		oldAlpha = layer.tintColor[index].a;

			layer.tintColor[index] = stateColor.second;

			if (oldAlpha != stateColor.second.a)
				_updateStateOpacity(index);

			layer.stateColorMask.setBit(index);
		}

		// Fill in fallback states and update opacity flag

		for (int i = 0; i < StateEnum_Nb; i++)
		{
			if (!layer.stateColorMask.bit(i))
			{
				uint8_t		oldAlpha = layer.tintColor[i].a;

				int fallbackIndex = bestStateIndexMatch(i, layer.stateBlockMask);
				layer.tintColor[i] = layer.tintColor[fallbackIndex];

				if (oldAlpha != layer.tintColor[i].a )
					_updateStateOpacity(i);
			}
		}

		//

		return true;
	}

	//____ SetLayerBlendMode() ____________________________________________________

	bool MultiBlockSkin::setLayerBlendMode(int layerIdx, BlendMode blendMode)
	{
		auto& layer = m_layers.at(layerIdx - 1);

		if (layer.blendMode != blendMode)
		{
			layer.blendMode = blendMode;

			for (int i = 0; i < StateEnum_Nb; i++)
				_updateStateOpacity(i);
		}

		return true;
	}



	//____ render() _______________________________________________________________

	void MultiBlockSkin::render( GfxDevice * pDevice, const Rect& _canvas, State state ) const
	{
		if (m_layers.empty() || m_blockSize.w <= 0 || m_blockSize.h <= 0 )
			return;

		int stateIndex = _stateToIndex(state);

		BlendMode orgBlendMode = pDevice->blendMode();
		Color		orgTintColor = pDevice->tintColor();

		BlendMode blendMode = orgBlendMode;
		Color		tintColor = orgTintColor;
		Color		mixedTint = orgTintColor;

		for (auto& layer : m_layers)
		{
			if (layer.blendMode != blendMode)
			{
				blendMode = layer.blendMode;
				pDevice->setBlendMode(blendMode);
			}

			if (layer.tintColor[stateIndex] != tintColor )
			{
				tintColor = layer.tintColor[stateIndex];
				mixedTint = orgTintColor * tintColor;
				pDevice->setTintColor(mixedTint);
			}

			pDevice->setBlitSource(layer.pSurface);

			const Rect&	src = Rect(layer.blockOfs[stateIndex], m_blockSize);

			// Shortcuts & optimizations for common special cases.

			if (src.w == _canvas.w && src.h == _canvas.h /*&& scale == m_scale */)
			{
				pDevice->blit( _canvas.pos(), src );
				continue;
			}

			if (m_frame.left + m_frame.top + m_frame.right + m_frame.bottom == 0)
			{
				pDevice->stretchBlit( _canvas, src );
				continue;
			}

//			const Border&    sourceBorders = m_frame.scale(m_scale);
//			const Border     canvasBorders = m_frame.scale(scale);
			const Border&    sourceBorders = m_frame;
			const Border     canvasBorders = m_frame;

			pDevice->blitNinePatch( _canvas, canvasBorders, src, sourceBorders );
		}

		if (mixedTint != orgTintColor)
			pDevice->setTintColor(orgTintColor);

		if (blendMode != orgBlendMode)
			pDevice->setBlendMode(orgBlendMode);
	}


	//____ minSize() ______________________________________________________________

	Size MultiBlockSkin::minSize() const
	{
		Size content = ExtendedSkin::minSize();
//		Size frame = m_frame.scale(scale).size();
		Size frame = m_frame.size();
		return Size( max(content.w, frame.w), max(content.h, frame.h) );
	}

	//____ preferredSize() ________________________________________________________

	Size MultiBlockSkin::preferredSize() const
	{
		Size sz = ExtendedSkin::preferredSize();
		return Size( max(m_blockSize.w,sz.w), max(m_blockSize.h,sz.h) );
	}

	//____ sizeForContent() _______________________________________________________

	Size MultiBlockSkin::sizeForContent( const Size contentSize ) const
	{
		Size sz = ExtendedSkin::sizeForContent(contentSize);
		Size min = m_frame.size();

		return Size( max(sz.w,min.w), max(sz.h,min.h) );
	}

	//____ markTest() _____________________________________________________________

	bool MultiBlockSkin::markTest( const Coord& _ofs, const Rect& canvas, State state, int opacityTreshold ) const
	{
		if (!canvas.contains(_ofs) || m_layers.empty() || m_blockSize.w <= 0 || m_blockSize.h <= 0)
			return false;

		int stateIndex = _stateToIndex(state);

		for (auto& layer : m_layers)
		{
			int alpha = 0;
			if (isOpaque(state))
				alpha = 255;
			else if( layer.blendMode == BlendMode::Blend )
			{
				Coord ofs = _ofs - canvas.pos();

//				Border canvasFrame = m_frame.scale(scale);
//				Border sourceFrame = m_frame.scale(m_scale);
				Border canvasFrame = m_frame;
				Border sourceFrame = m_frame;

				// Determine in which section the cordinate is (0-2 for x and y).

				int	xSection = 0;
				int ySection = 0;

				if (ofs.x >= canvas.w - canvasFrame.right)
					xSection = 2;
				else if (ofs.x > canvasFrame.left)
					xSection = 1;

				if (ofs.y >= canvas.h - canvasFrame.bottom)
					ySection = 2;
				else if (ofs.y > canvasFrame.top)
					ySection = 1;

				// Convert ofs.x to X-offset in bitmap, taking stretch/tile section into account.

				if (xSection == 0)
				{
//					ofs.x = (ofs.x * m_scale) / scale;
				}
				else if (xSection == 2)
				{
					ofs.x = ofs.x - (canvas.w - canvasFrame.right);           // Offset in right border of canvas
//					ofs.x = (ofs.x * m_scale) / scale;            // Scale from canvas to source coordinates
					ofs.x += m_blockSize.w - sourceFrame.right;          // Add offset for right border
				}
				else if (xSection == 1)
				{
					int tileAreaWidth = m_blockSize.w - sourceFrame.width();

					int canvasStretchWidth = canvas.w - canvasFrame.width();	// Width of stretch-area on screen.

					ofs.x = ofs.x - canvasFrame.left;               // Offset in middle section of canvas
//					ofs.x = (ofs.x * m_scale) / scale;        // Scale from canvas to source offset
					ofs.x = (int)((ofs.x / (float)canvasStretchWidth)*tileAreaWidth) + sourceFrame.left;
				}


				// Convert ofs.y to Y-offset in bitmap, taking stretch/tile section into account.

				if (ySection == 0)
				{
//					ofs.y = (ofs.y * m_scale) / scale;
				}
				if (ySection == 2)
				{
					ofs.y = ofs.y - (canvas.h - canvasFrame.bottom);           // Offset in bottom border of canvas
//					ofs.y = (ofs.y * m_scale) / scale;            // Scale from canvas to source coordinates
					ofs.y += m_blockSize.h - sourceFrame.bottom;          // Add offset for bottom border
				}
				else if (ySection == 1)
				{
					int tileAreaHeight = m_blockSize.h - sourceFrame.height();

					int canvasStretchHeight = canvas.h - canvasFrame.height();	// Height of stretch-area on screen.

					ofs.y = ofs.y - canvasFrame.top;               // Offset in middle section of canvas
//					ofs.y = (ofs.y * m_scale) / scale;        // Scale from canvas to source offset
					ofs.y = (int)((ofs.y / (float)canvasStretchHeight)*tileAreaHeight) + sourceFrame.top;
				}

				Coord srcOfs = layer.blockOfs[stateIndex];

				alpha = layer.pSurface->alpha(srcOfs.x + ofs.x, srcOfs.y + ofs.y);
			}

			if (alpha >= opacityTreshold)
				return true;
		}

		return false;
	}

	//____ isOpaque() _____________________________________________________________

	bool MultiBlockSkin::isOpaque() const
	{
		return m_bIsOpaque;
	}

	bool MultiBlockSkin::isOpaque( State state ) const
	{
		return m_bIsOpaque;
	}

	bool MultiBlockSkin::isOpaque( const Rect& rect, const Size& canvasSize, State state ) const
	{
		return m_bIsOpaque;
	}

	//____ isStateIdentical() _____________________________________________________

	bool MultiBlockSkin::isStateIdentical( State state, State comparedTo ) const
	{
		int i1 = _stateToIndex(state);
		int i2 = _stateToIndex(comparedTo);

		for (auto& layer : m_layers)
		{
			if (layer.blockOfs[i1] != layer.blockOfs[i2] || layer.tintColor[i1] != layer.tintColor[i2])
				return false;
		}

		return ExtendedSkin::isStateIdentical(state, comparedTo);
	}

	//____ _updateStateOpacity() __________________________________________________

	void MultiBlockSkin::_updateStateOpacity(int stateIdx)
	{
		bool bOpaque = false;

		for (auto& layer : m_layers)
		{
			if (layer.blendMode == BlendMode::Replace || (layer.blendMode == BlendMode::Blend && layer.pSurface->isOpaque() && layer.tintColor[stateIdx].a == 255) )
			{
				bOpaque = true;
				break;
			}
		}

		if (bOpaque != m_bStateOpaque[stateIdx])
		{
			m_bStateOpaque[stateIdx] = bOpaque;

			if (bOpaque != m_bIsOpaque)
			{
				m_bIsOpaque = true;
				for( int i = 0 ; i < StateEnum_Nb ; i++ )
					if (m_bStateOpaque[i] )
					{
						m_bIsOpaque = false;
						break;
					}
			}
		}
	}

}
