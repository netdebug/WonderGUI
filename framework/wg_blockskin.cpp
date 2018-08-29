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

#include <wg_blockskin.h>
#include <wg_gfxdevice.h>
#include <wg_geo.h>
#include <wg_util.h>


//____ create() _______________________________________________________________

WgBlockSkinPtr WgBlockSkin::Create()
{
	return WgBlockSkinPtr(new WgBlockSkin());
}


WgBlockSkinPtr WgBlockSkin::CreateStatic( WgSurface * pSurface, WgRect block, WgBorders frame )
{
	if( !pSurface || frame.Width() >= block.w || frame.Height() >= block.h ||
		block.x < 0 || pSurface->PixelSize().w < block.Right() ||
		block.y < 0 || pSurface->PixelSize().h < block.Bottom() )
		return 0;

	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );
	pSkin->SetBlockGeo( block.Size(), frame );
	pSkin->SetAllBlocks( block.Pos() );
	return WgBlockSkinPtr(pSkin);
}

WgBlockSkinPtr WgBlockSkin::CreateEnable( WgSurface * pSurface, WgSize blockSize, WgCoord ofsEnabled, WgCoord ofsDisabled, WgBorders frame )
{
	if( !pSurface || frame.Width() >= blockSize.w || frame.Height() >= blockSize.h ||
		pSurface->PixelSize().w < ofsEnabled.x + blockSize.w ||
		pSurface->PixelSize().w < ofsDisabled.x + blockSize.w ||
		pSurface->PixelSize().h < ofsEnabled.y + blockSize.h ||
		pSurface->PixelSize().h < ofsDisabled.y + blockSize.h )
		return 0;

	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );
	pSkin->SetBlockGeo( blockSize, frame );
	pSkin->SetAllBlocks( ofsEnabled );
	pSkin->SetDisabledBlock( ofsDisabled );
	return WgBlockSkinPtr(pSkin);
}

WgBlockSkinPtr WgBlockSkin::CreateClickable( WgSurface * pSurface, WgSize blockGeo, WgCoord blockStartOfs, WgSize blockPitch, WgBorders blockFrame )
{
	if( !pSurface || blockFrame.Width() >= blockGeo.w || blockFrame.Height() >= blockGeo.h ||
		pSurface->PixelSize().w < blockStartOfs.x + blockGeo.w + blockPitch.w*3 ||
		pSurface->PixelSize().h < blockStartOfs.y + blockGeo.h + blockPitch.h*3 )
		return 0;

	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );
	pSkin->SetBlockGeo( blockGeo, blockFrame );

	WgCoord blockOfs = blockStartOfs;

	pSkin->SetAllBlocks( blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);
	pSkin->SetHoveredBlocks( blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);
	pSkin->SetPressedBlocks( blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);
	pSkin->SetDisabledBlock( blockOfs );
	return WgBlockSkinPtr(pSkin);
}

WgBlockSkinPtr WgBlockSkin::CreateSelectable( WgSurface * pSurface, WgSize blockGeo, WgCoord blockStartOfs, WgSize blockPitch, WgBorders blockFrame )
{
	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );
	pSkin->SetBlockGeo( blockGeo, blockFrame );

	WgCoord blockOfs = blockStartOfs;

	pSkin->SetAllBlocks( blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);
	pSkin->SetSelectedBlocks( blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);
	pSkin->SetDisabledBlock( blockOfs );
	return WgBlockSkinPtr(pSkin);
}

WgBlockSkinPtr WgBlockSkin::CreateClickSelectable( WgSurface * pSurface, WgSize blockGeo, WgCoord blockStartOfs, WgSize blockPitch, WgBorders blockFrame )
{
	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );
	pSkin->SetBlockGeo( blockGeo, blockFrame );

	WgCoord blockOfs = blockStartOfs;

	pSkin->SetStateBlock( WG_STATE_NORMAL, blockOfs );				// Normal
	pSkin->SetStateBlock( WG_STATE_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_HOVERED, blockOfs );				// Hovered, not selected
	pSkin->SetStateBlock( WG_STATE_HOVERED_FOCUSED, blockOfs );
	pSkin->SetStateBlock( WG_STATE_PRESSED, blockOfs );
	pSkin->SetStateBlock( WG_STATE_PRESSED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_SELECTED, blockOfs );			// Selected, not hovered
	pSkin->SetStateBlock( WG_STATE_SELECTED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_SELECTED_HOVERED, blockOfs );	// Selected and hovered
	pSkin->SetStateBlock( WG_STATE_SELECTED_HOVERED_FOCUSED, blockOfs );
	pSkin->SetStateBlock( WG_STATE_SELECTED_PRESSED, blockOfs );
	pSkin->SetStateBlock( WG_STATE_SELECTED_PRESSED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_DISABLED, blockOfs );			// Disabled
	return WgBlockSkinPtr(pSkin);
}

