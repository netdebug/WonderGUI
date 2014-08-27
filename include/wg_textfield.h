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

#ifndef	WG_TEXTFIELD_DOT_H
#define WG_TEXTFIELD_DOT_H

#ifndef	WG_TYPES_DOT_H
#	include <wg_types.h>
#endif

#ifndef WG_IEDITTEXT_DOT_H
#	include <wg_iedittext.h>
#endif

#ifndef WG_COLOR_DOT_H
#	include <wg_color.h>
#endif

#ifndef WG_CHAR_DOT_H
#	include <wg_char.h>
#endif

#ifndef WG_CHARBUFFER_DOT_H
#	include <wg_charbuffer.h>
#endif

#ifndef WG_TEXTMANAGER_DOT_H
#	include <wg_textmanager.h>
#endif

#ifndef	WG_CARETINSTANCE_DOT_H
	#include <wg_caretinstance.h>
#endif

#ifndef WG_SKIN_DOT_H
#	include <wg_skin.h>
#endif

#ifndef WG_VALUEFORMAT_DOT_H
#	include <wg_valueformat.h>
#endif

#ifndef WG_EVENT_DOT_H
#	include <wg_event.h>
#endif

class WgEventHandler;
class WgTextField;

//____ WgTextHolder ___________________________________________________________

struct WgTextHolder
{
public:
	virtual void		_fieldModified( WgTextField * pText ) = 0;
};


//_____ WgTextLine _____________________________________________________________

struct WgTextLine
{
	int		nChars;			// Number of characters on this line (not including break).
	int		ofs;			// Offset in buffer for line.

	int		width;			// Width in pixels of line
	short	height;			// Height in pixels of line
	short	lineSpacing;	// Spacing from this line to next in pixels.
	short	baseline;		// Offset in pixels to the baseline.

	// Following characters can lead to a soft break:
	//
	// Whitespace, tab and BREAK_PERMITTED_HERE: Gives no hyphen.
	// HYPHEN_BREAK_PERMITTED_HERE: Gives additional hyphen when printed.
};

//____ WgTextField __________________________________________________________________

class WgTextField : public WgIEditText
{
friend class WgTextNode;

public:
	WgTextField();
	WgTextField( const WgCharSeq& seq );
	WgTextField( const WgCharBuffer * pBuffer );
	WgTextField( const WgString& str );

	void	Init();

	~WgTextField();

	//

	void				SetManager( const WgTextManagerPtr& pManager );
	WgTextManagerPtr	Manager() const { return m_pManagerNode?m_pManagerNode->GetManager():0; }

	void				SetProperties( const WgTextpropPtr& pProp );
	void				ClearProperties();
	WgTextpropPtr		Properties() const { return m_pBaseProp; }

	void				SetSelectionProperties( const WgTextpropPtr& pProp );
	void				ClearSelectionProperties();
	WgTextpropPtr		SelectionProperties() const { return m_pSelectionProp; }

	void				SetLinkProperties( const WgTextpropPtr& pProp );
	void				ClearLinkProperties();
	WgTextpropPtr		LinkProperties() const { return m_pLinkProp; }

	void				SetFont( const WgFontPtr& pFont );
	void				ClearFont();
	inline WgFontPtr	Font() const { return m_pBaseProp->Font(); }

	void				SetColor( const WgColor color );
	void				SetColor( const WgColor color, WgState state );
	void				ClearColor();
	void				ClearColor( WgState state );
	inline WgColor		Color(WgState state) const { return m_pBaseProp->Color(state); }

	void				SetStyle( WgFontStyle style );
	void				SetStyle( WgFontStyle style, WgState state );
	void				ClearStyle();
	void				ClearStyle( WgState state );
	WgFontStyle			Style(WgState state) const { return m_pBaseProp->Style(state); }

	void				SetBreakLevel( int level );
	void				ClearBreakLevel();
	inline int			BreakLevel() const { return m_pBaseProp->BreakLevel(); }

