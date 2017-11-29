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
#ifndef WG_TESTWIDGET_DOT_H
#define WG_TESTWIDGET_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef WG_COLORSET_DOT_H
#	include <wg_colorset.h>
#endif

//____ TestWidget ____________________________________________________________

class TestWidget : public WgWidget
{
public:
	TestWidget();
	virtual ~TestWidget();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new TestWidget(); };
	WgSize	PreferredPixelSize() const;

	void	Start();
	void	Stop();


protected:

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );

private:


	WgCoord m_coord[2];
	int		m_speed[2];
	bool	m_bPointsInitialized;
	bool	m_bAnimated;

};


#endif //WG_TESTWIDGET_DOT_H
