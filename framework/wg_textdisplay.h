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

#ifndef	WG_TEXTDISPLAY_DOT_H
#define	WG_TEXTDISPLAY_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef	WG_INTERFACE_EDITTEXT_DOT_H
#	include <wg_interface_edittext.h>
#endif

#ifndef	WG_TEXT_DOT_H
#	include <wg_text.h>
#endif

#ifndef WG_CURSORINSTANCE_DOT_H
#	include <wg_cursorinstance.h>
#endif


class WgTextDisplay:public WgWidget, public WgInterfaceEditText
{
public:
	WgTextDisplay();
	virtual ~WgTextDisplay();
	virtual const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgTextDisplay(); };



	//____ Methods __________________________________________

	void		SetSkin(const WgSkinPtr& pSkin);

	inline void		SetMaxLines( int nLines ) { m_maxLines = nLines; }
	inline int		MaxLines() { return m_maxLines; }

	Uint32	InsertTextAtCursor( const WgCharSeq& str );
	bool	InsertCharAtCursor( Uint16 c );

	void		GoBOL();
	void		GoEOL();
	void		GoBOF();
	void		GoEOF();

	virtual void			SetEditMode(WgTextEditMode mode);
	virtual WgTextEditMode	GetEditMode() const { return m_text.GetEditMode(); }

	WgPointerStyle		GetPointerStyle() const;
	WgString			GetTooltipString() const;

	virtual bool TempIsInputField() const { return IsEditable(); }
	virtual Wg_Interface_TextHolder* TempGetText() { return this; }

	int		MatchingPixelHeight( int width ) const;
	WgSize	PreferredPixelSize() const;
	bool	IsAutoEllipsisDefault() const { return true; };

	bool	IsEditable() const { return m_text.IsEditable(); }
	bool	IsSelectable() const { return m_text.IsSelectable(); }

    void    SelectAll() { m_text.selectAll(); }

    /* Added by Niklas */
    void    ChangeText( std::string str )
    {
        if(str.compare(m_str) != 0) {
            SetText(str);
            m_str.assign(str);
        }
    }

    std::string m_str;

protected:

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	void	_onNewSize( const WgSize& size );
	void	_onRefresh();
	void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onEnable();
	void	_onDisable();
	void	_onGotInputFocus();
	void	_onLostInputFocus();
	void 	_setScale( int scale );


private:
	void	_textModified();
	bool	_insertCharAtCursor( Uint16 c );

	WgState				m_state;

	WgText				m_text;
	bool				m_bHasFocus;
	int					m_maxLines;
	bool				m_bResetCursorOnFocus;
};





#endif // WG_TEXTDISPLAY_DOT_H
