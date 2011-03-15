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

#include <wg_event.h>
#include <wg_eventhandler.h>
#include <wg_root.h>
#include <wg_gizmo_container.h>

//____ Constructor ____________________________________________________________

WgEventHandler::WgEventHandler( int64_t startTime, WgRoot * pRoot )
{
	m_pRoot					= pRoot;
	m_time					= startTime;
	m_modKeys				= WG_MODKEY_NONE;

	m_doubleClickTimeTreshold		= 250;
	m_doubleClickDistanceTreshold	= 2;

	m_buttonRepeatDelay		= 300;
	m_buttonRepeatRate		= 200;

	m_keyRepeatDelay		= 300;
	m_keyRepeatRate			= 150;

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
		m_pLatestButtonEvents[i] = 0;
}

//____ Destructor _____________________________________________________________

WgEventHandler::~WgEventHandler()
{
}

//____ QueueEvent() ___________________________________________________________

bool WgEventHandler::QueueEvent( const WgEvent::Event& _event )
{
	if( m_bIsProcessing )
	{
		// Events that are posted as a direct result of another event being processed
		// are inserted before the next event in the queue.
		// If two or more events are posted by the same event being processed,
		// they need to be queued in the order of posting.

		m_eventQueue.insert( m_insertPos++, _event );
	}
	else
	{
		// Events being posted outside processing loop are simply added to the
		// queue.

		m_eventQueue.push_back( _event );
	}

	return true;
}

//____ ProcessEvents() ________________________________________________________

void WgEventHandler::ProcessEvents()
{
	m_bIsProcessing = true;

	while( !m_eventQueue.empty() )
	{
		WgEvent::Event& ev = m_eventQueue.front();

		m_insertPos = m_eventQueue.begin()+1;	// Insert position set to right after current event.
		
		FinalizeEvent( ev );

		if( ev.IsForGizmo() )
		{
			if( ev.Gizmo() )
			{
				//TODO: Send event to Gizmo
			}
		}
		else
		{
			ProcessGeneralEvent( ev );
		}
//		ProcessEventCallbacks( ev );

		m_eventQueue.pop_front();
	}

	m_bIsProcessing = false;
}


//____ FinalizeEvent() ________________________________________________________

void WgEventHandler::FinalizeEvent( WgEvent::Event& _event )
{
	// Fill in missing information in the event-class.

	_event.m_timestamp	= m_time;
	_event.m_modKeys	= m_modKeys;

	// Only global POINTER_ENTER & POINTER_MOVE events have these members
	// set, the rest needs to have them filled in.

	if( _event.IsForGizmo() || (_event.Id() != WG_EVENT_POINTER_MOVE && _event.Id() != WG_EVENT_POINTER_ENTER) )
	{
		_event.m_pointerScreenPos = m_pointerPos;
		_event.m_pointerLocalPos = m_pointerPos;

		if( _event.Gizmo() )
			_event.m_pointerLocalPos -= _event.Gizmo()->ScreenPos();
	}
}

//____ ProcessGeneralEvent() _________________________________________________

