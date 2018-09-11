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

#ifndef	WG_TYPES_DOT_H
#define WG_TYPES_DOT_H

#ifdef WIN32
#ifndef for
//#	define		for		if(false){}else for
#endif
#endif

//#ifdef _MSC_VER // MS do not support C99
//#include "WG/C99Compatibility/pstdint.h"
//#else
#	include <stdint.h>			// Use the C99 official header
//#endif


#ifndef Uint8
	typedef unsigned char	Uint8;
	typedef signed char		Sint8;
	typedef unsigned short	Uint16;
	typedef signed short		Sint16;
	typedef unsigned int		Uint32;
	typedef signed int		Sint32;
	typedef uint64_t			Uint64;
	typedef int64_t			Sint64;
#endif

#ifndef INT64_MIN
#define INT64_MIN (-(9223372036854775807 ## L)-1)
#endif

#ifndef INT64_MAX
#define INT64_MAX ((9223372036854775807 ## L))
#endif


#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)
#	 if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#		define WG_IS_BIG_ENDIAN 1
#		define WG_IS_LITTLE_ENDIAN 0
#	elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#		define WG_IS_BIG_ENDIAN 0
#		define WG_IS_LITTLE_ENDIAN 1
#	endif
#else

#	ifdef __BIG_ENDIAN__
#		if __BIG_ENDIAN__
#			define WG_IS_BIG_ENDIAN 1
#			define WG_IS_LITTLE_ENDIAN 0
#		endif
#	endif

#	ifdef __LITTLE_ENDIAN__
#		if __LITTLE_ENDIAN__
#			define WG_IS_BIG_ENDIAN 0
#			define WG_IS_LITTLE_ENDIAN 1
#		endif
#	endif
#endif

#define WG_IS_LITTLE_ENDIAN 1
#define WG_IS_BIG_ENDIAN 0

#ifndef WG_IS_BIG_ENDIAN
#error Could not detect endianness. You need to define WG_IS_BIG_ENDIAN and WG_IS_LITTLE_ENDIAN in wg_types.h
#define WG_IS_BIG_ENDIAN 0
#define WG_IS_LITTLE_ENDIAN 0
#endif

#define WG_SCALE_BINALS		12
#define WG_SCALE_BASE		(1<<WG_SCALE_BINALS)

template<typename T> inline T WgMin(const T &a, const T &b) { return a < b ? a : b; }
template<typename T> inline T WgMax(const T &a, const T &b) { return a > b ? a : b; }
template<typename T> inline T WgAbs(T x)					{ return x >= 0 ? x : -x; }
template<typename T> inline void WgSwap(T &a, T &b) { T c = a; a = b; b = c; }

#define		WG_LIMIT(x, min, max)	if( x < min) x = min; if( x > max) x = max;

//____ WgMode _________________________________________________________________

enum WgMode //: Uint8
{
	WG_MODE_NORMAL =	0,
	WG_MODE_MARKED =	1,		// Widget is highlighted (mouse over)
	WG_MODE_SELECTED =	2,		// Widget is selected (pressed)
	WG_MODE_DISABLED =	3,		// Widget is disabled
	WG_MODE_SPECIAL =	4,		// Graphics block for a special, widget-specific mode.
	WG_MODE_ALL		=	5		// Only supported by certain methods!!!
};

#define	WG_NB_MODES		5		// Number of modes (excluding WG_MODE_ALL )


enum WgStateEnum
{
	WG_STATE_NORMAL						= 0,
	WG_STATE_FOCUSED					= 1,
	WG_STATE_HOVERED					= 4,
	WG_STATE_HOVERED_FOCUSED			= 4 + 1,
	WG_STATE_PRESSED					= 4 + 2,
	WG_STATE_PRESSED_FOCUSED			= 4 + 2 + 1,
	WG_STATE_SELECTED					= 8,
	WG_STATE_SELECTED_FOCUSED			= 8 + 1,
	WG_STATE_SELECTED_HOVERED			= 8 + 4,
	WG_STATE_SELECTED_HOVERED_FOCUSED	= 8 + 4 + 1,
	WG_STATE_SELECTED_PRESSED			= 8 + 4 + 2,
	WG_STATE_SELECTED_PRESSED_FOCUSED	= 8 + 4 + 2 + 1,
	WG_STATE_DISABLED					= 16,
	WG_STATE_DISABLED_SELECTED			= 16 + 8,
};

