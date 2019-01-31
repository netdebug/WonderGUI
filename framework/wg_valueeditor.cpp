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


#include <wg_valueeditor.h>

#include <math.h>
#include <stdlib.h>
#include <wg_gfxdevice.h>
#include <wg_font.h>
#include <wg_cursorinstance.h>
#include <wg_key.h>
#include <wg_textmanager.h>
#include <wg_event.h>
#include <wg_eventhandler.h>

static const char Wdg_Type[] = {"ValueEditor"};


//____ WgValueEditor() _________________________________________________________________

WgValueEditor::WgValueEditor()
{
	_regenText();
	m_buttonDownOfs = 0;
	m_bSelectAllOnRelease = false;
	m_maxInputChars = 256;
	m_viewOfs = WgCoord(0,0);

	m_pointerStyle	= WgPointerStyle::Ibeam;

	m_text.SetWrap(false);
	m_text.SetAutoEllipsis(false);
	m_text.SetEditMode( WgTextEditMode::Editable );
}

//____ ~WgValueEditor() ___________________________________________________________

WgValueEditor::~WgValueEditor()
{
}

//____ Type() _________________________________________________________________

const char * WgValueEditor::Type( void ) const
{
	return GetClass();
}

const char * WgValueEditor::GetClass( void )
{
	return Wdg_Type;
}

//____ SetTextColor() _________________________________________________________

void WgValueEditor::SetTextColor( WgColor col )
{
	m_text.setColor( col );
	_regenText();			// Is this necessary?
	_requestRender();
}

WgColor WgValueEditor::GetTextColor() const
{
	return m_text.getColor(WG_MODE_NORMAL);
}

//____ SetTextprop() __________________________________________________________

bool WgValueEditor::SetTextprop( const WgTextpropPtr& _pProp )
{
	if( _pProp != m_text.getProperties() )
	{
		m_text.setProperties(_pProp);
		_regenText();		// Is this necessary?
		_requestRender();
	}

	return true;
}

WgTextpropPtr WgValueEditor::GetTextprop( ) const
{
	return m_text.getProperties();
}


//____ SetFormat() ____________________________________________________________

void WgValueEditor::SetFormat( const WgValueFormat& format )
{
	m_format		= format;
	m_useFormat		= format;

	_regenText();
	_requestRender();
}

//____ SetTextAlignment() _____________________________________________________

void WgValueEditor::SetTextAlignment( const WgOrigo alignment )
{
	if( m_text.alignment() != alignment )
	{
		m_text.setAlignment(alignment);
		_requestRender();
	}
}

//____ GetTextAlignment() _____________________________________________________

WgOrigo WgValueEditor::GetTextAlignment( ) const
{
	return m_text.alignment();
}

//____ SetTextManager() _______________________________________________________

void WgValueEditor::SetTextManager(WgTextManager * _pManager)
{
	if( m_text.getManager() != _pManager )
	{
		m_text.setManager( _pManager );
	}
}

//____ GetTextManager() _______________________________________________________

WgTextManager * WgValueEditor::GetTextManager() const
{
	return m_text.getManager();
}


//____ Clear() ________________________________________________________________

void WgValueEditor::Clear()
{
	// Force value to zero, emit signals and request render

	m_value = 0;
	_valueModified();

	// Make the inputfield empty

	m_useFormat.integers = 0;
	m_useFormat.bForcePeriod = false;
	m_useFormat.decimals = 0;
}

//____ SetMaxInputChars() _____________________________________________________

bool WgValueEditor::SetMaxInputChars( int max )
{
	if( max <= 0 )
		return false;
	m_maxInputChars = max;
	return true;
}


//____ PreferredPixelSize() __________________________________________________________

WgSize WgValueEditor::PreferredPixelSize() const
{
	WgSize	sz;

	sz.h = m_text.height();
	sz.w = 0;

	WgTextAttr	attr;
	m_text.GetBaseAttr( attr );
	if( attr.pFont )
		sz.w = attr.pFont->GetGlyphset(attr.style,attr.size)->GetMaxGlyphAdvance(attr.size)*5;	// By default fit at least 5 characters

	return sz;
}


//____ _valueModified() ________________________________________________________