/** Usable for state button etc, where a widget is normal/hovered/pressed/disabled as well as selected/unselected
*/
WgBlockSkinPtr WgBlockSkin::CreateClickSelectableWidget( WgSurface * pSurface, WgSize blockGeo, WgCoord blockStartOfs, WgSize blockPitch, WgBorders blockFrame )
{
	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );
	pSkin->SetBlockGeo( blockGeo, blockFrame );

	WgCoord blockOfs = blockStartOfs;

	pSkin->SetStateBlock( WG_STATE_NORMAL, blockOfs );				// Normal
	pSkin->SetStateBlock( WG_STATE_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_HOVERED, blockOfs );				// Hovered
	pSkin->SetStateBlock( WG_STATE_HOVERED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_PRESSED, blockOfs );				// Pressed
	pSkin->SetStateBlock( WG_STATE_PRESSED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_SELECTED, blockOfs );
	pSkin->SetStateBlock( WG_STATE_SELECTED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_SELECTED_HOVERED, blockOfs );
	pSkin->SetStateBlock( WG_STATE_SELECTED_HOVERED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_SELECTED_PRESSED, blockOfs );
	pSkin->SetStateBlock( WG_STATE_SELECTED_PRESSED_FOCUSED, blockOfs );
	blockOfs += WgCoord(blockPitch.w,blockPitch.h);

	pSkin->SetStateBlock( WG_STATE_DISABLED, blockOfs );

	return WgBlockSkinPtr(pSkin);
}




WgBlockSkinPtr WgBlockSkin::CreateStaticFromSurface( WgSurface * pSurface, WgBorders frame )
{
	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );
	pSkin->SetBlockGeo( pSurface->PixelSize(), frame );
	pSkin->SetAllBlocks( WgCoord(0,0) );
	return WgBlockSkinPtr(pSkin);
}

WgBlockSkinPtr WgBlockSkin::CreateEnableFromSurface( WgSurface * pSurface, int blockSpacing, WgBorders blockFrame )
{
	WgBlockSkin * pSkin = new WgBlockSkin();
	pSkin->SetSurface( pSurface );

	WgSize	sz = WgSize( (pSurface->PixelSize().w-blockSpacing)/2, pSurface->PixelSize().h );

	pSkin->SetBlockGeo( sz, blockFrame );
	pSkin->SetAllBlocks( WgCoord(0,0) );
	pSkin->SetDisabledBlock( WgCoord(sz.w+blockSpacing,0) );
	return WgBlockSkinPtr(pSkin);
}

WgBlockSkinPtr WgBlockSkin::CreateClickableFromSurface( WgSurface * pSurface, int blockSpacing, WgBorders blockFrame )
{
	WgSize	blockSize = WgSize( (pSurface->PixelSize().w-blockSpacing*3)/4, pSurface->PixelSize().h );
	return CreateClickable( pSurface, blockSize, WgCoord(0,0), WgSize(blockSize.w+blockSpacing,0), blockFrame );
}

WgBlockSkinPtr WgBlockSkin::CreateSelectableFromSurface( WgSurface * pSurface, int blockSpacing, WgBorders blockFrame )
{
	WgSize	blockSize( (pSurface->PixelSize().w-blockSpacing*2)/3, pSurface->PixelSize().h );
	return CreateSelectable( pSurface, blockSize, WgCoord(0,0), WgSize(blockSize.w+blockSpacing,0), blockFrame );
}

WgBlockSkinPtr WgBlockSkin::CreateClickSelectableFromSurface( WgSurface * pSurface, int blockSpacing, WgBorders blockFrame )
{
	WgSize	blockSize( (pSurface->PixelSize().w-blockSpacing*4)/5, pSurface->PixelSize().h );
	return CreateClickSelectable( pSurface, blockSize, WgCoord(0,0), WgSize(blockSize.w+blockSpacing,0), blockFrame );
}

