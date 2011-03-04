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

#ifndef WG_EVENTHANDLER_DOT_H
#define WG_EVENTHANDLER_DOT_H

#include <vector>

#ifndef WG_EVENT_DOT_H
#	include <wg_event.h>
#endif

class WgEventHandler
{
public:
	WgEventHandler( Int64 startTime );
	~WgEventHandler();

	bool QueueEvent( const WgEvent::EventBase& _event );

	void ProcessEvents();

	
/*
	bool AddCallback( WgEventId eventId, WgGizmo * pEmitter, void(*fp)(void*), void * pObj );
	bool AddCallback( const WgSignal::Signal_bool& signal, WgEmitter * pEmitter, void(*fp)(void*,bool), void * pObj );
	bool AddCallback( const WgSignal::Signal_int& signal, WgEmitter * pEmitter, void(*fp)(void*,int), void * pObj );
	bool AddCallback( const WgSignal::Signal_float& signal, WgEmitter * pEmitter, void(*fp)(void*,float), void * pObj );
*/


private:

	void	CompleteEvent( WgEvent::EventBase& _event );
	void	ProcessEventInternal( WgEvent::EventBase& _event );

	void	ProcessPointerMoved( WgEvent::PointerMoved * pEvent );

	//

	std::deque<WgEvent>		m_eventQueue;
	bool					m_bIsProcessing;	// Set when we are inside ProcessEvents().
	std::deque<WgEvent>::iterator	m_insertPos;// Position where we insert events being queued when processing.

	Int64					m_time;
	WgCord					m_pointerPos;
	WgModifierKeys			m_modKeys;

	// Settings for keyboard/pointer input

	int			m_doubleClickTreshold;		// Maximum millseconds between first and second press to count as a doubleclick.

	int			m_buttonRepeatDelay;
	int			m_buttonRepeatRate;

	int			m_keyRepeatDelay;
	int			m_keyRepeatRate;

	std::map<int,WgKey>	m_keycodeMap;		// Maps native keycodes to WgKey.

	// Current mouse state

	std::vector<WgGizmoWeakPtr>	m_vInsideWidgets;

	// Current button states

	WgEvent::EventBase	m_latestButtonEvents[WG_MAX_BUTTONS];

	// Current keyboard state



};




#endif //WG_EVENTHANDLER_DOT_H