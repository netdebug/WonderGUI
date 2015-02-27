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

#ifndef	WG_TOGGLEBUTTON_DOT_H
#define	WG_TOGGLEBUTTON_DOT_H

#ifndef	WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef WG_LEGACYMODTEXT_DOT_H
#	include <wg_legacymodtext.h>
#endif

#ifndef WG_ICON_DOT_H
#	include <wg_icon.h>
#endif

class	WgSurface;

class WgToggleButton;
typedef	WgStrongPtr<WgToggleButton,WgWidgetPtr>		WgToggleButtonPtr;
typedef	WgWeakPtr<WgToggleButton,WgWidgetWeakPtr>	WgToggleButtonWeakPtr;

class WgToggleGroup;
typedef	WgStrongPtr<WgToggleGroup,WgObjectPtr>		WgToggleGroupPtr;
typedef	WgWeakPtr<WgToggleGroup,WgObjectWeakPtr>	WgToggleGroupWeakPtr;


/**
 * @brief Combined ToggleButton, Checkbox and RadioButton widget.
 *
 * WgToggleButton is a button widget that toggles its selected-state each time it is pressed.
 * Press once to select, press again to deselect. WgToggleButton is also used for Checkboxes
 * and RadioButtons since these are technically just ToggleButtons with different skinning
 * and possibly (depending on exact behavior desired) a different ClickArea.
 *
 * The ToggleButton is built up by three different visual elements: The button itself,
 * the label and the icon. A typical StateButton will have the button itself skinned and
 * the label set to some suitable text while a CheckBox and RadioButton will leave the button
 * unskinned and use the icon and label for its visual representation.
 *
 **/

class	WgToggleButton : public WgWidget, protected WgIconHolder, protected WgLegacyTextHolder
{
friend class WgToggleGroup;
public:
	static WgToggleButtonPtr	Create() { return WgToggleButtonPtr(new WgToggleButton()); }

	bool						IsInstanceOf( const char * pClassName ) const;
	const char *				ClassName( void ) const;
	static const char			CLASSNAME[];
	static WgToggleButtonPtr	Cast( const WgObjectPtr& pObject );

	enum ClickArea
	{
		DEFAULT,		///< Full geometry of icon (no alpha test) + text + area between + alpha test on background.
		ALPHA,			///< Alpha test on background and icon.
		GEO,			///< Full geometry of Widget is clickable.
		ICON,			///< Only the icon (alpha test) is clickable.
		TEXT			///< Only the text (no alpha test) is clickable.
	};


	//____ Interfaces() ____________________________________

	WgLegacyModText		label;
	WgIcon			icon;


	//____ Methods() _______________________________________

 	void				SetClickArea( ClickArea clickArea );
	inline ClickArea	GetClickArea() const;

	inline bool			IsSelected();
	virtual bool		SetSelected( bool bSelected );

	void				SetFlipOnRelease( bool bFlipOnRelease );
	inline bool			FlipOnRelease();

	inline WgToggleGroupPtr ToggleGroup() const;
	WgSize				PreferredSize() const;
	inline bool			IsAutoEllipsisDefault() const;


protected:
	WgToggleButton();
	virtual ~WgToggleButton();
	virtual WgWidget* _newOfMyType() const { return new WgToggleButton(); };

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	void	_onEvent( const WgEventPtr& pEvent, WgEventHandler * pHandler );
	void	_onRefresh();
	void	_onNewSize( const WgSize& size );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onStateChanged( WgState oldState );
	void	_onSkinChanged( const WgSkinPtr& pOldSkin, const WgSkinPtr& pNewSkin );
	
	WgObject * 		_object() { return this; };
	void			_onFieldDirty( WgField * pField );
	void 			_onFieldResize( WgField * pField );

	void	_setToggleGroup( WgToggleGroup * pGroup );
	WgToggleGroup * _toggleGroup() const { return m_pToggleGroup.RawPtr(); }

private:

	void	_refreshTextArea();
	bool	_markTestTextArea( int _x, int _y );
	
	bool			m_bPressed;						// Set when mouse is pressed and over.
	bool			m_bReturnPressed;
	bool			m_bFlipOnRelease;				// Set if we want to flip StateButton on press (default), not click.

	WgLegacyTextField		m_label;
	WgIconField		m_icon;
	WgToggleGroupPtr	m_pToggleGroup;

	ClickArea		m_clickArea;
};


inline WgToggleButton::ClickArea WgToggleButton::GetClickArea() const
{
	return m_clickArea;
}

inline bool WgToggleButton::IsSelected()
{
	return m_state.IsSelected();
};

inline bool WgToggleButton::FlipOnRelease()
{
	return m_bFlipOnRelease;
}

inline WgToggleGroupPtr WgToggleButton::ToggleGroup() const
{
	return m_pToggleGroup;
}

inline bool WgToggleButton::IsAutoEllipsisDefault() const
{
	return false;
};



#endif //	WG_TOGGLEBUTTON_DOT_H
