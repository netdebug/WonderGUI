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
#pragma once

#include <wg_widget.h>
#include <wg_itextdisplay.h>
#include <wg_iicondisplay.h>

namespace wg
{

	class	Surface;

	class ToggleButton;
	typedef	StrongPtr<ToggleButton>		ToggleButton_p;
	typedef	WeakPtr<ToggleButton>	ToggleButton_wp;

	class ToggleGroup;
	typedef	StrongPtr<ToggleGroup>		ToggleGroup_p;
	typedef	WeakPtr<ToggleGroup>	ToggleGroup_wp;


	/**
	 * @brief Combined ToggleButton, Checkbox and RadioButton widget.
	 *
	 * ToggleButton is a button widget that toggles its selected-state each time it is pressed.
	 * Press once to select, press again to deselect. ToggleButton is also used for Checkboxes
	 * and RadioButtons since these are technically just ToggleButtons with different skinning
	 * and possibly (depending on exact behavior desired) a different ClickArea.
	 *
	 * The ToggleButton is built up by three different visual elements: The button itself,
	 * the label and the icon. A typical StateButton will have the button itself skinned and
	 * the label set to some suitable text while a CheckBox and RadioButton will leave the button
	 * unskinned and use the icon and label for its visual representation.
	 *
	 **/

	class	ToggleButton : public Widget
	{
	friend class ToggleGroup;
	public:
		enum ClickArea
		{
			DEFAULT,		///< Full geometry of icon (no alpha test) + text + area between + alpha test on background.
			ALPHA,			///< Alpha test on background and icon.
			GEO,			///< Full geometry of Widget is clickable.
			ICON,			///< Only the icon (alpha test) is clickable.
			TEXT			///< Only the text (no alpha test) is clickable.
		};

		//.____ Creation __________________________________________

		static ToggleButton_p	create() { return ToggleButton_p(new ToggleButton()); }

		//.____ Interfaces ____________________________________

		ITextDisplay		label;
		IIconDisplay		icon;

		//.____ Identification __________________________________________

		bool					isInstanceOf( const char * pClassName ) const override;
		const char *			className( void ) const override;
		static const char		CLASSNAME[];
		static ToggleButton_p	cast( Object * pObject );

		//.____ State _________________________________________________

		inline bool			isSelected();
		virtual bool		setSelected( bool bSelected );

		//.____ Behavior _______________________________________________________

	 	void				setClickArea( ClickArea clickArea );
		inline ClickArea	clickArea() const;

		void				setFlipOnRelease( bool bFlipOnRelease );
		inline bool			flipOnRelease();

		//.____ Geometry ____________________________________________

		Size				preferredSize() const override;

		//.____ Misc _________________________________________________________________

		inline ToggleGroup_p toggleGroup() const;

	protected:
		ToggleButton();
		virtual ~ToggleButton();
		virtual Widget* _newOfMyType() const override { return new ToggleButton(); };

		void	_cloneContent( const Widget * _pOrg ) override;
		void	_render( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window ) override;
		void	_receive( Msg * pMsg ) override;
		void	_refresh() override;
		void	_setSize( const Size& size ) override;
		bool	_alphaTest( const Coord& ofs ) override;
		void	_setState( State state ) override;
		void	_setSkin( Skin * pSkin ) override;

		void	_setToggleGroup( ToggleGroup * pGroup );
		ToggleGroup * _toggleGroup() const { return m_pToggleGroup.rawPtr(); }

		Coord	_componentPos( const Component * pComponent ) const override;
		Size	_componentSize( const Component * pComponent ) const override;
		Rect	_componentGeo( const Component * pComponent ) const override;



	private:

		void	_refreshTextArea();
		bool	_markTestTextArea( int _x, int _y );

		bool			m_bPressed;						// Set when mouse is pressed and over.
		bool			m_bReturnPressed;
		bool			m_bFlipOnRelease;				// Set if we want to flip StateButton on press (default), not click.

		CTextDisplay		m_label;
		CIconDisplay		m_icon;
		ToggleGroup_p	m_pToggleGroup;

		ClickArea		m_clickArea;
	};


	inline ToggleButton::ClickArea ToggleButton::clickArea() const
	{
		return m_clickArea;
	}

	inline void ToggleButton::setClickArea( ClickArea clickArea )
	{
		m_clickArea = clickArea;
	}

	inline bool ToggleButton::isSelected()
	{
		return m_state.isSelected();
	};

	inline bool ToggleButton::flipOnRelease()
	{
		return m_bFlipOnRelease;
	}

	inline ToggleGroup_p ToggleButton::toggleGroup() const
	{
		return m_pToggleGroup;
	}




} // namespace wg
#endif //	WG_TOGGLEBUTTON_DOT_H
