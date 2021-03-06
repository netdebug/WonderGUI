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

#ifndef	WG_CONTAINER_DOT_H
#define	WG_CONTAINER_DOT_H
#pragma once

#include <wg_widget.h>


namespace wg
{

	class Patches;
	class ModalLayer;
	class PopupLayer;

	class Container;
	typedef	StrongPtr<Container>			Container_p;
	typedef	WeakPtr<Container>		Container_wp;

	/**
	 * @brief Base class for all widgets that can hold child widgets.
	 *
	 * Base class for all widgets that can hold child widgets.
	 */

	class Container : public Widget, protected WidgetHolder
	{
		friend class MsgRouter;


		friend class Menu;
		friend class Menubar;
		friend class Combobox;
		friend class PopupOpener;

		friend class RootPanel;
		friend class FlexPanel;
		friend class PopupLayer;
		friend class ModalLayer;
		friend class DragNDropLayer;
		friend class ScrollPanel;
		friend class StackPanel;
		friend class Capsule;
		friend class PackList;
		friend class WidgetSlot;

		public:

			//.____ Identification __________________________________________

			virtual bool			isInstanceOf( const char * pClassName ) const;
			virtual const char *	className( void ) const;
			static const char		CLASSNAME[];
			static Container_p		cast( Object * pObject );

			bool					isContainer() const;

			//.____ Hierarchy _________________________________________________

			inline Widget_p			firstChild() const { return Widget_p(_firstChild()); }
			inline Widget_p			lastChild() const { return Widget_p(_lastChild()); }


			//.____ Misc _________________________________________________________________

			virtual Widget_p		findWidget( const Coord& ofs, SearchMode mode ) { return Widget_p(_findWidget(ofs,mode)); }
			bool					contains(const Widget * pWidget) const;

		protected:
			Container();
			virtual ~Container() {};

			// WidgetHolder methods, default implementations for widgets

			/* Left to implement in individual classes:

				Coord		_childPos( Slot * pSlot ) const;
				Size		_childSize( Slot * pSlot ) const;

				void		_childRequestRender( Slot * pSlot );
				void		_childRequestRender( Slot * pSlot, const Rect& rect );
				void		_childRequestResize( Slot * pSlot );

				Widget *	_prevChild( const Slot * pSlot ) const;
				Widget *	_nextChild( const Slot * pSlot ) const;

				void		_releaseChild( Slot * pSlot );
			*/


			virtual Coord			_childGlobalPos( Slot * pSlot ) const;

			virtual bool			_isChildVisible( Slot * pSlot ) const;
			virtual Rect			_childWindowSection( Slot * pSlot ) const;		// Returns the window section within the childs canvas.

			virtual Container *  	_childParent();
			virtual RootPanel *		_root();

			virtual bool			_childRequestFocus( Slot * pSlot, Widget * pWidget );					// Request focus on behalf of me, child or grandchild.
			virtual bool			_childReleaseFocus( Slot * pSlot, Widget * pWidget );

			virtual void			_childRequestInView( Slot * pSlot );
			virtual void			_childRequestInView( Slot * pSlot, const Rect& mustHaveArea, const Rect& niceToHaveArea );

			//

			virtual bool			_isPanel() const;

			virtual Widget *		_firstChild() const = 0;
			virtual Widget *		_lastChild() const = 0;


			virtual Widget * 		_findWidget( const Coord& ofs, SearchMode mode );
			virtual void			_setState( State state );

			virtual void			_renderPatches( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window, const Patches& _patches );

			struct SlotWithGeo
			{
				Rect			geo;
//				Widget *		pWidget;
//				bool			bVisible;
				const Slot *	pSlot;
			};

			virtual void			_firstSlotWithGeo( SlotWithGeo& package ) const = 0;
			virtual void			_nextSlotWithGeo( SlotWithGeo& package ) const = 0;

			virtual ModalLayer *	_getModalLayer() const;
			virtual PopupLayer*		_getPopupLayer() const;

			virtual void			_maskPatches( Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode );
			virtual void			_collectPatches( Patches& container, const Rect& geo, const Rect& clip );
			virtual void			_cloneContent( const Widget * _pOrg );

			bool			m_bSiblingsOverlap;	// Set if children (might be) overlapping each other (special considerations to be taken during rendering).

	};


} // namespace wg
#endif //WG_CONTAINER_DOT_H