void WgValueEditor::_valueModified()
{
	_queueEvent( new WgEvent::EditvalueSet(this,m_value,FractionalValue()) );

	m_useFormat	= m_format;

	if( 0 != m_format.noDecimalThreshold && (int)m_value >= m_format.noDecimalThreshold )
	{
		// value is >= noDecimalThreshold so prevent period and decimals
		m_useFormat.bForcePeriod = false;
		m_useFormat.decimals = 0;
	}
	_regenText();
	_requestRender();
}

//____ _rangeModified() ________________________________________________________

void WgValueEditor::_rangeModified()
{
		_queueEvent( new WgEvent::EditvalueSet(this,m_value,FractionalValue()) );
}


//____ _onRefresh() ____________________________________________________________

void WgValueEditor::_onRefresh( void )
{
	if( m_text.getFont() != 0 )
	{
		_regenText();
		_requestRender();
	}

}


//____ _onRender() _____________________________________________________________

void WgValueEditor::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip )
{
	// Leave if we have nothing to print

	if( m_text.nbLines() == 0 )
		return;

	// Adjust text canvas

	WgRect	canvas = _canvas;

	if( m_text.isCursorShowing() )
	{
		int textW = m_text.width();
		if( textW > canvas.w )
			canvas.w = textW;

		WgCursorInstance * pCursor = m_text.GetCursor();
		m_viewOfs = m_text.FocusWindowOnRange( canvas.size(), WgRect(m_viewOfs,_canvas.size()), WgRange( pCursor->column(),0 ) );
	}
	else
		m_viewOfs = WgCoord(0,0);

	// Print the text

	pDevice->PrintText( _clip, &m_text, canvas - m_viewOfs );
}

//____ _regenText() ____________________________________________________________

void WgValueEditor::_regenText()
{
		m_text.setScaledValue( m_value, m_format.scale, m_useFormat );
		m_text.goEOL();
}

//____ _parseValueFromInput() __________________________________________________

bool WgValueEditor::_parseValueFromInput( int64_t * wpResult )
{
	// Parse text as value as a positive or negative integer within boundaries
	// return false if value had to be modified in any way.

	const WgChar * p = m_text.getBuffer()->Chars();
	int		nbChars = m_text.getBuffer()->NbChars();

	int64_t	value = 0;
	bool	bModified = false;			// Set when value displayed isn't fully acceptable (outside boundaries or has too many decimals)
	int		nDecimals = 0;

	if( nbChars > 0 )
	{
		for( int i = 0 ; i < nbChars ; i++ )
		{
			Uint16 glyph = p[i].Glyph();

			if( glyph == '-' )
				continue;

			if( glyph == m_format.period )
			{
				nDecimals = nbChars-i-1;
				if( nDecimals > m_format.decimals )
				{
					// Only parse as many decimals as we need, avoid unnecessary overflows.
					nbChars -= nDecimals - m_format.decimals;
					nDecimals = m_format.decimals;
					bModified = true;
				}

			}
			else if( glyph >= '0' && glyph <= '9' )
			{
				if( value <= LLONG_MAX/10 )
					value *= 10;
				else
				{
					value = LLONG_MAX;
					bModified = true;
					break;
				}

				int toAdd = glyph - '0';

				if( value <= LLONG_MAX-toAdd )
					value += toAdd;
				else
				{
					value = LLONG_MAX;
					bModified = true;
					break;
				}
			}

		}

		if( nDecimals < m_format.decimals )
		{
			int64_t multiplier = (int64_t) pow(10.0,m_format.decimals-nDecimals);

			if( value <= LLONG_MAX/multiplier )
				value *= multiplier;
			else
			{
				value = LLONG_MAX;
				bModified = true;
			}
		}

		if( p[0].Glyph() == '-' )
		{
			if( value == LLONG_MAX )
				value = m_rangeMin;
			else
			{
				value = -value;
				if( value < m_rangeMin )
					value = m_rangeMin;
			}
		}
		else
		{
			if( value > m_rangeMax )
				value = m_rangeMax;
		}

	}

	* wpResult = value;
	return bModified;
}

//____ _onEvent() ______________________________________________________________

