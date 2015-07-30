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

#include <wg_standardformatter.h>

const char WgStandardFormatter::CLASSNAME[] = {"StandardFormatter"};

//____ Constructor _____________________________________________________________

WgStandardFormatter::WgStandardFormatter()
{
}

WgStandardFormatter::WgStandardFormatter( const WgCharSeq& format )
{
}

//____ Destructor ______________________________________________________________

WgStandardFormatter::~WgStandardFormatter()
{
}


//____ isInstanceOf() _________________________________________________________

bool WgStandardFormatter::isInstanceOf( const char * pClassName ) const
{ 
	if( pClassName==CLASSNAME )
		return true;

	return WgValueFormatter::isInstanceOf(pClassName);
}

//____ className() ____________________________________________________________

const char * WgStandardFormatter::className( void ) const
{ 
	return CLASSNAME; 
}

//____ cast() _________________________________________________________________

WgStandardFormatter_p WgStandardFormatter::cast( const WgObject_p& pObject )
{
	if( pObject && pObject->isInstanceOf(CLASSNAME) )
		return WgStandardFormatter_p( static_cast<WgStandardFormatter*>(pObject.rawPtr()) );

	return 0;
}

//____ format() _______________________________________________________________

WgString WgStandardFormatter::format( Sint64 value, int scale ) const
{	
}

WgString WgStandardFormatter::format( double value ) const
{
}
