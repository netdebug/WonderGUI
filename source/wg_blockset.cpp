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

#include <wg_blockset.h>
#include <wg_colorset.h>
#include <wg_geo.h>
#include <wg_surface.h>
#include <assert.h>



WgBlock::WgBlock(	const WgSurface * pSurf, const WgRect& rect, const WgBorders& gfxBorders, const WgBorders& contentBorders, WgCoord contentShift, Uint32 flags )
{
	m_pSurf			= pSurf;
	m_rect			= rect;
	m_gfxBorders	= gfxBorders;
	m_flags			= flags;
	m_contentBorders= contentBorders;
	m_contentShift	= contentShift;

	if( m_gfxBorders.left || m_gfxBorders.right || m_gfxBorders.top || m_gfxBorders.bottom )
		m_flags |= WG_HAS_BORDERS;

	if(m_rect.x + m_rect.w > (int)m_pSurf->Width())
		m_rect.w = m_pSurf->Width() - m_rect.x;
	if(m_rect.y + m_rect.h > (int)m_pSurf->Height())
		m_rect.h = m_pSurf->Height() - m_rect.y;
}


bool WgBlock::operator==( const WgBlock& b) const
{
	if( m_pSurf == b.m_pSurf && m_rect == b.m_rect /*Ś&& m_gfxBorders == b.m_gfxBorders && 
	    m_flags == b.m_flags && m_contentBorders == b.m_contentBorders &&
		m_contentShift == b.m_contentShift*/ )
		return true;

	return false;
}

bool WgBlock::operator!=( const WgBlock& b) const
{
	if( m_pSurf == b.m_pSurf && m_rect == b.m_rect && m_gfxBorders == b.m_gfxBorders && 
	    m_flags == b.m_flags && m_contentBorders == b.m_contentBorders &&
		m_contentShift == b.m_contentShift )
		return false;

	return true;
}



WgBlockSet::WgBlockSet(	WgMemPool * pPool, const WgSurface * pSurf, const WgRect& normal,
					   const WgRect& marked, const WgRect& selected, const WgRect& disabled,
					   const WgRect& special, const WgBorders& gfxBorders,
					   const WgBorders& contentBorders, const WgColorSetPtr& pTextColors, Uint32 flags ) : WgRefCountedPooled(pPool)
{
	m_pTextColors				= pTextColors;
	m_flags						= _verifyFlags(flags);

	m_base.pSurf				= pSurf;
	m_base.gfxBorders			= gfxBorders;
	m_base.contentBorders		= contentBorders;
	m_base.w					= normal.w;
	m_base.h					= normal.h;

	m_base.x[WG_MODE_NORMAL]	= normal.x;
	m_base.x[WG_MODE_MARKED]	= marked.x;
	m_base.x[WG_MODE_SELECTED]	= selected.x;
	m_base.x[WG_MODE_DISABLED]	= disabled.x;
	m_base.x[WG_MODE_SPECIAL]	= special.x;

	m_base.y[WG_MODE_NORMAL]	= normal.y;
	m_base.y[WG_MODE_MARKED]	= marked.y;
	m_base.y[WG_MODE_SELECTED]	= selected.y;
	m_base.y[WG_MODE_DISABLED]	= disabled.y;
	m_base.y[WG_MODE_SPECIAL]	= special.y;
}

WgBlockSet::WgBlockSet(	WgMemPool * pPool, const WgSurface * pSurf, const WgBorders& gfxBorders, const WgBorders& contentBorders,
						const WgColorSetPtr& pTextColors, Uint32 flags ) : WgRefCountedPooled(pPool)
{
	m_pTextColors				= pTextColors;
	m_flags						= _verifyFlags(flags);

	m_base.pSurf				= pSurf;
	m_base.gfxBorders			= gfxBorders;
	m_base.contentBorders		= contentBorders;
	m_base.w					= 0;
	m_base.h					= 0;

	m_base.x[WG_MODE_NORMAL]	= 0;
	m_base.x[WG_MODE_MARKED]	= 0;
	m_base.x[WG_MODE_SELECTED]	= 0;
	m_base.x[WG_MODE_DISABLED]	= 0;
	m_base.x[WG_MODE_SPECIAL]	= 0;

	m_base.y[WG_MODE_NORMAL]	= 0;
	m_base.y[WG_MODE_MARKED]	= 0;
	m_base.y[WG_MODE_SELECTED]	= 0;
	m_base.y[WG_MODE_DISABLED]	= 0;
	m_base.y[WG_MODE_SPECIAL]	= 0;
}

//____ AddAlternative() _______________________________________________________________

