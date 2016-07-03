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

#include <wg_spanitem.h>

namespace wg 
{
	
	SpanItem::SpanItem( Widget * pWidget ) : Item( pWidget )
	{
		min = MIN;
		max = MAX;
		begin = 0;
		length = 0;
	}
	
	
	bool SpanItem::setMin( int _min )
	{
		if( _min < MIN || _min > MAX )
			return false;
	
		if( min != _min )
		{
			min = _min;
			if( max < min )
				max = MAX;
	
			if( begin < min )
			{
				begin = min;
				if( length < max - min )
					length = max - min;
			}
	
			_onModified();
		}
		return true;
	}
	
	bool SpanItem::setMax( int _max )
	{
		if( _max < MIN || _max > MAX )
			return false;
	
		if( max != _max )
		{
			max = _max;
			if( max < min )
				min = MIN;
	
			if( begin + length > max )
			{
				if( length > max - min )
					length = max - min;
	
				begin = max - length;
			}
	
			_onModified();
		}
		return true;
	}
	
	bool SpanItem::setRange( int _min, int _max )
	{
		if( _min > _max || _min < MIN || _max > MAX )
			return false;
	
		if( min != _min || max != _max )
		{
			min = _min;
			max = _max;
	
			if( length > max - min )
				length = max - min;
	
			if( begin < min )
				begin = min;
	
			if( begin + length > max )
				begin = max - length;
	
			_onModified();
		}
		return true;
	}
	
	void SpanItem::setSpan( int _begin, int _length )
	{
		if( _length > max - min )
			_length = max - min;
	
		if( _begin + _length > max )
			_begin = max - _length;
	
		if( _begin < min )
			_begin = min;
	
		if( begin != _begin || length != _length )
		{
			begin = _begin;
			length = _length;
			_onModified();
		}
	}
	
	void SpanItem::setBegin( int _begin )
	{
		if( _begin < min )
			_begin = min;
		else if( _begin + length > max )
			_begin = max - length;
	
		if( begin != _begin )
		{
			begin = _begin;
			_onModified();
		}
	}
	
	void SpanItem::setLength( int _length )
	{
		if( _length > max - min )
			_length = max - min;
	
		if( begin + _length > max )
			begin = max - _length;
	
		if( length != _length )
		{
			length = _length;
			_onModified();
		}
	}
	
	void SpanItem::setRelativeSpan( float _begin, float _length )
	{
		int range = max - min;
		setSpan( min + (int) (_begin*range), (int) _length*range );
	}
	
	void SpanItem::setRelativePos( float _pos )
	{
		int range = max - min;
		setBegin( min + (int) ((_pos*range*2)- length)/2 );
	}
	
	void SpanItem::setRelativeBegin( float _begin )
	{
		int range = max - min;
		setBegin( min + (int) (_begin*range) );
	}
	
	void SpanItem::setRelativeLength( float _length )
	{	int range = max - min;
		setLength( (int) _length*range );
	}
	
	bool SpanItem::stepForward()
	{
		int beg = begin;
		setBegin( beg + _stepSize() );
		return beg != begin;
	}
	
	bool SpanItem::stepBackward()
	{	int beg = begin;
		setBegin( beg - _stepSize() );
		return beg != begin;
	}
	
	bool SpanItem::skipForward()
	{
		int beg = begin;
		setBegin( beg + _skipSize() );
		return beg != begin;
	}
	
	bool SpanItem::skipBackward()
	{
		int beg = begin;
		setBegin( beg - _skipSize() );
		return beg != begin;
	}

} // namespace wg
