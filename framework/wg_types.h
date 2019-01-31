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

#include <wg3_types.h>

typedef wg::Orientation         WgOrientation;

typedef wg::StateEnum           WgStateEnum;

static const int    WG_NB_STATES = wg::StateEnum_Nb;
static const int    WG_MAX_STATE_VALUE = wg::StateEnum_MaxValue;

typedef wg::State               WgState;

typedef wg::ExtChar     WgExtChar;

typedef wg::CodePage    WgCodePage;

const static int   WG_NB_CODEPAGES = wg::CodePage_size;

typedef wg::BreakRules  WgBreakRules;

typedef wg::BlendMode   WgBlendMode;

typedef wg::PointerStyle WgPointerStyle;

typedef wg::SearchMode  WgSearchMode;
typedef wg::Origo       WgOrigo;
typedef wg::Direction   WgDirection;
typedef wg::SizePolicy  WgSizePolicy;

typedef wg::AccessMode  WgAccessMode;

typedef wg::ScaleMode   WgScaleMode;
typedef wg::PixelFormat WgPixelType;

typedef wg::SortOrder   WgSortOrder;
typedef wg::SelectMode  WgSelectMode;
typedef wg::TextEditMode WgTextEditMode;
typedef wg::MaskOp      WgMaskOp;
typedef wg::AnimMode    WgAnimMode;


#	include <stdint.h>			// Use the C99 official header


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


//____ WgTxtAttr ______________________________________________________________

// Bitmask for various text attributes.

enum WgTxtAttr
{
	WG_TXT_UNDERLINED	= 0x1,
	WG_TXT_SELECTED		= 0x2
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

    WG_EVENT_SLIDER_PRESS,
	WG_EVENT_SLIDER_MOVED,
    WG_EVENT_SLIDER_ENTER,
    WG_EVENT_SLIDER_LEAVE,

    WG_EVENT_SLIDER_HANDLE_ENTER,
    WG_EVENT_SLIDER_HANDLE_LEAVE,

    
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



#endif // WG_TYPES_DOT_H