#define	WG_NB_STATES		14			// Number of states
#define	WG_MAX_STATE_VALUE	24			// Highest value for WgStateEnum

class WgState 
{
public:
	WgState() { m_state = WG_STATE_NORMAL; }
	WgState( WgStateEnum state ) { m_state = state; }

//	void		set( WgModeEnum state ) { m_state = state; }
//	WgModeEnum	getEnum() const { return (WgModeEnum) m_state; }

	bool	setEnabled(bool bEnabled) { if(bEnabled) m_state &= ~WG_STATE_DISABLED; else m_state = WG_STATE_DISABLED; return true; }
	bool	setSelected(bool bSelected) { if( m_state == WG_STATE_DISABLED ) return false; if(bSelected) m_state |= WG_STATE_SELECTED; else m_state &= ~WG_STATE_SELECTED; return true; }
	bool	setFocused(bool bFocused) { if( m_state == WG_STATE_DISABLED ) return false; if(bFocused) m_state |= WG_STATE_FOCUSED; else m_state &= ~WG_STATE_FOCUSED; return true; }
	bool	setHovered(bool bHovered) { if( m_state == WG_STATE_DISABLED ) return false; if(bHovered) m_state |= WG_STATE_HOVERED; else m_state &= ~WG_STATE_PRESSED; return true; }
	bool	setPressed(bool bPressed) { if( m_state == WG_STATE_DISABLED ) return false; if(bPressed) m_state |= WG_STATE_PRESSED; else m_state &= ~(WG_STATE_PRESSED - WG_STATE_HOVERED); return true; }


	bool	isEnabled() { return (m_state & WG_STATE_DISABLED) == WG_STATE_NORMAL; }
	bool	isSelected() { return (m_state & WG_STATE_SELECTED) == WG_STATE_SELECTED; }
	bool	isFocused() { return (m_state & WG_STATE_FOCUSED) == WG_STATE_FOCUSED; }
	bool	isHovered() { return (m_state & WG_STATE_HOVERED) == WG_STATE_HOVERED; }
	bool	isPressed() { return (m_state & WG_STATE_PRESSED) == WG_STATE_PRESSED; }

	inline bool operator==(WgStateEnum state) const { return m_state == state; }
	inline bool operator!=(WgStateEnum state) const { return m_state != state; }

	inline void operator=(WgStateEnum state) { m_state = state; }

	operator WgStateEnum() const { return (WgStateEnum) m_state; }

private:
	int		m_state;
};

//____ WgTxtAttr ______________________________________________________________

// Bitmask for various text attributes.

enum WgTxtAttr
{
	WG_TXT_UNDERLINED	= 0x1,
	WG_TXT_SELECTED		= 0x2
};


//____ WgExtChar __________________________________________________________


//0x1b

// Double escape codes should give the escape-code character.

/*
		NEW ONES

		{prop}		Set the named property. Named properties should start with a-z/A-Z.
					If property is unnamed you get {123} where the number is the current handle for the prop.

		-			break permitted
		=			hyphen break permitted
		n			linebreak (like \n).

		Predefined properties

		{n}			empty property (normal/default)
		{b}			bold
		{i}			italic
		{u}			underlined
		{b-i}		bold italic
		{b-u}		bold underlined
		{b-i-u}		bold italic underlined
		{i-u}		italic underlined
		{h1}		heading 1
		{h2}		heading 2
		{h3}		heading 3
		{h4}		heading 4
		{h5}		heading 5
		{u1}		user defined style 1
		{u2}		user defined style 2
		{u3}		user defined style 3
		{u4}		user defined style 4
		{u5}		user defined style 5

		{super}		superscript		// Includes top positioning
		{sub}		subscript		// Includes bottom positioning
		{mono}		monospaced		// Includes monospacing

		{black}		black text
		{white}		white text

*/



/*
	{[rrggbbaa]		begin color
	}				end color

	[123			begin size, exactly 3 decimal digits sets the size.
	]				end size

	_				begin underlined
	| 				end underlined

	:[0-4]			set break level
	;				end break level


	-				break permitted
	=				hyphen break permitted

	d				begin normal (default)
	b				begin bold
	i				begin italic
	I				begin bold italic
	s				begin subscript
	S				begin superscript
	m				begin monospace
	h[1-5]			begin heading
	u[1-5]			begin userdefined style

	#				end style

	(prop)			set a new text property, looked up from a ResDB.
					Other style/color settings are applied ontop of this text property.
					Setting prop as (null) sets an empty prop.



*/


