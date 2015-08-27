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

#include <assert.h>
#include <memory.h>
#include <wchar.h>
#include <wctype.h>
#include <wg_codepages.h>
#include <wg_util.h>

#include <string>

#include <wg_texttool.h>
#include <wg_font.h>
#include <wg_legacytextfield.h>
#include <wg_textpropmanager.h>
#include <wg_charseq.h>
#include <wg_caretinstance.h>
#include <wg_resdb.h>
#include <wg_base.h>
#include <wg_pen.h>

namespace wg 
{
	
	const static char itoa_table [35+36+1]= { "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" };
	
	static char	breakRulesTab[256];
	
	
	//____ parseChar() ____________________________________________________________
	
	Uint16 TextTool::parseChar( const char * pChar )
	{
		if( pChar[0] == 0 )
			return 0;
	
		if( pChar[0] == '0' && pChar[1] == 'x' )
		{
			Uint16 value = 0;
			pChar += 2;
	
			for( char in = * pChar++ ; in != 0 ; in = * pChar++ )
			{
				if( in >= '0' && in <= '9' )
					value = (value << 4) + (in - '0' );
				else if( in >= 'a' && in <= 'f' )
					value = (value << 4 )+ (in - 'a') + 10;
				else if( in >= 'A' && in <= 'F' )
					value = (value << 4 )+ (in - 'A') + 10;
				else
					break;
			}
	
			return value;
		}
		else
		{
			return readChar( pChar );
		}
	
	}
	
	
	
	//____ clearBreakRules() ______________________________________________________
	
	void TextTool::clearBreakRules()
	{
		for( int i = 0 ; i < 256 ; i++ )
			breakRulesTab[i] = WG_NO_BREAK | 0xF;
	}
	
	
	//____ addBreakRule() _________________________________________________________
	
	bool TextTool::setBreakRule( unsigned char character, int level, WgBreakRules rule )
	{
		if( level < 0 || level > 15 )
			return false;
	
		breakRulesTab[character] = rule | level;
		return true;
	}
	
	//____ setDefaultBreakRules() _________________________________________________
	
	void TextTool::setDefaultBreakRules()
	{
		clearBreakRules();
	
		// Set level 1 breaks
	
		breakRulesTab[WG_BREAK_PERMITTED] = WG_BREAK_ON | 0x1;
		breakRulesTab[WG_HYPHEN_BREAK_PERMITTED] = WG_BREAK_ON | 0x1;
	
		// Set level 2 breaks
	
		breakRulesTab[0x20] = WG_BREAK_ON | 0x2;			// Whitespace
		breakRulesTab[0x09] = WG_BREAK_ON | 0x2;			// Tab
	
		// Set level 3 breaks
	
		breakRulesTab['\\']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['/']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['-']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['+']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['*']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['~']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['=']	= WG_BREAK_AFTER | 0x3;
	
		breakRulesTab[')']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['}']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab[']']	= WG_BREAK_AFTER | 0x3;
		breakRulesTab['>']	= WG_BREAK_AFTER | 0x3;
	
		breakRulesTab['(']	= WG_BREAK_BEFORE | 0x3;
		breakRulesTab['{']	= WG_BREAK_BEFORE | 0x3;
		breakRulesTab['[']	= WG_BREAK_BEFORE | 0x3;
		breakRulesTab['<']	= WG_BREAK_BEFORE | 0x3;
	
		// Set level 4 breaks
	
		breakRulesTab[0xbf]	= WG_BREAK_BEFORE | 0x5;	// inverted question mark (beginning of question).
		breakRulesTab['?']	= WG_BREAK_AFTER | 0x5;
		breakRulesTab[':']	= WG_BREAK_AFTER | 0x4;
		breakRulesTab[';']	= WG_BREAK_AFTER | 0x4;
		breakRulesTab['.']	= WG_BREAK_AFTER | 0x5;
		breakRulesTab[',']	= WG_BREAK_AFTER | 0x5;
		breakRulesTab['!']	= WG_BREAK_AFTER | 0x5;
		breakRulesTab['_']	= WG_BREAK_AFTER | 0x4;
		breakRulesTab['|']	= WG_BREAK_AFTER | 0x4;
	}
	
	
	//____ isBreakAllowed() _______________________________________________________
	
	WgBreakRules TextTool::isBreakAllowed( Uint16 chr, int breakLevel )
	{
		if( chr > 255 )
			return WG_NO_BREAK;
	
		if( breakLevel >= (breakRulesTab[chr] & 0xF) )
			return (WgBreakRules) (breakRulesTab[chr] & 0xF0);
	
		return WG_NO_BREAK;
	}
	
	
	//____ itoa() _________________________________________________________________
	
	char * TextTool::itoa( int value, char * str, int base )
	{
	
	    char * p1 = str+33;
	    char * p2 = str;
	
	    if( value < 0 )
	        * p2++ = '-';
	
	    while( value != 0 )
	    {
	        * --p1 = itoa_table[(value%base)+35];
	        value /= base;
	    }
	
	    while( p1 < str+33 )
	        * p2++ = * p1++;
	
	    * p2++ = 0;
	
	    return str;
	}
	
	
	//____ readString() ___________________________________________________________
	
	Uint32 TextTool::readString( const char *& pSrc, Uint16 * pDst, Uint32 maxChars )
	{
		if( !pSrc )
		{
		    if( maxChars > 0 )
				pDst[0] = 0;
			return 0;
		}
		Uint32 n = 0;
	
		while( * pSrc != 0 && n < maxChars )
			pDst[n++] = readChar(pSrc);
	
		if( n != maxChars )
			pDst[n] = 0;
		return n;
	}
	
	
	Uint32 TextTool::readString( const char *& pSrc, Char * pDst, Uint32 maxChars )
	{
		if( !pSrc )
		{
		    if( maxChars > 0 )
	            pDst[0].setGlyph(0);
			return 0;
		}
		Uint32 n = 0;
	
		while( * pSrc != 0 && n < maxChars )
		{
			pDst[n++].setGlyph(readChar(pSrc));
		}
		if( n != maxChars )
			pDst[n].setGlyph(0);
		return n;
	}
	
	Uint32 TextTool::readString( const Uint16 *& pSrc, Char * pDst, Uint32 maxChars )
	{
		if( !pSrc )
		{
		    if( maxChars > 0 )
	            pDst[0].setGlyph(0);
			return 0;
		}
		Uint32 n = 0;
	
		while( * pSrc != 0 && n < maxChars )
			pDst[n++].setGlyph( * pSrc++);
	
		if( n != maxChars )
			pDst[n].setGlyph(0);
		return n;
	}
	
	
	Uint32 TextTool::readString( const char *& pSrc, WgCodePage codepage, Char * pDst, Uint32 maxChars )
	{
		Uint16 * pCP = CodePages::getCodePage( codepage );
		if( !pCP || !pSrc )
		{
		    if( maxChars > 0 )
	            pDst[0].setGlyph(0);
			return 0;
		}
	
		Uint32 n = 0;
		for( unsigned char * p = (unsigned char *) pDst ; p[n] != 0 && n < maxChars ; n++ )
			pDst[n].setGlyph( pCP[p[n]] );
	
		if( n != maxChars )
			pDst[n].setGlyph(0);
		return n;
	}
	
	Uint32 TextTool::readString( const char *& pSrc, WgCodePage codepage, Uint16 * pDst, Uint32 maxChars )
	{
		Uint16 * pCP = CodePages::getCodePage( codepage );
		if( !pCP || !pSrc )
		{
		    if( maxChars > 0 )
	            pDst[0] = 0;
			return 0;
		}
	
		Uint32 n = 0;
		for( unsigned char * p = (unsigned char *) pDst ; p[n] != 0 && n < maxChars ; n++ )
			pDst[n] = pCP[p[n]];
	
		if( n != maxChars )
			pDst[n] = 0;
		return n;
	}
	
	
	//____ CountWhitespaces() ______________________________________________________
	
	Uint32 TextTool::countWhitespaces( const char * pStr, Uint32 len )
	{
		if( !pStr )
			return 0;
	
		Uint32 n = 0;
		for( unsigned int i = 0 ; i < len && pStr[i] != 0 ; i++ )
		{
			if( pStr[i] == ' ' || pStr[i] == WG_NO_BREAK_SPACE )
				n++;
		}
		return n;
	}
	
	Uint32 TextTool::countWhitespaces( const Uint16 * pStr, Uint32 len  )
	{
		if( !pStr )
			return 0;
	
		Uint32 n = 0;
		for( unsigned int i = 0 ; i < len && pStr[i] != 0 ; i++ )
		{
			if( pStr[i] == ' ' || pStr[i] == WG_NO_BREAK_SPACE )
				n++;
		}
		return n;
	}
	
	Uint32 TextTool::countWhitespaces( const Char * pStr, Uint32 len  )
	{
		if( !pStr )
			return 0;
	
		Uint32 n = 0;
		for( unsigned int i = 0 ; i < len && !pStr[i].isEndOfText() ; i++ )
		{
			if( pStr[i].isWhitespace() )
				n++;
		}
		return n;
	}
	
	
	
	//____ countNonFormattingChars() ______________________________________________
	
	Uint32 TextTool::countNonFormattingChars( const char * pStr, Uint32 strlen )
	{
		if( !pStr )
			return 0;
	
		const char * pEnd = pStr + strlen;
		Uint32 n = 0;
		while( * pStr != 0 && pStr != pEnd )
		{
			Uint16 c = readChar(pStr);
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						n++;				// Newline should be counted
						break;
					case '{':				// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
				n++;
		}
		return n;
	}
	
	
	Uint32 TextTool::countNonFormattingChars( const Uint16 * pStr, Uint32 strlen )
	{
		if( !pStr )
			return 0;
	
		const Uint16 * pEnd = pStr + strlen;
		Uint32 n = 0;
		while( * pStr != 0 && pStr != pEnd )
		{
			Uint16 c = * pStr++;
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						n++;				// Newline should be counted
						break;
					case '{':			// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
				n++;
		}
		return n;
	}
	
	
	//____ getTextSizeStripped() ______________________________________________
	
