/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_MODALLAYER_DOT_H
#define WG_MODALLAYER_DOT_H
#pragma once

#include <wg_layer.h>
#include <wg_idynamicchildren.h>

namespace wg
{

	class ModalLayer;
	typedef	StrongPtr<ModalLayer>	ModalLayer_p;
	typedef	WeakPtr<ModalLayer>		ModalLayer_wp;

	//____ ModalSlot ___________________________________________________________

	class ModalSlot : public LayerSlot		/** @private */
	{
	public:
		ModalSlot() : origo(Origo::NorthWest) {}

		const static bool safe_to_relocate = false;

		Origo		origo;
		Rect		placementGeo;		// Widgets geo relative anchor and hotspot. Setting width and height to 0 uses Widgets preferredSize() dynamically.
										// Setting just one of them to 0 uses Widgets matchingHeight() or matchingWidth() dynamically.
		Widget_wp	pKeyFocus;			// Pointer at child that held focus when this modal was last on top.

	};



	//____ ModalChildrenHolder _________________________________________________

	class ModalChildrenHolder : public DynamicChildrenHolder		/** @private */
	{
	public:
		virtual void	_refreshRealGeo(ModalSlot * pSlot) = 0;
	};

	class IModalChildren;
	typedef	StrongInterfacePtr<IModalChildren>	IModalChildren_p;
	typedef	WeakInterfacePtr<IModalChildren>	IModalChildren_wp;

	//____ IModalChildren ________________________________________________________

	class IModalChildren : public IDynamicChildren<ModalSlot,ModalChildrenHolder>
	{
	public:
		/** @private */

		IModalChildren( SlotArray<ModalSlot> * pSlotArray, ModalChildrenHolder * pHolder ) : IDynamicChildren<ModalSlot,ModalChildrenHolder>(pSlotArray,pHolder) {}

		//.____ Misc __________________________________________________________

		inline IModalChildren_p	ptr() { return IModalChildren_p(this); }

		//.____ Content _______________________________________________________

		iterator	add( Widget * pWidget, const Rect& geometry, Origo origo = Origo::NorthWest );
		iterator	add( Widget * pWidget, const Coord& pos, Origo origo = Origo::NorthWest ) { return add( pWidget, Rect(pos,0,0), origo); }

		//.____ Geometry ______________________________________________________

		void		setOrigo( int index, const Origo origo );
		void		setOrigo( iterator it, const Origo origo );
		Origo		origo( int index ) const;
		Origo		origo( iterator it ) const;

		void		setGeo( int index, const Rect& geometry );
		void		setGeo( iterator it, const Rect& geometry );
		Rect		geo( int index ) const;
		Rect		geo( iterator it ) const;

		void		setOfs( int index, const Coord& ofs );
		void		setOfs( iterator it, const Coord& ofs );
		Coord		ofs( int index ) const;
		Coord		ofs( iterator it ) const;

		void		setSize( int index, const Size& size );
		void		setSize( iterator it, const Size& size );
		Size		size( int index ) const;
		Size		size( iterator it ) const;

		void		move( int index, const Coord& ofs );
		void		move( iterator it, const Coord& ofs );

	protected:
		void 		_setOrigo(ModalSlot * p, const Origo origo);
		void 		_setGeo(ModalSlot * p, const Rect& geometry);
		void 		_setOfs(ModalSlot * p, const Coord& ofs);
		void 		_setSize(ModalSlot * p, const Size& size);
		void 		_move( ModalSlot * p, const Coord& ofs );
	};


	//____ ModalLayer __________________________________________________________

	class ModalLayer : public Layer, protected ModalChildrenHolder
	{
		friend class IModalChildren;

	public:

		//.____ Creation __________________________________________

		static ModalLayer_p	create() { return ModalLayer_p(new ModalLayer()); }

		//.____ Interfaces _______________________________________

		IModalChildren	modals;

		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static ModalLayer_p	cast( Object * pObject );


		//.____ Geometry ____________________________________________

		int				matchingHeight( int width ) const;
		int				matchingWidth( int height ) const;

		Size			preferredSize() const;

	protected:
		ModalLayer();
		virtual ~ModalLayer();
		virtual Widget* _newOfMyType() const { return new ModalLayer(); };


	private:

		ModalLayer *	_getModalLayer() const { return const_cast<ModalLayer*>(this); }

		void			_updateKeyboardFocus();
		void			_refreshRealGeo( ModalSlot * pSlot );

		// Overloaded from Panel

		Widget *		_findWidget( const Coord& ofs, SearchMode mode );

		// Overloaded from WidgetHolder

		void			_childRequestResize( Slot * pSlot );
		void			_releaseChild(Slot * pSlot);

		// Methods for ModalChildren

		void			_didAddSlots(Slot * pSlot, int nb);
		void			_didMoveSlots(Slot * pFrom, Slot * pTo, int nb);
		void			_willRemoveSlots(Slot * pSlot, int nb);
		Object *		_object() { return this;  }
		WidgetHolder *	_widgetHolder() { return this; }

		// Overloaded from Layer

		const LayerSlot * 	_beginLayerSlots() const;
		const LayerSlot * 	_endLayerSlots() const;
		int				_sizeOfLayerSlot() const;

		// Overloaded from Widget

		void			_cloneContent( const Widget * _pOrg );
		void			_setSize( const Size& size );
		void			_receive( Msg * pMsg );


		Widget_wp			m_pBaseKeyFocus;

		SlotArray<ModalSlot>	m_modals;

	};


} // namespace wg
#endif //WG_MODALLAYER_DOT_H
