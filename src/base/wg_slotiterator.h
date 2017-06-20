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

#ifndef WG_SLOTITERATOR_DOT_H
#define WG_SLOTITERATOR_DOT_H
#pragma once


namespace wg
{

	class Widget;

	template<class SlotType> class SlotIterator
	{
	public:

		// Iterator traits, previously from std::iterator.
		using value_type = SlotType;
		using difference_type = std::ptrdiff_t;
		using pointer = Widget*;
		using reference = Widget&;
		using iterator_category = std::bidirectional_iterator_tag;

		//.____ Creation ___________________________________________________

		SlotIterator() = default;
		explicit SlotIterator(SlotType* pSlot)
		{
			this->pSlot = pSlot;
		}

		//.____ Operators ______________________________________________________

		reference operator*() const
		{
			return *pSlot->pWidget;
		}

		pointer operator->() const
		{
			return pSlot->pWidget;
		}


		SlotIterator& operator++()
		{
			pSlot++;
			return *this;
		}

		SlotIterator operator++(int)
		{
			SlotIterator tmp = *this;
			pSlot++;
			return tmp;
		}

		SlotIterator& operator--()
		{
			pSlot--;
			return *this;
		}

		SlotIterator operator--(int)
		{
			SlotIterator tmp = *this;
			pSlot--;
			return tmp;
		}

		bool operator==(const SlotIterator& rhs)
		{
			return pSlot == rhs.pSlot;
		}

		bool operator!=(const SlotIterator& rhs)
		{
			return pSlot != rhs.pSlot;
		}

		//.____ Internal _______________________________________________________

		SlotType * _slot() const
		{
			return pSlot;
		}

	private:
		SlotType * pSlot;

	};

} // namespace wg
#endif //WG_SLOTITERATOR_DOT_H