bool WgBlockSet::AddAlternative( WgSize activationSize, const WgSurface * pSurf, const WgRect& normal, const WgRect& marked,
						const WgRect& selected, const WgRect& disabled, const WgRect& special,
						WgBorders gfxBorders, WgBorders contentBorders, WgCoord shiftMarked, WgCoord shiftPressed )
{
	if( activationSize.w == 0 && activationSize.h == 0 )
		return false;

	LinkedAlt * p = new LinkedAlt();

	p->activationSize		= activationSize;
	p->data.contentBorders	= contentBorders;
	p->data.gfxBorders		= gfxBorders;
	p->data.w				= normal.w;
	p->data.h				= normal.h;
	p->data.pSurf			= pSurf;

	p->data.x[WG_MODE_NORMAL]	= normal.x;
	p->data.x[WG_MODE_MARKED]	= marked.x;
	p->data.x[WG_MODE_SELECTED]	= selected.x;
	p->data.x[WG_MODE_DISABLED]	= disabled.x;
	p->data.x[WG_MODE_SPECIAL]	= special.x;

	p->data.y[WG_MODE_NORMAL]	= normal.y;
	p->data.y[WG_MODE_MARKED]	= marked.y;
	p->data.y[WG_MODE_SELECTED]	= selected.y;
	p->data.y[WG_MODE_DISABLED]	= disabled.y;
	p->data.y[WG_MODE_SPECIAL]	= special.y;



	LinkedAlt * pOther = m_altChain.First();
	while( pOther )
	{
		if( pOther->activationSize.w == p->activationSize.w && pOther->activationSize.h == p->activationSize.h )
		{
			delete p;
			return false;			// Already have a Alt with this activation size.
		}

		if( pOther->activationSize.w < p->activationSize.w || pOther->activationSize.h < p->activationSize.h )
		{
			if( pOther->activationSize.w > p->activationSize.w || pOther->activationSize.h > p->activationSize.h )
			{
				delete p;
				return false;			// Can't determine Alt-order, conflicting activation-sizes (smaller in one dimension but bigger in the other).
			}

			p->MoveBefore( pOther );
			return true;
		}

		pOther = pOther->Next();
	}

	m_altChain.PushBack( p );
	return true;
}

//____ NbAlternatives() ____________________________________________________

int WgBlockSet::NbAlternatives() const
{
	int nb = 1;
	LinkedAlt * p = m_altChain.First();
	while( p )
	{
		nb++;
		p = p->Next();
	}
	return nb;
}

//____ ActivationSize() ____________________________________________________

WgSize WgBlockSet::ActivationSize( int alt ) const
{
	if( alt == 0 )
		return WgSize(0,0);

	int nb = 1;
	LinkedAlt * p = m_altChain.First();
	while( p )
	{
		if( nb == alt )
			return p->activationSize;

		nb++;
		p = p->Next();
	}

	return WgSize(0,0);
}


//____ GetAlt() _______________________________________________________________

WgBlockSet::Alt_Data* WgBlockSet::_getAlt( int n )
{
	if( n == 0 )
		return &m_base;

	int nb = 1;
	LinkedAlt * p = m_altChain.First();
	while( p )
	{
		if( nb == n )
			return &p->data;

		nb++;
		p = p->Next();
	}
	return 0;
}

WgBlockSet::Alt_Data* WgBlockSet::_getAlt( WgSize destSize )
{
	Alt_Data * pData = &m_base;

	LinkedAlt * p = m_altChain.First();
	while( p )
	{
		if( destSize.w > p->activationSize.w && destSize.h > p->activationSize.h )
			break;

		pData = &p->data;
		p = p->Next();
	}
	return pData;
}

const WgBlockSet::Alt_Data* WgBlockSet::_getAlt( int n ) const
{
	if( n == 0 )
		return &m_base;

	int nb = 1;
	LinkedAlt * p = m_altChain.First();
	while( p )
	{
		if( nb == n )
			return &p->data;

		nb++;
		p = p->Next();
	}
	return 0;
}

const WgBlockSet::Alt_Data* WgBlockSet::_getAlt( WgSize destSize ) const
{
	const Alt_Data * pData = &m_base;

	LinkedAlt * p = m_altChain.First();
	while( p )
	{
		if( destSize.w > p->activationSize.w && destSize.h > p->activationSize.h )
			break;

		pData = &p->data;
		p = p->Next();
	}
	return pData;
}


//____ SetPos() _______________________________________________________________

bool WgBlockSet::SetPos( WgMode mode, WgCoord pos, int alt )
{
	Alt_Data * p = _getAlt(alt);
	if( !p )
		return false;

	p->x[mode] = pos.x;
	p->y[mode] = pos.y;
	return true;
}

//____ SetSize() ______________________________________________________________

bool WgBlockSet::SetSize( WgSize size, int alt )
{
	Alt_Data * p = _getAlt(alt);
	if( !p )
		return false;

	p->w = size.w;
	p->h = size.h;
	return true;
}

//____ TextColor() _____________________________________________________________

WgColor WgBlockSet::TextColor( WgMode mode ) const
{
	if( !m_pTextColors )
		return WgColor::transparent;

	return m_pTextColors->Color(mode);
}


