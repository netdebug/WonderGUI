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

#ifndef	WG_BASE_DOT_H
#define	WG_BASE_DOT_H
#pragma once

/*
#include <wg_textmgr.h>
*/

#include <assert.h>

#include <wg_userdefines.h>
#include <wg_key.h>
#include <wg_strongptr.h>



namespace wg
{
	class Font;
	class MemPool;
	class WeakPtrHub;
	class MemStack;
	class MsgRouter;
	class ValueFormatter;
	class InputHandler;
	class TextMapper;
	class Caret;
	class TextStyle;

	typedef	StrongPtr<MsgRouter>		MsgRouter_p;
	typedef	StrongPtr<ValueFormatter>	ValueFormatter_p;
	typedef	StrongPtr<InputHandler>		InputHandler_p;
	typedef	StrongPtr<TextMapper>		TextMapper_p;
	typedef	StrongPtr<Caret>			Caret_p;
	typedef	StrongPtr<TextStyle>		TextStyle_p;


	/**
	 * @brief	Static base class for WonderGUI.
	 *
	 * A static base class for WonderGUI that handles initialization and general
	 * housekeeping and resource allocation.
	 *
	 * The first thing that you need to do when starting WonderGUI is to call Base::init()
	 * and the last thing you should do is to call Base::exit().
	 *
	 */

	class Base
	{
//		friend class Object_wp;
		friend class Interface_wp;
		friend class WeakPtrHub;
	public:

		//.____ Creation __________________________________________

		static void init();
		static int exit();

		//.____ Content _____________________________________________

		static MsgRouter_p	msgRouter();
		static InputHandler_p	inputHandler();

		static void			setDefaultTextMapper( TextMapper * pTextMapper );
		static TextMapper_p defaultTextMapper();

		static void			setDefaultCaret(Caret * pCaret);
		static Caret_p		defaultCaret();

		static void			setDefaultStyle( TextStyle * pStyle );
		static TextStyle_p 	defaultStyle();

		static void			setDefaultValueFormatter(ValueFormatter * pFormatter);
		static ValueFormatter_p defaultValueFormatter();


		//.____ Misc ________________________________________________

		static char *		memStackAlloc( int bytes );
		static void			memStackRelease( int bytes );

	private:

		static WeakPtrHub *	_allocWeakPtrHub();
		static void			_freeWeakPtrHub(WeakPtrHub * pHub);

		struct Data
		{
			MsgRouter_p		pMsgRouter;
			InputHandler_p	pInputHandler;


			TextMapper_p		pDefaultTextMapper;
			Caret_p				pDefaultCaret;
			TextStyle_p			pDefaultStyle;
			ValueFormatter_p	pDefaultValueFormatter;


			//

			MemPool *		pPtrPool;
			MemStack *		pMemStack;


		};

		static Data *	s_pData;

	};



} // namespace wg
#endif //WG_BASE_DOT_H
