#ifndef WG_KEY_DOT_H
#define WG_KEY_DOT_H


enum WgKey
{
	WG_KEY_LEFT = 0,
	WG_KEY_RIGHT,
	WG_KEY_UP,
	WG_KEY_DOWN,

	WG_KEY_HOME,
	WG_KEY_END,
	WG_KEY_PAGEUP,
	WG_KEY_PAGEDOWN,

	WG_KEY_RETURN,
	WG_KEY_BACKSPACE,
	WG_KEY_DELETE,
	WG_KEY_TAB,
	WG_KEY_ESCAPE,

	WG_KEY_F1,
	WG_KEY_F2,
	WG_KEY_F3,
	WG_KEY_F4,
	WG_KEY_F5,
	WG_KEY_F6,
	WG_KEY_F7,
	WG_KEY_F8,
	WG_KEY_F9,
	WG_KEY_F10,
	WG_KEY_F11,
	WG_KEY_F12,

	WG_KEY_SHIFT,
	WG_KEY_CONTROL,
	WG_KEY_ALT,
	WG_KEY_SUPER,

	WG_KEY_UNMAPPED		// This one must always be last!
};

#define	WG_KEY_ARRAYSIZE		WG_KEY_UNMAPPED


#endif // WG_KEY_DOT_H
