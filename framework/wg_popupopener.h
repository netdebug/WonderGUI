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
#ifndef WG_POPUPOPENER_DOT_H
#define WG_POPUPOPENER_DOT_H

#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef	WG_TEXT_DOT_H
#	include <wg_text.h>
#endif

#ifndef	WG_INTERFACE_TEXTHOLDER_DOT_H
#	include <wg_interface_textholder.h>
#endif

#ifndef	WG_ICONHOLDER_DOT_H
#	include <wg_iconholder.h>
#endif

//____ WgPopupOpener ____________________________________________________________

class WgPopupOpener : public WgWidget, public Wg_Interface_TextHolder, public WgIconHolder
{
public:
	WgPopupOpener();
	virtual ~WgPopupOpener();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgPopupOpener(); };




	//____ Methods __________________________________________

	bool			SetIcon( const WgBlocksetPtr& pIconGfx, WgOrigo origo, WgBorders borders = WgBorders(0), float _scale = 0.f, bool _bPushText = true );
	void			SetIcon( const WgBlocksetPtr& pIconGfx );
	WgBlocksetPtr	GetIconSource() const { return m_pIconGfx; }

	void			SetSkin(const WgSkinPtr& pSkin);

	//.____ Behavior ____________________________________________

	void		SetPopup(WgWidget * pPopup);
	WgWidget*   Popup() const { return m_pPopup; }

	void		SetOpenOnHover(bool bOpen);
	bool		OpenOnHover() const { return m_bOpenOnHover; }

	void		SetAttachPoint(WgOrigo attachPoint);
	WgOrigo		AttachPoint() const { return m_attachPoint; }


	Uint32			GetTextAreaWidth();

	virtual int		MatchingPixelHeight( int width ) const;
//	virtual int		MatchingPixelWidth( int height ) const;

	virtual WgSize	PreferredPixelSize() const;

	bool			IsAutoEllipsisDefault() const { return false; };


protected:

	virtual void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	virtual void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	void			_onRefresh();
	virtual void	_onCloneContent( const WgWidget * _pOrg );
	bool			_onAlphaTest( const WgCoord& ofs );
	void			_onEnable();
	void			_onDisable();
	virtual void	_onNewSize( const WgSize& size );
	void 			_setScale( int scale );

	void			_onGotInputFocus();
	void			_onLostInputFocus();


	void			_setState(WgState state);

	void			_open();
	void			_close();

	void			_textModified();
	void			_iconModified();


	WgText			m_text;

	WgBlocksetPtr	m_pIconGfx;

	WgState			m_state;

	bool			m_bReturnPressed;

	WgWidget *		m_pPopup = nullptr;

	WgOrigo			m_attachPoint;
	bool			m_bOpenOnHover;
	bool			m_bOpen;
	WgState			m_closeState;


};

#endif //WG_POPUPOPENER_DOT_H
