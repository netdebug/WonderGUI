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

//____ create() _______________________________________________________________

WgMultiBlockSkinPtr WgMultiBlockSkin::Create(WgSize blockSize, WgBorders frame)
{
	return WgMultiBlockSkinPtr(new WgMultiBlockSkin(blockSize,frame));
}


//____ Constructor ____________________________________________________________

WgMultiBlockSkin::WgMultiBlockSkin(WgSize blockSize, WgBorders frame)
{
	m_blockSize = blockSize;
	m_frame = frame;
	m_bIsOpaque = false;

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	m_bStateOpaque[i] = false;
}


//____ AddLayer() _____________________________________________________________

int WgMultiBlockSkin::AddLayer(WgSurface * pSurf, WgCoord ofs)
{
	m_layers.emplace_back();
	auto& layer = m_layers.back();

	layer.blendMode = WG_BLENDMODE_BLEND;
	layer.pSurface = pSurf;

	if (pSurf->IsOpaque())
		m_bIsOpaque = true;


	for (int i = 0; i < WG_NB_STATES; i++)
	{
		layer.blockOfs[i] = ofs;
		layer.tintColor[i] = WgColor::white;

		_updateStateOpacity(i);
	}


	return m_layers.size();
}

int WgMultiBlockSkin::AddLayer(WgSurface * pSurf, WgCoord blockStartOfs, WgSize blockPitch, std::initializer_list<WgState> stateBlocks)
{
	m_layers.emplace_back();
	auto& layer = m_layers.back();

	layer.blendMode = WG_BLENDMODE_BLEND;
	layer.pSurface = pSurf;

	if (pSurf->IsOpaque())
		m_bIsOpaque = true;

	//

	bool	bSlotUsed[WG_NB_STATES];

	for (int i = 0; i < WG_NB_STATES; i++)
	{
		layer.tintColor[i] = WgColor::white;
		bSlotUsed[i] = false;
	}

	//

	int ofs = 0;
	for (WgStateEnum state : stateBlocks)
	{
		int index = WgUtil::_stateToIndex(state);

		bSlotUsed[index] = true;
		layer.blockOfs[index] = blockStartOfs + WgCoord(blockPitch.w*ofs, blockPitch.h*ofs);
		ofs++;
	}

	//

	assert(bSlotUsed[WgUtil::_stateToIndex(WG_STATE_NORMAL)]);				// A block for state normal is required.

	// Fill in fallback states and update opacity flag

	for (int i = 0; i < WG_NB_STATES; i++)
	{
		if (!bSlotUsed[i])
		{
			WgState state = WgUtil::_indexToState(i);

			int step = 0;
			int fallbackIndex = WgUtil::_stateToIndex(WgUtil::FallbackState(state,step++));
			while (!bSlotUsed[fallbackIndex])
				fallbackIndex = WgUtil::_stateToIndex(WgUtil::FallbackState(state, step++));

			layer.blockOfs[i] = layer.blockOfs[fallbackIndex];
		}

		_updateStateOpacity(i);
	}

	//

	return m_layers.size();
}

//____ SetLayerTint() _________________________________________________________

bool WgMultiBlockSkin::SetLayerTint(int layerIdx, WgColor tintColor)
{
	auto& layer = m_layers.at(layerIdx-1);

	for (int i = 0; i < WG_NB_STATES; i++)
	{
		uint8_t		oldAlpha = layer.tintColor[i].a;

		layer.tintColor[i] = tintColor;

		if (oldAlpha != layer.tintColor[i].a)
			_updateStateOpacity(i);
	}
	return true;
}


bool WgMultiBlockSkin::SetLayerTint(int layerIdx, std::initializer_list<WgStateColor> stateColors)
{
	auto& layer = m_layers.at(layerIdx-1);

	// 

	bool	bSlotUsed[WG_NB_STATES];

	layer.tintColor[0] = WgColor::white;
	bSlotUsed[0] = true;						// StateColor doesn't need to be specified for normal state, assumed to be white if not included

	for (int i = 1; i < WG_NB_STATES; i++)
		bSlotUsed[i] = false;

	//

	for (WgStateColor stateColor : stateColors)
	{
		int index = WgUtil::_stateToIndex(stateColor.state);

		uint8_t		oldAlpha = layer.tintColor[index].a;

		layer.tintColor[index] = stateColor.color;

		if (oldAlpha != stateColor.color.a)
			_updateStateOpacity(index);

		bSlotUsed[index] = true;
	}

	// Fill in fallback states and update opacity flag

	for (int i = 0; i < WG_NB_STATES; i++)
	{
		if (!bSlotUsed[i])
		{
			WgState state = WgUtil::_indexToState(i);

			int step = 0;
			int fallbackIndex = WgUtil::_stateToIndex(WgUtil::FallbackState(state, step++));
			while (!bSlotUsed[fallbackIndex])
				fallbackIndex = WgUtil::_stateToIndex(WgUtil::FallbackState(state, step++));

			uint8_t		oldAlpha = layer.tintColor[i].a;

			layer.tintColor[i] = layer.tintColor[fallbackIndex];

			if (oldAlpha != layer.tintColor[i].a )
				_updateStateOpacity(i);
		}
	}

	//

	return true;
}

