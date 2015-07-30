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



#include <wg_filler.h>
#include <wg_gfxdevice.h>

const char WgFiller::CLASSNAME[] = {"Filler"};

//____ Constructor ____________________________________________________________

WgFiller::WgFiller()
{
	m_preferredSize = WgSize(-1,-1);
}

//____ Destructor _____________________________________________________________

WgFiller::~WgFiller()
{
}

//____ isInstanceOf() _________________________________________________________

bool WgFiller::isInstanceOf( const char * pClassName ) const
{ 
	if( pClassName==CLASSNAME )
		return true;

	return WgWidget::isInstanceOf(pClassName);
}

//____ className() ____________________________________________________________

const char * WgFiller::className( void ) const
{ 
	return CLASSNAME; 
}

//____ cast() _________________________________________________________________

WgFiller_p WgFiller::cast( const WgObject_p& pObject )
{
	if( pObject && pObject->isInstanceOf(CLASSNAME) )
		return WgFiller_p( static_cast<WgFiller*>(pObject.rawPtr()) );

	return 0;
}


//____ setPreferredSize() _______________________________________________________

void WgFiller::setPreferredSize( const WgSize& size )
{
	if( size != m_preferredSize )
	{
		m_preferredSize = size;
		_requestResize();
	}
}

//____ preferredSize() __________________________________________________________

WgSize WgFiller::preferredSize() const
{
	if( m_preferredSize.w >= 0 && m_preferredSize.h >= 0 )
		return m_preferredSize;
	else
		return WgWidget::preferredSize();
}


//____ _onCloneContent() _______________________________________________________

void WgFiller::_onCloneContent( const WgWidget * _pOrg )
{
	WgFiller * pOrg = (WgFiller*) _pOrg;

	m_preferredSize = pOrg->m_preferredSize;
}