	void				SetLink( const WgTextLinkPtr& pLink );
	void				ClearLink();
	WgTextLinkPtr		Link() const { return m_pBaseProp->Link(); }

	bool				SetMaxChars( int max );
	int					MaxChars() const { return m_maxChars; }

	inline void			SetAlignment( WgOrigo alignment ) { m_alignment = alignment; }
	inline WgOrigo		Alignment() const { return m_alignment; }

	inline void			SetTintMode( WgTintMode mode ) { m_tintMode = mode; }
	inline WgTintMode	TintMode() const { return m_tintMode; }

	void				Clear();

	void				Set( const WgCharSeq& seq );
	void				Set( const WgCharBuffer * buffer );
	void				Set( const WgString& str );

	int					Append( const WgCharSeq& seq );
	int					Insert( int ofs, const WgCharSeq& seq );
	int					Replace( int ofs, int nDelete, const WgCharSeq& seq );
	int					Delete( int ofs, int len );
	void				DeleteSelected();

	inline void			SetLineSpacing( float adjustment ) { m_lineSpaceAdj = adjustment; }
	inline float		LineSpacing() const { return m_lineSpaceAdj; }

	void				SetWrap( bool bWrap );
	bool				Wrap() const { return m_bWrap; }

	void				SetAutoEllipsis( bool bAutoEllipsis );
	bool				AutoEllipsis() const { return m_bAutoEllipsis; }

	inline WgState		State() const { return m_state; }
	int					Length() const;
	int					Lines() const;
	inline bool			IsEmpty() const { return Length()==0?true:false; }
	inline bool			IsEditable() const { return m_editMode == WG_TEXT_EDITABLE; }
	inline bool			IsSelectable() const { return m_editMode != WG_TEXT_STATIC; }

	int					Width() const;
	int					Height() const;

	void				Select( int ofs, int len ) { if(m_pCursor) m_pCursor->selectRange( WgRange(ofs,len)); }	//TODO: Should not be dependent on a cursor!
	void				SelectAll() { if(m_pCursor) m_pCursor->selectAll(); }									//TODO: Should not be dependent on a cursor!
	int					SelectionStart() const;
	int					SelectionLength() const;
	void				ClearSelection( );

	//

	void				SetCursorSkin( const WgCaretPtr& pCursor );
	inline WgCaretPtr	CursorSkin() const { return m_pCursorStyle; }

	int					InsertAtCursor( const WgCharSeq& str ) { return putText(str); }
	bool				InsertAtCursor( Uint16 c ) { return putChar(c); }

	inline void			GoBOF(){ if(m_pCursor) m_pCursor->goBOF(); }
	inline void			GoEOF(){ if(m_pCursor) m_pCursor->goEOF(); }
	inline void			GoBOL(){ if(m_pCursor) m_pCursor->goBOL(); }
	inline void			GoEOL(){ if(m_pCursor) m_pCursor->goEOL(); }


	//


	//TODO: operator= should copy the whole object, not just the text.

	inline void operator=( const WgTextField& t) { setText(&t); }; // Fastest order to do this in.

	void		setText( const WgTextField * pText );
	void		clone( const WgTextField * pText );

	int			addChar( const WgChar& character );
	int			insertChar( int ofs, const WgChar& character );
	int			replaceChar( int ofs, const WgChar& character );
	int			deleteChar( int ofs );




	inline const WgChar * getText() const { return m_buffer.Chars(); }
	inline WgCharBuffer * getBuffer() { return &m_buffer; }
	inline const WgCharBuffer * getBuffer() const { return &m_buffer; }

	const WgTextLine *	getLines() const;
	WgTextLine *		getLine( int line ) const;
	const WgChar * 		getLineText( int line ) const;
	int					getLineWidth( int line ) const;
	int					getLineOfsY( int line ) const;