	Uint32 TextTool::getTextSizeStripped( const char * pStr, Uint32 maxChars )
	{
		Uint32 nChar = 0;
	
		while( * pStr != 0 && nChar < maxChars )
		{
			char c = * pStr++;
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						nChar++;				// Newline should be counted
						break;
					case '{':			// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
				nChar++;
	}
		return nChar;
	
	
	}
	
	Uint32 TextTool::getTextSizeStripped( const Uint16 * pStr, Uint32 maxChars )
	{
		Uint32 nChar = 0;
	
		while( * pStr != 0 && nChar < maxChars )
		{
			Uint16 c = * pStr++;
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						nChar++;				// Newline should be counted
						break;
					case '{':			// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
				nChar++;
		}
		return nChar;
	}
	
	
	//____ getTextSizeStrippedUTF8() ______________________________________________
	
	Uint32 TextTool::getTextSizeStrippedUTF8( const char * pStr, Uint32 maxChars )
	{
		Uint32 nChar = 0;
		Uint32 nBytes = 0;
	
		while( * pStr != 0 && nChar < maxChars )
		{
			char c = * pStr++;
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						nChar++;				// Newline should be counted
						nBytes++;
						break;
					case '{':			// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
			{
				nChar++;
				if( (c & 0x80) == 0 )
					nBytes++;
				else if( (c & 0xE0) == 0xC0 )
					nBytes += 2;
				else
					nBytes += 3;
			}
		}
		return nBytes;
	
	
	}
	
	Uint32 TextTool::getTextSizeStrippedUTF8( const Uint16 * pStr, Uint32 maxChars )
	{
		Uint32 nChar = 0;
		Uint32 nBytes = 0;
	
		while( * pStr != 0 && nChar < maxChars )
		{
			Uint16 c = * pStr++;
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						nChar++;				// Newline should be counted
						nBytes++;
						break;
					case '{':			// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
			{
				nChar++;
				if( c < 128 )
					nBytes++;
				else if( c < (1<<11) )
					nBytes += 2;
				else
					nBytes += 3;
			}
		}
		return nBytes;
	}
	
	
	//____ readFormattedString() __________________________________________________
	/**
		@brief	Reads a formatted UTF-8 string into Chars.
	
		Reads a UTF-8 string equipped with escape codes for text properties into
		an array of Char.
	
		@param pSrc		Pointer at source to read from. This must be a null-terminated
						string which may include WG escape codes.
		@param pDst		Pointer at an array of Char where to write the Char converted
						string. The string will be null-terminated if the null-termination
						will fit into maxChars.
		@param maxChars	Maximum number of characters to write to the destination.
	
		@return			Number of Char written into the array, excluding null-terminator.
						This value is <= maxChars. If return value == maxChars, the
						destination is not null-terminated.
	*/
	
	Uint32 TextTool::readFormattedString( const char * _pSrc, Char * pDst, Uint32 maxChars, const ResDB * pResDB )
	{
		if( maxChars == 0 )
			return 0;
	
		if( !_pSrc )
		{
			pDst[0].setGlyph(0);
			return 0;
		}
	
		Char *	pBeginUnderlined = 0;
		Uint32		nUnderlinedRecursions = 0;
	
		Char *	pBeginStyle = 0;
		FontAlt	styleStack[16];
		Uint32		nStyleRecursions = 0;
	
		Char *	pBeginColor = 0;
		Color		colorStack[16];
		Uint32		nColorRecursions = 0;
	
		Char *	pBeginSize = 0;
		int			sizeStack[16];
		Uint32		nSizeRecursions = 0;
	
		Char *	pBeginBreakLevel = 0;
		int			breakLevelStack[16];
		Uint32		nBreakLevelRecursions = 0;
	
	
	
		Char		myChar;
	
		const char * pSrc = _pSrc;		// We need a pointer that can be increased.
	
		Uint32 n = 0;
		while( * pSrc != 0 )
		{
			Uint16 c = readChar(pSrc);
	
			if( c == WG_ESCAPE_CODE )
			{
				Uint8 cmd = * pSrc++;
	
				switch( cmd )
				{
					case 'n':					// Encoded newline
						if( n < maxChars )
						{
							myChar.setGlyph( '\n' );
							pDst[n++] = myChar;
						}
					break;
	
					case ':':					// BEGIN BREAK LEVEL
						if( nBreakLevelRecursions != 0 )
							TextTool::setBreakLevel( breakLevelStack[nBreakLevelRecursions-1], pBeginBreakLevel, pDst + n - pBeginBreakLevel );
	
						if( nBreakLevelRecursions < 16 )
						{
							pBeginBreakLevel = &pDst[n];
	
							int level = pSrc[0] - '0';
							breakLevelStack[nBreakLevelRecursions++] = level;
							pSrc+=1;
						}
					break;
	
					case ';':					// END BREAK LEVEL
						if( nBreakLevelRecursions > 0 )
						{
							nBreakLevelRecursions--;
							TextTool::setBreakLevel( breakLevelStack[nBreakLevelRecursions], pBeginBreakLevel, pDst + n - pBeginBreakLevel );
							pBeginSize = &pDst[n];
						}
					break;
	
	
					case '[':					// BEGIN SIZE
						if( nSizeRecursions != 0 )
							TextTool::setSize( sizeStack[nSizeRecursions-1], pBeginSize, pDst + n - pBeginSize );
	
						if( nSizeRecursions < 16 )
						{
							pBeginSize = &pDst[n];
	
							int size = (pSrc[0] - '0') * 100 + (pSrc[1] - '0') * 10 + (pSrc[2] - '0');
							sizeStack[nSizeRecursions++] = size;
							pSrc+=3;
						}
					break;
	
					case ']':					// END SIZE
						if( nSizeRecursions > 0 )
						{
							nSizeRecursions--;
							TextTool::setSize( sizeStack[nSizeRecursions], pBeginSize, pDst + n - pBeginSize );
							pBeginSize = &pDst[n];
						}
					break;
	
	
					case '{':					// BEGIN COLOR
						if( nColorRecursions != 0 )
							TextTool::setColor( colorStack[nColorRecursions-1], pBeginColor, pDst + n - pBeginColor );
	
						if( nColorRecursions < 16 )
						{
							pBeginColor = &pDst[n];
	
							Color col;
	
	/*						if( pSrc[0] == '#' )
							{
	
							}
							else
	*/						{
								col.a = TextTool::asciiToUint8( &pSrc[0] );
								col.r = TextTool::asciiToUint8( &pSrc[2] );
								col.g = TextTool::asciiToUint8( &pSrc[4] );
								col.b = TextTool::asciiToUint8( &pSrc[6] );
								pSrc += 8;
							}
	
							colorStack[nColorRecursions++] = col;
						}
					break;
	
					case '}':					// END COLOR
						if( nColorRecursions > 0 )
						{
							nColorRecursions--;
							TextTool::setColor( colorStack[nColorRecursions], pBeginColor, pDst + n - pBeginColor );
							pBeginColor = &pDst[n];
						}
					break;
	
					case '_':					// BEGIN UNDERLINED
						if( nUnderlinedRecursions == 0 )
							pBeginUnderlined = &pDst[n];
	
						nUnderlinedRecursions++;
					break;
	
					case '|':					// END UNDERLINED
						nUnderlinedRecursions--;
						if( nUnderlinedRecursions == 0 )
							TextTool::setUnderlined( pBeginUnderlined, pDst + n - pBeginUnderlined );
					break;
	
					case '#':					// END STYLE
						if( nStyleRecursions > 0 )
						{
							nStyleRecursions--;
							TextTool::setStyle( styleStack[nStyleRecursions], pBeginStyle, pDst + n - pBeginStyle );
							pBeginStyle = &pDst[n];
						}
					break;
	
					case '(':					// SET PROP
					{
						if( pResDB )
						{
							// Read the prop id (as normal 7-bit ascii).
	
							std::string id;
							while( * pSrc != ')' )
							{
								assert( * pSrc != 0 );
								id += * pSrc++;
							}
							pSrc++;
	
							if( id == "null" )
								myChar.setProperties( Textprop_p() );
							else
								myChar.setProperties( pResDB->getTextprop(id) );
						}
					}
					break;
	
					default:					// Either setting a font style or this is an invalid command
					{
						bool bOk = true;
						FontAlt style;
	
						// Determine style or that this is an invalid command
	
						switch( cmd )
						{
							case 'd':
								style = WG_FONT_NORMAL;
								break;
							case 'b':
								style = WG_FONT_BOLD;
								break;
							case 'i':
								style = WG_FONT_ITALIC;
								break;
							case 'I':
								style = WG_FONT_BOLD_ITALIC;
								break;
							case 'S':
								style = WG_FONT_SUPERSCRIPT;
								break;
							case 's':
								style = WG_FONT_SUBSCRIPT;
								break;
							case 'm':
								style = WG_FONT_MONOSPACE;
								break;
							case 'h':
							{
								Uint8 c = * pSrc++;
								if( c < '1' || c > '5' )
									bOk = false;
								else
								{
									style = (FontAlt) (WG_FONT_HEADING_1 + c - '1');
									if((Uint32)style >= WG_NB_FONTSTYLES)
										bOk = false;
								}
								break;
							}
							case 'u':
							{
								Uint8 c = * pSrc++;
								if( c < '1' || c > '5' )
									bOk = false;
								else
								{
									style = (FontAlt) (WG_FONT_USER_1 + c - '1');
									if((Uint32)style >= WG_NB_FONTSTYLES)
										bOk = false;
								}
								break;
							}
							default:
								bOk = false;
						}
	
						// Set style unless it was an invalid command
	
						if( bOk )
						{
							if( nStyleRecursions != 0 )
								TextTool::setStyle( styleStack[nStyleRecursions-1], pBeginStyle, pDst + n - pBeginStyle );
	
							if( nStyleRecursions < 16 )
							{
								pBeginStyle = &pDst[n];
								styleStack[nStyleRecursions++] = style;
							}
						}
					}
					break;
				}
			}
			else						// Just a normal character, copy it.
			{
				if( n < maxChars )
				{
					myChar.setGlyph( c );
					pDst[n++] = myChar;
				}
			}
		}
	
		// Terminate the string if possible.
	
		if( n != maxChars )
			pDst[n].setGlyph(0);
	
		return n;
	}
	
