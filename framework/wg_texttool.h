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

#ifndef	WG_TEXTTOOL_DOT_H
#define WG_TEXTTOOL_DOT_H

#ifndef WG_USERDEFINES_DOT_H
#	include <wg_userdefines.h>
#endif

#ifndef	WG_TYPES_DOT_H
#	include <wg_types.h>
#endif

#ifndef WG_COLOR_DOT_H
#	include <wg_color.h>
#endif

#ifndef WG_TEXTPROP_DOT_H
#	include <wg_textprop.h>
#endif

#ifndef WG_GLYPHSET_DOT_H
#	include <wg_glyphset.h>
#endif

#ifndef WG_CURSOR_DOT_H
#	include <wg_cursor.h>
#endif

#ifndef WG_CHARSEQ_DOT_H
#	include <wg_charseq.h>
#endif

#ifndef WG_SMARTPTR_DOT_H
#	include <wg_smartptr.h>
#endif

class WgFont;
class WgGlyphset;
class WgText;
class WgChar;
class WgCursorInstance;
class WgTextNode;
class WgColorset;

typedef	WgSmartPtr<WgColorset>	WgColorsetPtr;


//____ WgTextTool _____________________________________________________________

class WgTextTool
{
	friend class WgCharBuffer;
	friend class WgString;
	friend class WgCharSeq;
	friend class WgCharSeq::WgCharBasket;

public:

	inline static bool	cmpUTF8( const char * pChar1, Uint16 char2 );
	inline static int	sizeUTF8( Uint16 ch );

	inline static Uint16 readChar( const char *& pStr );
	static Uint16	parseChar( const char * pStr );	// Should be null terminated

	static Uint32	readString( const char *& pSrc, Uint16 * pDst, Uint32 maxChars = INT_MAX );
	static Uint32	readString( const char *& pSrc, WgChar * pDst, Uint32 maxChars = INT_MAX );
	static Uint32	readString( const Uint16 *& pSrc, WgChar * pDst, Uint32 maxChars = INT_MAX );
	static Uint32	readString( const char *& pSrc, WgCodePage codepage, WgChar * pDst, Uint32 maxChars = INT_MAX );
	static Uint32	readString( const char *& pSrc, WgCodePage codepage, Uint16 * pDst, Uint32 maxChars = INT_MAX );

	static Uint32	countChars( const char * pStr, Uint32 strlen = INT_MAX );

	static Uint32	countLines( const char * pStr );
	static Uint32	countLines( const Uint16 * pStr );
	static Uint32	countLines( const WgChar * pStr );

	static Uint32	countWhitespaces( const char * pStr, Uint32 len = INT_MAX );
	static Uint32	countWhitespaces( const Uint16 * pStr, Uint32 len = INT_MAX  );
	static Uint32	countWhitespaces( const WgChar * pStr, Uint32 len = INT_MAX  );

	static Uint32	countLineChars( const char * pStr, Uint32 len = INT_MAX );
	static Uint32	countLineChars( const Uint16 * pStr, Uint32 len = INT_MAX );
	static Uint32	countLineChars( const WgChar * pStr, Uint32 len = INT_MAX );

	static void		countCharsLines( const char * pStr, Uint32& putChars, Uint32& putLines, Uint32 strlen = INT_MAX );
	static void		countCharsLines( const Uint16 * pStr, Uint32& putChars, Uint32& putLines, Uint32 strlen = INT_MAX );
	static void		countCharsLines( const WgChar * pStr, Uint32& putChars, Uint32& putLines, Uint32 strlen = INT_MAX );

	static Uint32	countNonFormattingChars( const char * pStr, Uint32 strlen = INT_MAX );
	static Uint32	countNonFormattingChars( const Uint16 * pStr, Uint32 strlen = INT_MAX );



	static Uint32	readFormattedString( const char * pSrc, WgChar * pDst, Uint32 maxChars = INT_MAX );
	static Uint32	readFormattedString( const Uint16 * pSrc, WgChar * pDst, Uint32 maxChars = INT_MAX );