	const WgTextLine *	getSoftLines() const;
	WgTextLine *		getSoftLine( int line ) const;
	const WgChar * 		getSoftLineText( int line ) const;
	int					getSoftLineWidth( int line ) const;
	int					getSoftLineWidthPart( int line, int startCol, int nCol = INT_MAX ) const;
	int					nbSoftLines() const;
	int					getSoftLineSelectionWidth( int line ) const;

	void				posSoft2Hard( int &line, int &col ) const;
	void				posHard2Soft( int &line, int &col ) const;


	void				refresh();

//  --------------

	void				selectText( int startLine, int startCol, int endLine, int endCol );
	bool				getSelection( int& startLine, int& startCol, int& endLine, int& endCol ) const;
	WgRange				getSelection() const;

//  --------------

	inline WgTextNode *	getNode() const { return m_pManagerNode; }

	void				_setHolder( WgTextHolder * pHolder ) { m_pHolder = pHolder; }

//  --------------


	void				setSelectionBgColor(WgColor c);
	inline WgColor		getSelectionBgColor() const { return m_pSelectionProp->Color(); }


// -------------

	void				GetBaseAttr( WgTextAttr& attr ) const;
	bool				GetCharAttr( WgTextAttr& attr, int charOfs ) const;
	bool				IsCharUnderlined(	int charOfs ) const;
	WgColor				GetCharColor(		int charOfs ) const;
	WgColor				GetCharBgColor(		int charOfs ) const;
	WgFontStyle			GetCharStyle(		int charOfs ) const;
	int					GetCharSize(		int charOfs ) const;
	WgFontPtr			GetCharFont(		int charOfs ) const;
//	WgGlyphset *		GetCharGlyphset(	int charOfs ) const;
//	bool				GetCharVisibility(	int charOfs ) const;
	int					GetCharBreakLevel(	int charOfs ) const;
	WgTextLinkPtr		GetCharLink(		int charOfs ) const;
	bool				IsCharLink(			int charOfs ) const;

// -------------

	void				setValue( double value, const WgValueFormatPtr& pFormat );
	void				setScaledValue( Sint64 value, Uint32 scale, const WgValueFormatPtr& pFormat );
//	int				compareTo( const WgTextField * pOther, bool bCheckCase = true ) const;	// Textual compare in the style of strcmp().

	WgSize				unwrappedSize() const;
	int					unwrappedWidth() const;				// Width of text if no lines are wrapped.

	int					heightForWidth( int width ) const;

	int					softLineHeight( int line );
	int					softLineSpacing( int line );

	void				setLineWidth( int width );
	inline int			getLineWidth() const { return m_lineWidth; }

	void					SetEditMode(WgTextEditMode mode);							// Maybe should be protected with Widgets/Items as friends?
	inline WgTextEditMode	EditMode() const { return m_editMode; }



	inline void			setState( WgState state ) { m_state = state; }


	// Get-methods


	char *	getTextUTF8() const;
	int		getTextUTF8( char * pDest, int maxBytes ) const;
	int		getTextSizeUTF8() const;

	char *	getTextFormattedUTF8() const;
	int		getTextFormattedUTF8( char * pDest, int maxBytes ) const;
	int		getTextSizeFormattedUTF8() const;

	char *	getSelectedTextFormattedUTF8() const;
	char *	getSelectedTextUTF8() const;


	// cursor instance forwards. TODO: merge cursor instance with WgText
	WgCaretInstance*GetCursor() const { return m_pCursor; }

	void			CursorGotoCoord( const WgCoord& coord, const WgRect& container );
	void			CursorGotoLine( int line, const WgRect& container );
	void			CursorGoUp( int nbLines, const WgRect& container );
	void			CursorGoDown( int nbLines, const WgRect& container );