enum WgExtChar
{
	// Code inside WonderGUI assumes these are all
	// in the range 0x80-0xA0, but that might change in the future.

	WG_BREAK_PERMITTED			= 0x82,
	WG_HYPHEN_BREAK_PERMITTED	= 0x83,
	WG_NO_BREAK_SPACE			= 0xA0,

	WG_ELLIPSIS					= 0x2026
};

//____ WgCodePage ______________________________________________________________

enum WgCodePage
{
	WG_CODEPAGE_LATIN1 = 0,
	WG_CODEPAGE_1250 = 1,		// Windows Cental Europe
	WG_CODEPAGE_1251 = 2,		// Windows Cyrillic
	WG_CODEPAGE_1252 = 3,		// Windows Latin-1 extended
	WG_CODEPAGE_1253 = 4,		// Windows Greek
	WG_CODEPAGE_1254 = 5,		// Windows Turkish
	WG_CODEPAGE_1255 = 6,		// Windows Hebrew
	WG_CODEPAGE_1256 = 7,		// Windows Arabic
	WG_CODEPAGE_1257 = 8,		// Windows Baltic
	WG_CODEPAGE_1258 = 9,		// Windows Vietnam
	WG_CODEPAGE_874 = 10,		// Windows Thai
};

#define WG_NB_CODEPAGES 11

//____ WgBreakRules ____________________________________________________________

enum WgBreakRules
{
	WG_NO_BREAK		= 0,
	WG_BREAK_BEFORE	= 16,
	WG_BREAK_ON		= 32,
	WG_BREAK_AFTER	= 64
};

//____ WgBlendMode ____________________________________________________________

// BlendModes control how blits and fills are blended against the background.

enum WgBlendMode //: Uint8
{
	WG_BLENDMODE_OPAQUE,			///< Completely opaque blitting, ignoring alpha of source and tint-color.
	WG_BLENDMODE_BLEND,				///< Normal mode, alpha of source and tint-color is taken into account.
	WG_BLENDMODE_ADD,				///< RGBA Additive, alpha of source and tint-color is taken into account.
	WG_BLENDMODE_MULTIPLY,			///< RGBA Multiply, alpha of source and tint-color is taken into account.
	WG_BLENDMODE_INVERT				///< Inverts destination RGB values where alpha of source is non-zero. Ignores RBG components. Uses alpha of tint-color.
};

//____ WgFontStyle ____________________________________________________________

enum WgFontStyle
{
	// Needs to stay in sync with WgExtChar::WG_BEGIN_NORMAL and following enums!

	WG_STYLE_NORMAL		= 0,
	WG_STYLE_BOLD		= 1,
	WG_STYLE_BOLD_ITALIC= 2,
	WG_STYLE_ITALIC		= 3,
	WG_STYLE_SUPERSCRIPT= 4,
	WG_STYLE_SUBSCRIPT	= 5,
	WG_STYLE_MONOSPACE	= 6,
	WG_STYLE_HEADING_1	= 7,
	WG_STYLE_HEADING_2	= 8,
	WG_STYLE_HEADING_3	= 9,
	WG_STYLE_HEADING_4	= 10,
	WG_STYLE_HEADING_5	= 11,

	WG_STYLE_USER_1		= 12,
	WG_STYLE_USER_2		= 13,
	WG_STYLE_USER_3		= 14,
	WG_STYLE_USER_4		= 15,
	WG_STYLE_USER_5		= 16,

};

#define WG_NB_FONTSTYLES	17


//____ WgPointerStyle __________________________________________________________

enum WgPointerStyle
{
	WG_POINTER_ARROW,						// default arrow
	WG_POINTER_DEFAULT = WG_POINTER_ARROW,	// default arrow
	WG_POINTER_HOURGLASS,					// hourglass
	WG_POINTER_HAND,							// hand
	WG_POINTER_CROSSHAIR,					// crosshair
	WG_POINTER_HELP,							// help
	WG_POINTER_IBEAM,						// I-beam
	WG_POINTER_STOP,							// slashed circle
	WG_POINTER_UP_ARROW,						// arrow pointing up
	WG_POINTER_SIZE_ALL,						// four-pointed arrow in all directions
	WG_POINTER_SIZE_NE_SW,					// double-pointed arrow pointing northeast and southwest
	WG_POINTER_SIZE_NW_SE,					// double-pointed arrow pointing northwest and southeast
	WG_POINTER_SIZE_N_S,						// double-pointed arrow pointing north and south
	WG_POINTER_SIZE_W_E,						// double-pointed arrow pointing west and east
};