	static Uint32	CopyChars( const WgChar * pSrc, WgChar * pDst, Uint32 maxChars = INT_MAX );

	static char * nextLine( char * pStr );


	static Uint32	writeUTF8( Uint16 charcode, char * pDest );

	//____

	static bool		Uint16ToAscii( Uint16 value, Uint16 * pDest, Uint32 maxChars );
	static Uint16	AsciiToUint16( const Uint16 * pAscii );
	static bool		Uint16ToAscii( Uint16 value, char * pDest, Uint32 maxChars );
	static Uint16	AsciiToUint16( const char * pAscii );

	static bool		Uint8ToAscii( Uint8 value, Uint16 * pDest, Uint32 maxChars );
	static Uint8	AsciiToUint8( const Uint16 * pAscii );
	static bool		Uint8ToAscii( Uint8 value, char * pDest, Uint32 maxChars );
	static Uint8	AsciiToUint8( const char * pAscii );

//	static Uint32	formatColor( const WgColor& color, Uint16 * pDest, Uint32 maxChars );
	static Uint32	formatBeginColor( const WgColor& color, char * pDest );

	static Uint32	Uint16ToUTF8( Uint16 value, char * pDest, Uint32 maxChars );


	static Uint32	getTextUTF8( const WgChar * pSrc, char * pDest, Uint32 maxBytes );
	static Uint32	getTextUTF8( const Uint16 * pSrc, char * pDest, Uint32 maxBytes );
	static Uint32	getTextUTF8( const char * pSrc, WgCodePage codepage, char * pDest, int maxChars = INT_MAX );

	static Uint32	getTextSizeUTF8( const WgChar * pSrc, Uint32 maxChars = INT_MAX );
	static Uint32	getTextSizeUTF8( const Uint16 * pSrc, Uint32 maxChars = INT_MAX );
	static Uint32	getTextSizeUTF8( const char * pSrc, WgCodePage codepage, int maxChars = INT_MAX );

	static Uint32	getTextFormatted( const WgChar * pSrc, Uint16 * pDest, Uint32 maxChars );
	static Uint32	getTextSizeFormatted( const WgChar * pSrc, Uint32 maxChars = INT_MAX );

	static Uint32	getTextFormattedUTF8( const WgChar * pSrc, char * pDest, Uint32 maxBytes );
	static Uint32	getTextSizeFormattedUTF8( const WgChar * pSrc, Uint32 maxChars = INT_MAX );

	static Uint32	getTextSizeStrippedUTF8( const char * pStr, Uint32 maxChars = INT_MAX );
	static Uint32	getTextSizeStrippedUTF8( const Uint16 * pStr, Uint32 maxChars = INT_MAX );
	static Uint32	getTextSizeStripped( const char * pStr, Uint32 maxChars = INT_MAX );
	static Uint32	getTextSizeStripped( const Uint16 * pStr, Uint32 maxChars = INT_MAX );



	static int		stripTextCommands( const Uint16* pSrc, Uint16* pDest, int maxChars );
	static int		stripTextCommands( const char* pSrc, char* pDest, int maxBytes );

	static int		stripTextCommandsConvert( const Uint16* pSrc, char* pDest, int maxChars );
	static int		stripTextCommandsConvert( const char* pSrc, Uint16* pDest, int maxChars );


	static int		stripTextColorCommands( const Uint16* pSrc, Uint16* pDest, int maxChars );
	static int		stripTextColorCommands( const char* pSrc, char* pDest, int maxChars );

	static Uint32	strlen( const Uint16 * pSrc );
	static Uint32	strlen( const WgChar * pSrc );
	static int		strcmp( const Uint16 * pStr1, const Uint16 * pStr2 );
	static int		strcmp( const WgChar * pStr1, const WgChar * pStr2 );
	static int		glyphcmp( const WgChar * pStr1, const WgChar * pStr2 );
	static int		glyphcmpIgnoreCase( const WgChar * pStr1, const WgChar * pStr2 );


	static Uint32	lineWidth( WgTextNode * pNode, const WgTextAttr& attr, const char * pString );
	static Uint32	lineWidth( WgTextNode * pNode, const WgTextAttr& attr, const Uint16 * pString );
	static Uint32	lineWidth( WgTextNode * pNode, const WgTextAttr& attr, WgMode mode, const WgChar * pString );