	void			hideCursor() { if(m_pCursor) m_pCursor->hide(); }
	void			showCursor() { if(m_pCursor) m_pCursor->show(); }
	bool			isCursorShowing() const { return m_pCursor ? !m_pCursor->isHidden() : false; }

	bool			incTime( int ms ) { return m_pCursor ? m_pCursor->incTime(ms) : false; }
	void			insertMode( bool bInsert ) { if(m_pCursor) m_pCursor->insertMode(bInsert); }

	inline void		goLeft( int nChars = 1 ){ if(m_pCursor) m_pCursor->goLeft(nChars); }
	inline void		goRight( int nChars = 1 ){ if(m_pCursor) m_pCursor->goRight(nChars); }

	void			gotoSoftLine( int line, const WgRect& container ){ if(m_pCursor) m_pCursor->gotoSoftLine(line, container); }


	void			gotoSoftPos( int line, int col ){ if(m_pCursor) m_pCursor->gotoSoftPos( line, col );}
	void			gotoColumn( int col ){ if(m_pCursor) m_pCursor->gotoColumn( col );}

	void			gotoPrevWord(){ if(m_pCursor) m_pCursor->gotoPrevWord();}
	void			gotoBeginningOfWord(){ if(m_pCursor) m_pCursor->gotoBeginningOfWord();}
	void			gotoNextWord(){ if(m_pCursor) m_pCursor->gotoNextWord();}
	void			gotoEndOfWord(){ if(m_pCursor) m_pCursor->gotoEndOfWord();}

	void			getSoftPos( int &line, int &col ) const{if(m_pCursor) m_pCursor->getSoftPos( line, col );}

	bool			putChar( Uint16 character ){return m_pCursor ? m_pCursor->putChar( character ):false;}
	int				putText( const WgCharSeq& seq ){return m_pCursor ? m_pCursor->putText( seq ):0;}
	void			unputText( int nChar ){if(m_pCursor) m_pCursor->unputText( nChar );}
	void 			delPrevWord(){if(m_pCursor) m_pCursor->delPrevWord();}
	void 			delNextWord(){if(m_pCursor) m_pCursor->delNextWord();}
	bool			delPrevChar(){return m_pCursor ? m_pCursor->delPrevChar():false;}
	bool			delNextChar(){return m_pCursor ? m_pCursor->delNextChar():false;}

	int				ofsX() const{return m_pCursor ? m_pCursor->ofsX():0;}
	int				ofsY() const{return m_pCursor ? m_pCursor->ofsY():0;}

	inline int		line() const{return m_pCursor ? m_pCursor->line():0;}
	inline int		column() const{return m_pCursor ? m_pCursor->column():0;}
	inline int		time() const{return m_pCursor ? m_pCursor->time():0;}

	WgCaret::Mode	cursorMode() const { return m_pCursor ? m_pCursor->cursorMode() : WgCaret::EOL; }

	void			setSelectionMode(bool bOn){ if(m_pCursor) m_pCursor->setSelectionMode(bOn); }
	bool			hasSelection()const { return m_pCursor ? m_pCursor->hasSelection() : false; }
	void			delSelection(){ if(m_pCursor) m_pCursor->delSelection(); }

	int				LineColToOffset(int line, int col) const;						// HARD LINES!!!!!
//	void			OffsetToSoftLineCol(int ofs, int* wpLine, int* wpCol) const;

	//

	int				LineStartX( int line, const WgRect& container ) const;
	int				LineStartY( int line, const WgRect& container ) const;
	WgCoord			LineStart( int line, const WgRect& container ) const;

