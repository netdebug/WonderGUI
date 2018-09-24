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
#ifndef WG_BLOCKSKIN_DOT_H
#define WG_BLOCKSKIN_DOT_H
#pragma once

#include <wg_extendedskin.h>
#include <wg_surface.h>

namespace wg 
{
	
	
	class BlockSkin;
	typedef	StrongPtr<BlockSkin>	BlockSkin_p;
	
	
	class BlockSkin : public ExtendedSkin
	{
		//TODO: Add sanity-checking to all Set-methods.
		//TODO: Optimize rendering based on invisibleSections and opaqueSections!
	
	public:

		//.____ Creation __________________________________________

		static BlockSkin_p	create();
		static BlockSkin_p 	create(Surface * pSurface, Border frame = { 0 } );
		static BlockSkin_p	create(Surface * pSurface, Rect block, Border frame = { 0 } );
		static BlockSkin_p	create(Surface * pSurface, Rect firstBlock, const std::initializer_list<State>& stateBlocks, Border frame = { 0 }, Orientation orientation = Orientation::Vertical, int spacing = 0);

		// DEPRECATED CREATE METHODS!

	
		//.____ Identification __________________________________________
	
		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static BlockSkin_p	cast( Object * pObject );
	
		//.____ Rendering ________________________________________________

		void	render( GfxDevice * pDevice, const Rect& _canvas, State state, const Rect& _clip ) const;
	
		//.____ Geometry _________________________________________________

		Size	minSize() const;
		Size	preferredSize() const;
	
		Size	sizeForContent( const Size contentSize ) const;

		//.____ Appearance _________________________________________________

		virtual void	setBlock(Coord ofs);
		virtual void	setBlock(State state, Coord ofs);
		virtual void	setBlocks(const std::initializer_list<State>& stateBlocks, Orientation orientation = Orientation::Vertical, int spacing = 0, Coord blockStartOfs = { 0,0 });
		virtual Rect	block(State state) const;

		void			setBlendMode(BlendMode mode);
		BlendMode		blendMode() const { return m_blendMode; }

		void			setSurface( Surface * pSurf );
		Surface_p		surface() const { return m_pSurface; }

		void			setBlockSize(Size size);
		Size			blockSize() const { return m_dimensions; }

		void			setFrame(Border frame);
		Border			frame() const { return m_frame; }

		//.____ Misc ____________________________________________________	
	
		bool	markTest( const Coord& ofs, const Rect& canvas, State state, int opacityTreshold ) const;
	
		bool	isOpaque() const;
		bool	isOpaque( State state ) const;
		bool	isOpaque( const Rect& rect, const Size& canvasSize, State state ) const;
	
		bool	isStateIdentical( State state, State comparedTo ) const;
	
		//.____ Deprecated ____________________________________________________

		static BlockSkin_p createStatic(Surface * pSurface, Rect block, Border frame = Border(0));
		static BlockSkin_p createEnable(Surface * pSurface, Size blockSize, Coord ofsEnabled, Coord ofsDisabled, Border frame = Border(0));
		static BlockSkin_p createClickable(Surface * pSurface, Size blockGeo, Coord blockStartOfs, Size blockPitch, Border blockFrame = Border(0));
		static BlockSkin_p createSelectable(Surface * pSurface, Size blockGeo, Coord blockStartOfs, Size blockPitch, Border blockFrame = Border(0));
		static BlockSkin_p createClickSelectable(Surface * pSurface, Size blockGeo, Coord blockStartOfs, Size blockPitch, Border blockFrame = Border(0));
		static BlockSkin_p createClickSelectableWidget(Surface * pSurface, Size blockGeo, Coord blockStartOfs, Size blockPitch, Border blockFrame = Border(0));

		static BlockSkin_p createStaticFromSurface(Surface * pSurface, Border frame = Border(0));
		static BlockSkin_p createEnableFromSurface(Surface * pSurface, int blockSpacing, Border blockFrame = Border(0));
		static BlockSkin_p createClickableFromSurface(Surface * pSurface, int blockSpacing, Border blockFrame = Border(0));
		static BlockSkin_p createSelectableFromSurface(Surface * pSurface, int blockSpacing, Border blockFrame = Border(0));
		static BlockSkin_p createClickSelectableFromSurface(Surface * pSurface, int blockSpacing, Border blockFrame = Border(0));


		void	setStateBlock(StateEnum state, const Coord& ofs);
		bool	setBlockGeo(Size size, Border frame = Border(0));

		void	setAllBlocks(const Coord& ofs);
		void	setDisabledBlock(const Coord& ofs);
		void	setHoveredBlocks(const Coord& ofs);
		void	setPressedBlocks(const Coord& ofs);
		void	setSelectedBlocks(const Coord& ofs);

	private:
	
		BlockSkin();
		BlockSkin(Surface * pSurface, Rect block, Border frame);
		~BlockSkin() {};

		void		_updateOpaqueFlag();
		void		_updateUnsetStates();
	
		Surface_p	m_pSurface;
		Size		m_dimensions;
		Border		m_frame;


		bool		m_bOpaque;
		BlendMode	m_blendMode = BlendMode::Blend;

		Bitmask<uint32_t>	m_stateBlockMask = 1;

		Coord	m_stateBlocks[StateEnum_Nb];
	};
	

} // namespace wg
#endif //WG_BLOCKSKIN_DOT_H
