
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


#include <wg_base.h>
#include <wg_rectchain.h>
#include <wg_textpropmanager.h>
#include <wg_texttool.h>

#ifdef WG_USE_FREETYPE
#	include <ft2build.h>
#	include <wg_vectorglyphs.h>
#	include FT_FREETYPE_H

	bool		WgBase::s_bFreeTypeInitialized;
	FT_Library	WgBase::s_freeTypeLibrary;
#endif


WgTextPropPtr	WgBase::s_pDefaultTextProp;
WgTextPropPtr	WgBase::s_pDefaultSelectionProp;
WgTextPropPtr	WgBase::s_pDefaultLinkProp;
WgCursor *		WgBase::s_pDefaultCursor = 0;

WgMemPool *		WgBase::s_pWeakPtrPool;


//____ Init() __________________________________________________________________

void WgBase::Init()
{
	s_pWeakPtrPool = new WgMemPool( 128, sizeof( WgWeakPtrHub ) );

	WgTextTool::setDefaultBreakRules();

	WgRectChain::Init();
#ifdef WG_USE_FREETYPE
	s_bFreeTypeInitialized = false;
#endif
}

//____ Exit() __________________________________________________________________

void WgBase::Exit()
{
#ifdef WG_USE_FREETYPE

	WgVectorGlyphs::SetSurfaceFactory(0);
	WgVectorGlyphs::ClearCache();

	if( s_bFreeTypeInitialized )
		FT_Done_FreeType( s_freeTypeLibrary );
#endif
	WgRectChain::Exit();
	s_pDefaultTextProp = 0;

	delete s_pWeakPtrPool;
}

//____ AllocWeakPtrHub() ______________________________________________________

WgWeakPtrHub * WgBase::AllocWeakPtrHub()
{
	return (WgWeakPtrHub*) s_pWeakPtrPool->allocEntry();
}

//____ FreeWeakPtrHub() _______________________________________________________

void WgBase::FreeWeakPtrHub( WgWeakPtrHub * pHub )
{
	s_pWeakPtrPool->freeEntry( pHub );
}


//____ InitFreeType() _________________________________________________________

#ifdef WG_USE_FREETYPE
bool WgBase::InitFreeType()
{
	if( s_bFreeTypeInitialized )
		return true;

	FT_Error err = FT_Init_FreeType( &s_freeTypeLibrary );
	if( err == 0 )
	{
		s_bFreeTypeInitialized = true;
		return true;
	}

	return false;
}
#endif



//____ SetDefaultTextManager() _________________________________________________

/*void WgBase::SetDefaultTextManager( const WgTextMgrPtr& pManager )
{
	m_pDefTextMgr = pManager;
}


//____ GetDefaultTextManager() _________________________________________________

const WgTextMgrPtr& WgBase::GetDefaultTextManager()
{
	return m_pDefTextMgr;
}
*/

//____ SetDefaultTextProp() ___________________________________________________

void WgBase::SetDefaultTextProp( const WgTextPropPtr& pProp )
{
	s_pDefaultTextProp = pProp;
}

//____ SetDefaultSelectionProp() ___________________________________________________

void WgBase::SetDefaultSelectionProp( const WgTextPropPtr& pProp )
{
	s_pDefaultSelectionProp = pProp;
}

//____ SetDefaultLinkProp() ___________________________________________________

void WgBase::SetDefaultLinkProp( const WgTextPropPtr& pProp )
{
	s_pDefaultLinkProp = pProp;
}


//____ SetDefaultCursor() ___________________________________________________

void WgBase::SetDefaultCursor( WgCursor * pCursor )
{
	s_pDefaultCursor = pCursor;
}