//____ WgAnimMode _____________________________________________________________

enum WgAnimMode
{
	WG_FORWARD_ONCE,
	WG_BACKWARD_ONCE,
	WG_FORWARD_LOOPING,
	WG_BACKWARD_LOOPING,
	WG_FORWARD_PINGPONG,
	WG_BACKWARD_PINGPONG
};


//____ WgTintMode _____________________________________________________________

// WgTintMode controls how TintColors are blended hierarchically.

enum WgTintMode
{
	WG_TINTMODE_OPAQUE,
	WG_TINTMODE_MULTIPLY
};


/*
enum	WgExtChar
{
		BREAK_PERMITTED_HERE	= 0x82,
//		BEGIN_SELECTED_AREA		= 0x86,
//		END_SELECTED_AREA		= 0x87,
		HYPHEN_BREAK_PERMITTED_HERE	= 0x91,
//		BEGIN_UNDERLINE			= 0x91,
//		END_UNDERLINE			= 0x92,

//		BEGIN_BOLD				= 0x93,
//		END_BOLD				= 0x94,
//		BEGIN_ITALIC			= 0x95,
//		END_ITALIC				= 0x96,
//		BEGIN_USEREFFECT1		= 0x97,
//		END_USEREFFECT1			= 0x98,
//		BEGIN_USEREFFECT2		= 0x99,
//		END_USEREFFECT2			= 0x9A,


//		END_COLOR				= 0x9C,
//		START_OF_OS_COMMAND		= 0x9D,
//		END_OF_OS_COMMAND		= 0x9E
};
*/


//____ WgSearchMode _____________________________________________________________

enum WgSearchMode
{
	WG_SEARCH_MARKPOLICY,			// Perform a mark test on Widget.
	WG_SEARCH_GEOMETRY,				// Goes strictly on geometry, ignores alpha.
	WG_SEARCH_ACTION_TARGET,		// Like MARKPOLICY, but takes modality into account.
};

//____ WgOrigo _____________________________________________________________

enum WgOrigo
{
	// Clockwise from upper left corner, center last. Must be in range 0-8

	WG_NORTHWEST	= 0,
	WG_NORTH		= 1,
	WG_NORTHEAST	= 2,
	WG_EAST			= 3,
	WG_SOUTHEAST	= 4,
	WG_SOUTH		= 5,
	WG_SOUTHWEST	= 6,
	WG_WEST			= 7,
	WG_CENTER		= 8
};

//____ WgDirection ____________________________________________________________

enum WgDirection
{
	WG_UP,
	WG_DOWN,
	WG_LEFT,
	WG_RIGHT
};

//____ WgOrientation __________________________________________________________

enum WgOrientation
{
	WG_HORIZONTAL,
	WG_VERTICAL
};

//____ WgSizePolicy ___________________________________________________________

enum WgSizePolicy
{
	WG_DEFAULT = 0,
	WG_BOUND,
	WG_CONFINED,
	WG_EXPANDED,
};

//____ WgEventType ______________________________________________________________

enum	WgEventType
{
	WG_EVENT_DUMMY = 0,
	WG_EVENT_TICK,
	WG_EVENT_POINTER_CHANGE,

	WG_EVENT_FOCUS_GAINED,
	WG_EVENT_FOCUS_LOST,

	WG_EVENT_MOUSE_ENTER,
	WG_EVENT_MOUSE_MOVE,
	WG_EVENT_MOUSE_POSITION,
	WG_EVENT_MOUSE_LEAVE,

	WG_EVENT_MOUSEBUTTON_PRESS,
	WG_EVENT_MOUSEBUTTON_REPEAT,
	WG_EVENT_MOUSEBUTTON_DRAG,
	WG_EVENT_MOUSEBUTTON_RELEASE,
	WG_EVENT_MOUSEBUTTON_CLICK,
	WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK,

	WG_EVENT_KEY_PRESS,
	WG_EVENT_KEY_REPEAT,
	WG_EVENT_KEY_RELEASE,
	WG_EVENT_CHARACTER,
	WG_EVENT_MOUSEWHEEL_ROLL,

	WG_EVENT_SELECTED,						// Posted every time a popup menu entry is selected. Should be used by more widgets in the future.
	WG_EVENT_BUTTON_PRESS,					// Posted every time a button is pressed, either by left mouse button or enter key.