	/**
		@brief	Reads a formatted Uint16 string into Chars.
	
		Reads a Uint16 string equipped with escape codes for text properties into
		an array of Char.
	
		@param pSrc		Pointer at source to read from. This must be a null-terminated
						string which may include WG escape codes.
		@param pDst		Pointer at an array of Char where to write the Char converted
						string. The string will be null-terminated if the null-termination
						will fit into maxChars.
		@param maxChars	Maximum number of characters to write to the destination.
	
		@return			Number of Char written into the array, excluding null-terminator.
						This value is <= maxChars. If return value == maxChars, the
						destination is not null-terminated.
	*/
	
	Uint32 TextTool::readFormattedString( const Uint16 * _pSrc, Char * pDst, Uint32 maxChars, const ResDB * pResDB )
	{
		if( maxChars == 0 )
			return 0;
	
		if( !_pSrc )
		{
			pDst[0].setGlyph(0);
			return 0;
		}
	
		Char *	pBeginUnderlined = 0;
		Uint32		nUnderlinedRecursions = 0;
	
		Char *	pBeginStyle = 0;
		FontAlt	styleStack[16];
		Uint32		nStyleRecursions = 0;
	
		Char *	pBeginColor = 0;
		Color		colorStack[16];
		Uint32		nColorRecursions = 0;
	
		Char *	pBeginSize = 0;
		int			sizeStack[16];
		Uint32		nSizeRecursions = 0;
	
		Char *	pBeginBreakLevel = 0;
		int			breakLevelStack[16];
		Uint32		nBreakLevelRecursions = 0;
	
		Char		myChar;
	
		const Uint16 * pSrc = _pSrc;		// We need a pointer that can be increased.
	
		Uint32 n = 0;
		while( * pSrc != 0  )
		{
			Uint16 c = * pSrc++;
	
			if( c == WG_ESCAPE_CODE )
			{
				Uint16 cmd = * pSrc++;
	
				switch( cmd )
				{
					case 'n':					// Encoded newline
						if( n < maxChars )
						{
							myChar.setGlyph( '\n' );
							pDst[n++] = myChar;
						}
					break;
	
					case '[':					// BEGIN SIZE
						if( nSizeRecursions != 0 )
							TextTool::setSize( sizeStack[nSizeRecursions-1], pBeginSize, pDst + n - pBeginSize );
	
						if( nSizeRecursions < 16 )
						{
							pBeginSize = &pDst[n];
	
							int size = (pSrc[0] - '0') * 100 + (pSrc[1] - '0') * 10 + (pSrc[2] - '0');
							sizeStack[nSizeRecursions++] = size;
							pSrc+=3;
						}
					break;
	
					case ']':					// END SIZE
						if( nSizeRecursions > 0 )
						{
							nSizeRecursions--;
							TextTool::setSize( sizeStack[nSizeRecursions], pBeginSize, pDst + n - pBeginSize );
							pBeginSize = &pDst[n];
						}
					break;
	
					case ':':					// BEGIN BREAK LEVEL
						if( nBreakLevelRecursions != 0 )
							TextTool::setBreakLevel( breakLevelStack[nBreakLevelRecursions-1], pBeginBreakLevel, pDst + n - pBeginBreakLevel );
	
						if( nBreakLevelRecursions < 16 )
						{
							pBeginBreakLevel = &pDst[n];
	
							int level = pSrc[0] - '0';
							breakLevelStack[nBreakLevelRecursions++] = level;
							pSrc+=1;
						}
					break;
	
					case ';':					// END BREAK LEVEL
						if( nBreakLevelRecursions > 0 )
						{
							nBreakLevelRecursions--;
							TextTool::setBreakLevel( breakLevelStack[nBreakLevelRecursions], pBeginBreakLevel, pDst + n - pBeginBreakLevel );
							pBeginSize = &pDst[n];
						}
					break;
	
	
					case '{':					// BEGIN COLOR
						if( nColorRecursions != 0 )
							TextTool::setColor( colorStack[nColorRecursions-1], pBeginColor, pDst + n - pBeginColor );
	
						if( nColorRecursions < 16 )
						{
							pBeginColor = &pDst[n];
	
							Color col;
							col.a = TextTool::asciiToUint8( &pSrc[0] );
							col.r = TextTool::asciiToUint8( &pSrc[2] );
							col.g = TextTool::asciiToUint8( &pSrc[4] );
							col.b = TextTool::asciiToUint8( &pSrc[6] );
							pSrc += 8;
	
							colorStack[nColorRecursions++] = col;
						}
					break;
	
					case '}':					// END COLOR
						if( nColorRecursions > 0 )
						{
							nColorRecursions--;
							TextTool::setColor( colorStack[nColorRecursions], pBeginColor, pDst + n - pBeginColor );
							pBeginColor = &pDst[n];
						}
					break;
	
					case '_':					// BEGIN UNDERLINED
						if( nUnderlinedRecursions == 0 )
							pBeginUnderlined = &pDst[n];
	
						nUnderlinedRecursions++;
					break;
	
					case '|':					// END UNDERLINED
						nUnderlinedRecursions--;
						if( nUnderlinedRecursions == 0 )
							TextTool::setUnderlined( pBeginUnderlined, pDst + n - pBeginUnderlined );
					break;
	
					case '#':					// END STYLE
						if( nStyleRecursions > 0 )
						{
							nStyleRecursions--;
							TextTool::setStyle( styleStack[nStyleRecursions], pBeginStyle, pDst + n - pBeginStyle );
							pBeginStyle = &pDst[n];
						}
					break;
	
					case '(':					// SET PROP
					{
						if( pResDB )
						{
							// Read the prop id (as normal 7-bit ascii).
	
							std::string id;
							while( * pSrc != ')' )
							{
								assert( * pSrc != 0 );
								id += (char) (* pSrc++);
							}
							pSrc++;
	
							if( id == "null" )
								myChar.setProperties( Textprop_p() );
							else
								myChar.setProperties( pResDB->getTextprop(id) );
						}
					}
					break;
	
	
					default:					// Either setting a font style or this is an invalid command
					{
						bool bOk = true;
						FontAlt style;
	
						// Determine style or that this is an invalid command
	
						switch( cmd )
						{
							case 'd':
								style = WG_FONT_NORMAL;
								break;
							case 'b':
								style = WG_FONT_BOLD;
								break;
							case 'i':
								style = WG_FONT_ITALIC;
								break;
							case 'I':
								style = WG_FONT_BOLD_ITALIC;
								break;
							case 'S':
								style = WG_FONT_SUPERSCRIPT;
								break;
							case 's':
								style = WG_FONT_SUBSCRIPT;
								break;
							case 'm':
								style = WG_FONT_MONOSPACE;
								break;
							case 'h':
							{
								Uint16 c = * pSrc++;
								if( c < '1' || c > '5' )
									bOk = false;
								else
									style = (FontAlt) (WG_FONT_HEADING_1 + c - '1');
								break;
							}
							case 'u':
							{
								Uint16 c = * pSrc++;
								if( c < '1' || c > '5' )
									bOk = false;
								else
									style = (FontAlt) (WG_FONT_USER_1 + c - '1');
								break;
							}
							default:
								bOk = false;
						}
	
						// Set style unless it was an invalid command
	
						if( bOk )
						{
							if( nStyleRecursions != 0 )
								TextTool::setStyle( styleStack[nStyleRecursions-1], pBeginStyle, pDst + n - pBeginStyle );
	
							if( nStyleRecursions < 16 )
							{
								pBeginStyle = &pDst[n];
								styleStack[nStyleRecursions++] = style;
							}
						}
					}
					break;
				}
			}
			else								// Just a normal character, copy it.
			{
				if( n < maxChars )
				{
					myChar.setGlyph( c );
					pDst[n++] = myChar;
				}
			}
		}
	
		// Terminate the string if possible.
	
		if( n != maxChars )
			pDst[n].setGlyph(0);
	
		return n;
	}
	
	
	
	
	
	//____ getTextFormattedUTF8() __________________________________________________
	