void WgEventHandler::ProcessGeneralEvent( WgEvent::Event& _event )
{

	switch( _event.m_id )
	{
		case WG_EVENT_POINTER_ENTER:
			ProcessPointerEnter( (WgEvent::PointerEnter*) &_event );
			break;

		case WG_EVENT_POINTER_MOVE:
			ProcessPointerMove( (WgEvent::PointerMove*) &_event );
			break;
		case WG_EVENT_END_POINTER_MOVE:
			ProcessEndPointerMove( (WgEvent::EndPointerMove*) &_event );
			break;

		case WG_EVENT_POINTER_EXIT:
			ProcessPointerExit( (WgEvent::PointerExit*) &_event );
			break;

		case WG_EVENT_BUTTON_PRESS:
			ProcessButtonPress( (WgEvent::ButtonPress*) &_event );
			break;

		case WG_EVENT_BUTTON_REPEAT:
			ProcessButtonRepeat( (WgEvent::ButtonRepeat*) &_event );
			break;

		case WG_EVENT_BUTTON_DRAG:
			ProcessButtonDrag( (WgEvent::ButtonDrag*) &_event );
			break;

		case WG_EVENT_BUTTON_RELEASE:
			ProcessButtonRelease( (WgEvent::ButtonRelease*) &_event );
			break;

		case WG_EVENT_BUTTON_CLICK:
			ProcessButtonClick( (WgEvent::ButtonClick*) &_event );
			break;
		case WG_EVENT_BUTTON_DOUBLECLICK:
			ProcessButtonDoubleClick( (WgEvent::ButtonDoubleClick*) &_event );
			break;

		case WG_EVENT_KEY_PRESS:
		case WG_EVENT_KEY_RELEASE:
		case WG_EVENT_CHARACTER:
		case WG_EVENT_WHEEL_ROLL:
			break;

		case WG_EVENT_TIME_PASS:
			ProcessTimePass( (WgEvent::TimePass*) &_event );
			break;

		case WG_EVENT_DUMMY:
			break;
	}

}

//____ ProcessTimePass() ______________________________________________________

void WgEventHandler::ProcessTimePass( WgEvent::TimePass * pEvent )
{
	// Check if we need to post BUTTON_REPEAT

	for( int button = 0 ; button < WG_MAX_BUTTONS ; button++ )
	{
		if( m_pLatestButtonEvents[button] == &m_latestPress[button] )
		{
			int msSinceRepeatStart = (int) (m_time - m_latestPress[button].Timestamp() - m_buttonRepeatDelay );
			
			// First BUTTON_REPEAT event posted separately.

			if( msSinceRepeatStart < 0 && msSinceRepeatStart + pEvent->Millisec() >= 0 )
				QueueEvent( WgEvent::ButtonRepeat(button) );	

			// Calculate ms since last BUTTON_REPEAT event

			int msToProcess;
			if( msSinceRepeatStart < 0 )
				msToProcess = msSinceRepeatStart + pEvent->Millisec();
			else
				msToProcess = (msSinceRepeatStart % m_buttonRepeatRate) + pEvent->Millisec();

			// Post the amount of BUTTON_REPEAT that should be posted.

			while( msToProcess >= m_buttonRepeatRate )
			{
				QueueEvent( WgEvent::ButtonRepeat(button) );
				msToProcess -= m_buttonRepeatRate;
			}	
		}
	}


	// Increase time counter
	
	m_time += pEvent->Millisec();
}

//____ ProcessPointerEnter() __________________________________________________

void WgEventHandler::ProcessPointerEnter( WgEvent::PointerEnter * pEvent )
{
}

//____ ProcessPointerExit() ___________________________________________________

void WgEventHandler::ProcessPointerExit( WgEvent::PointerExit * pEvent )
{
	// Post POINTER_EXIT events for all gizmos we had marked

	for( size_t i = 0 ; i < m_vMarkedGizmos.size() ; i++ )
	{
		WgGizmo * pGizmo = m_vMarkedGizmos[i].GetRealPtr();

		if( pGizmo )
			QueueEvent( WgEvent::PointerExit( pGizmo ) );
	}

	m_vMarkedGizmos.clear();
}


//____ ProcessPointerMove() ___________________________________________________

void WgEventHandler::ProcessPointerMove( WgEvent::PointerMove * pEvent )
{
	// Post events for button drag

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
	{
		if( m_pLatestButtonEvents[i] && m_pLatestButtonEvents[i]->Id() == WG_EVENT_BUTTON_PRESS )
			QueueEvent( WgEvent::ButtonDrag( i, m_latestPress[i].PointerPos(), m_pointerPos, pEvent->Pos() ) );
	}

	// Post event for finalizing move once button drag is taken care of.

	QueueEvent( WgEvent::EndPointerMove( pEvent->Pos() ) );

	// Update pointer position

	m_pointerPos = pEvent->Pos();
}

//____ ProcessEndPointerMove() _______________________________________________