//____ SetLayerBlendMode() ____________________________________________________

bool WgMultiBlockSkin::SetLayerBlendMode(int layerIdx, WgBlendMode blendMode)
{
	auto& layer = m_layers.at(layerIdx - 1);

	if (layer.blendMode != blendMode)
	{
		layer.blendMode = blendMode;

		for (int i = 0; i < WG_NB_STATES; i++)
			_updateStateOpacity(i);
	}

	return true;
}



//____ render() _______________________________________________________________

void WgMultiBlockSkin::Render( WgGfxDevice * pDevice, WgState state, const WgRect& _canvas, const WgRect& _clip, int scale ) const
{
	if (m_layers.empty() || m_blockSize.w <= 0 || m_blockSize.h <= 0 )
		return;

	int stateIndex = WgUtil::_stateToIndex(state);

	WgBlendMode orgBlendMode = pDevice->GetBlendMode();
	WgColor		orgTintColor = pDevice->GetTintColor();

	WgBlendMode blendMode = orgBlendMode;
	WgColor		tintColor = orgTintColor;
	WgColor		mixedTint = orgTintColor;

	for (auto& layer : m_layers)
	{
		if (layer.blendMode != blendMode)
		{
			blendMode = layer.blendMode;
			pDevice->SetBlendMode(blendMode);
		}

		if (layer.tintColor[stateIndex] != tintColor )
		{
			tintColor = layer.tintColor[stateIndex];
			mixedTint = orgTintColor * tintColor;
			pDevice->SetTintColor(mixedTint);
		}


		const WgRect&	src = WgRect(layer.blockOfs[stateIndex], m_blockSize);

		// Shortcuts & optimizations for common special cases.

		if (src.w == _canvas.w && src.h == _canvas.h && scale == m_scale)
		{ 
			pDevice->ClipBlit(_clip, layer.pSurface, src, _canvas.Pos().x, _canvas.Pos().y);
			goto exit;
		}

		if (m_frame.left + m_frame.top + m_frame.right + m_frame.bottom == 0)
		{
			pDevice->ClipStretchBlit(_clip, layer.pSurface, src, _canvas);
			goto exit;
		}

		const WgBorders&    sourceBorders = m_frame.Scale(m_scale);
		const WgBorders     canvasBorders = m_frame.Scale(scale);


		if (src.w == _canvas.w)
		{
			pDevice->ClipBlitVertStretchBar(_clip, layer.pSurface, src, sourceBorders, _canvas, canvasBorders);
			goto exit;
		}

		// Render upper row (top-left corner, top stretch area and top-right corner)

		if (canvasBorders.top > 0)
		{
			WgRect sourceRect(src.x, src.y, src.w, sourceBorders.top);
			WgRect destRect(_canvas.x, _canvas.y, _canvas.w, canvasBorders.top);

			pDevice->ClipBlitHorrStretchBar(_clip, layer.pSurface, sourceRect, sourceBorders, destRect, canvasBorders);
		}

		// Render mid row (left and right stretch area and middle section)

		if (_canvas.h - canvasBorders.Height() > 0)
		{
			WgRect sourceRect(src.x, src.y + sourceBorders.top, src.w, src.h - sourceBorders.Height());
			WgRect destRect(_canvas.x, _canvas.y + canvasBorders.top, _canvas.w, _canvas.h - canvasBorders.Height());

			pDevice->ClipBlitHorrStretchBar(_clip, layer.pSurface, sourceRect, sourceBorders, destRect, canvasBorders);
		}

		// Render lowest row (bottom-left corner, bottom stretch area and bottom-right corner)

		if (canvasBorders.bottom > 0)
		{
			WgRect sourceRect(src.x, src.y + src.h - sourceBorders.bottom, src.w, sourceBorders.bottom);
			WgRect destRect(_canvas.x, _canvas.y + _canvas.h - canvasBorders.bottom, _canvas.w, canvasBorders.bottom);

			pDevice->ClipBlitHorrStretchBar(_clip, layer.pSurface, sourceRect, sourceBorders, destRect, canvasBorders);
		}

	}

exit:
	if (mixedTint != orgTintColor)
		pDevice->SetTintColor(orgTintColor);

	if (blendMode != orgBlendMode)
		pDevice->SetBlendMode(orgBlendMode);
}


//____ minSize() ______________________________________________________________

WgSize WgMultiBlockSkin::MinSize(int scale) const
{
	WgSize content = WgExtendedSkin::MinSize(scale);
	WgSize frame = m_frame.Scale(scale).Size();
	return WgSize( WgMax(content.w, frame.w), WgMax(content.h, frame.h) );
}

//____ preferredSize() ________________________________________________________

WgSize WgMultiBlockSkin::PreferredSize(int scale) const
{
	WgSize sz = WgExtendedSkin::PreferredSize(scale);
	return WgSize( WgMax((m_blockSize.w*scale) / m_scale,sz.w), WgMax((m_blockSize.h*scale) / m_scale,sz.h) );
}