	Uint32 TextTool::getTextFormattedUTF8( const Char * pSrc, char * pDst, Uint32 maxBytes, const ResDB * pResDB )
	{
		Uint32	ofs			= 0;
		Uint16	hActiveProp = 0;
	
		TextpropEncoder	enc(pResDB);
	
		Uint32 n = enc.beginString();
		assert( n == 0 );						// If this has changed we need to add some code here...
	
		while( !pSrc->isEndOfText() )
		{
	
			if( hActiveProp != pSrc->propHandle() )
			{
				// Handle changed properties
	
				n = enc.setProp( pSrc->getProperties() );
	
				if( n > maxBytes - ofs )
					break;								// Can't fit this encoding in, just quit.
	
				strcpy( pDst+ofs, enc.getCodes() );
				ofs += n;
	
				hActiveProp = pSrc->propHandle();
			}
	
	
			// Copy the glyph.
	
			Uint16 glyph = pSrc->getGlyph();
	
			if( glyph < 128 )
			{
				if( ofs >= maxBytes )
					break;					// Can't fit character, so we just terminate.
	
				pDst[ofs++] = (char) glyph;
			}
			else if( glyph < 0x800 )
			{
				if( ofs+1 >= maxBytes )
					break;					// Can't fit character, so we just terminate.
	
				pDst[ofs++] =  0xC0 | ((char) ((glyph & 0x07C0) >> 6));
				pDst[ofs++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			}
			else
			{
				if( ofs+2 > maxBytes )
					break;					// Can't fit character, so we just terminate.
	
				pDst[ofs++] =  0xE0 | ((char) ((glyph & 0xF000) >> 12));
				pDst[ofs++] =  0x80 | ((char) ((glyph & 0x0FC0) >> 6));
				pDst[ofs++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			}
	
			pSrc++;
		}
	
		// Finalize string by adding any last needed encoding.
	
		n = enc.endString();
		if( n <= maxBytes - ofs )
		{
			strcpy( pDst+ofs, enc.getCodes() );
			ofs += n;
		}
	
		//
	
		if( ofs != maxBytes )
			pDst[ofs] = 0;
	
		return ofs;
	}
	
	//____ getTextFormatted() _____________________________________________________
	
	Uint32 TextTool::getTextFormatted( const Char * pSrc, Uint16 * pDst, Uint32 maxBytes, const ResDB * pResDB )
	{
		Uint32	ofs			= 0;
		Uint16	hActiveProp = 0;
	
		TextpropEncoder	enc(pResDB);
		Uint32 n = enc.beginString();
		assert( n == 0 );						// If this has changed we need to add some code here...
	
		while( !pSrc->isEndOfText() )
		{
			if( hActiveProp != pSrc->propHandle() )
			{
				// Handle changed properties
	
				n = enc.setProp( pSrc->getProperties() );
	
				if( n > maxBytes - ofs )
					break;								// Can't fit this encoding in, just quit.
	
				const char * p = enc.getCodes();
				for( unsigned int i = 0 ; i < n ; i++ )
					pDst[ofs++] = * p++;
	
				hActiveProp = pSrc->propHandle();
			}
	
			// Copy the glyph.
	
			if( ofs >= maxBytes )
				break;					// Can't fit character, so we just terminate.
	
			pDst[ofs++] = pSrc->getGlyph();
	
			pSrc++;
		}
	
		// Finalize string by adding any last needed encoding.
	
		n = enc.endString();
		if( n <= maxBytes - ofs )
		{
			const char * p = enc.getCodes();
			for( unsigned int i = 0 ; i < n ; i++ )
				pDst[ofs++] = * p++;
		}
	
		//
	
		if( ofs != maxBytes )
			pDst[ofs] = 0;
	
		return ofs;
	}
	
	//____ getTextSizeFormattedUTF8() _____________________________________________
	
	Uint32 TextTool::getTextSizeFormattedUTF8( const Char * pSrc, Uint32 maxChars, const ResDB * pResDB )
	{
		Uint32 ofs = 0;
		Uint32 charsRead = 0;
	
		Textprop_p	pActiveProp;
		Uint16			hActiveProp = 0;
	
		TextpropEncoder	enc(pResDB);
		ofs += enc.beginString();
	
	
		while( charsRead < maxChars && !pSrc->isEndOfText() )
		{
			// Handle any changes to properties
	
			if( hActiveProp != pSrc->propHandle() )
			{
				ofs += enc.setProp( pSrc->getProperties() );
				hActiveProp = pSrc->propHandle();
			}
	
			// Add the glyph
	
			Uint16 glyph = pSrc->getGlyph();
	
			if( glyph < 128 )
				ofs++;
			else if( glyph < 0x800 )
				ofs+=2;
			else
				ofs+=3;
	
			pSrc++;
			charsRead++;
		}
	
		//
	
		ofs += enc.endString();
	
	
		return ofs;
	}
	
	
	//____ getTextSizeFormatted() _________________________________________________
	
	Uint32 TextTool::getTextSizeFormatted( const Char * pSrc, Uint32 maxChars, const ResDB * pResDB )
	{
		Uint32 ofs = 0;
		Uint32 charsRead = 0;
	
		Textprop_p	pActiveProp;
		Uint16			hActiveProp = 0;
	
		TextpropEncoder	enc(pResDB);
		ofs += enc.beginString();
	
	
		while( charsRead < maxChars && !pSrc->isEndOfText() )
		{
			// Handle any changes to properties
	
			if( hActiveProp != pSrc->propHandle() )
			{
				ofs += enc.setProp( pSrc->getProperties() );
				hActiveProp = pSrc->propHandle();
			}
	
			// Add the glyph
	
			ofs++;
	
			pSrc++;
			charsRead++;
		}
	
		//
	
		ofs += enc.endString();
	
		return ofs;
	}
	
	
	//____ copyChars() ____________________________________________________________
	
	Uint32 TextTool::copyChars( const Char * pSrc, Char * pDst, Uint32 maxChars )
	{
		// Special refProps() equivalent which also counts characters and stops at NULL.
	
		Uint16	hProp = 0;
		Uint16	nProp = 0;
	
		unsigned int n = 0;					// Also number of characters after the for-loop...
		for( ; n < maxChars ; n++ )
		{
	
			Uint16 h = pSrc[n].propHandle();
			if( h == hProp )
				nProp++;
			else
			{
				if( hProp )
					TextpropManager::incRef(hProp, nProp );
	
				hProp = h;
				nProp = 1;
			}
			if( pSrc[n].isEndOfText() )
			{
				n++;
				break;
			}
		}
	
		if( hProp )
			TextpropManager::incRef(hProp, nProp );
	
		// Dereference the same amount of characters from destination.
	
		derefProps( pDst, n );
	
		// Copy chars in a fast and straight way...
	
		memcpy( pDst, pSrc, n*sizeof(Char) );
	
		return n;
	}
	
	
	
	//____ derefProps() ____________________________________________________________
	
	void TextTool::derefProps( Char * p, Uint32 n )
	{
		Uint16	hProp = 0;
		Uint16	nProp = 0;
	
		for( int i = 0 ; i < (int) n ; i++ )
		{
	
			Uint16 h = p[i].propHandle();
			if( h == hProp )
				nProp++;
			else
			{
				if( hProp )
					TextpropManager::decRef(hProp, nProp );
	
				hProp = h;
				nProp = 1;
			}
		}
	
		if( hProp )
			TextpropManager::decRef(hProp, nProp );
	}
	
	
	//____ refProps() ______________________________________________________________
	
	void TextTool::refProps( Char * p, Uint32 n )
	{
		Uint16	hProp = 0;
		Uint16	nProp = 0;
	
		for( unsigned int i = 0 ; i < n ; i++ )
		{
	
			Uint16 h = p[i].propHandle();
			if( h == hProp )
				nProp++;
			else
			{
				if( hProp )
					TextpropManager::incRef(hProp, nProp );
	
				hProp = h;
				nProp = 1;
			}
		}
	
		if( hProp )
			TextpropManager::incRef(hProp, nProp );
	}
	
	
	//____ countChars() ___________________________________________________________
	
	Uint32	TextTool::countChars( const char * pStr, Uint32 strlen )
	{
		if( !pStr )
			return 0;
	
		Uint32 n = 0;
		for( const char * p = pStr ; (Uint32) (p - pStr) < strlen ; n++ )
			if( readChar(p) == 0 )
				break;
	
		return n;
	}
	
	
	//____ countLines() ___________________________________________________________
	
	Uint32	TextTool::countLines( const char * pStr )
	{
		if( !pStr )
			return 0;
	
		Uint32 n = 1;
		while( * pStr != 0 )
		{
			if( * pStr++ == '\n' )
				n++;
		}
	
		return n;
	}
	
	Uint32	TextTool::countLines( const Uint16 * pStr )
	{
		if( !pStr )
			return 0;
	
		Uint32 n = 1;
		while( * pStr != 0 )
		{
			if( * pStr++ == '\n' )
				n++;
		}
	
		return n;
	}
	
	Uint32	TextTool::countLines( const Char * pStr )
	{
		if( !pStr )
			return 0;
	
		Uint32 n = 1;
		while( !pStr->isEndOfText() )
		{
			if( pStr->isEndOfLine() )
				n++;
			pStr++;
		}
	
		return n;
	}
	
	
	
	//____ countLineChars() _______________________________________________________
	/**
		Count characters up until (but not including) the end of the string
		or the first line termination character (CR or LF) found.
	*/
	Uint32	TextTool::countLineChars( const char * pStr, Uint32 len )
	{
		if( !pStr || len == 0 )
			return 0;
	
		Uint32 n = 0;
		const char * pEnd = pStr + len;
	
		Uint16 ch = readChar(pStr);
		while( ch != 0 && ch != 10 && ch != 13 && pStr < pEnd )
		{
			n++;
			ch = readChar(pStr);
		}
		return n;
	}
	
	//____ countLineChars() _______________________________________________________
	/**
		Count characters up until (but not including) the end of the string
		or the first line termination character (CR or LF) found.
	*/
	Uint32	TextTool::countLineChars( const Uint16 * pStr, Uint32 len )
	{
		if( !pStr || len == 0 )
			return 0;
	
		Uint32 n = 0;
	
		Uint16 ch = * pStr++;
		while( ch != 0 && ch != 10 && ch != 13 && n < len )
		{
			n++;
			ch = * pStr++;
		}
		return n;
	}
	
	//____ countLineChars() _______________________________________________________
	/**
		Count characters up until (but not including) the end of the string
		or the first line termination character (CR or LF) found.
	*/
	Uint32	TextTool::countLineChars( const Char * pStr, Uint32 len )
	{
		if( !pStr || len == 0 )
			return 0;
	
		Uint32 n = 0;
	
		while( !pStr->isEndOfLine() && n < len )
			n++;
	
		return n;
	}
	
	
	//____ countCharsLines() ______________________________________________________
	
	void	TextTool::countCharsLines( const char * pStr, Uint32& nChars, Uint32& nLines, Uint32 strlen )
	{
		if( !pStr )
			return;
	
		const char * pEnd = pStr + strlen;
	
		nChars = 0;
		nLines = 1;
	
		while( pStr != pEnd )
		{
			Uint16 c = readChar(pStr);
			if( c == 0 )
				break;
	
			if( c == (Uint16) '\n' )
				nLines++;
	
			nChars++;
		}
	}
	
	void	TextTool::countCharsLines( const Uint16 * pStr, Uint32& nChars, Uint32& nLines, Uint32 strlen )
	{
		if( !pStr )
			return;
	
		const Uint16 * pEnd = pStr + strlen;
	
		nChars = 0;
		nLines = 1;
	
		while( pStr != pEnd )
		{
			Uint16 c = * pStr++;
			if( c == 0 )
				break;
	
			if( c == (Uint16) '\n' )
				nLines++;
	
			nChars++;
		}
	}
	
	
	void	TextTool::countCharsLines( const Char * pStr, Uint32& nChars, Uint32& nLines, Uint32 strlen )
	{
		if( !pStr )
			return;
	
		const Char * pEnd = pStr + strlen;
	
		nChars = 0;
		nLines = 1;
	
		while( pStr + nChars != pEnd )
		{
			if( pStr[nChars].isEndOfText() )
				break;
	
			if( pStr[nChars].isEndOfLine() )
				nLines++;
	
			nChars++;
		}
	}
	
	
	//____ nextLine() _____________________________________________________________
	
	char * TextTool::nextLine( char * p )
	{
		while( * p != 10 )
		{
			if( * p == 0 )
				return p;
			p++;
		}
		p++;
	
		if( * p == 13 )
			p++;
	
		return p;
	}
	
	//____ writeUTF8() ____________________________________________________________
	
	Uint32 TextTool::writeUTF8( Uint16 glyph, char * pDest )
	{
		if( glyph < 128 )
		{
			pDest[0] = (char) glyph;
			return 1;
		}
		else if( glyph < 0x800 )
		{
			pDest[0] =  0xC0 | ((char) ((glyph & 0x07C0) >> 6));
			pDest[1] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			return 2;
		}
		else
		{
			pDest[0] =  0xE0 | ((char) ((glyph & 0xF000) >> 12));
			pDest[1] =  0x80 | ((char) ((glyph & 0x0FC0) >> 6));
			pDest[2] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			return 3;
		}
	}
	
	
	//____ strlen() ____________________________________________________________
	Uint32 TextTool::strlen( const Uint16 * pSrc )
	{
		Uint32 nChars = 0;
		while( * pSrc != 0 )
		{
			++nChars;
			++pSrc;
		}
		return nChars;
	}
	
	
	//____ strlen() ____________________________________________________________
	Uint32 TextTool::strlen( const Char * pSrc )
	{
		Uint32 nChars = 0;
		while( !pSrc->isEndOfText() )
		{
			++nChars;
			++pSrc;
		}
		return nChars;
	}
	
	//____ strcmp() ____________________________________________________________
	int TextTool::strcmp( const Uint16* pStr1, const Uint16* pStr2 )
	{
		while( *pStr1 != L'\0' && *pStr1 == *pStr2 )
		{
			pStr1++;
			pStr2++;
		}
		return *pStr1 - *pStr2;
	}
	
	//____ strcmp() ____________________________________________________________
	int TextTool::strcmp( const Char * pStr1, const Char * pStr2 )
	{
		while( !pStr1->isEndOfText() && pStr1->equals(*pStr2) )
		{
			pStr1++;
			pStr2++;
		}
	
		if( pStr1->isEndOfText() && pStr2->isEndOfText() )
			return 0;
	
		return pStr1->all - pStr2->all;
	}
	
	
	//____ glyphcmp() ____________________________________________________________
	int TextTool::glyphcmp( const Char * pStr1, const Char * pStr2 )
	{
		while( !pStr1->isEndOfText() && pStr1->glyph == pStr2->glyph )
		{
			pStr1++;
			pStr2++;
		}
	
		if( pStr1->isEndOfText() && pStr2->isEndOfText() )
			return 0;
	
		return pStr1->glyph - pStr2->glyph;
	}
	
	//____ glyphcmpIgnoreCase() _______________________________________________________
	int TextTool::glyphcmpIgnoreCase( const Char * pStr1, const Char * pStr2 )
	{
		while( !pStr1->isEndOfText() && towlower(pStr1->glyph) == towlower(pStr2->glyph) )
		{
			pStr1++;
			pStr2++;
		}
	
		if( pStr1->isEndOfText() && pStr2->isEndOfText() )
			return 0;
	
		return towlower(pStr1->glyph) - towlower(pStr2->glyph);
	}
	
	
	
	//____ nibbleToAscii() ____________________________________________________________
	inline Uint8 TextTool::nibbleToAscii( Uint8 nibble )
	{
		if( nibble <= 9 )
			return '0' + nibble;
		else
			return 'A' + ( nibble - 0x0A );
	}
	
	//____ asciiToNibble() ____________________________________________________________
	inline Uint8 TextTool::asciiToNibble( Uint8 ascii )
	{
		if( ascii >= '0' && ascii <= '9' )
			return 0x00 + ascii - '0';
		else if( ascii >= 'A' && ascii <= 'F' )
			return 0x0A + (ascii - 'A');
		else if( ascii >= 'a' && ascii <= 'f' )
			return 0x0A + (ascii - 'a');
	
		return 0;
	}
	
	//____ uint16ToAscii() ____________________________________________________________
	bool TextTool::uint16ToAscii( Uint16 value, Uint16 * pDest, Uint32 maxChars )
	{
		if( 0 == pDest || maxChars < 4 )
			return false;
	
		uint8ToAscii( value >> 8, (Uint16*)&pDest[0], 2 );
		uint8ToAscii( value & 0xFF, (Uint16*)&pDest[2], 2 );
	
		return true;
	}
	
	//____ asciiToUint16() ____________________________________________________________
	Uint16 TextTool::asciiToUint16( const Uint16 * pAscii )
	{
		Uint16 high = asciiToUint8( (Uint16*)&pAscii[0] );
		Uint16 low = asciiToUint8( (Uint16*)&pAscii[2] );
	
		Uint16 value = high << 8 | low;
	
		return value;
	}
	
	//____ uint16ToAscii() ____________________________________________________________
	bool TextTool::uint16ToAscii( Uint16 value, char * pDest, Uint32 maxChars )
	{
		if( 0 == pDest || maxChars < 4 )
			return false;
	
		uint8ToAscii( value >> 8, (char*)&pDest[0], 2 );
		uint8ToAscii( value & 0xFF, (char*)&pDest[2], 2 );
	
		return true;
	}
	
	//____ asciiToUint16() ____________________________________________________________
	Uint16 TextTool::asciiToUint16( const char * pAscii )
	{
		Uint16 high = asciiToUint8( (char*)&pAscii[0] );
		Uint16 low = asciiToUint8( (char*)&pAscii[2] );
	
		Uint16 value = high << 8 | low;
	
		return value;
	}
	
	//____ uint8ToAscii( Uint16* ) ____________________________________________________________
	bool TextTool::uint8ToAscii( Uint8 value, Uint16 * pDest, Uint32 maxChars )
	{
		if( 0 == pDest || maxChars < 2 )
			return false;
	
		Uint8 high = nibbleToAscii( value >> 4 );
		Uint8 low = nibbleToAscii( value & 0x0F );
	
		pDest[ 0 ] = high;
		pDest[ 1 ] = low;
	
		return true;
	}
	
	//____ asciiToUint8() ____________________________________________________________
	Uint8 TextTool::asciiToUint8( const Uint16 * pAscii )
	{
		Uint8 high = asciiToNibble( (Uint8)pAscii[ 0 ] );
		Uint8 low = asciiToNibble( (Uint8)pAscii[ 1 ] );
	
		Uint8 value = high << 4 | low;
	
		return value;
	}
	
	//____ uint8ToAscii( char* ) ____________________________________________________________
	bool TextTool::uint8ToAscii( Uint8 value, char * pDest, Uint32 maxChars )
	{
		if( 0 == pDest || maxChars < 2 )
			return false;
	
		Uint8 high = nibbleToAscii( value >> 4 );
		Uint8 low = nibbleToAscii( value & 0x0F );
	
		pDest[ 0 ] = high;
		pDest[ 1 ] = low;
	
		return true;
	}
	
	//____ asciiToUint8() ____________________________________________________________
	Uint8 TextTool::asciiToUint8( const char * pAscii )
	{
		Uint8 high = asciiToNibble( pAscii[ 0 ] );
		Uint8 low = asciiToNibble( pAscii[ 1 ] );
	
		Uint8 value = high << 4 | low;
	
		return value;
	}
	
	//____ uint16ToUtf8() ____________________________________________________________
	Uint32 TextTool::uint16ToUtf8( Uint16 value, char * pDest, Uint32 maxChars )
	{
		if( maxChars < 1 )
			return 0;
	
		unsigned int nChars = 0;
	
		Uint16 glyph = value;
		if( glyph < 128 )
		{
			pDest[nChars++] = (char) glyph;
			if( nChars == maxChars )
				return nChars;
		}
		else if( glyph < 0x800 )
		{
			pDest[nChars++] =  0xC0 | ((char) ((glyph & 0x07C0) >> 6));
			if( nChars == maxChars )
				return nChars;
	
			pDest[nChars++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			if( nChars == maxChars )
				return nChars;
		}
		else
		{
			pDest[nChars++] =  0xE0 | ((char) ((glyph & 0xF000) >> 12));
			if( nChars == maxChars )
				return nChars;
	
			pDest[nChars++] =  0x80 | ((char) ((glyph & 0x0FC0) >> 6));
			if( nChars == maxChars )
				return nChars;
	
			pDest[nChars++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			if( nChars == maxChars )
				return nChars;
		}
	
		return nChars;
	}
	
	//____ formatBeginColor() _____________________________________________________
	
	Uint32 TextTool::formatBeginColor( const Color& color, char * pDest )
	{
		pDest += writeUTF8( WG_ESCAPE_CODE, pDest );
		* pDest++ = '{';
	
		uint8ToAscii( color.a, pDest, 2 );
		uint8ToAscii( color.r, pDest+2, 2 );
		uint8ToAscii( color.g, pDest+4, 2 );
		uint8ToAscii( color.b, pDest+6, 2 );
	
		return sizeUTF8( WG_ESCAPE_CODE ) + 9;
	}
	
	
	//____ getTextUTF8() __________________________________________________________
	
	/**
		Writes a UTF8 representation of the Char-string (or first part thereof) into
		the specified area. Char effects such as underline, font changes etc are
		ignored.
	
	
		@param	maxBytes		Maximum number of bytes for the line, including
								termination character.
	
	
	
		@returns		Length of UTF8-string written to the area (excluding
						termination character). No incomplete multibyte characters
						are written, so number of bytes written can be less
						than maxChars-1 even when whole string didn't fit.
	
	*/
	
	Uint32 TextTool::getTextUTF8( const Char * pSrc, char * pDest, Uint32 maxBytes )
	{
		if( maxBytes < 1 || pSrc == 0 || pDest == 0 )
			return 0;
	
		Uint32 nChars = 0;
		Uint16 glyph;
	
		while( (glyph = pSrc->getGlyph()) != 0 )
		{
			if( glyph < 128 )
			{
				if( nChars +2 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.
	
				pDest[nChars++] = (char) glyph;
			}
			else if( glyph < 0x800 )
			{
				if( nChars +3 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.
	
				pDest[nChars++] =  0xC0 | ((char) ((glyph & 0x07C0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			}
			else
			{
				if( nChars +4 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.
	
				pDest[nChars++] =  0xE0 | ((char) ((glyph & 0xF000) >> 12));
				pDest[nChars++] =  0x80 | ((char) ((glyph & 0x0FC0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			}
	
			pSrc++;
		}
	
		pDest[nChars] = 0;
		return nChars;
	}
	
	Uint32 TextTool::getTextUTF8( const Uint16 * pSrc, char * pDest, Uint32 maxBytes )
	{
		if( maxBytes < 1 || pSrc == 0 || pDest == 0 )
			return 0;
	
		Uint32 nChars = 0;
		Uint16 glyph;
	
		while( (glyph = * pSrc) != 0 )
		{
			if( glyph < 128 )
			{
				if( nChars +2 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.
	
				pDest[nChars++] = (char) glyph;
			}
			else if( glyph < 0x800 )
			{
				if( nChars +3 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.
	
				pDest[nChars++] =  0xC0 | ((char) ((glyph & 0x07C0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			}
			else
			{
				if( nChars +4 > maxBytes )
					break;					// Can't fit character + termination, so we just terminate.
	
				pDest[nChars++] =  0xE0 | ((char) ((glyph & 0xF000) >> 12));
				pDest[nChars++] =  0x80 | ((char) ((glyph & 0x0FC0) >> 6));
				pDest[nChars++] =  0x80 | ((char) ((glyph & 0x003F) >> 0));
			}
	
			pSrc++;
		}
	
		pDest[nChars] = 0;
		return nChars;
	}
	
	//____ getTextUTF8() __________________________________________________________
	
	Uint32 TextTool::getTextUTF8( const char * pSrc, WgCodePage codepage, char * pDest, int maxChars )
	{
		Uint16 * pCP = CodePages::getCodePage( codepage );
		if( !pCP )
		{
			pDest[0] = 0;
			return 0;
		}
	
		unsigned char * p = (unsigned char *) pSrc;
	
		int nBytes = 0;
		for( int i = 0 ; i < maxChars && p[i] != 0 ; i++ )
			nBytes += writeUTF8( pCP[p[i]], pDest + nBytes );
		
		pDest[nBytes] = 0;					// Yes, we do terminate destination!
		return nBytes;
	}
	
	
	
	//____ getTextSizeUTF8() ______________________________________________________
	
	Uint32 TextTool::getTextSizeUTF8( const Char* pSrc, Uint32 len )
	{
		Uint32 size = 0;
		Uint16 glyph = 0;
		for( Uint32 i = 0 ; i < len && (0 != ( glyph = pSrc->getGlyph()) ) ; i++ )
		{
			size++;
			if( glyph > 127 )
			{
				size++;
				if( glyph > 0x7FF )
					size++;
			}
			pSrc++;
		}
	
		return size;
	}
	
	//____ getTextSizeUTF8() ____________________________________________________________
	
	Uint32 TextTool::getTextSizeUTF8( const Uint16* pSrc, Uint32 len )
	{
		Uint32 size = 0;
		Uint16 glyph = 0;
		for( Uint32 i = 0 ; i < len && (0 != ( glyph = * pSrc) ) ; i++ )
		{
			size++;
			if( glyph > 127 )
			{
				size++;
				if( glyph > 0x7FF )
					size++;
			}
			pSrc++;
		}
	
		return size;
	}
	
	//____ getTextSizeUTF8() ____________________________________________________________
	
	Uint32 TextTool::getTextSizeUTF8( const char * pSrc, WgCodePage codepage, int maxChars )
	{
		Uint16 * pCP = CodePages::getCodePage( codepage );
		if( !pCP )
			return 0;
	
		unsigned char * p = (unsigned char *) pSrc;
	
		int nBytes = 0;
		for( int i = 0 ; i < maxChars && p[i] != 0 ; i++ )
			nBytes += sizeUTF8( p[i] );
	
		return nBytes;
	}
	
	
	//____ lineWidth() ____________________________________________________________
	
	Uint32 TextTool::lineWidth( const TextAttr& attr, const char * pString )
	{
		Pen pen;
		pen.setAttributes( attr );
	
		while( * pString != 0 && * pString != '\n' )
		{
			pen.setChar( * pString++ );
			pen.applyKerning();
			pen.advancePos();
		}
	
		// We include the terminator in case it is set to be visible.
	
		pen.setChar( * pString );
		pen.applyKerning();
		pen.advancePos();
	
		return pen.getPosX();
	}
	
	Uint32 TextTool::lineWidth( const TextAttr& attr, const Uint16 * pString )
	{
		Pen pen;
		pen.setAttributes( attr );
	
		while( * pString != 0 && * pString != '\n' )
		{
			pen.setChar( * pString++ );
			pen.applyKerning();
			pen.advancePos();
		}
	
		// We include the terminator in case it is set to be visible.
	
		pen.setChar( * pString );
		pen.applyKerning();
		pen.advancePos();
	
		return pen.getPosX();
	}
	
	
	Uint32 TextTool::lineWidth( const TextAttr& attr, State state, const Char * pString )
	{
		TextAttr	attr2;
	
		Pen pen;
		Uint16 hProp = 0xFFFF;
	
		while( !pString->isEndOfLine() )
		{
			if( pString->propHandle() != hProp )
			{
				attr2 = attr;
				addPropAttributes( attr2, pString->getProperties(), state );
				pen.setAttributes( attr2 );
			}
			pen.setChar( pString->getGlyph() );
			pen.applyKerning();
			pen.advancePos();
			pString++;
		}
	
		// We include the terminator in case it is set to be visible.
	
		pen.setChar( pString->getGlyph() );
		pen.applyKerning();
		pen.advancePos();
	
		return pen.getPosX();
	}
	
	//____ forwardCharacters() ____________________________________________________
	
	void TextTool::forwardCharacters( const char *& pChar, Uint32 nChars )
	{
		for( unsigned int i = 0 ; i < nChars && * pChar != 0 ; i++ )
		{
			char c = * pChar++;
	
			if( (c & 0xC0) == 0xC0 )			// Beginning of multibyte character
			{
				pChar++;						// skip one byte
	
				if( (c & 0xE0) == 0xE0 )		// Beginning of 3+ byte character?
				{
					pChar++;					// skip one byte
	
					if( (c & 0xF0) == 0xF0 )	// Beginning of 4 byte character? (not really supported, but we don't want a crash)
						pChar++;
				}
			}
		}
	
	}
	
	//____ forwardEscapedCharacters() ____________________________________________________
	
	void TextTool::forwardEscapedCharacters( const char *& pStr, Uint32 nChars )
	{
		Uint32 n = 0;
		while( * pStr != 0 && n < nChars )
		{
			Uint16 c = readChar(pStr);
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						n++;				// Newline should be counted
						break;
					case '{':				// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
				n++;
		}
	}
	
	
	//____ forwardEscapedCharacters() ____________________________________________________
	
	void TextTool::forwardEscapedCharacters( const Uint16 *& pStr, Uint32 nChars )
	{
		Uint32 n = 0;
		while( * pStr != 0 && n < nChars )
		{
			Uint16 c = * pStr++;
	
			if( c == WG_ESCAPE_CODE )
			{
				switch( * pStr )
				{
					case 'n':				// Newline
						pStr++;
						n++;				// Newline should be counted
						break;
					case '{':				// Begin color
						pStr += 7;
					break;
	
					case 'h':			// Start heading
					case 'u':			// Start user style
						pStr += 2;
					break;
	
					case '(':			// Set textprop
						while( * pStr != 0 && * pStr != ')' )
							pStr++;
						if( pStr != 0 )
							pStr++;
					break;
	
					default:
						pStr++;
				}
			}
			else
				n++;
		}
	}
	
	
	//____ stripTextCommands() ____________________________________________________
	
	int TextTool::stripTextCommands( const Uint16* pSrc, Uint16* pDest, int maxChars )
	{
		int n = 0;
		while( n < maxChars )
		{
			Uint16 c = * pSrc++;
	
			if( c == 0 )
			{
				pDest[n] = 0;
				break;
			}
	
			if( c == WG_ESCAPE_CODE )
			{
				if( * pSrc == '(' )
				{
					while( * pSrc != ')' )
						pSrc++;
					pSrc++;
				}
				else if( * pSrc == 'h' || * pSrc == 'u' )
					pSrc += 2;
				else if( * pSrc == '{' )
					pSrc += 9;
				else
					pSrc++;
			}
			else
				pDest[n++] = c;
		}
	
		return n;
	}
	
	
	int TextTool::stripTextCommands( const char* pSrc, char* pDest, int maxChars )
	{
		int n = 0;
		while( n < maxChars )
		{
			Uint16 c = readChar( pSrc );
	
			if( c == 0 )
			{
				pDest[n] = 0;
				break;
			}
	
			if( c == WG_ESCAPE_CODE )
			{
				if( * pSrc == '(' )
				{
					while( * pSrc != ')' )
						pSrc++;
					pSrc++;
				}
				if( * pSrc == 'h' || * pSrc == 'u' )
					pSrc += 2;
				else if( * pSrc == '{' )
					pSrc += 9;
				else
					pSrc++;
			}
			else
				n += writeUTF8( c, pDest + n );
		}
	
		return n;
	}
	
	
	int TextTool::stripTextColorCommands( const Uint16* pSrc, Uint16* pDest, int maxChars )
	{
		int n = 0;
		while( n < maxChars )
		{
			Uint16 c = * pSrc++;
	
			if( c == 0 )
			{
				pDest[n] = 0;
				break;
			}
	
			if( c == WG_ESCAPE_CODE && *pSrc == '{')
				pSrc += 9;
			else if( c == WG_ESCAPE_CODE && *pSrc == '}' )
				pSrc++;
			else
				pDest[n++] = c;
		}
	
		return n;
	}
	
	
	int TextTool::stripTextColorCommands( const char* pSrc, char* pDest, int maxBytes )
	{
		int n = 0;
		while( n < maxBytes )
		{
			Uint16 c = readChar( pSrc );
	
			if( c == 0 )
			{
				pDest[n] = 0;
				break;
			}
	
			if( c == WG_ESCAPE_CODE && *pSrc == '{')
				pSrc += 9;
			else if( c == WG_ESCAPE_CODE && *pSrc == '}' )
				pSrc++;
			else
				n += writeUTF8( c, pDest+n );
		}
	
		return n;
	}
	
	
	//____ stripTextCommandsConvert() _____________________________________________
	
	int TextTool::stripTextCommandsConvert( const Uint16* pSrc, char* pDest, int maxChars )
	{
		int n = 0;
		int ofs = 0;
		while( n < maxChars )
		{
			Uint16 c = * pSrc++;
	
			if( c == 0 )
			{
				pDest[ofs] = 0;
				break;
			}
	
			if( c == WG_ESCAPE_CODE )
			{
				if( * pSrc == '(' )
				{
					while( * pSrc != ')' )
						pSrc++;
					pSrc++;
				}
				else if( * pSrc == 'h' || * pSrc == 'u' )
					pSrc += 2;
				else if( * pSrc == '{' )
					pSrc += 9;
				else
					pSrc++;
			}
			else
			{
				ofs += writeUTF8( c, pDest + ofs );
				n++;
			}
		}
	
		return n;
	}
	
	
	int TextTool::stripTextCommandsConvert( const char* pSrc, Uint16* pDest, int maxChars )
	{
		int n = 0;
		while( n < maxChars )
		{
			Uint16 c = readChar( pSrc );
	
			if( c == 0 )
			{
				pDest[n] = 0;
				break;
			}
	
			if( c == WG_ESCAPE_CODE )
			{
				if( * pSrc == '(' )
				{
					while( * pSrc != ')' )
						pSrc++;
					pSrc++;
				}
				if( * pSrc == 'h' || * pSrc == 'u' )
					pSrc += 2;
				else if( * pSrc == '{' )
					pSrc += 9;
				else
					pSrc++;
			}
			else
				pDest[n++] = c;
		}
	
		return n;
	}
	
	//____ setSize() _____________________________________________________________
	
	void TextTool::setSize( int size, Char * pChar, Uint32 nb )
	{
		modifyProperties( PropSizeModifier(size), pChar, nb );
	}
	
	void TextTool::setSize( int size, Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateSizeModifier(size,state), pChar, nb );
	}
	
	
	//____ clearSize() ___________________________________________________________
	
	void TextTool::clearSize( Char * pChar, Uint32 nb )
	{
		modifyProperties( PropSizeClearer(), pChar, nb );
	}
	
	void TextTool::clearSize( Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateSizeClearer(state), pChar, nb );
	}
	
	//____ setColor() _____________________________________________________________
	
	void TextTool::setColor( const Color col, Char * pChar, Uint32 nb )
	{
		modifyProperties( PropColorModifier(col), pChar, nb );
	}
	
	void TextTool::setColor( const Color col, Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateColorModifier(col,state), pChar, nb );
	}
	
	
	//____ clearColor() ___________________________________________________________
	
	void TextTool::clearColor( Char * pChar, Uint32 nb )
	{
		modifyProperties( PropColorClearer(), pChar, nb );
	}
	
	void TextTool::clearColor( Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateColorClearer(state), pChar, nb );
	}
	
	
	//____ setStyle() _____________________________________________________________
	
	void TextTool::setStyle( FontAlt style, Char * pChar, Uint32 nb )
	{
		modifyProperties( PropStyleModifier(style), pChar, nb );
	}
	
	void TextTool::setStyle( FontAlt style, Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateStyleModifier(style,state), pChar, nb );
	}
	
	//____ clearStyle() ___________________________________________________________
	
	void TextTool::clearStyle( Char * pChar, Uint32 nb )
	{
		modifyProperties( PropStyleModifier(WG_FONT_NORMAL), pChar, nb );
	}
	
	void TextTool::clearStyle( Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateStyleModifier(WG_FONT_NORMAL,state), pChar, nb );
	}
	
	
	//____ setUnderlined() ________________________________________________________
	
	void TextTool::setUnderlined( Char * pChar, Uint32 nb )
	{
		modifyProperties( PropUnderlinedModifier(true), pChar, nb );
	}
	
	void TextTool::setUnderlined( Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateUnderlinedModifier(true,state), pChar, nb );
	}
	
	//____ clearUnderlined() ________________________________________________________
	
	void TextTool::clearUnderlined( Char * pChar, Uint32 nb )
	{
		modifyProperties( PropUnderlinedModifier(false), pChar, nb );
	}
	
	void TextTool::clearUnderlined( Char * pChar, Uint32 nb, State state )
	{
		modifyProperties( PropStateUnderlinedModifier(false,state), pChar, nb );
	}
	
	//____ setBreakLevel() ________________________________________________________
	
	void TextTool::setBreakLevel( int breakLevel, Char * pChar, Uint32 nb )
	{
		modifyProperties( PropBreakLevelModifier(breakLevel), pChar, nb );
	}
	
	//____ setLink() ______________________________________________________________
	
	void TextTool::setLink( const TextLink_p& pLink, Char * pChar, Uint32 nb )
	{
		modifyProperties( PropLinkModifier(pLink), pChar, nb );
	}
	
	
	//____ setFont() ______________________________________________________________
	
	void TextTool::setFont( const Font_p& pFont, Char * pChar, Uint32 nb )
	{
		modifyProperties( PropFontModifier(pFont), pChar, nb );
	}
	
	//____ setGlyph() ______________________________________________________________
	
	void TextTool::setGlyph( Uint16 glyph, Char * pChar, Uint32 nb )
	{
		for( unsigned int i = 0 ; i < nb ; i++ )
			pChar[i].glyph = glyph;
	}
	
	//____ setChars() ______________________________________________________________
	
	void TextTool::setChars( const Char& ch, Char * pChar, Uint32 nb )
	{
		derefProps( pChar, nb );
	
		for( unsigned int i = 0 ; i < nb ; i++ )
			pChar[i].all = ch.all;
	
		if( ch.propHandle() != 0 )
			TextpropManager::incRef( ch.propHandle(), nb );
	}
	
	
	
	//____ setProperties() ________________________________________________________
	
	void TextTool::setProperties( const Textprop_p& pProp, Char * pChar, Uint32 nb )
	{
		Uint32		refCnt = 0;
		Uint32		refCntTotal = 0;
		Uint16		old_prop = 0xFFFF;
		Uint16		new_prop = pProp.getHandle();
	
		for( unsigned int i = 0 ; i < nb ; i++ )
		{
			if( pChar[i].properties != old_prop )
			{
				if( refCnt != 0 )
				{
					if( old_prop != 0 )
						TextpropManager::decRef( old_prop, refCnt );
	
					refCntTotal += refCnt;
					refCnt = 0;
				}
				old_prop = pChar[i].properties;
			}
	
			pChar[i].properties = new_prop;
			refCnt++;
		}
	
		if( refCnt != 0 && old_prop != 0 )
			TextpropManager::decRef( old_prop, refCnt );
	
		if( new_prop != 0 )
			TextpropManager::incRef( new_prop, refCntTotal + refCnt );
	}
	
	//____ modifyProperties() __________________________________________________________
	
	void TextTool::modifyProperties( const PropModifier& modif, Char * pChar, Uint32 nb  )
	{
		Uint32		refCnt = 0;
		Uint16		old_prop = 0xFFFF;
		Uint16		new_prop = 0xFFFF;
	
		for( unsigned int i = 0 ; i < nb ; i++ )
		{
			if( pChar[i].properties != old_prop )
			{
				if( refCnt != 0 )
				{
					// Increase first, in case they are the same...
	
					if( new_prop != 0 )
						TextpropManager::incRef( new_prop, refCnt );
	
					if( old_prop != 0 )
						TextpropManager::decRef( old_prop, refCnt );
	
					refCnt = 0;
				}
				old_prop = pChar[i].properties;
	
				Textprop prop = TextpropManager::getProp(pChar[i].properties);
				modif.modify( prop );
				new_prop = TextpropManager::registerProp(prop);
			}
	
			pChar[i].properties = new_prop;
			refCnt++;
		}
	
		if( refCnt != 0 )
		{
			if( new_prop != 0 )
				TextpropManager::incRef( new_prop, refCnt );
	
			if( old_prop != 0 )
				TextpropManager::decRef( old_prop, refCnt );
		}
	}
	
	
	//____ addPropAttributes() ________________________________________________________
	
	void TextTool::addPropAttributes( TextAttr& attr, const Textprop_p& pProp, State state )
	{
		if( !pProp )
			return;
	
		if( pProp->font() )
			attr.pFont = pProp->font();
	
		if( pProp->size(state) != 0 )
			attr.size = pProp->size(state);
	
		if( pProp->style(state) != WG_FONT_NORMAL )
			attr.style = pProp->style(state);
	
		if( pProp->isColored(state) )
			attr.color = pProp->color(state);
	
		if( pProp->isBgColored(state) )
			attr.bgColor = pProp->bgColor(state);
	
		if( pProp->isUnderlined(state) )
			attr.bUnderlined = true;
	
		if( pProp->breakLevel() != -1 )
			attr.breakLevel = pProp->breakLevel();
	
		attr.visibilityFlags |= pProp->charVisibilityFlags();
	
		if( pProp->link() )
			attr.pLink = pProp->link();
	}
	
	/*
	//____ setAttrColor() _______________________________________________________
	
	void TextTool::setAttrColor( TextAttr& attr, const ColorsetPtr& pColors, State state )
	{
		if( !pColors )
			return;
	
		attr.color = pColors->color(state);
	}
	*/
	
	//____ getCursor() ____________________________________________________________
	
	Caret_p TextTool::getCursor( const LegacyTextField * pText )
	{
		Caret_p p = pText->cursorSkin();
		if( p )
			return p;
	
		return Base::getDefaultCursor();
	}
	
	//____ getSelectionProperties() _______________________________________________
	
	Textprop_p TextTool::getSelectionProperties( const LegacyTextField * pText )
	{
		Textprop_p p = pText->selectionProperties();
		if( p )
			return p;
	
		return Base::getDefaultSelectionProp();
	}
	
	//____ getLinkProperties() ____________________________________________________
	
	Textprop_p TextTool::getLinkProperties( const LegacyTextField * pText )
	{
		Textprop_p p = pText->linkProperties();
		if( p )
			return p;
	
		return Base::getDefaultLinkProp();
	}
	
	//____ TextpropEncoder::Constructor ___________________________________________
	
	TextTool::TextpropEncoder::TextpropEncoder( const ResDB * pResDB )
	{
		m_pResDB = pResDB;
	}
	
	//____ TextpropEncoder::beginString() _________________________________________
	
	Uint32 TextTool::TextpropEncoder::beginString()
	{
		m_bColorTagOpen = false;
		m_bStyleTagOpen = false;
		m_bSizeTagOpen	= false;
		m_bUnderTagOpen = false;
	
		m_pBaseProp = 0;
		m_pActiveProp = 0;
	
		return 0;
	}
	
	//____ TextpropEncoder::setProp() _____________________________________________
	
	Uint32 TextTool::TextpropEncoder::setProp( const Textprop_p& pNewProp )
	{
		Uint32 i = 0;
	
		// First, see if we can do this using only current "baseprop" + style/color/size/underline settings.
	
		if( pNewProp->font() == m_pBaseProp->font() && pNewProp->link() == m_pBaseProp->link() &&
			((pNewProp->isColored() && pNewProp->isColorStatic()) || pNewProp->compareColorTo( m_pBaseProp )) &&
			(pNewProp->isStyleStatic() || pNewProp->compareStyleTo( m_pBaseProp )) &&
			(pNewProp->isSizeStatic() || pNewProp->compareSizeTo( m_pBaseProp )) &&
			((pNewProp->isUnderlined() && pNewProp->isUnderlineStatic()) || pNewProp->compareUnderlineTo( m_pBaseProp )) )
		{
			// Yes we can!
	
			// nothing to do here, we are all setup.
		}
		else if( m_pResDB )
		{
			// Secondly, if nullprop isn't our current baseprop we see if we can do this using only nullprop
			// + style/color/size/underline settings.
	
			if( !m_pBaseProp && !pNewProp->font() && !pNewProp->link() &&
				pNewProp->isColorStatic() && pNewProp->isStyleStatic() && pNewProp->isSizeStatic() && pNewProp->isUnderlined() )
			{
				// Yes we can! Switch to nullprop as our baseprop
	
				i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
				strcpy( m_temp + i, "(null)" );
				i += 6;
	
				m_pBaseProp = 0;
			}
			else
			{
				// Thirdly, see if we have a perfect match
	
				std::string id = m_pResDB->findTextpropId( pNewProp );
	
				if( id.length() > 0 )
				{
					// Yes, we have! Switch to this prop.
	
					i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
					strcpy( m_temp + i, id.c_str() );
					i += id.length();
	
					m_pBaseProp = m_pResDB->getTextprop( id );
				}
				else
				{
					// Fourthly, look for the first possible match which can be combined with style/color/size/underline settings
					// to make a perfect match.
	
					ResDB::TextpropRes * pRes = m_pResDB->getFirstResTextprop();
					while( pRes )
					{
						Textprop_p pProp = pRes->res;
	
						if( pNewProp->font() == pProp->font() && pNewProp->link() == pProp->link() &&
							((pNewProp->isColored() && pNewProp->isColorStatic()) || pNewProp->compareColorTo( pProp )) &&
							(pNewProp->isStyleStatic() || pNewProp->compareStyleTo( pProp )) &&
							(pNewProp->isSizeStatic() || pNewProp->compareSizeTo( pProp )) &&
							((pNewProp->isUnderlined() && pNewProp->isUnderlineStatic()) || pNewProp->compareUnderlineTo( pProp )) )
						{
							// This one works! Switch to this prop.
	
							i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
							strcpy( m_temp + i, pRes->id.c_str() );
							i += pRes->id.length();
	
							m_pBaseProp = pProp;
	
						}
	
						pRes = pRes->next();
					}
				}
			}
	
		}
		else
		{
			// Failure! We switch to nullprop and do the best of the situation with style/color/size/underline-settings.
	
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			strcpy( m_temp + i, "(null)" );
			i += 6;
	
			m_pBaseProp = 0;
		}
	
		//================================================================
	
		// Possibly end active color
	
		if( m_bColorTagOpen && (!pNewProp->isColored() || pNewProp->compareColorTo( m_pBaseProp ) || !pNewProp->compareColorTo( m_pActiveProp ) )  )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '}';
	
			m_bColorTagOpen = false;
		}
	
		// Possibly start new color.
	
		if( !m_bColorTagOpen && pNewProp->isColored() && !pNewProp->compareColorTo( m_pBaseProp ) )
		{
			Color col = pNewProp->color();
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '{';
	
			uint8ToAscii( col.a, &m_temp[i], 2 );
			i += 2;
			uint8ToAscii( col.r, &m_temp[i], 2 );
			i += 2;
			uint8ToAscii( col.g, &m_temp[i], 2 );
			i += 2;
			uint8ToAscii( col.b, &m_temp[i], 2 );
			i += 2;
	
			m_bColorTagOpen = true;
		}
	
		// Possibly begin/end underline
	
		if( (pNewProp->isUnderlined() && pNewProp->isUnderlineStatic()) &&
			!m_bUnderTagOpen && !pNewProp->compareUnderlineTo( m_pBaseProp ) )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '_';
	
			m_bUnderTagOpen = true;
		}
		else if( m_bUnderTagOpen )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '|';
	
			m_bUnderTagOpen = false;
		}
	
		// Possibly end current style and/or start new style.
	
		if( m_bStyleTagOpen && (!pNewProp->compareStyleTo( m_pActiveProp ) || pNewProp->compareStyleTo( m_pBaseProp )) )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '#';
	
			m_bStyleTagOpen = false;
		}
	
		if( !m_bStyleTagOpen && !pNewProp->compareStyleTo( m_pBaseProp ) && pNewProp->isStyleStatic() )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
	
			switch( pNewProp->style() )
			{
				case WG_FONT_NORMAL:
					m_temp[i++] = 'd';
					break;
				case WG_FONT_BOLD:
					m_temp[i++] = 'b';
					break;
				case WG_FONT_BOLD_ITALIC:
					m_temp[i++] = 'i';
					break;
				case WG_FONT_ITALIC:
					m_temp[i++] = 'I';
					break;
				case WG_FONT_SUPERSCRIPT:
					m_temp[i++] = 'S';
					break;
				case WG_FONT_SUBSCRIPT:
					m_temp[i++] = 's';
					break;
				case WG_FONT_MONOSPACE:
					m_temp[i++] = 'm';
					break;
				case WG_FONT_HEADING_1:
				case WG_FONT_HEADING_2:
				case WG_FONT_HEADING_3:
				case WG_FONT_HEADING_4:
				case WG_FONT_HEADING_5:
					m_temp[i++] = 'h';
					m_temp[i++] = '1' + (pNewProp->style() - WG_FONT_HEADING_1);
					break;
				case WG_FONT_USER_1:
				case WG_FONT_USER_2:
				case WG_FONT_USER_3:
				case WG_FONT_USER_4:
				case WG_FONT_USER_5:
					m_temp[i++] = 'u';
					m_temp[i++] = '1' + (pNewProp->style() - WG_FONT_USER_1);
					break;
			}
	
			m_bStyleTagOpen = true;
		}
	
		// Possibly end current size and/or start new size.
	
		if( m_bSizeTagOpen && (!pNewProp->compareSizeTo( m_pActiveProp ) || pNewProp->compareSizeTo( m_pBaseProp )) )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = ']';
	
			m_bSizeTagOpen = false;
		}
	
		if( !m_bSizeTagOpen && !pNewProp->compareSizeTo( m_pBaseProp ) && pNewProp->isSizeStatic() )
		{
			int size = pNewProp->size();
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '[';
	
			m_temp[i++] = size/100 + '0';
			m_temp[i++] = ((size/10) % 10) + '0';
			m_temp[i++] = (size % 10) + '0';
	
			m_bSizeTagOpen = true;
		}
	
		m_pActiveProp = pNewProp;
		m_temp[i] = 0;
		return i;
	}
	
	//____ TextpropEncoder::endString() ___________________________________________
	
	Uint32 TextTool::TextpropEncoder::endString()
	{
		Uint32 i = 0;
	
		if( m_pActiveProp->style() != WG_FONT_NORMAL )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '#';
		}
	
		if( m_pActiveProp->isUnderlined() )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '|';
		}
	
		if( m_pActiveProp->isColored() )
		{
			i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
			m_temp[i++] = '}';
		}
	
		if( m_pBaseProp )
		{
				i += writeUTF8( WG_ESCAPE_CODE, m_temp+i );
				strcpy( m_temp + i, "(null)" );
				i += 6;
		}
	
		return i;
	}

} // namespace wg