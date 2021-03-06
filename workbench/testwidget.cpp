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
#include <wg_msgrouter.h>
#include <wg_base.h>
#include <wg_msg.h>

namespace wg
{
	const char TestWidget::CLASSNAME[] = {"TestWidget"};

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

	//____ isInstanceOf() _________________________________________________________
	
	bool TestWidget::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Widget::isInstanceOf(pClassName);
	}
	
	//____ className() ____________________________________________________________
	
	const char * TestWidget::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	//____ cast() _________________________________________________________________
	
	TestWidget_p TestWidget::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return TestWidget_p( static_cast<TestWidget*>(pObject.rawPtr()) );
	
		return 0;
	}

	//____ preferredSize() __________________________________________________________

	Size TestWidget::preferredSize() const
	{
		return Size(200,200);
	}

	//____ start() _________________________________________________________________

	void TestWidget::start()
	{
        if( !m_tickRouteId )
            m_tickRouteId = Base::msgRouter()->addRoute( MsgType::Tick, this );
	}

	//____ stop() __________________________________________________________________

	void TestWidget::stop()
	{
        if( m_tickRouteId )
        {
            Base::msgRouter()->deleteRoute( m_tickRouteId );
            m_tickRouteId = 0;
        }
	}

	//____ _cloneContent() _______________________________________________________

	void TestWidget::_cloneContent( const Widget * _pOrg )
	{
	}

	//____ _render() _____________________________________________________________

	void TestWidget::_render( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window, const Rect& _clip )
	{
 //       pDevice->setBlendMode(BlendMode::Invert);
        
        

		if( m_bPointsInitialized )
		{
		
			pDevice->drawLine( Coord(_canvas.x, _canvas.y), Coord(_canvas.x + _canvas.w, _canvas.y + _canvas.h ), Color(255,000,000), 10.f );

			for( int i = 0 ; i < 2 ; i+=2 )
			{
				pDevice->drawLine( _canvas.pos() + m_coord[i], _canvas.pos() + m_coord[i+1], Color(000,255,000), 15.5f );
			}
		}


 //       pDevice->setBlendMode(BlendMode::Blend);
        
	}

	//____ _receive() ______________________________________________________________

	void TestWidget::_receive( const Msg_p& pMsg )
	{
		switch( pMsg->type() )
		{
			case MsgType::Tick:
			{
                const TickMsg_p pTick = TickMsg::cast(pMsg);
				
				Rect area( 0,0, size() );
				
				
				if( !m_bPointsInitialized )
				{
					for( int i = 0 ; i < 2 ; i+=2 )
					{
						Coord& c = m_coord[i];
						
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
					Coord& c = m_coord[i];
					
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
			break;
		}
	}

} // namespace wg
