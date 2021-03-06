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
#ifndef	WG_CVALUE_DOT_H
#define WG_CVALUE_DOT_H
#pragma once

#include <wg_ctext.h>
#include <wg_valueformatter.h>

namespace wg
{

	//____ CValue ____________________________________________________________

	class CValue : public CText		/** @private */
	{
	public:
		CValue(ComponentHolder * pHolder);

		void				setFormatter( ValueFormatter * pFormatter );
		void				clearFormatter();
		inline ValueFormatter_p	formatter() const { return m_pFormatter; }

		virtual void		clear();
		virtual bool		set( int64_t value, int scale );

		void				refresh();
		inline int64_t		value() const { return m_value; }
		inline int			scale() const { return m_scale; }

	protected:
		void				_regenText();

		int64_t				m_value;
		int					m_scale;

		ValueFormatter_p	m_pFormatter;
	};



} // namespace wg
#endif //WG_CVALUE_DOT_H