void WgValueEditor::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	WgEventType event = pEvent->Type();

	if( event == WG_EVENT_TICK )
	{
		if( m_text.GetCursor() )
		{
			m_text.GetCursor()->incTime( static_cast<const WgEvent::Tick*>(pEvent)->Millisec() );
			_requestRender();					//TODO: Should only render the cursor (and only when updated)!
		}
		return;
	}


	bool	bTextChanged = false;
	int		mousebutton = 0;

	if( pEvent->IsMouseButtonEvent() )
		mousebutton = static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button();

	WgCoord ofs = pEvent->PointerPixelPos();

	if( event == WG_EVENT_MOUSEBUTTON_PRESS && mousebutton == 1 )
	{
		if( !m_bFocused )
		{
			GrabFocus();
			m_bSelectAllOnRelease = true;
		}
		else
			m_bSelectAllOnRelease = false;

		if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
		{
			m_text.setSelectionMode(true);
			m_text.CursorGotoCoord( ofs, WgRect(0,0,PixelSize()) );
			_limitCursor();
		}
		else
		{
			m_text.setSelectionMode(false);
			m_text.clearSelection();
			m_text.CursorGotoCoord( ofs, WgRect(0,0,PixelSize()) );
			_limitCursor();
			m_text.setSelectionMode(true);
		}

		m_buttonDownOfs = ofs.x;
	}

	if( event == WG_EVENT_MOUSEBUTTON_DRAG && mousebutton == 1 )
	{
		if( m_bFocused && ofs.x != m_buttonDownOfs )
		{
			m_text.CursorGotoCoord( ofs, WgRect(0,0,PixelSize()) );
			_limitCursor();
			m_buttonDownOfs = ofs.x;
			m_bSelectAllOnRelease = false;
		}
	}

	if( event == WG_EVENT_MOUSEBUTTON_RELEASE )
	{
		if( m_bFocused && mousebutton == 1 )
		{
			m_text.setSelectionMode(false);

			if( m_bSelectAllOnRelease )
				_selectAll();
		}
	}


	if( event == WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK && mousebutton == 1 )
	{
		_selectAll();
		m_text.setSelectionMode(true);
	}


	if( event == WG_EVENT_KEY_PRESS || event == WG_EVENT_KEY_REPEAT )
	{
		int key = static_cast<const WgEvent::KeyEvent*>(pEvent)->TranslatedKeyCode();
		switch( key )
		{
			case WG_KEY_RETURN:
			{
				// We already have correct value in m_value, but we want
				// to set the bModified flag if value is entered in a weird way.

				int64_t		value;
				bool bModified = _parseValueFromInput( &value );

				if( m_value < m_rangeMin )
				{
					m_value = m_rangeMin;
					bModified = true;
				}

				if( m_value > m_rangeMax )
				{
					m_value = m_rangeMax;
					bModified = true;
				}

				if( bModified )
				{
					_updateSlider( FractionalValue(), 0.f );
					_queueEvent( new WgEvent::EditvalueModify(this, m_value, FractionalValue()) );

					m_text.setScaledValue( m_value, m_format.scale, m_useFormat );
					m_text.goEOL();
					_limitCursor();
				}
				else
				{
					ReleaseFocus();
				}

			}
			break;

			case WG_KEY_LEFT:
				if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
					m_text.setSelectionMode(true);
				else
					m_text.setSelectionMode(false);

				if( pEvent->ModKeys() & WG_MODKEY_CTRL )
					m_text.gotoPrevWord();
				else
					m_text.goLeft();
				_limitCursor();
				break;
			case WG_KEY_RIGHT:
				if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
					m_text.setSelectionMode(true);
				else
					m_text.setSelectionMode(false);

				if( pEvent->ModKeys() & WG_MODKEY_CTRL )
					m_text.gotoNextWord();
				else
					m_text.goRight();
				_limitCursor();
				break;

			case WG_KEY_BACKSPACE:
				if(m_text.hasSelection())
				{
					m_text.delSelection();
					bTextChanged = true;
				}
				else if( pEvent->ModKeys() & WG_MODKEY_CTRL )
				{
					int ofs1 = m_text.column();
					m_text.gotoPrevWord();
					_limitCursor();
					int ofs2 = m_text.column();

					if( ofs2 < ofs1 )
					{
						m_text.deleteText( ofs2, ofs1 - ofs2 );
						bTextChanged = true;
					}
//					m_text.delPrevWord();
				}
				else
				{
					if( m_text.column() > m_format.getPrefix().Length() )
					{
						m_text.delPrevChar();
						bTextChanged = true;
					}
				}

				break;

			case WG_KEY_DELETE:
				if(m_text.hasSelection())
				{
					m_text.delSelection();
					bTextChanged = true;
				}
				else if( pEvent->ModKeys() & WG_MODKEY_CTRL )
				{
					int ofs1 = m_text.column();
					m_text.gotoNextWord();
					_limitCursor();
					int ofs2 = m_text.column();

					if( ofs2 > ofs1 )
					{
						m_text.deleteText( ofs1, ofs2 - ofs1 );
						bTextChanged = true;
					}
//					m_text.delNextWord();
				}
				else
				{
					if( m_text.column() < m_text.nbChars() - m_format.getSuffix().Length() )
					{
						m_text.delNextChar();
						bTextChanged = true;
					}
//					m_text.delNextChar();
				}

				break;

			case WG_KEY_HOME:

				/*
				 *	I am not sure if this is the proper way to this, but in my opinion, the default
				 *	"actions" has to be separated from any modifier key action combination
				 */
				switch( pEvent->ModKeys() )
				{

				case WG_MODKEY_CTRL:
					break;

				default: // no modifier key was pressed
					if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
						m_text.setSelectionMode(true);

					m_text.goBOL();
					_limitCursor();
					break;
				}

				break;

			case WG_KEY_END:

				/*
			 	 *	I am not sure if this is the proper way to this, but in my opinion, the default
		 		 *	"actions" has to be separated from any modifier key action combination
				 */
				switch( pEvent->ModKeys() )
				{

				case WG_MODKEY_CTRL:
					break;

				default: // no modifier key was pressed
					if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
						m_text.setSelectionMode(true);

					m_text.goEOL();
					_limitCursor();
					break;
				}

				break;



			default:
				break;
		}
	}

	if( event == WG_EVENT_CHARACTER )
	{
		int character = static_cast<const WgEvent::Character*>(pEvent)->Char();

		// Period is only allowed if it isn't displayed yet and value has decimals.
		// It's not allowed before a minus sign.
		// A zero is inserted (always INSERTED even if mode is replace) if there is no number before it.

		WgCursorInstance * pCursor = m_text.GetCursor();
		if( pCursor )
		{
			if( character == m_format.period )
			{
				if( m_format.decimals > 0 && m_text.getBuffer()->FindFirst( m_format.period ) == -1 &&
					(pCursor->column() != 0 || (*m_text.getBuffer())[0].Glyph() != '-' ) )
				{
					if(m_text.hasSelection())
						m_text.delSelection();
					m_text.setSelectionMode(false);

					if( m_text.nbChars() < m_maxInputChars )
					{
						if( pCursor->column() == 0 || (pCursor->column() == 1 && (*m_text.getBuffer())[0].Glyph() == '-' ) )
						{
							m_text.insertChar( 0, WgChar('0') );
							pCursor->goRight();
						}

						pCursor->putChar( m_format.period );
					}
					bTextChanged = true;
				}
			}

			// Handle minus
			// Only allow minus at start of text and only if range allows negative values.

			if( character == '-' )
			{
				if( pCursor->column() == m_format.getPrefix().Length() && m_text.getBuffer()->FindFirst( m_format.period ) == -1 &&
					m_rangeMin < 0 )
				{
					if(m_text.hasSelection())
						m_text.delSelection();
					m_text.setSelectionMode(false);

					if( m_text.nbChars() < m_maxInputChars )
						pCursor->putChar( '-' );
					bTextChanged = true;
				}
			}

			// Take care of typed numbers 0 through 9.

			if( character >= '0' && character <= '9' )
			{
				if( pCursor->column() == 0 && (*m_text.getBuffer())[0].Glyph() == '-' )
				{
				}
				else
				{
					if(m_text.hasSelection())
						m_text.delSelection();
					m_text.setSelectionMode(false);

					if( m_text.nbChars() < m_maxInputChars )
						pCursor->putChar( character );
					bTextChanged = true;
				}
			}
		}
	}

	if( bTextChanged )
	{
		int64_t		value;
		bool bModified = _parseValueFromInput( &value );

		if( value != m_value )
		{
			m_value = value;

			if( pHandler )
				pHandler->QueueEvent( new WgEvent::EditvalueModify(this,value,FractionalValue()) );

			_updateSlider( FractionalValue(), 0.f );
		}
	}
	
	// Forward event depending on rules.

	if( pEvent->IsMouseButtonEvent() )
	{
		if( static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button() != 1 )
			pHandler->ForwardEvent( pEvent );
	}
	else if( pEvent->IsKeyEvent() )
	{
		int key = static_cast<const WgEvent::KeyEvent*>(pEvent)->TranslatedKeyCode();
		if( static_cast<const WgEvent::KeyEvent*>(pEvent)->IsMovementKey() == false &&
			key != WG_KEY_DELETE && key != WG_KEY_BACKSPACE )
				pHandler->ForwardEvent( pEvent );
		
		//TODO: Would be good if we didn't forward any character-creating keys either...
	}
	else if( event != WG_EVENT_CHARACTER )
		pHandler->ForwardEvent( pEvent );
}