void WgEventHandler::ProcessEndPointerMove( WgEvent::EndPointerMove * pEvent )
{
	std::vector<WgGizmo*>	vNowMarked;

	// Collect widgets we now are inside

	WgGizmo * pGizmo = m_pRoot->FindGizmo( m_pointerPos, WG_SEARCH_ACTION_TARGET );

	while( pGizmo )
	{
		vNowMarked.push_back(pGizmo);
		pGizmo = pGizmo->Hook()->Parent()->CastToGizmo();
	}

	// Post POINTER_EXIT events for gizmos no longer marked

	for( size_t i = 0 ; i < m_vMarkedGizmos.size() ; i++ )
	{
		pGizmo = m_vMarkedGizmos[i].GetRealPtr();

		size_t j = 0;
		while( j < vNowMarked.size() )
		{
			if( pGizmo == vNowMarked[j] )
				break;
			j++;
		}

		if( j == vNowMarked.size() )
			QueueEvent( WgEvent::PointerExit( pGizmo ) );
	}

	// Post POINTER_ENTER events for new marked gizmos
	// and POINTER_MOVE events for those already marked

	for( size_t i = 0 ; i < vNowMarked.size() ; i++ )
	{
		pGizmo = vNowMarked[i];

		size_t j = 0;
		while( j < m_vMarkedGizmos.size() )
		{
			if( pGizmo == m_vMarkedGizmos[j].GetRealPtr() )
				break;
			j++;
		}

		if( j == vNowMarked.size() )
			QueueEvent( WgEvent::PointerEnter( pGizmo ) );
		else
			QueueEvent( WgEvent::PointerMove( pGizmo ) );
	}

	// Copy content of vNowMarked to m_vMarkedGizmos

	m_vMarkedGizmos.clear();
	for( size_t i = 0 ; i < vNowMarked.size() ; i++ )
		m_vMarkedGizmos.push_back( vNowMarked[i] );
}

//____ ProcessButtonPress() ___________________________________________________

void WgEventHandler::ProcessButtonPress( WgEvent::ButtonPress * pEvent )
{
	int button = pEvent->Button();

	// Update m_previousPressGizmos

	m_previousPressGizmos[button].clear();

	for( size_t i = 0 ; i < m_latestPressGizmos[button].size() ; i++ )
	{
		WgGizmo * pGizmo = m_latestPressGizmos[button][i].GetRealPtr();

		if( pGizmo )
			m_previousPressGizmos[button].push_back(pGizmo);
	}

	// Post BUTTON_PRESS events for marked gizmos and remember which one we have posted it for

	m_latestPressGizmos[button].clear();

	for( size_t i = 0 ; i < m_vMarkedGizmos.size() ; i++ )
	{
		WgGizmo * pGizmo = m_vMarkedGizmos[i].GetRealPtr();

		if( pGizmo )
		{
			QueueEvent( WgEvent::ButtonPress( button, pGizmo ) );
			m_latestPressGizmos[button].push_back(pGizmo);
		}
	}

	// Handle possible double-click

	if( m_latestPress[button].Timestamp() + m_doubleClickTimeTreshold > pEvent->Timestamp() )
	{
		WgCord distance = pEvent->PointerPos() - m_latestPress[button].PointerPos();

		if( distance.x <= m_doubleClickDistanceTreshold &&
			distance.x >= -m_doubleClickDistanceTreshold &&
			distance.y <= m_doubleClickDistanceTreshold &&
			distance.y >= -m_doubleClickDistanceTreshold )
			QueueEvent( WgEvent::ButtonDoubleClick(button) );
	}

	// Save info for the future

	m_latestPress[button] = *pEvent;
	m_pLatestButtonEvents[button] = &m_latestPress[button];

}


//____ ProcessButtonRepeat() __________________________________________________

