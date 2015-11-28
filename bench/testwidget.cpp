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



#include "testwidget.h"
#include <wg_gfxdevice.h>
#include <wg_eventhandler.h>

static const char	c_widgetType[] = {"TestWidget"};

//____ Constructor ____________________________________________________________

TestWidget::TestWidget()
{
	m_bOpaque = false;
	m_bPointsInitialized = false;
	m_bAnimated = false;
}

//____ Destructor _____________________________________________________________

TestWidget::~TestWidget()
{
	
}

//____ Type() _________________________________________________________________

const char * TestWidget::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * TestWidget::GetClass()
{
	return c_widgetType;
}

//____ PreferredSize() __________________________________________________________

WgSize TestWidget::PreferredSize() const
{
	return WgSize(200,200);
}

//____ Start() _________________________________________________________________

void TestWidget::Start()
{
	_startReceiveTicks();
	m_bAnimated = true;}


//____ Stop() __________________________________________________________________

void TestWidget::Stop()
{
	_stopReceiveTicks();
	m_bAnimated = false;
}



//____ _onCloneContent() _______________________________________________________

void TestWidget::_onCloneContent( const WgWidget * _pOrg )
{
	TestWidget * pOrg = (TestWidget*) _pOrg;

}

//____ _onRender() _____________________________________________________________

void TestWidget::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	if( m_bPointsInitialized )
	{
	
		for( int i = 0 ; i < 2 ; i+=2 )
		{
			pDevice->DrawLine( _canvas.Pos() + m_coord[i], _canvas.Pos() + m_coord[i+1], WgColor(255,0,0), 15.f );
		}
	
		pDevice->DrawLine( WgCoord(_canvas.x + 20, _canvas.y +10), WgCoord(_canvas.x + 30, _canvas.y + _canvas.h -10 ), WgColor(0,255,0), 10.f );
	}
}

//____ _onEvent() ______________________________________________________________

void TestWidget::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case WG_EVENT_TICK:
		{
			const WgEvent::Tick * pTick = static_cast<const WgEvent::Tick*>(pEvent);
			
			WgRect area( 10,10, Size() - WgSize(10,10) );
			
			
			if( !m_bPointsInitialized )
			{
				for( int i = 0 ; i < 2 ; i+=2 )
				{
					WgCoord& c = m_coord[i];
					
					c.x = 0;
					c.y = 0;

					c = m_coord[i+1];

					c.x = 100000;
					c.y = 100000;
					
				}
				m_bPointsInitialized = true;
			}
			
			
			for( int i = 0 ; i < 2 ; i++ )
			{
				WgCoord& c = m_coord[i];
				
				if( c.x < area.x )
					c.x = area.x;
					
				if( c.y < area.y )
					c.y = area.y;

				if( c.x > area.x + area.w )
					c.x = area.x + area.w;

				if( c.y > area.y + area.h )
					c.y = area.y + area.h;					

				
				
				if( (c.y == area.y) && (c.x < area.x + area.w) )
					c.x++;
				else if( (c.x == area.x + area.w) && (c.y < area.y + area.h) )
					c.y++;
				else if( (c.y == area.y + area.h) && (c.x > area.x) )
					c.x--;
				else
					c.y--;
			}
			
			_requestRender();

		}
		break;

		default:
			pHandler->ForwardEvent( pEvent );
		break;
	}
}