	WG_EVENT_CHECKBOX_CHECK,
	WG_EVENT_CHECKBOX_UNCHECK,
	WG_EVENT_CHECKBOX_TOGGLE,

	WG_EVENT_RADIOBUTTON_SELECT,
	WG_EVENT_RADIOBUTTON_UNSELECT,
	WG_EVENT_RADIOBUTTON_TOGGLE,

	WG_EVENT_ANIMATION_UPDATE,				// Posted every time the animation changes frame.

	WG_EVENT_TAB_SELECT,					// Posted when a new tab is selected.
	WG_EVENT_TAB_PRESS,						// Posted every time any mousebutton is pressed on a widget.

	WG_EVENT_EDITVALUE_MODIFY,				// Posted for every change to value through keyboard (once for every accepted keystroke).
	WG_EVENT_EDITVALUE_SET,					// Posted when edited value is 'set' by removing cursor.

	WG_EVENT_SLIDER_MOVE,
	WG_EVENT_SLIDER_STEP_FWD,
	WG_EVENT_SLIDER_STEP_BWD,
	WG_EVENT_SLIDER_JUMP_FWD,
	WG_EVENT_SLIDER_JUMP_BWD,
	WG_EVENT_SLIDER_WHEEL_ROLL,

	WG_EVENT_TEXT_MODIFY,					// Posted for every change to text through cursor/selection.
	WG_EVENT_TEXT_SET,						// Posted when edited text is 'set' by removing cursor.

	WG_EVENT_MENUITEM_SELECT,				// Posted when a WgMenuEntry or WgMenuRadioButton is selected.
	WG_EVENT_MENUITEM_CHECK,				// Posted when a WgMenuCheckBox is checked.
	WG_EVENT_MENUITEM_UNCHECK,				// Posted when a WgMenuCheckBox is unchecked.

	WG_EVENT_POPUP_CLOSED,

	WG_EVENT_MODAL_MOVE_OUTSIDE,
	WG_EVENT_MODAL_BLOCKED_PRESS,
	WG_EVENT_MODAL_BLOCKED_RELEASE,

	WG_EVENT_TABLE_CELL_MARK,
	WG_EVENT_TABLE_CELL_UNMARK,
	
	WG_EVENT_LINK_MARK,
	WG_EVENT_LINK_UNMARK,
	WG_EVENT_LINK_PRESS,
	WG_EVENT_LINK_REPEAT,
	WG_EVENT_LINK_RELEASE,
	WG_EVENT_LINK_CLICK,
	WG_EVENT_LINK_DOUBLE_CLICK,
	
	WG_EVENT_KNOB_TURN,

	WG_EVENT_SLIDER_MOVED,
	WG_EVENT_SLIDER_PRESS,
//	WG_EVENT_SLIDER_PRESS_REPEAT,
//	WG_EVENT_SLIDER_RELEASE,	
//	WG_EVENT_SLIDER_CLICK,
//	WG_EVENT_SLIDER_DOUBLE_CLICK,


/*
	WG_EVENT_ITEM_ENTER,
	WG_EVENT_ITEM_LEAVE,
	WG_EVENT_ITEM_PRESS,
	WG_EVENT_ITEM_DRAG,
	WG_EVENT_ITEM_REPEAT,
	WG_EVENT_ITEM_RELEASE,
	WG_EVENT_ITEM_CLICK,
	WG_EVENT_ITEM_DOUBLE_CLICK,
*/
	WG_EVENT_MAX
};


//____ WgSortOrder ____________________________________________________________

enum WgSortOrder
{
	WG_SORT_ASCENDING,
	WG_SORT_DESCENDING
};

//____ WgUnit ____________________________________________________________

enum WgUnit
{
	WG_PIXELS,
	WG_FRACTION
};


//____ WgSelectMode ___________________________________________________________

enum WgSelectMode
{
	WG_SELECT_NONE,
	WG_SELECT_SINGLE,
	WG_SELECT_MULTI
};

//____ WgTextEditMode _________________________________________________________

enum WgTextEditMode
{
	WG_TEXT_STATIC,
	WG_TEXT_SELECTABLE,
	WG_TEXT_EDITABLE,
};


//____ WgTextPos ______________________________________________________________

class WgTextPos
{
public:
	WgTextPos() : line(0), col(0) {}
	WgTextPos( int line, int col ) : line(line), col(col) {}