	static void		forwardCharacters( const char *& pChar, Uint32 nChars );
	static void		forwardEscapedCharacters( const char *& pChar, Uint32 nChars );
	static void		forwardEscapedCharacters( const Uint16 *& pChar, Uint32 nChars );

	static void			SetGlyph( Uint16 glyph, WgChar * pChar, Uint32 nb );
	static void			SetChars( const WgChar& ch, WgChar * pChar, Uint32 nb );

	static void			SetProperties( const WgTextpropPtr& pProp, WgChar * pChar, Uint32 nb );
	static void			SetLink( const WgTextLinkPtr& pLink, WgChar * pChar, Uint32 nb );
	static void			SetFont( WgFont * pFont, WgChar * pChar, Uint32 nb );
	static void			SetBreakLevel( int breakLevel, WgChar * pChar, Uint32 nb );
	static void			SetColor( const WgColor col, WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );
	static void			SetSize( int size, WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );
	static void			SetStyle( WgFontStyle style, WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );
	static void			SetUnderlined( WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );

	inline static void	ClearProperties( WgChar * pChar, Uint32 nb ) { SetProperties( 0, pChar, nb ); }
	inline static void	ClearLink( WgChar * pChar, Uint32 nb ) { SetLink(0,pChar,nb); }
	inline static void	ClearFont( WgChar * pChar, Uint32 nb ) { SetFont(0,pChar,nb); }
	inline static void	ClearBreakLevel( WgChar * pChar, Uint32 nb ) { SetBreakLevel(-1,pChar,nb); }
	static void			ClearColor( WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );
	static void			ClearSize( WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );
	static void			ClearStyle( WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );
	static void			ClearUnderlined( WgChar * pChar, Uint32 nb, WgMode mode = WG_MODE_ALL );



	static void			AddPropAttributes( WgTextAttr& attr, const WgTextpropPtr& pProp, WgMode mode = WG_MODE_NORMAL );
	static void			SetAttrColor( WgTextAttr& attr, const WgColorsetPtr& pColors, WgMode mode = WG_MODE_NORMAL );
/*
	static bool				IsCharUnderlined(	Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static const WgColor	GetCharColor(		Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static const WgColor	GetCharBgColor(		Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static WgFontStyle		GetCharStyle(		Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static int				GetCharSize(		Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static WgFont *			GetCharFont(		Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static WgGlyphset *		GetCharGlyphset(	Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static bool				GetCharVisibility(	Uint16 character, Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL, bool bSelected = false );
	static int				GetCharBreakLevel(	Uint16 hCharProp, const WgTextAttr * pAttr, WgMode linkMode = WG_MODE_NORMAL );
	static WgTextLinkPtr	GetCharLink(		Uint16 hCharProp, const WgTextAttr * pAttr );
	static bool				IsCharLink(			Uint16 hCharProp, const WgTextAttr * pAttr );
*/

/*
	static bool				IsCombUnderlined( Uint16 hTextprop, Uint16 hCharProp = 0, WgMode mode = WG_MODE_NORMAL );
	static const WgColor	GetCombColor( Uint16 hTextprop, Uint16 hCharProp = 0, WgMode mode = WG_MODE_NORMAL );
	static const WgColor	GetCombBgColor( Uint16 hTextprop, Uint16 hCharProp = 0, WgMode mode = WG_MODE_NORMAL );
	static WgFontStyle		GetCombStyle( Uint16 hTextprop, Uint16 hCharProp = 0, WgMode mode = WG_MODE_NORMAL );
	static int				GetCombSize( Uint16 hTextprop, Uint16 hCharProp = 0, WgMode mode = WG_MODE_NORMAL );
	static WgTextLinkPtr	GetCombLink( Uint16 hTextprop, Uint16 hCharProp = 0 );
	static WgFont *			GetCombFont( Uint16 hTextprop, Uint16 hCharProp = 0 );
	static WgGlyphset *		GetCombGlyphset( Uint16 hTextprop, Uint16 hCharProp = 0, WgMode mode = WG_MODE_NORMAL );
	static bool				GetCombCharVisibility( Uint16 character, Uint16 hTextprop, Uint16 hCharProp = 0 );
	static int				GetCombBreakLevel( Uint16 hTextprop, Uint16 hCharProp = 0 );
*/
	static WgCursor *		GetCursor( const WgText * pText );
	static WgTextpropPtr	GetSelectionProperties( const WgText * pText );
	static WgTextpropPtr	GetLinkProperties( const WgText * pText );