//____ sizeForContent() _______________________________________________________

WgSize WgMultiBlockSkin::SizeForContent( const WgSize contentSize, int scale ) const
{
	WgSize sz = WgExtendedSkin::SizeForContent(contentSize, scale);
	WgSize min = m_frame.Scale(scale).Size();

	return WgSize( WgMax(sz.w,min.w), WgMax(sz.h,min.h) );
}

//____ markTest() _____________________________________________________________

bool WgMultiBlockSkin::MarkTest( const WgCoord& _ofs, const WgSize& canvas, WgState state, int opacityTreshold, int scale ) const
{
	if (m_layers.empty() || m_blockSize.w <= 0 || m_blockSize.h <= 0)
		return false;

	int stateIndex = WgUtil::_stateToIndex(state);

	for (auto& layer : m_layers)
	{
		int alpha = 0;
		if (IsOpaque(state))
			alpha = 255;
		else if( layer.blendMode == WG_BLENDMODE_BLEND )
		{
			WgCoord ofs = _ofs;

			WgBorders canvasFrame = m_frame.Scale(scale);
			WgBorders sourceFrame = m_frame.Scale(m_scale);

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
				ofs.x = (ofs.x * m_scale) / scale;
			}
			else if (xSection == 2)
			{
				ofs.x = ofs.x - (canvas.w - canvasFrame.right);           // Offset in right border of canvas
				ofs.x = (ofs.x * m_scale) / scale;            // Scale from canvas to source coordinates
				ofs.x += m_blockSize.w - sourceFrame.right;          // Add offset for right border
			}
			else if (xSection == 1)
			{
				int tileAreaWidth = m_blockSize.w - sourceFrame.Width();

				int canvasStretchWidth = canvas.w - canvasFrame.Width();	// Width of stretch-area on screen.

				ofs.x = ofs.x - canvasFrame.left;               // Offset in middle section of canvas
				ofs.x = (ofs.x * m_scale) / scale;        // Scale from canvas to source offset
				ofs.x = (int)((ofs.x / (float)canvasStretchWidth)*tileAreaWidth) + sourceFrame.left;
			}


			// Convert ofs.y to Y-offset in bitmap, taking stretch/tile section into account.

			if (ySection == 0)
			{
				ofs.y = (ofs.y * m_scale) / scale;
			}
			if (ySection == 2)
			{
				ofs.y = ofs.y - (canvas.h - canvasFrame.bottom);           // Offset in bottom border of canvas
				ofs.y = (ofs.y * m_scale) / scale;            // Scale from canvas to source coordinates
				ofs.y += m_blockSize.h - sourceFrame.bottom;          // Add offset for bottom border
			}
			else if (ySection == 1)
			{
				int tileAreaHeight = m_blockSize.h - sourceFrame.Height();

				int canvasStretchHeight = canvas.h - canvasFrame.Height();	// Height of stretch-area on screen.

				ofs.y = ofs.y - canvasFrame.top;               // Offset in middle section of canvas
				ofs.y = (ofs.y * m_scale) / scale;        // Scale from canvas to source offset
				ofs.y = (int)((ofs.y / (float)canvasStretchHeight)*tileAreaHeight) + sourceFrame.top;
			}

			WgCoord srcOfs = layer.blockOfs[stateIndex];

			alpha = layer.pSurface->GetOpacity(srcOfs.x + ofs.x, srcOfs.y + ofs.y);
		}

		if (alpha >= opacityTreshold)
			return true;
	}

	return false;
}

//____ isOpaque() _____________________________________________________________

bool WgMultiBlockSkin::IsOpaque() const
{
	return m_bIsOpaque;
}

bool WgMultiBlockSkin::IsOpaque( WgState state ) const
{
	return m_bIsOpaque;
}

bool WgMultiBlockSkin::IsOpaque( const WgRect& rect, const WgSize& canvasSize, WgState state, int scale ) const
{
	return m_bIsOpaque;
}

//____ isStateIdentical() _____________________________________________________

bool WgMultiBlockSkin::IsStateIdentical( WgState state, WgState comparedTo ) const
{
	int i1 = WgUtil::_stateToIndex(state);
	int i2 = WgUtil::_stateToIndex(comparedTo);

	for (auto& layer : m_layers)
	{
		if (layer.blockOfs[i1] != layer.blockOfs[i2] || layer.tintColor[i1] != layer.tintColor[i2])
			return false;
	}

	return true;
}

//____ _updateStateOpacity() __________________________________________________

void WgMultiBlockSkin::_updateStateOpacity(int stateIdx)
{
	bool bOpaque = false;

	for (auto& layer : m_layers)
	{
		if (layer.blendMode == WG_BLENDMODE_OPAQUE || (layer.blendMode == WG_BLENDMODE_BLEND && layer.pSurface->IsOpaque() && layer.tintColor[stateIdx].a == 255) )
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
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
				if (m_bStateOpaque[i] )
				{
					m_bIsOpaque = false;
					break;
				}
		}
	}
}

