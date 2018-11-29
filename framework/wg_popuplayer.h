/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_POPUPLAYER_DOT_H
#define WG_POPUPLAYER_DOT_H
#pragma once

#ifndef WG_LAYER_DOT_H
	#include <wg_layer.h>
#endif



class WgPopupLayer;
typedef	WgSmartPtr<WgPopupLayer>	WgPopupLayer_p;
typedef	WgWeakPtr<WgPopupLayer>	WgPopupLayer_wp;

typedef class WgWeakPtr<WgWidget> Widget_wp;


//____ WgPopupHook ___________________________________________________________

class WgPopupHook : public WgLayerHook, protected WgLink		/** @private */
{
	friend class WgPopupLayer;
	friend class WgChain<WgPopupHook>;

public:
	const char *Type(void) const;
	static const char * ClassType();

	// Standard Hook methods

	WgPopupHook *	Prev() const { return _prev(); }
	WgPopupHook *	Next() const { return _next(); }

	WgPopupLayer *	Parent() const;


protected:
	PROTECTED_LINK_METHODS(WgPopupHook);

	WgPopupHook(WgPopupLayer * pParent) : m_pParent(pParent), attachPoint(WgOrigo::WG_NORTHWEST), maxSize(INT_MAX,INT_MAX) {}

	void		_requestRender();
	void		_requestRender(const WgRect& rect);
	void		_requestResize();

	WgHook *	_prevHook() const;
	WgHook *	_nextHook() const;
	WgContainer* _parent() const;


	WgPopupLayer * m_pParent;


	enum class State
	{
		OpeningDelay,			// Popup is in "delayed opening" mode. Some ms before it starts to open.
		Opening,				// Popup is opening (fading in).
		PeekOpen,				// Popup is open until pointer leaves launcherGeo (mode only allowed if bAutoClose is set).
		WeakOpen,				// Popup is open, but closed if other entry of ancestors is peeked (mode only allowed if bAutoClose is set).
		FixedOpen,				// Popup is open until it is closed by a pop() call. 
		ClosingDelay,			// Popup is in countdown to closing mode (mode only allowed if bAutoClose is set).
		Closing,				// Popup is closing (fading out).
	};

	WgRect		launcherGeo;		// Launcher geo relative sibling or parent, measured in pixels.
	WgOrigo		attachPoint;
	WgCoord		attachOfs;			// Offset in points from attachPoint.
	bool		bAutoClose;			// Has been opened in auto-close mode.
	State		state;
	int			stateCounter;		// Counts millisec the slot has been in a transitative state (Delay, Opening, Coundown and Closing).
	WgSize		maxSize;
	Widget_wp	pOpener;			// Widget that opened this popup.
	Widget_wp	pKeyFocus;			// Pointer at widget that held focus when this popup was ontop.
};


	
//____ WgPopupLayer ____________________________________________________________
	
class WgPopupLayer : public WgLayer
{
	friend class WgPopupHook;
public:
	WgPopupLayer();
	virtual ~WgPopupLayer();



	virtual const char *Type(void) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgPopupLayer(); };


	int				NbPopups() const;
	void			Push(WgWidget * pPopup, WgWidget * pOpener, const WgRect& launcherGeo, WgOrigo attachPoint = WgOrigo::WG_NORTHEAST, WgCoord attachOffset = { 0,0 }, bool bAutoClose = false, bool bDelay = false, WgSize maxSize = WgSize(INT_MAX, INT_MAX));
	void			Pop(int nb = 1);
	void			Pop(WgWidget * pPopup);
	void			Clear();

	WgWidget * 		FindWidget(const WgCoord& ofs, WgSearchMode mode) override;

	bool			DeleteChild(WgWidget * pWidget) { return 0; }
	WgWidget *		ReleaseChild(WgWidget * pWidget) { return 0; }

	bool			DeleteAllChildren() { return 0; }
	bool			ReleaseAllChildren() { return 0; }

	void			SetDelays(int popupOpenDelay, int popupFadeInDelay, int popupCloseDelay, int popupFadeOutDelay);

protected:
	virtual WgWidget* _newOfMyType() const { return new WgPopupLayer(); };

	WgPopupLayer *	_getPopupLayer() const { return const_cast<WgPopupLayer*>(this); }
	 
	void			_stealKeyboardFocus();
	void			_restoreKeyboardFocus();
	bool			_updateGeo(WgPopupHook * pSlot, bool bInitialUpdate = false );

	void			_removeSlots(int nb);
	void			_removeSlot(WgPopupHook * p);

	void			_addSlot(WgWidget * pPopup, WgWidget * pOpener, const WgRect& launcherGeo, WgOrigo attachPoint, WgCoord attachOfs, bool bAutoClose, bool bDelay, WgSize maxSize);

	void			_closeAutoOpenedUntil(WgWidget * pStayOpen);


	// Overloaded from Panel

	
	void			_childRequestResize(WgPopupHook * pSlot);
	void			_onRequestRender(const WgRect& rect, const WgPopupHook * pHook);	// rect is in our coordinate system.


	// Overloaded from Layer

	WgLayerHook *	_firstLayerHook() const override;

	WgHook*			_firstHook() const override;		// Fist Hook returned is the normal child, then follows the modal ones.
	WgHook*			_lastHook() const override;		//

	WgHook *		_firstHookWithGeo(WgRect& geo) const override;
	WgHook *		_nextHookWithGeo(WgRect& geo, WgHook * pHook) const override;

	WgHook *		_lastHookWithGeo(WgRect& geo) const override;
	WgHook *		_prevHookWithGeo(WgRect& geo, WgHook * pHook) const override;



	// Overloaded from container

	void			_renderPatches(WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches) override;
//		void			_maskPatches(Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode);
//		void			_collectPatches(Patches& container, const Rect& geo, const Rect& clip);


	// Overloaded from Widget
	
	void			_onCloneContent(const WgWidget * _pOrg) override;
	void			_onNewSize(const WgSize& size) override;
	void			_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;


	WgChain<WgPopupHook>	m_popupHooks;		// First menu lies at the bottom.
	Widget_wp				m_pKeyFocus;	// Pointer at child that held focus before any menu was opened.


	int				m_openingDelayMs = 200;
	int				m_openingFadeMs = 100;
	int				m_closingDelayMs = 200;
	int				m_closingFadeMs = 100;
};
	

#endif //WG_POPUPLAYER_DOT_H
