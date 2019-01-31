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

#include <vector>
#include <wg_container.h>
#include <wg_panel.h>

#include <wg_patches.h>

#ifndef WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif

//____ Constructor _____________________________________________________________

WgContainer::WgContainer() : m_bSiblingsOverlap(true)
{
}



//____ IsWidget() ______________________________________________________________

bool WgContainer::IsWidget() const
{
	return true;
}

//____ IsContainer() ______________________________________________________________

bool WgContainer::IsContainer() const
{
	return true;
}

//____ IsPanel() ______________________________________________________________

bool WgContainer::IsPanel() const
{
	return false;
}

//____ IsCapsule() ______________________________________________________________

bool WgContainer::IsCapsule() const
{
	return false;
}

//____ IsLayer() ______________________________________________________________

bool WgContainer::IsLayer() const
{
	return false;
}


//____ CastToWidget() _______________________________________________________

WgWidget * WgContainer::CastToWidget()
{
	return this;
}

const WgWidget * WgContainer::CastToWidget() const
{
	return this;
}


//____ CastToContainer() _______________________________________________________

WgContainer * WgContainer::CastToContainer()
{
	return this;
}

const WgContainer * WgContainer::CastToContainer() const
{
	return this;
}

//____ CastToPanel() _______________________________________________________

WgPanel * WgContainer::CastToPanel()
{
	return 0;
}

const WgPanel * WgContainer::CastToPanel() const
{
	return 0;
}

//____ CastToCapsule() _______________________________________________________

WgCapsule * WgContainer::CastToCapsule()
{
	return 0;
}

const WgCapsule * WgContainer::CastToCapsule() const
{
	return 0;
}

//____ CastToLayer() _______________________________________________________

WgLayer * WgContainer::CastToLayer()
{
	return 0;
}

const WgLayer * WgContainer::CastToLayer() const
{
	return 0;
}

//____ FindWidget() ____________________________________________________________

WgWidget * WgContainer::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	WgRect childGeo;
	WgHook * pHook = _lastHookWithGeo( childGeo );
	WgWidget * pResult = 0;

	while( pHook && !pResult )
	{
		bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(pHook)->IsVisible():true;

		if( bVisibleHook && childGeo.contains( ofs ) )
		{
			if( pHook->Widget()->IsContainer() )
			{
				pResult = pHook->Widget()->CastToContainer()->FindWidget( ofs - childGeo.pos(), mode );
			}
			else if( mode == WgSearchMode::Geometry || pHook->Widget()->MarkTest( ofs - childGeo.pos() ) )
			{
				pResult = pHook->Widget();
			}
		}
		pHook = _prevHookWithGeo( childGeo, pHook );
	}

	// Return us if search mode is GEOMETRY

	if( !pResult && (mode == WgSearchMode::Geometry || MarkTest(ofs)) )
		pResult = this;

	return pResult;
}


//____ SetScale() ______________________________________________________________

void WgContainer::SetScale( int scale )
{
	if( m_scale != scale )
		_setScale(scale);
}


//____ _onNewRoot() ___________________________________________________________

void WgContainer::_onNewRoot( WgRootPanel * pRoot )
{
	for( WgHook * p = FirstHook() ; p != 0 ; p = p->Next() )
		p->Widget()->_onNewRoot( pRoot );

	WgWidget::_onNewRoot(pRoot);
}

//____ _focusRequested() _______________________________________________________

bool WgContainer::_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting )
{
	WgHook * p = Hook();
	if( p )
		return p->Holder()->_focusRequested( p, pWidgetRequesting );
	else
		return false;
}

//____ _focusReleased() ________________________________________________________

bool WgContainer::_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing )
{
	WgHook * p = Hook();
	if( p )
		return p->Holder()->_focusReleased( p, pWidgetReleasing );
	else
		return false;
}

//____ _getModalLayer() _______________________________________________________

WgModalLayer *  WgContainer::_getModalLayer() const
{
	const WgContainer * p = Parent();

	if( p )
		return p->_getModalLayer();
	else
		return 0;
} 

//____ _getPopupLayer() _______________________________________________________

WgPopupLayer *  WgContainer::_getPopupLayer() const
{
	const WgContainer * p = Parent();

	if (p)
		return p->_getPopupLayer();
	else
		return 0;
}



//____ _onEnable() _____________________________________________________________

void WgContainer::_onEnable()
{
	WgWidget * p = FirstWidget();
	while( p )
	{
		p->SetEnabled(true);
		p = p->NextSibling();
	}
}