//____ _selectAll() ___________________________________________________________

void WgValueEditor::_selectAll()
{
	int min = m_format.getPrefix().Length();
	int max = m_text.nbChars() - m_format.getSuffix().Length();

	m_text.selectRange( WgRange( min, max-min ) );
}

//____ _limitCursor() _________________________________________________________

void WgValueEditor::_limitCursor()
{
	int min = m_format.getPrefix().Length();
	int max = m_text.nbChars() - m_format.getSuffix().Length();

	WgCursorInstance * pCursor = m_text.GetCursor();

	// Save selection (might get destroyed by moving cursor)

//	bool bSelectionMode = pCursor->getSelectionMode();
//	WgRange sel = pCursor->getSelection();

	// Move cursor to within prefix/suffix

	if( pCursor->column() < min )
		pCursor->gotoColumn( min );
	if( pCursor->column() > max )
		pCursor->gotoColumn( max );

	// Tweak selection if cursor was not in selection mode
/*
	if( !bSelectionMode )
	{
		if( sel.ofs < min )
		{
			sel.len -= min - sel.ofs;
			sel.ofs = min;
		}

		if( sel.len < 0 )
			sel.len = 0;

		if( sel.ofs + sel.len > max )
			sel.len = max - sel.ofs;

		m_text.selectText( 0, sel.ofs, 0, sel.ofs + sel.len );
	}
*/
}