//____ Constructor ____________________________________________________________

WgBlockSkin::WgBlockSkin()
{

	m_pSurface = 0;
	m_tiledSections = 0;
	m_bIsOpaque = false;

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_state[i].invisibleSections = 0;
		m_state[i].opaqueSections = 0;
	}
}


//____ setBlockGeo() __________________________________________________________

bool WgBlockSkin::SetBlockGeo( WgSize size, WgBorders frame )
{
    // NOTE! size is in pixels, frame is in base-size pixels. This is API inconsistency!
    
	if( size.w <= frame.Width() || size.h <= frame.Height() )
		return false;

	m_dimensions	= (size*m_scale)/WG_SCALE_BASE;
	m_frame			= frame;
	return true;
}

//____ setSurface() ______________________________________________________

void WgBlockSkin::SetSurface( WgSurface * pSurf )
{
	m_pSurface = pSurf;
	if( m_pSurface )
	{
		m_bIsOpaque = pSurf->IsOpaque();
		m_scale = m_pSurface->ScaleFactor();		
	}
	else
	{
		m_bIsOpaque = false;
		m_scale = WG_SCALE_BASE;		
	}
}

//____ setStateBlock() ________________________________________________________

void WgBlockSkin::SetStateBlock( WgStateEnum state, const WgCoord& ofs )
{
	m_state[WgUtil::_stateToIndex(state)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
}

//____ setAllBlocks() _________________________________________________________

void WgBlockSkin::SetAllBlocks( const WgCoord& ofs )
{
	for( int i = 0 ; i < WG_NB_STATES ; i++ )
		m_state[i].ofs = (ofs*m_scale)/WG_SCALE_BASE;

}

//____ setDisabledBlock() _____________________________________________________

void WgBlockSkin::SetDisabledBlock( const WgCoord& ofs )
{
	m_state[WgUtil::_stateToIndex(WG_STATE_DISABLED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
}

//____ setHoveredBlocks() _____________________________________________________

void WgBlockSkin::SetHoveredBlocks( const WgCoord& ofs )
{
	m_state[WgUtil::_stateToIndex(WG_STATE_HOVERED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_HOVERED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_HOVERED_FOCUSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_HOVERED_FOCUSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;

	SetPressedBlocks( ofs );
}

//____ setPressedBlocks() _____________________________________________________

void WgBlockSkin::SetPressedBlocks( const WgCoord& ofs )
{
	m_state[WgUtil::_stateToIndex(WG_STATE_PRESSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_PRESSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_PRESSED_FOCUSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_PRESSED_FOCUSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
}

//____ setSelectedBlocks() ____________________________________________________

void WgBlockSkin::SetSelectedBlocks( const WgCoord& ofs )
{
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_FOCUSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_HOVERED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_HOVERED_FOCUSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_PRESSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
	m_state[WgUtil::_stateToIndex(WG_STATE_SELECTED_PRESSED_FOCUSED)].ofs = (ofs*m_scale)/WG_SCALE_BASE;
}

//____ setTiled() _____________________________________________________________

void WgBlockSkin::SetTiled( bool bTiled )
{
	_setBitFlag( m_tiledSections, (int)WG_NORTH, bTiled );
	_setBitFlag( m_tiledSections, (int)WG_SOUTH, bTiled );
	_setBitFlag( m_tiledSections, (int)WG_EAST, bTiled );
	_setBitFlag( m_tiledSections, (int)WG_WEST, bTiled );
	_setBitFlag( m_tiledSections, (int)WG_CENTER, bTiled );
}

//____ setTiledTopBorder() ____________________________________________________

void WgBlockSkin::SetTiledTopBorder( bool bTiled )
{
	_setBitFlag( m_tiledSections, (int)WG_NORTH, bTiled );
}

//____ setTiledBottomBorder() _________________________________________________

void WgBlockSkin::SetTiledBottomBorder( bool bTiled )
{
	_setBitFlag( m_tiledSections, (int)WG_SOUTH, bTiled );
}

//____ setTiledLeftBorder() ___________________________________________________

void WgBlockSkin::SetTiledLeftBorder( bool bTiled )
{
	_setBitFlag( m_tiledSections, (int)WG_WEST, bTiled );
}

//____ setTiledRightBorder() __________________________________________________

void WgBlockSkin::SetTiledRightBorder( bool bTiled )
{
	_setBitFlag( m_tiledSections, (int)WG_EAST, bTiled );
}

//____ setTiledCenter() _______________________________________________________

void WgBlockSkin::SetTiledCenter( bool bTiled )
{
	_setBitFlag( m_tiledSections, (int)WG_CENTER, bTiled );
}

//____ optimizeRenderMethods() ________________________________________________

void WgBlockSkin::OptimizeRenderMethods()
{
    //TODO: Scaling support has probably broken this method!
    
	// Handle non-alpha surfaces the easy way

	if( m_pSurface->IsOpaque() )
	{
		for( int i = 0 ; i < WG_NB_STATES ; i++ )
		{
			m_state[i].invisibleSections = 0;
			m_state[i].opaqueSections = ALL_SECTIONS;
		}
		return;
	}

	// Clear all flags

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		m_state[i].invisibleSections = 0;
		m_state[i].opaqueSections = 0;
	}

	// Bail out if we have no surface

	if( !m_pSurface )
		return;

	//

	m_pSurface->Lock( WG_READ_ONLY );

	for( int i = 0 ; i < WG_NB_STATES ; i++ )
	{
		int x1 = m_state[i].ofs.x;
		int x2 = m_state[i].ofs.x + m_frame.left;
		int x3 = m_state[i].ofs.x + m_dimensions.w - m_frame.right;

		int y1 = m_state[i].ofs.y;
		int y2 = m_state[i].ofs.y + m_frame.top;
		int y3 = m_state[i].ofs.y + m_dimensions.h - m_frame.bottom;

		int centerW = m_dimensions.w - m_frame.Width();
		int	centerH = m_dimensions.h - m_frame.Height();

		if( m_frame.top > 0 )
		{
			if( m_frame.left > 0 )
				_scanStateBlockSectionArea( &m_state[i], WG_NORTHWEST, WgRect(x1, y1, m_frame.left, m_frame.top) );

			_scanStateBlockSectionArea( &m_state[i], WG_NORTH, WgRect(x2, y1, centerW, m_frame.top) );

			if( m_frame.right > 0 )
				_scanStateBlockSectionArea( &m_state[i], WG_NORTHEAST, WgRect(x3, y1, m_frame.right, m_frame.top) );
		}

		if( centerH > 0 )
		{
			if( m_frame.left > 0 )
				_scanStateBlockSectionArea( &m_state[i], WG_WEST, WgRect(x1, y2, m_frame.left, centerH) );

			_scanStateBlockSectionArea( &m_state[i], WG_CENTER, WgRect(x2, y2, centerW, centerH) );

			if( m_frame.right > 0 )
				_scanStateBlockSectionArea( &m_state[i], WG_EAST, WgRect(x3, y2, m_frame.right, centerH) );
		}

		if( m_frame.bottom > 0 )
		{
			if( m_frame.left > 0 )
				_scanStateBlockSectionArea( &m_state[i], WG_SOUTHWEST, WgRect(x1, y3, m_frame.left, m_frame.bottom) );

			_scanStateBlockSectionArea( &m_state[i], WG_SOUTH, WgRect(x2, y3, centerW, m_frame.bottom) );

			if( m_frame.right > 0 )
				_scanStateBlockSectionArea( &m_state[i], WG_SOUTHEAST, WgRect(x3, y3, m_frame.right, m_frame.bottom) );
		}
	}

	m_pSurface->Unlock();
}

//_____ _scanStateBlockSectionArea() __________________________________________

void WgBlockSkin::_scanStateBlockSectionArea( StateData * pState, WgOrigo section, const WgRect& sectionArea )
{
	const WgPixelFormat * pFormat = m_pSurface->PixelFormat();
	int pitch = m_pSurface->Pitch();

	if( pFormat->type != WG_PIXEL_BGRA_8 )
		return;												// Only supports ARGB_8 for the moment.

	unsigned char * p = ((unsigned char*)m_pSurface->Pixels()) + sectionArea.x * pFormat->bits/8 + sectionArea.y * pitch;

	unsigned int alphaAcc = 0;

	for( int y = 0 ; y < sectionArea.h ; y++ )
	{
		for( int x = 0 ; x < sectionArea.w ; x++ )
			alphaAcc += (int) p[x*4+3];

		p += pitch;
	}

	if( alphaAcc == 0 )
		pState->invisibleSections |= 1 << (int)section;
	else if( alphaAcc == ((unsigned)(sectionArea.w * sectionArea.h)) * 255 )
		pState->opaqueSections |= 1 << (int)section;
}



//____ render() _______________________________________________________________

void WgBlockSkin::Render( WgGfxDevice * pDevice, WgState state, const WgRect& _canvas, const WgRect& _clip, int scale ) const
{
	if( !m_pSurface )
		return;

	const StateData * pState = &m_state[WgUtil::_stateToIndex(state)];
	if( pState->invisibleSections == ALL_SECTIONS )
		return;

    const WgRect&	src		= WgRect(pState->ofs, m_dimensions);

	// Shortcuts & optimizations for common special cases.

	if( src.w == _canvas.w && src.h == _canvas.h && scale == m_scale )
	{
		pDevice->ClipBlit( _clip, m_pSurface, src, _canvas.Pos().x, _canvas.Pos().y);
		return;
	}

    if( m_frame.left + m_frame.top + m_frame.right + m_frame.bottom == 0 )
	{
        pDevice->ClipStretchBlit( _clip, m_pSurface, src, _canvas );
		return;
	}

    const WgBorders&    sourceBorders = m_frame.Scale(m_scale);
    const WgBorders     canvasBorders = m_frame.Scale(scale);
    
    
    if( src.w == _canvas.w )
	{
        pDevice->ClipBlitVertStretchBar( _clip, m_pSurface, src, sourceBorders, _canvas, canvasBorders );
		return;
	}

	// Render upper row (top-left corner, top stretch area and top-right corner)

	if( canvasBorders.top > 0 )
	{
		WgRect sourceRect( src.x, src.y, src.w, sourceBorders.top );
        WgRect destRect( _canvas.x, _canvas.y, _canvas.w, canvasBorders.top );

        pDevice->ClipBlitHorrStretchBar( _clip, m_pSurface, sourceRect, sourceBorders, destRect, canvasBorders );
	}

    // Render mid row (left and right stretch area and middle section)

    if( _canvas.h - canvasBorders.Height() > 0 )
    {
        WgRect sourceRect( src.x, src.y + sourceBorders.top, src.w, src.h - sourceBorders.Height() );
        WgRect destRect( _canvas.x, _canvas.y + canvasBorders.top, _canvas.w, _canvas.h - canvasBorders.Height() );

        pDevice->ClipBlitHorrStretchBar( _clip, m_pSurface, sourceRect, sourceBorders, destRect, canvasBorders );
    }

	// Render lowest row (bottom-left corner, bottom stretch area and bottom-right corner)

	if( canvasBorders.bottom > 0 )
	{
		WgRect sourceRect( src.x, src.y + src.h - sourceBorders.bottom, src.w, sourceBorders.bottom );
        WgRect destRect( _canvas.x, _canvas.y + _canvas.h - canvasBorders.bottom, _canvas.w, canvasBorders.bottom );

        pDevice->ClipBlitHorrStretchBar( _clip, m_pSurface, sourceRect, sourceBorders, destRect, canvasBorders );
	}
}


//____ minSize() ______________________________________________________________

WgSize WgBlockSkin::MinSize(int scale) const
{
	WgSize content = WgExtendedSkin::MinSize(scale);
	WgSize frame = m_frame.Scale(scale).Size();
	return WgSize( WgMax(content.w, frame.w), WgMax(content.h, frame.h) );
}

//____ preferredSize() ________________________________________________________

WgSize WgBlockSkin::PreferredSize(int scale) const
{
	WgSize sz = WgExtendedSkin::PreferredSize(scale);
	return WgSize( WgMax((m_dimensions.w*scale) / m_scale,sz.w), WgMax((m_dimensions.h*scale) / m_scale,sz.h) );
}

//____ sizeForContent() _______________________________________________________

WgSize WgBlockSkin::SizeForContent( const WgSize contentSize, int scale ) const
{
	WgSize sz = WgExtendedSkin::SizeForContent(contentSize, scale);
	WgSize min = m_frame.Scale(scale).Size();

	return WgSize( WgMax(sz.w,min.w), WgMax(sz.h,min.h) );
}

//____ markTest() _____________________________________________________________

bool WgBlockSkin::MarkTest( const WgCoord& _ofs, const WgSize& canvas, WgState state, int opacityTreshold, int scale ) const
{
	if( !m_pSurface )
		return false;

	int alpha;
	if( IsOpaque( state ) )
		alpha = 255;
	else
	{
		WgCoord ofs = _ofs;

        WgBorders canvasFrame = m_frame.Scale(scale);
        WgBorders sourceFrame = m_frame.Scale(m_scale);
	
		// Determine in which section the cordinate is (0-2 for x and y).

		int	xSection = 0;
		int ySection = 0;

		if( ofs.x >= canvas.w - canvasFrame.right )
			xSection = 2;
		else if( ofs.x > canvasFrame.left )
			xSection = 1;

		if( ofs.y >= canvas.h - canvasFrame.bottom )
			ySection = 2;
		else if( ofs.y > canvasFrame.top )
			ySection = 1;

		// Convert ofs.x to X-offset in bitmap, taking stretch/tile section into account.

        if( xSection == 0 )
        {
            ofs.x = (ofs.x * m_scale) / scale;
        }
		else if( xSection == 2 )
		{
            ofs.x = ofs.x - (canvas.w - canvasFrame.right);           // Offset in right border of canvas
            ofs.x = (ofs.x * m_scale) / scale;            // Scale from canvas to source coordinates
            ofs.x += m_dimensions.w - sourceFrame.right;          // Add offset for right border
		}
		else if( xSection == 1 )
		{
			int tileAreaWidth = m_dimensions.w - sourceFrame.Width();

			bool bTile;

			if( ySection == 0 )
				bTile = (m_tiledSections & (1 << (int)WG_NORTH)) != 0;
			else if( ySection == 1 )
				bTile = (m_tiledSections & (1 << (int)WG_CENTER)) != 0;
			else
				bTile = (m_tiledSections & (1 << (int)WG_SOUTH)) != 0;

			if( bTile )
            {
                ofs.x = ofs.x - canvasFrame.left;               // Offset in middle section of canvas
                ofs.x = (ofs.x * m_scale) / scale;        // Scale from canvas to source offset
                ofs.x = (ofs.x % tileAreaWidth) + sourceFrame.left; // Get offset in source
            }
			else
			{
                int canvasStretchWidth = canvas.w - canvasFrame.Width();	// Width of stretch-area on screen.

                ofs.x = ofs.x - canvasFrame.left;               // Offset in middle section of canvas
                ofs.x = (ofs.x * m_scale) / scale;        // Scale from canvas to source offset
                ofs.x = (int)((ofs.x / (float)canvasStretchWidth)*tileAreaWidth) + sourceFrame.left;
			}
		}


		// Convert ofs.y to Y-offset in bitmap, taking stretch/tile section into account.

        if( ySection == 0 )
        {
            ofs.y = (ofs.y * m_scale) / scale;
        }
		if( ySection == 2 )
		{
            ofs.y = ofs.y - (canvas.h - canvasFrame.bottom);           // Offset in bottom border of canvas
            ofs.y = (ofs.y * m_scale) / scale;            // Scale from canvas to source coordinates
            ofs.y += m_dimensions.h - sourceFrame.bottom;          // Add offset for bottom border
		}
		else if( ySection == 1 )
		{
			int tileAreaHeight = m_dimensions.h - sourceFrame.Height();

			bool bTile;

			if( xSection == 0 )
				bTile = (m_tiledSections & (1 << (int)WG_WEST)) != 0;
			else if( xSection == 1 )
				bTile = (m_tiledSections & (1 << (int)WG_CENTER)) != 0;
			else
				bTile = (m_tiledSections & (1 << (int)WG_EAST)) != 0;

			if( bTile )
            {
                ofs.y = ofs.y - canvasFrame.top;               // Offset in middle section of canvas
                ofs.y = (ofs.y * m_scale) / scale;        // Scale from canvas to source offset
                ofs.y = (ofs.y % tileAreaHeight) + sourceFrame.top; // Get offset in source
            }
			else
			{
                int canvasStretchHeight = canvas.h - canvasFrame.Height();	// Height of stretch-area on screen.

                ofs.y = ofs.y - canvasFrame.top;               // Offset in middle section of canvas
                ofs.y = (ofs.y * m_scale) / scale;        // Scale from canvas to source offset
                ofs.y = (int)((ofs.y / (float)canvasStretchHeight)*tileAreaHeight) + sourceFrame.top;
			}
		}

		WgCoord srcOfs = m_state[WgUtil::_stateToIndex(state)].ofs;

		alpha = m_pSurface->GetOpacity(srcOfs.x+ofs.x, srcOfs.y+ofs.y);
	}

	return ( alpha >= opacityTreshold);
}

//____ isOpaque() _____________________________________________________________

bool WgBlockSkin::IsOpaque() const
{
	return m_bIsOpaque;
}

bool WgBlockSkin::IsOpaque( WgState state ) const
{
	if( m_bIsOpaque )
		return true;

	return (m_state[WgUtil::_stateToIndex(state)].opaqueSections == ALL_SECTIONS);
}

bool WgBlockSkin::IsOpaque( const WgRect& rect, const WgSize& canvasSize, WgState state, int scale ) const
{
	// Quick exit in optimal case

	if( m_bIsOpaque )
		return true;

	// Semi-quick exit

	int index = WgUtil::_stateToIndex(state);

	if( rect.w == canvasSize.w && rect.h == canvasSize.h )
		return (m_state[index].opaqueSections == ALL_SECTIONS);

	WgRect center = WgRect(canvasSize) - m_frame.Scale(scale);
	if( center.Contains(rect) )
        return ( m_state[index].opaqueSections & (1<<(int)WG_CENTER) ) != 0;

	//
/*
	To be implemented optimized solution.

	int xParts = 0;
	int yParts = 0;

	if( rect.x < center.x )
		xParts |= 1;
	if( rect.x < center.x + center.w && rect.x + rect.w > center.x )
		xParts |= 2;
	if( rect.x + rect.w > center.x + center.w )
		xParts |= 4;

	if( rect.y < center.y )
		yParts |= 1;
	if( rect.y < center.y + center.h && rect.y + rect.h > center.y )
		yParts |= 2;
	if( rect.y + rect.h > center.y + center.h )
		yParts |= 4;

	int bitmask = lookupTab[xParts][yParts];

*/
	int bitmask = 0;

	if( rect.y < center.y )
	{
		if( rect.x < center.x )
			bitmask |= (1<<(int)WG_NORTHWEST);
		if( rect.x < center.x + center.w && rect.x + rect.w > center.x )
			bitmask |= (1<<(int)WG_NORTH);
		if( rect.x + rect.w > center.x + center.w )
			bitmask |= (1<<(int)WG_NORTHEAST);
	}

	if( rect.y < center.y + center.h && rect.y + rect.h > center.y )
	{
		if( rect.x < center.x )
			bitmask |= (1<<(int)WG_WEST);
		if( rect.x < center.x + center.w && rect.x + rect.w > center.x )
			bitmask |= (1<<(int)WG_CENTER);
		if( rect.x + rect.w > center.x + center.w )
			bitmask |= (1<<(int)WG_EAST);
	}

	if( rect.y + rect.h > center.y + center.h )
	{
		if( rect.x < center.x )
			bitmask |= (1<<(int)WG_SOUTHWEST);
		if( rect.x < center.x + center.w && rect.x + rect.w > center.x )
			bitmask |= (1<<(int)WG_SOUTH);
		if( rect.x + rect.w > center.x + center.w )
			bitmask |= (1<<(int)WG_SOUTHEAST);
	}


	if( (m_state[index].opaqueSections & bitmask) == bitmask )
		return true;

	return false;
}

//____ isStateIdentical() _____________________________________________________
/*
bool WgBlockSkin::IsStateIdentical( WgState state, WgState comparedTo ) const
{
	int i1 = _stateToIndex(state);
	int i2 = _stateToIndex(comparedTo);

	return ( m_state[i1].ofs == m_state[i2].ofs && WgExtendedSkin::IsStateIdentical(state,comparedTo) );
}
*/

//____ _setBitFlag() __________________________________________________________

void WgBlockSkin::_setBitFlag( int& bitmask, int bit, bool bSet )
{
	if( bSet )
		bitmask |= 1 << bit;
	else
		bitmask &= ~(1 << bit);

}

