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

#ifndef	WG_GFXINSTREAM_DOT_H
#define	WG_GFXINSTREAM_DOT_H
#pragma once

#include <wg_interface.h>
#include <wg_types.h>
#include <wg_geo.h>
#include <wg_color.h>
#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_gfxstream.h>


namespace wg
{

	class GfxInStream;
	typedef	StrongInterfacePtr<GfxInStream>		GfxInStream_p;
	typedef	WeakInterfacePtr<GfxInStream>		GfxInStream_wp;


	//____ GfxInStreamHolder ___________________________________________________

	class GfxInStreamHolder /** @private */
	{
	public:
		virtual Object * _object() = 0;

		virtual bool	_hasChunk() = 0;
		virtual GfxStream::Header	_peekChunk() = 0;	// Is only called if _hasChunk() has returned true.

		virtual char	_pullChar() = 0;
		virtual short	_pullShort() = 0;
		virtual int		_pullInt() = 0;
		virtual float	_pullFloat() = 0;
		virtual void	_pullBytes(int nBytes, char * pBytes) = 0;

		virtual bool	_isStreamOpen() = 0;
		virtual void	_closeStream() = 0;
		virtual bool	_reopenStream() = 0;
	};

	//____ GfxInStream ________________________________________________________

	class GfxInStream : public Interface
	{
	public:
		GfxInStream(GfxInStreamHolder * pHolder) : m_pHolder(pHolder) {};

		//.____ Control _______________________________________________________

		inline void		close() { m_pHolder->_closeStream(); }
		inline bool		isOpen() { return m_pHolder->_isStreamOpen(); }
		inline bool		reopen() { return m_pHolder->_reopenStream(); }

		bool				isEmpty();
		GfxStream::Header	peek();

		GfxInStream& operator>> (GfxStream::Header& header);

		GfxInStream& operator>> (uint16_t&);
		GfxInStream& operator>> (int32_t&);
		GfxInStream& operator>> (float&);


		GfxInStream& operator>> (Coord& coord);
		GfxInStream& operator>> (Rect& rect);
		GfxInStream& operator>> (RectF& rect);
		GfxInStream& operator>> (Color& color);
		GfxInStream& operator>> (BlendMode& blendMode);

		//.____ Misc __________________________________________________

		inline GfxInStream_p	ptr() { return GfxInStream_p(this); }

	protected:
		Object *				_object() const { return m_pHolder->_object(); }

		GfxInStreamHolder * 	m_pHolder;
	};


} // namespace wg
#endif //WG_GFXINSTREAM_DOT_H
#pragma once