	int				CoordToLine( const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	int				CoordToColumn( int line, const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	WgTextPos		CoordToPos( const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	int				CoordToOfs( const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	WgTextLinkPtr	CoordToLink( const WgCoord& coord, const WgRect& container ) const;

	// WgTextPos is automatically clamped for all PosTo...-methods.

	int				PosToCoordX( const WgTextPos& pos, const WgRect& container ) const;
	int				PosToCoordY( const WgTextPos& pos, const WgRect& container ) const;
	WgCoord			PosToCoord( const WgTextPos& pos, const WgRect& container ) const;
	int				PosToOfs( const WgTextPos& pos ) const;

	int				OfsToCoordX( int ofs, const WgRect& container ) const;
	int				OfsToCoordY( int ofs, const WgRect& container ) const;
	WgCoord			OfsToCoord( int ofs, const WgRect& container ) const;
	WgTextPos		OfsToPos( int ofs ) const;

	WgTextPos		ClampPos( WgTextPos ) const;

	//

	WgCoord			FocusWindowOnRange( const WgSize& canvas, const WgRect& window, WgRange range ) const;

	bool 			OnEvent( const WgEventPtr& pEvent, WgEventHandler * pEventHandler, const WgRect& container );
//	bool			OnAction( WgInput::UserAction action, int button_key, const WgRect& textRect, const WgCoord& pointerOfs );

	WgTextLinkPtr	GetMarkedLink() const { return m_pMarkedLink; }
	WgState			GetMarkedLinkState() const { return m_markedLinkState; }

	inline void		SetColorSkin( const WgSkinPtr& pSkin ) { m_pColorSkin = pSkin; }
	inline WgSkinPtr ColorSkin() const { return m_pColorSkin; }

protected:



	static const int	s_parseBufLen = 9+16+1+16+8;
	WgChar *		_parseValue( double value, const WgValueFormat * pFormat, WgChar[s_parseBufLen] );
	WgChar *		_parseScaledValue( Sint64 value, Uint32 scale, const WgValueFormat * pFormat, WgChar[s_parseBufLen] );


	void			_regenHardLines();		// regenerate the softlines-array (if necessary).
	void			_regenSoftLines();		// regenerate the softlines-array (if necessary).
	int 			_countWriteSoftLines( int maxWidth, const WgChar * pStart, WgTextLine * pWriteLines, int maxWrite ) const; // Central algorithm of regenSoftLines().

	void			_refreshAllLines();
	void			_refreshLineInfo( WgTextLine * pLine ) const;

	int				_cursorMaxWidth() const;



	WgCharBuffer	m_buffer;
	WgCaretPtr		m_pCursorStyle;
	WgCaretInstance*	m_pCursor;
	WgTextNode *	m_pManagerNode;

	WgTextEditMode	m_editMode;


	WgTintMode		m_tintMode;
	WgOrigo	m_alignment;
	WgState			m_state;

	WgTextLinkPtr	m_pMarkedLink;
	WgState			m_markedLinkState;

//	int				m_markedLinkOfs;	// Offset in buffer for first character of link currently marked or -1 if none.

	int				m_selStartLine;
	int				m_selEndLine;
	int				m_selStartCol;
	int				m_selEndCol;
	float			m_lineSpaceAdj;		// Adjustment of linespacing for this text.

	WgSkinPtr		m_pColorSkin;		// Skin from which we take low-prio text colors (lowest prio).

	WgTextpropPtr	m_pBaseProp;		// Default properties for this text. Used for all characters who have
										// properties set to 0.
	WgTextpropPtr	m_pLinkProp;		// Props used for links, overriding certain text and char properties.
	WgTextpropPtr	m_pSelectionProp;	// Props used for selected text, overriding certain text, char and link properties.

	WgTextLine*		m_pHardLines;
	WgTextLine*		m_pSoftLines;

	int				m_nSoftLines;
	int				m_nHardLines;

	int				m_lineWidth;

	int				m_newCursorPos;

	int				m_maxChars;			// Maximum number of characters allowed.

	bool			m_bWrap;
	bool			m_bAutoEllipsis;	// Use ellipsis for text that doesn't fit.

	WgTextHolder *	m_pHolder;

	static WgTextLine	g_emptyLine;
};


#endif // WG_TEXTFIELD_DOT_H