    static char *           itoa( int value, char * str, int base );

	static WgBreakRules		isBreakAllowed( Uint16 chr, int breakLevel );

	static void				clearBreakRules();
	static bool				setBreakRule( unsigned char character, int level, WgBreakRules rule );
	static void				setDefaultBreakRules();


	//____

	class PropModifier
	{
	public:
		virtual void Modify( WgTextprop& prop ) const = 0;

	protected:
		PropModifier() {};
		virtual ~PropModifier() {}
	};

	class PropSizeModifier : public PropModifier
	{
	public:
		PropSizeModifier( int size, WgMode mode ) { m_size = size; m_mode = mode; }
		void Modify( WgTextprop& prop ) const { prop.SetSize(m_size,m_mode); }
	private:
		int		m_size;
		WgMode	m_mode;
	};

	class PropSizeClearer : public PropModifier
	{
	public:
		PropSizeClearer( WgMode mode ) { m_mode = mode; }
		void Modify( WgTextprop& prop ) const { prop.ClearSize(m_mode); }
	private:
		WgMode	m_mode;
	};


	class PropColorModifier : public PropModifier
	{
	public:
		PropColorModifier( WgColor col, WgMode mode ) { m_col = col; m_mode = mode; }
		void Modify( WgTextprop& prop ) const { prop.SetColor(m_col,m_mode); }
	private:
		WgColor m_col;
		WgMode	m_mode;
	};

	class PropColorClearer : public PropModifier
	{
	public:
		PropColorClearer( WgMode mode ) { m_mode = mode; }
		void Modify( WgTextprop& prop ) const { prop.ClearColor(m_mode); }
	private:
		WgMode	m_mode;
	};


	class PropStyleModifier : public PropModifier
	{
	public:
		PropStyleModifier( WgFontStyle style, WgMode mode ) { m_style = style; m_mode = mode; }
		void Modify( WgTextprop& prop ) const { prop.SetStyle(m_style,m_mode); }
	private:
		WgFontStyle m_style;
		WgMode	m_mode;
	};

	class PropUnderlinedModifier : public PropModifier
	{
	public:
		PropUnderlinedModifier( bool bUnderlined, WgMode mode ) { m_bUnderlined = bUnderlined; m_mode = mode; }
		void Modify( WgTextprop& prop ) const { if( m_bUnderlined) prop.SetUnderlined(m_mode); else prop.ClearUnderlined(m_mode); }
	private:
		bool	m_bUnderlined;
		WgMode	m_mode;
	};

	class PropBreakLevelModifier : public PropModifier
	{
	public:
		PropBreakLevelModifier( int level ) { m_level = level; }
		void Modify( WgTextprop& prop ) const { prop.SetBreakLevel(m_level); }
	private:
		int		m_level;
	};


	class PropLinkModifier : public PropModifier
	{
	public:
		PropLinkModifier( const WgTextLinkPtr& pLink ) { m_pLink = pLink; }
		void Modify( WgTextprop& prop ) const { prop.SetLink( m_pLink ); }
	private:
		WgTextLinkPtr	m_pLink;
	};

	class PropFontModifier : public PropModifier
	{
	public:
		PropFontModifier( WgFont * pFont ) { m_pFont = pFont; }
		void Modify( WgTextprop& prop ) const { prop.SetFont( m_pFont ); }
	private:
		WgFont *	m_pFont;
	};


	//____

	static void ModifyProperties( const PropModifier& modif, WgChar * pChar, Uint32 nb );


	//____