//____ Rect() __________________________________________________________________

WgRect WgBlockSet::Rect( WgMode mode, int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return WgRect();

	return WgRect( p->x[mode], p->y[mode], p->w, p->h );
}

//____ Size() __________________________________________________________________

WgSize WgBlockSet::Size( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return WgSize();

	return WgSize( p->w, p->h );
}


//____ Width() _________________________________________________________________

int WgBlockSet::Width( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return 0;

	return p->w;
}

//____ Height() ________________________________________________________________

int WgBlockSet::Height( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return 0;

	return p->h;
}

//____ MinSize() _______________________________________________________________

WgSize WgBlockSet::MinSize( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return WgSize();

	return p->gfxBorders.Size();
}

//____ MinWidth() ______________________________________________________________

int WgBlockSet::MinWidth( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return 0;

	return p->gfxBorders.Width();
}

//____ MinHeight() _____________________________________________________________

int WgBlockSet::MinHeight( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return 0;

	return p->gfxBorders.Height();
}

//____ Surface() _______________________________________________________________

const WgSurface * WgBlockSet::Surface( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return 0;

	return p->pSurf;
}


//____ GfxBorders() ____________________________________________________________

WgBorders WgBlockSet::GfxBorders( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return WgBorders();

	return p->gfxBorders;
}

//____ SetGfxBorders() ________________________________________________________

void WgBlockSet::SetGfxBorders( const WgBorders& borders, int alt )
{
	Alt_Data * p = _getAlt(alt);
	if( !p )
		return;

	p->gfxBorders = borders;
}

//____ ContentBorders() ____________________________________________________

WgBorders WgBlockSet::ContentBorders( int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return WgBorders();

	return p->contentBorders;
}

//____ SetContentBorders() ____________________________________________________

void WgBlockSet::SetContentBorders( const WgBorders& borders, int alt )
{
	Alt_Data * p = _getAlt(alt);
	if( !p )
		return;

	p->contentBorders = borders;
}


//____ ContentShift() __________________________________________________

WgCoord WgBlockSet::ContentShift( WgMode mode, int alt ) const
{
	const Alt_Data * p = _getAlt(alt);
	if( !p )
		return WgCoord();

	return WgCoord(p->contentShift[mode]);
}

//____ SetContentShift() _______________________________________________

bool WgBlockSet::SetContentShift( WgMode mode, WgCoord ofs, int alt )
{
	Alt_Data * p = _getAlt(alt);
	if( !p || !(mode == WG_MODE_MARKED || mode == WG_MODE_SELECTED) )
		return false;

	p->contentShift[mode].x = ofs.x;
	p->contentShift[mode].y = ofs.y;
	return true;
}


//____ SetTile() ______________________________________________________________

bool WgBlockSet::SetTile(Uint32 place, bool bTile)
{
	if((place & WG_TILE_ALL) != place)
		return false;

	// cannot scale and tile at the same time
	if((place & WG_TILE_CENTER) && (IsScaled() || IsFixedSize()))
		return false;

	if(bTile)
		m_flags |= place;
	else
		m_flags &= ~place;

	return true;
}

//____ SetScale() _____________________________________________________________

bool WgBlockSet::SetScale(bool bScale)
{
	// cannot scale and tile at the same time
	if((IsFixedSize() || HasTiledCenter()) && bScale)
		return false;

	if(bScale)
		m_flags |= WG_SCALE_CENTER;
	else
		m_flags &= ~WG_SCALE_CENTER;

	return true;
}

//____ SetFixedSize() _____________________________________________________________

bool WgBlockSet::SetFixedSize(bool bFixedSize)
{
	if( IsScaled() || HasTiledCenter() )
		return false;

	if(bFixedSize)
		m_flags |= WG_FIXED_CENTER;
	else
		m_flags &= ~WG_FIXED_CENTER;

	return true;
}


//____ VerifyFlags() __________________________________________________________

Uint32 WgBlockSet::_verifyFlags(Uint32 _flags)
{
	// Make sure not more than one of SCALE/TILE/FIXED is set on center.

	int centerFlags = _flags & (WG_SCALE_CENTER | WG_TILE_CENTER | WG_FIXED_CENTER);

	if( centerFlags != 0 && centerFlags != WG_SCALE_CENTER && centerFlags != WG_TILE_CENTER && centerFlags != WG_FIXED_CENTER )
	{
		assert(0);
		return _flags &~ (WG_SCALE_CENTER | WG_TILE_CENTER | WG_FIXED_CENTER);		// Fall back to stretch.
	}
	return _flags;
}

//____ SameBlock() ____________________________________________________________

bool WgBlockSet::SameBlock( WgMode one, WgMode two, int alt )
{
	Alt_Data * p = _getAlt(alt);
	if( !p )
		return false;

	if( p->x[one] == p->x[two] && p->y[one] == p->y[two] )
		return true;

	return false;
}