	int	line;
	int col;
};

//____ WgRange ____________________________________________________________

class WgRange
{
public:
	WgRange() : ofs(0), len(0) {}
	WgRange( int ofs, int len ) : ofs(ofs), len(len) {}

	inline int	Begin() const { return ofs; }
	inline int	End() const { return ofs+len; }

	int ofs;
	int len;
};


//____ WgSortContext __________________________________________________________

struct WgSortContext
{
public:
	bool	bAscend;
};

//____ WgAccessMode ____________________________________________________________

enum WgAccessMode
{
	WG_NO_ACCESS,
	WG_READ_ONLY,
	WG_WRITE_ONLY,
	WG_READ_WRITE
};

//____ DownsampleMethod _______________________________________________________

enum WgDownsampleMethod
{
	WG_DOWNSAMPLE_NEAREST,
	WG_DOWNSAMPLE_AVERAGE,
	WG_DOWNSAMPLE_PEAK				// Keep the "peak value", highest or lowest value depending or situation
};

//____ WgUpsampleMethod _______________________________________________________

enum WgUpsampleMethod
{
	WG_UPSAMPLE_NEAREST,
	WG_UPSAMPLE_INTERPOLATE
};


//____ ScaleMode ____________________________________________________________

enum WgScaleMode
{
	WG_SCALEMODE_NEAREST,
	WG_SCALEMODE_INTERPOLATE,
};



//____ WgPixelType _____________________________________________________________

enum WgPixelType
{
	WG_PIXEL_UNKNOWN,			///< Pixelformat is unkown or can't be expressed in a PixelFormat struct.
	WG_PIXEL_CUSTOM,			///< Pixelformat has no PixelType enum, but is fully specified through the PixelFormat struct.
    WG_PIXEL_BGR_8,				///< One byte of blue, green and red in exactly that order in memory.
    WG_PIXEL_BGRX_8,			///< One byte of blue, green, red and padding in exactly that order in memory.
    WG_PIXEL_BGRA_8,			///< One byte of blue, green, red and alpha in exactly that order in memory.
    WG_PIXEL_BGRA_4,			///< 4 bits each of blue, green, red and alpha in exactly that order in memory.
    WG_PIXEL_BGR_565,			///< 5 bits of blue, 6 bits of green and 5 bits of red in exactly that order in memory.
    WG_PIXEL_I8,				///< 8 bits of index into the CLUT (Color Lookup Table).
    WG_PIXEL_A8					///< 8 bits of alpha only.

};


//____ WgPixelFormat __________________________________________________________
/**
 * @brief Describes the size of a pixel and the placement and amount of bits for red, green, blue and alpha.
 * 
 * The mask and shift values assumes that the pixel has been read into a register. Therefore the
 * values are different on a big-endian and little-endian system. 
 */

struct WgPixelFormat
{
	WgPixelType	type;			///< Enum specifying the format if it exacty matches a predefined format, otherwise set to CUSTOM or UNKNOWN.
	int			bits;			///< Number of bits for the pixel, includes any non-used padding bits.

	Uint32	R_mask;				///< bitmask for getting the red bits out of the pixel
	Uint32	G_mask;				///< bitmask for getting the green bits out of the pixel
	Uint32	B_mask;				///< bitmask for getting the blue bits out of the pixel
	Uint32	A_mask;				///< bitmask for getting the alpha bits out of the pixel

	int		R_shift;			///< amount to shift the red bits to get an 8-bit representation of red. This can be negative.
	int		G_shift;			///< amount to shift the green bits to get an 8-bit representation of red. This can be negative.
	int		B_shift;			///< amount to shift the blue bits to get an 8-bit representation of red. This can be negative.
	int		A_shift;			///< amount to shift the alpha bits to get an 8-bit representation of red. This can be negative.

	Uint8	R_bits;				///< number of bits for red in the pixel
	Uint8	G_bits;				///< number of bits for green in the pixel
	Uint8	B_bits;				///< number of bits for blue in the pixel
	Uint8	A_bits;				///< number of bits for alpha in the pixel
};


//____ WgMaskOp ____________________________________________________________

enum WgMaskOp
{
	WG_MASKOP_RECURSE = 0,	///< Recurse through children, let them mask background individually.
	WG_MASKOP_SKIP = 1,		///< Do not mask background against container or children.
	WG_MASKOP_MASK = 2		///< Mask background against whole container.
};


#endif // WG_TYPES_DOT_H
