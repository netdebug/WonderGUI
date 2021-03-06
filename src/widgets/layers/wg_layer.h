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
#ifndef WG_LAYER_DOT_H
#define WG_LAYER_DOT_H
#pragma once

#include <wg_container.h>
#include <wg_ichild.h>


namespace wg
{

	class Layer;
	typedef	StrongPtr<Layer>	Layer_p;
	typedef	WeakPtr<Layer>		Layer_wp;



	//____ LayerSlot ___________________________________________________________

	class LayerSlot : public Slot		/** @private */
	{
	public:
		Rect	geo;
	};



	/**
	 * @brief	Base class for containers that provides layers of different kinds.
	 *
	 * Layer is the base class for widget containers that are used as layers.
	 *
	 * The concept of Layers in WonderGUI is that a layer is a widget that provides
	 * a special purpose layer on top of the rest of the GUI hierarchy. The placement of
	 * this layer in the widget hierarchy decides on what layer things such as popups,
	 * modal dialog boxes, tooltips, notifications etc appear. (note: not all of these
	 * mentioned conceptual layers are implemented yet).
	 *
	 * For example, a Menubar widget that opens a menu will place the menu widget in the
	 * first PopupLayer it encounters while traversing it ancestors from itself upwards.
	 * In similar way, placing a widget in a ModalLayer will make it modal only against the
	 * children of that specific layer.
	 *
	 * All layers have a base child, which is the child through which the normal hierarchy continues.
	 * Children that resides within the layer itself are usually referred to as layer children.
	 *
	 **/

	class Layer : public Container, protected ChildHolder
	{

	public:

		//.____ Interfaces _______________________________________

		IChild<Slot,ChildHolder> base;


		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static Layer_p	cast( Object * pObject );

		//.____ Geometry ____________________________________________

		int					matchingHeight( int width ) const;
		int					matchingWidth( int height ) const;

		Size				preferredSize() const;

	protected:
		Layer();

		// Overloaded from WidgetHolder

		Coord		_childPos( Slot * pSlot ) const;
		Size		_childSize( Slot * pSlot ) const;

		void		_childRequestRender( Slot * pSlot );
		void		_childRequestRender( Slot * pSlot, const Rect& rect );
//		void		_childRequestResize( Slot * pSlot );

		Widget *	_prevChild( const Slot * pSlot ) const;
		Widget *	_nextChild( const Slot * pSlot ) const;

		void		_releaseChild( Slot * pSlot );

		// Overloaded from Container

		Widget *	_firstChild() const;
		Widget *	_lastChild() const;

		void		_firstSlotWithGeo( SlotWithGeo& package ) const;
		void		_nextSlotWithGeo( SlotWithGeo& package ) const;


		// Overloaded from ChildHolder

		void		_setWidget( Slot * pSlot, Widget * pNewWidget );
		Object *	_object() { return this; }

		// Overloaded from Widget

		void		_setSize(const Size& size);
		void		_cloneContent( const Widget * _pOrg );

		//

		virtual	void	_onRequestRender( const Rect& rect, const LayerSlot * pSlot );	// rect is in our coordinate system.

		virtual const LayerSlot * _beginLayerSlots() const = 0;
		virtual const LayerSlot * _endLayerSlots() const = 0;
		virtual int			_sizeOfLayerSlot() const = 0;

		inline LayerSlot * _beginLayerSlots() { return const_cast<LayerSlot*>(const_cast<const Layer*>(this)->_beginLayerSlots()); }
		inline LayerSlot * _endLayerSlots() { return const_cast<LayerSlot*>(const_cast<const Layer*>(this)->_endLayerSlots()); }

		inline LayerSlot * _incLayerSlot( LayerSlot * pSlot, int sizeOf ) const { return (LayerSlot*) (((char*)pSlot)+sizeOf); }
		inline const LayerSlot * _incLayerSlot( const LayerSlot * pSlot, int sizeOf ) const { return (const LayerSlot*) (((char*)pSlot)+sizeOf); }

		inline LayerSlot * _decLayerSlot( LayerSlot * pSlot, int sizeOf ) const { return (LayerSlot*) (((char*)pSlot)-sizeOf); }
		inline const LayerSlot * _decLayerSlot( const LayerSlot * pSlot, int sizeOf ) const { return (const LayerSlot*) (((char*)pSlot)-sizeOf); }


		Slot				m_baseSlot;
	};



} // namespace wg
#endif //WG_LAYER_DOT_H