//____ _onCloneContent() _______________________________________________________

void WgValueEditor::_onCloneContent( const WgWidget * _pOrg )
{
	WgValueEditor * pOrg = (WgValueEditor *) _pOrg;

	Wg_Interface_ValueHolder::_onCloneContent( pOrg );

	m_format		= pOrg->m_format;
	m_text.setText(&pOrg->m_text);
	m_text.setFont(pOrg->m_text.getFont());
	m_text.setAlignment(pOrg->m_text.alignment());
}

//____ _onEnable() _____________________________________________________________

void WgValueEditor::_onEnable( void )
{
	m_text.setMode(WG_MODE_NORMAL);
	_requestRender();
}

//____ _onDisable() ____________________________________________________________

void WgValueEditor::_onDisable( void )
{
	m_text.setMode(WG_MODE_DISABLED);
	_requestRender();
}


//____ _onGotInputFocus() ______________________________________________________

void WgValueEditor::_onGotInputFocus()
{
	WgWidget::_onGotInputFocus();

	_startReceiveTicks();
	m_text.showCursor();
	m_text.goEOL();
	m_useFormat = m_format;

	if( m_format.decimals != 0 )
		m_useFormat.bForcePeriod = true;	// Force period if decimals are involved.

	if( m_value < 0.f )
		m_useFormat.bZeroIsNegative = true;	// Force minus sign if value is negative.

	m_text.setScaledValue( m_value, m_format.scale, m_useFormat );

	_requestRender();
}

//____ _onLostInputFocus() _____________________________________________________

void WgValueEditor::_onLostInputFocus()
{
	WgWidget::_onLostInputFocus();

	_stopReceiveTicks();
	_queueEvent( new WgEvent::EditvalueSet(this,m_value,FractionalValue()) );

	m_text.hideCursor();
	m_useFormat = m_format;
	_regenText();

	_requestRender();
}

//____ _setScale() _____________________________________________________________

void WgValueEditor::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	m_text.SetScale(scale);
}


//____ SelectAllText() __________________________________________________________

bool WgValueEditor::SelectAllText()
{
	if( !m_text.GetCursor() )
	{
		if( !GrabFocus() )
			return false;				// Couldn't get cursor.
	}

	_selectAll();
	return true;
}