//____ _onDisable() ____________________________________________________________

void WgContainer::_onDisable()
{
	WgWidget * p = FirstWidget();
	while( p )
	{
		p->SetEnabled(false);
		p = p->NextSibling();
	}
}

//____ _setScale() _________________________________________________________

void WgContainer::_setScale( int scale )
{
	WgWidget::_setScale( scale );

	WgWidget * p = FirstWidget();
	while( p )
	{
		p->_setScale( scale );
		p = p->NextSibling();
	}
}


//____ _renderPatches() _____________________________________________________
// Default implementation for panel rendering patches.
class WidgetRenderContext
{
public:
	WidgetRenderContext() : pWidget(0) {}
	WidgetRenderContext( WgWidget * pWidget, const WgRect& geo ) : pWidget(pWidget), geo(geo) {}

	WgWidget *	pWidget;
	WgRect		geo;
	WgPatches	patches;
};

void WgContainer::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{

	// We start by eliminating dirt outside our geometry

	WgPatches 	patches( _pPatches->Size() );								// TODO: Optimize by pre-allocating?

	for( const WgRect * pRect = _pPatches->Begin() ; pRect != _pPatches->End() ; pRect++ )
	{
		if( _canvas.intersectsWith( *pRect ) )
			patches.Push( WgRect(*pRect,_canvas) );
	}


	// Render container itself

	for( const WgRect * pRect = patches.Begin() ; pRect != patches.End() ; pRect++ )
		_onRender(pDevice, _canvas, _window, *pRect );


	// Render children

	WgRect	dirtBounds = patches.Union();

	if( m_bSiblingsOverlap )
	{

		// Create WidgetRenderContext's for siblings that might get dirty patches

		std::vector<WidgetRenderContext> renderList;

		WgRect childGeo;
		WgHook * p = _firstHookWithGeo( childGeo );
		while(p)
		{
			WgRect geo = childGeo + _canvas.pos();

			bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;

			if( bVisibleHook && geo.intersectsWith( dirtBounds ) )
				renderList.push_back( WidgetRenderContext(p->Widget(), geo ) );

			p = _nextHookWithGeo( childGeo, p );
		}

		// Go through WidgetRenderContexts in reverse order (topmost first), push and mask dirt

		for( int i = renderList.size()-1 ; i >= 0 ; i-- )
		{
			WidgetRenderContext * p = &renderList[i];

			p->patches.Push( &patches );

			p->pWidget->_onMaskPatches( patches, p->geo, p->geo, pDevice->GetBlendMode() );		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

			if( patches.IsEmpty() )
				break;
		}

		// Go through WidgetRenderContexts and render the patches

		for( int i = 0 ; i < (int) renderList.size() ; i++ )
		{
			WidgetRenderContext * p = &renderList[i];
			p->pWidget->_renderPatches( pDevice, p->geo, p->geo, &p->patches );
		}

	}
	else
	{
		WgRect childGeo;
		WgHook * p = _firstHookWithGeo( childGeo );

		while(p)
		{
			WgRect canvas = childGeo + _canvas.pos();
			bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
			if( bVisibleHook && canvas.intersectsWith( dirtBounds ) )
				p->Widget()->_renderPatches( pDevice, canvas, canvas, &patches );
			p = _nextHookWithGeo( childGeo, p );
		}

	}
}

//____ _onCloneContent() _______________________________________________________

void WgContainer::_onCloneContent( const WgContainer * _pOrg )
{
	m_bSiblingsOverlap 	= _pOrg->m_bSiblingsOverlap;
}

//____ _onCollectPatches() _______________________________________________________

void WgContainer::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	WgRect childGeo;
	WgHook * p = _firstHookWithGeo( childGeo );

	while(p)
	{
		bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
		if( bVisibleHook )
			p->Widget()->_onCollectPatches( container, childGeo + geo.pos(), clip );
		p = _nextHookWithGeo( childGeo, p );
	}
}

//____ _onMaskPatches() __________________________________________________________

void WgContainer::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	// Default implementation, should probably be made redundant over time...

	WgRect childGeo;
	WgHook * p = _firstHookWithGeo( childGeo );

	while(p)
	{
		bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
		if( bVisibleHook )
			p->Widget()->_onMaskPatches( patches, childGeo + geo.pos(), clip, blendMode );
		p = _nextHookWithGeo( childGeo, p );
	}
}