	class TextpropEncoder
	{
	public:
		TextpropEncoder();

		Uint32	BeginString();
		Uint32	SetProp( const WgTextpropPtr& pNewProp );
		Uint32	EndString();

		inline const char * GetCodes() const { return m_temp; }

	private:
		char			m_temp[128];

		bool			m_bColorTagOpen;
		bool			m_bStyleTagOpen;
		bool			m_bSizeTagOpen;
		bool			m_bUnderTagOpen;

		WgTextpropPtr	m_pBaseProp;
		WgTextpropPtr	m_pActiveProp;
	};


private:

	inline static Uint8	NibbleToAscii( Uint8 nibble );
	inline static Uint8	AsciiToNibble( Uint8 ascii );

	static void	RefProps( WgChar * p, Uint32 n );
	static void	DerefProps( WgChar * p, Uint32 n );
};

//____ readChar() _____________________________________________________________

inline Uint16 WgTextTool::readChar( const char *& pStr )
{

#ifdef WG_USE_UTF8

#ifdef WG_SAFE_UTF8	// SAFE UTF8 VERSION

	Uint16 g = * ((const unsigned char *&)pStr)++;

	if( g < 128 )
		return g;											// 1 byte character (7 bits data).

	// Skip incomplete character if we started in the middle of one.

	while( g & 0x40 == 0 )
	{
		if( g == 0 )
			return 0;
		g = * ((const unsigned char *&)pStr)++;
	}

	// Decode character the safer way.

	if( g & 0xE0 == 0xC0 )					// 2 bytes character (11 bits data).
	{
		g &= 0x1F;
		g <<= 6;
		if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
		g += ((*pStr++) & 0x3F);
	}
	else if( g & 0xF0 == 0xE0 )			// 3 bytes character (16 bits data).
	{
		g &= 0x0F;
		g <<= 6;
		if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
		g += ((*pStr++) & 0x3F);
		g <<= 6;
		if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
		g += ((*pStr++) & 0x3F);
	}
	else if( g & 0xF8 == 0xF0 )			// 4 bytes character (21 bits data).
	{
		g &= 0x07;
		g <<= 6;
		if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
		g += ((*pStr++) & 0x3F);
		g <<= 6;
		if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
		g += ((*pStr++) & 0x3F);
		g <<= 6;
		if( * ((const unsigned char *&)pStr) < 128 ) return 0xFFFFFFFF; // Broken character.
		g += ((*pStr++) & 0x3F);
	}
	else
		return 0xFFFFFFFF;					// Broken character (or more than 4 bytes which we don't support)

#else // FAST UTF8 VERSION

	Uint32 g = *((const unsigned char *&)pStr)++;

	if( g < 128 )
		return g;											// 1 byte character (7 bits data).

	g &= 0x3F;
	g <<= 6;
	g += ((*pStr++) & 0x3F);

	if( g >= (1<<11) )								// 3 bytes character (16 bits data).
	{
		g &= (1<<11)-1;
		g <<= 6;
		g += ((*pStr++) & 0x3F);
	}
#endif
	return g;

#else // 8 BIT CHARACTER VERSION
	return (Uint32) * ((const unsigned char *&)pStr)++;
#endif
}

//____ cmpUTF8() ______________________________________________________________

inline bool WgTextTool::cmpUTF8( const char * pChar1, Uint16 char2 )
{
	Uint32 g = (unsigned char) pChar1[0];

	if( g >= 128 )
	{
		g &= 0x3F;
		g <<= 6;
		g += ((pChar1[1]) & 0x3F);

		if( g >= (1<<11) )								// 3 bytes character (16 bits data).
		{
			g &= (1<<11)-1;
			g <<= 6;
			g += ((pChar1[2]) & 0x3F);
		}
	}

	return (g == char2);
}

//____ sizeUTF8() _____________________________________________________________

inline int WgTextTool::sizeUTF8( Uint16 ch )
{
	if( ch < 128 )
		return 1;

	if( ch < (1<<11) )
		return 2;

	return 3;
}



typedef WgTextTool WgTextParser;

#endif //WG_TEXTTOOL_DOT_H