void WgEventHandler::ProcessButtonRepeat( WgEvent::ButtonRepeat * pEvent )
{
	int button = pEvent->Button();

	// Post BUTTON_REPEAT events for all widgets that received the press and we 
	// still are inside.

	for( size_t i = 0 ; i < m_latestPressGizmos[button].size() ; i++ )
	{
		WgGizmo * pGizmo = m_latestPressGizmos[button][i].GetRealPtr();
		if( pGizmo && IsGizmoInList( pGizmo, m_vMarkedGizmos ) )
			QueueEvent( WgEvent::ButtonRepeat(button, pGizmo) );
	}
}



//____ ProcessButtonRelease() _________________________________________________

void WgEventHandler::ProcessButtonRelease( WgEvent::ButtonRelease * pEvent )
{
	int button = pEvent->Button();

	// Post BUTTON_RELEASE events for all gizmos that were pressed

	for( size_t i = 0 ; i < m_latestPressGizmos[button].size() ; i++ )
	{
		WgGizmo * pGizmo = m_latestPressGizmos[button][i].GetRealPtr();
		if( pGizmo )
		{
			bool bIsInside = pGizmo->ScreenGeometry().contains(pEvent->PointerPos());
			QueueEvent( WgEvent::ButtonRelease( button, pGizmo, true, bIsInside ) );
		}
	}

	// Post BUTTON_RELEASE events for all gizmos that were NOT pressed

	for( size_t i = 0 ; i < m_vMarkedGizmos.size() ; i++ )
	{
		WgGizmo * pGizmo = m_vMarkedGizmos[i].GetRealPtr();
		if( pGizmo )
		{
			if( !IsGizmoInList( pGizmo, m_latestPressGizmos[button] ) )
			{
				bool bIsInside = pGizmo->ScreenGeometry().contains(pEvent->PointerPos());
				QueueEvent( WgEvent::ButtonRelease( button, pGizmo, false, bIsInside ) );
			}
		}
	}	

	// As long as the button was pressed inside our window we have a click
	// on this level.

	if( m_pLatestButtonEvents[button] == &m_latestPress[button] )
		QueueEvent( WgEvent::ButtonClick( button ) );

	// Save info for the future

	m_latestRelease[button] = *pEvent;
	m_pLatestButtonEvents[button] = &m_latestRelease[button];

}

//____ ProcessButtonDrag() ____________________________________________________

void WgEventHandler::ProcessButtonDrag( WgEvent::ButtonDrag * pEvent )
{
}

//____ ProcessButtonClick() _________________________________________________

void WgEventHandler::ProcessButtonClick( WgEvent::ButtonClick * pEvent )
{
	int button = pEvent->Button();

	// Post BUTTON_CLICK events for all widgets that received the press and we 
	// still are inside.

	for( size_t i = 0 ; i < m_latestPressGizmos[button].size() ; i++ )
	{
		WgGizmo * pGizmo = m_latestPressGizmos[button][i].GetRealPtr();
		if( pGizmo && IsGizmoInList( pGizmo, m_vMarkedGizmos ) )
			QueueEvent( WgEvent::ButtonClick(button, pGizmo) );
	}
}

//____ ProcessButtonDoubleClick() _________________________________________________

void WgEventHandler::ProcessButtonDoubleClick( WgEvent::ButtonDoubleClick * pEvent )
{
	int button = pEvent->Button();

	// Post BUTTON_DOUBLECLICK events for all widgets that received both this and previous press.

	for( size_t i = 0 ; i < m_latestPressGizmos[button].size() ; i++ )
	{
		WgGizmo * pGizmo = m_latestPressGizmos[button][i].GetRealPtr();
		if( pGizmo && IsGizmoInList( pGizmo, m_previousPressGizmos[button] ) )
			QueueEvent( WgEvent::ButtonDoubleClick(button, pGizmo) );
	}
}

//____ IsGizmoInList() ________________________________________________________

bool WgEventHandler::IsGizmoInList( const WgGizmo * pGizmo, const std::vector<WgGizmoWeakPtr>& list )
{
	for( size_t i = 0 ; i < list.size() ; i++ )
		if( list[i].GetRealPtr() == pGizmo )
			return true;

	return false;
}
