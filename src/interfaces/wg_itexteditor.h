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

#ifndef WG_ITEXTEDITOR_DOT_H
#define WG_ITEXTEDITOR_DOT_H
#pragma once

#include <wg_itextdisplay.h>
#include <wg_ctexteditor.h>


namespace wg
{

	class IROTextDisplayEditor;
	typedef	StrongInterfacePtr<IROTextDisplayEditor>	EditText_p;
	typedef	WeakInterfacePtr<IROTextDisplayEditor>	EditText_wp;

	/**
	 * @brief Interface for text which can be modified through the UI
	 *
	 * Interface that provides access to text which can be modified through the UI. Provides methods
	 * to get/set the text, modify its appearance, position caret and select region.
	 *
	 */

	class IROTextDisplayEditor : public ITextDisplay
	{
	public:
		/** @private */

		IROTextDisplayEditor(CTextEditor * pComponent) : ITextDisplay(pComponent) {}

		//.____ State __________________________________________________

		void			setEditMode(TextEditMode mode);
		TextEditMode	editMode() const;
		inline bool		isSelectable() const;
		inline bool		isEditable() const;

		//.____ Content _____________________________________________

		inline int		append( const CharSeq& seq );
		inline int		insert( int ofs, const CharSeq& seq );
		inline int		replace( int ofs, int nDelete, const CharSeq& seq );
		inline int		erase( int ofs, int len );

		inline int		caretPut(const CharSeq& str);	// Will insert or overwrite depending on caret mode
		inline bool		caretPut(uint16_t c);			// " -

		inline int		eraseSelected();

		inline bool		setMaxLines(int maxLines) { return _comp()->setMaxLines(maxLines); }
		inline int		maxLines() const { return _comp()->maxLines(); }

		inline bool		setMaxChars(int maxChars) { return _comp()->setMaxChars(maxChars); }
		inline int		maxChars() const { return _comp()->maxChars(); }

		//.____ Control _____________________________________________

		// These methods will fail if editMode is Static

		inline bool		select( int begin, int end );		// Cursor (if enabled) gets end position. End can be smaller than begin.
		inline bool		selectAll();
		inline bool		unselect();

		inline int		selectionBegin() const;
		inline int		selectionEnd() const;
		inline int		selectionSize() const;


		// These methods will fail unless caret is present

		inline bool		setCaretOfs( int ofs );			// Move cursor to offset. Any selection will be unselected.
		inline int		caretOfs() const;


		inline bool		caretLineBegin();
		inline bool		caretLineEnd();
		inline bool		caretTextBegin();
		inline bool		caretTextEnd();

		//.____ Misc __________________________________________________

		inline EditText_p		ptr() { return EditText_p(this); }

	private:
		inline	CTextEditor * 	_comp() { return static_cast<CTextEditor*>(m_pComponent); }
		inline	const CTextEditor * 	_comp() const { return static_cast<CTextEditor*>(m_pComponent); }
	};


	//_____________________________________________________________________________
	inline void IROTextDisplayEditor::setEditMode(TextEditMode mode)
	{
		_comp()->setEditMode(mode);
	}

	//_____________________________________________________________________________
	inline TextEditMode IROTextDisplayEditor::editMode() const
	{
		return _comp()->editMode();
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::isSelectable() const
	{
		return !(_comp()->editMode() == TextEditMode::Static);
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::isEditable() const
	{
		return (_comp()->editMode() == TextEditMode::Editable);
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::append( const CharSeq& seq )
	{
		return _comp()->append( seq );
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::insert( int ofs, const CharSeq& seq )
	{
		return _comp()->insert(ofs, seq);
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::replace( int ofs, int nDelete, const CharSeq& seq )
	{
		return _comp()->replace(ofs, nDelete, seq);
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::erase( int ofs, int len )
	{
		return _comp()->erase(ofs,len);
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::select( int begin, int end )
	{
		return _comp()->select(begin,end);
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::selectAll()
	{
		return _comp()->selectAll();
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::unselect()
	{
		return _comp()->unselect();
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::eraseSelected()
	{
		return _comp()->eraseSelected();
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::selectionBegin() const
	{
		return _comp()->selectionBegin();
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::selectionEnd() const
	{
		return _comp()->selectionEnd();
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::selectionSize() const
	{
		return _comp()->selectionSize();
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::setCaretOfs( int ofs )
	{
		return _comp()->setCaretOfs(ofs);
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::caretOfs() const
	{
		return _comp()->caretOfs();
	}

	//_____________________________________________________________________________
	inline int IROTextDisplayEditor::caretPut( const CharSeq& seq )
	{
		return _comp()->caretPut(seq);
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::caretPut( uint16_t c )
	{
		return _comp()->caretPut(c);
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::caretLineBegin()
	{
		return _comp()->caretLineBegin();
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::caretLineEnd()
	{
		return _comp()->caretLineEnd();
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::caretTextBegin()
	{
		return _comp()->caretTextBegin();
	}

	//_____________________________________________________________________________
	inline bool IROTextDisplayEditor::caretTextEnd()
	{
		return _comp()->caretTextEnd();
	}



} // namespace wg
#endif //WG_ITEXTEDITOR_DOT_H
