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

#ifndef WG_INTERFACE_TEXTHOLDER_DOT_H
#define WG_INTERFACE_TEXTHOLDER_DOT_H

#ifndef WG_TEXT_DOT_H
#	include <wg_text.h>
#endif

#ifndef WG_COLOR_DOT_H
#	include <wg_color.h>
#endif

class WgFont;

//____ Wg_Interface_TextHolder ________________________________________________

class Wg_Interface_TextHolder : protected WgTextHolder
{

public:
	virtual WgTextEditMode GetEditMode() const  { return WgTextEditMode::Static; }

	virtual void			SetTextManager( WgTextManager * pManager ) { m_pText->setManager( pManager ); _textModified(); }
	inline WgTextManager * GetTextManager() const { return m_pText->getManager(); }

// -------------

	inline WgTextpropPtr GetTextProperties() { return m_pText->getProperties(); }

	inline WgColor			GetTextColor() const { return m_pText->getColor(); }
	inline WgColor			GetTextColor(WgMode mode) const { return m_pText->getColor(mode); }
	inline WgFontStyle		GetTextStyle(WgMode mode) const { return m_pText->getStyle(mode); }
	inline int				GetBreakLevel() const { return m_pText->getBreakLevel(); }
	inline WgFont *			GetFont() const { return m_pText->getFont(); }
	inline WgTextLinkPtr	GetLink() const { return m_pText->getLink(); }

// -------------

	inline void			SetTextProperties( const WgTextpropPtr& pProp )		{ m_pText->setProperties(pProp); _textModified(); }

	inline void			SetTextColor( const WgColor color )					{ if( m_pText->setColor(color) ) _textModified(); }
	inline void			SetTextColor( const WgColor color, WgMode mode )		{ if( m_pText->setColor(color,mode) )  _textModified(); }

	inline void			SetTextStyle( WgFontStyle style )					{ m_pText->setStyle(style); _textModified(); }
	inline void			SetTextStyle( WgFontStyle style, WgMode mode )		{ m_pText->setStyle(style,mode); _textModified(); }

	inline void			SetBreakLevel( int level )							{ m_pText->setBreakLevel(level); _textModified(); }

	inline void			SetFont( WgFont * pFont )							{ m_pText->setFont(pFont); _textModified(); }
	inline void			SetLink( WgTextLinkPtr pLink )						{ m_pText->setLink(pLink); _textModified(); }

// -------------

	inline void			ClearTextProperties()								{ m_pText->clearProperties(); _textModified(); }

	inline void			ClearTextColor()										{ m_pText->clearColor(); _textModified(); }
	inline void			ClearTextColor( WgMode mode )						{ m_pText->clearColor(mode); _textModified(); }

	inline void			ClearTextStyle()										{ m_pText->clearStyle(); _textModified(); }
	inline void			ClearTextStyle( WgMode mode )						{ m_pText->clearStyle(mode); _textModified(); }

	inline void			ClearBreakLevel()									{ m_pText->clearBreakLevel(); _textModified(); }

	inline void			ClearFont()											{ m_pText->clearFont(); _textModified(); }
	inline void			ClearLink()											{ m_pText->clearLink(); _textModified(); }

//	-----------

	inline void			SetSelectionProperties( const WgTextpropPtr& pProp ) { m_pText->setSelectionProperties(pProp); _textModified(); }
	inline WgTextpropPtr GetSelectionProperties() const { return m_pText->getSelectionProperties(); }
	inline void			ClearSelectionProperties() { m_pText->clearSelectionProperties(); _textModified(); }

//	-----------

	inline void			SetLinkProperties( const WgTextpropPtr& pProp ) { m_pText->setLinkProperties(pProp); _textModified(); }
	inline WgTextpropPtr GetLinkProperties() const { return m_pText->getLinkProperties(); }
	inline void			ClearLinkProperties() { m_pText->clearLinkProperties(); _textModified(); }

//	-----------

	inline void			SetCursor( WgCursor * pCursor ) { m_pText->setCursorStyle(pCursor); _textModified(); }
	inline WgCursor *	GetCursor() const { return m_pText->getCursorStyle(); }

//	-----------

	inline void			SetTextBaseColors( const WgColorsetPtr& pColors ) { m_pText->SetBaseColors(pColors); }
	inline WgColorsetPtr TextBaseColors() const { return m_pText->BaseColors(); }

//	-----------

	inline bool			SetMaxChars( int max ) { bool ret = m_pText->SetMaxChars(max); _textModified(); return ret; }
	inline int			MaxChars() const { return m_pText->MaxChars(); }


//	inline void			SetSelectionColor( WgColor color )							{ m_pText->setSelectionBgColor(color); }
//	inline WgColor		GetSelectionColor() const									{ return m_pText->getSelectionBgColor(); }

	void				SetTextMode( WgMode mode );
	void				SetTextAlignment( const WgOrigo alignment );
	void				SetTextTintMode( WgTintMode mode );

	void				ClearText();

	void		SetText( const WgCharSeq& seq );
	void		SetText( const WgCharBuffer * buffer );
	void		SetText( const WgString& str );
	void		SetText( const WgText * pText );

	int			AddText( const WgCharSeq& seq );
	int			InsertText( int ofs, const WgCharSeq& seq );
	int			ReplaceText( int ofs, int nDelete, const WgCharSeq& seq );
	int			DeleteText( int ofs, int len );
	void		DeleteSelectedText();

	int			AddChar( const WgChar& character );
	int			InsertChar( int ofs, const WgChar& character );
	int			ReplaceChar( int ofs, const WgChar& character );
	int			DeleteChar( int ofs );

	void				SetLineSpaceAdjustment( float adjustment );
	float				GetLineSpaceAdjustment() const;

	virtual bool		SetTextWrap(bool bWrap);
	inline bool			GetTextWrap() const { return m_pText->IsWrap(); }

	bool				SetAutoEllipsis(bool bAutoEllipsis);
	inline bool			AutoEllipsis() const { return m_pText->IsAutoEllipsis(); }
	virtual bool		IsAutoEllipsisDefault() const = 0;

	inline WgMode		TextMode() const { return m_pText->mode(); }
	inline WgTintMode	TextTintMode() const { return m_pText->tintMode(); }
	inline WgOrigo TextAlignment() const;
	inline WgText *		TextObj() const;
	inline void			SetTextObj(WgText* text) const;
	inline int			TextLines() const;

	inline Uint32		TextWidth() const { return m_pText->width(); }
	inline Uint32		TextHeight() const { return m_pText->height(); }

protected:
	Wg_Interface_TextHolder();

	inline void			_textModified( WgText * pText ) { _textModified(); }
	virtual void		_textModified() = 0;
	void				_cloneInterface( Wg_Interface_TextHolder * _pClone );
	void				_onCloneContent( const Wg_Interface_TextHolder * pOrg );

	WgText *			m_pText;			// Must always be set by derived class!

};

//____ inline methods __________________________________________________________


WgOrigo Wg_Interface_TextHolder::TextAlignment() const
{
	return m_pText->alignment();
}

WgText* Wg_Interface_TextHolder::TextObj() const
{
	return m_pText;
}

int Wg_Interface_TextHolder::TextLines() const
{
	return m_pText->nbLines();
}



#endif // WG_INTERFACE_TEXTHOLDER_DOT_H
