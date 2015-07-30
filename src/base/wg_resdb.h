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

#ifndef WG_RESDB_DOT_H
#define WG_RESDB_DOT_H

#include <string>
#include <vector>
#include <map>


#ifndef WG_OBJECT_DOT_H
#	include <wg_object.h>
#endif

#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef WG_CHAIN_DOT_H
#	include <wg_chain.h>
#endif

#ifndef WG_TEXTPROP_DOT_H
#	include <wg_textprop.h>
#endif

#ifndef WG_SKIN_DOT_H
#	include <wg_skin.h>
#endif

#ifndef WG_GFXANIM_DOT_H
#	include <wg_gfxanim.h>
#endif

#ifndef WG_CARET_DOT_H
#	include <wg_caret.h>
#endif

#ifndef WG_GLYPHSET_DOT_H
#	include	<wg_glyphset.h>
#endif

class WgFont;
class WgResLoader;


class WgResDB;
typedef	WgStrongPtr<WgResDB,WgObject_p>	WgResDB_p;
typedef	WgWeakPtr<WgResDB,WgObject_wp>	WgResDB_wp;

class WgResDB : public WgObject
{
public:
	static WgResDB_p	create() { return WgResDB_p(new WgResDB()); }

	bool				isInstanceOf( const char * pClassName ) const;
	const char *		className( void ) const;
	static const char	CLASSNAME[];
	static WgResDB_p	cast( const WgObject_p& pObject );


	class MetaData
	{
	public:
		virtual ~MetaData() {}
		virtual MetaData* clone() const = 0;

		virtual std::string getAttribute(const std::string& name) const = 0;
		virtual bool		setAttribute(const std::string& name, const std::string& value) = 0;
		virtual bool		hasAttribute(const std::string& name) const = 0;
	};

	struct Resource : public WgLink
	{
		Resource( const std::string& id, MetaData* meta) : id(id), meta(meta) {}
		virtual ~Resource() { delete meta; }

		std::string		id;
		MetaData*		meta;
	};

	template<typename T>
	struct ResWrapper : public Resource
	{
		LINK_METHODS( ResWrapper<T> );
		ResWrapper(const std::string& id, T res, MetaData* meta) : Resource(id,meta), res(res) {}
		T res;
	};

	template<typename T>
	struct ResWrapper2 : public Resource
	{
		LINK_METHODS( ResWrapper2<T> );
		ResWrapper2(const std::string& id, T res, const std::string& file, MetaData* meta) : Resource(id,meta), res(res), file(file) {}
		T res;
		std::string	file;
	};

	struct LegoSource : public Resource
	{
		LegoSource(const std::string& id, const std::string& surface, WgRect rect, Uint32 nStates, MetaData* meta) : Resource(id, meta), surface(surface), rect(rect), nStates(nStates) { }
		std::string surface;
		WgRect rect;
		Uint32 nStates;
	};

	struct ConnectRes : public Resource
	{
		LINK_METHODS( ConnectRes );
		ConnectRes( MetaData* meta) : Resource("", meta) {}
	};


	typedef ResWrapper2<WgResDB_p>		ResDBRes;
	typedef ResWrapper2<WgSurface_p>	SurfaceRes;
	typedef ResWrapper2<WgGlyphset_p>	GlyphsetRes;
	typedef ResWrapper<WgFont_p>		FontRes;
	typedef ResWrapper<WgGfxAnim_p>	GfxAnimRes;
	typedef ResWrapper<WgCaret_p>		CursorRes;
	typedef ResWrapper<WgTextprop_p>	TextpropRes;
	typedef ResWrapper<WgColor>			ColorRes;
	typedef ResWrapper<WgSkin_p>		SkinRes;
	typedef ResWrapper<WgWidget_p>		WidgetRes;
	typedef ResWrapper<void*>			DataSetRes;


	//----

	static std::string	generateName( const WgSurface_p& data );
	static std::string	generateName( const WgGlyphset_p& data );
	static std::string	generateName( const WgFont_p& data );
	static std::string	generateName( const WgGfxAnim_p& data );
	static std::string	generateName( const WgCaret_p& data );
	static std::string	generateName( const WgColor data );
	static std::string	generateName( const WgTextprop_p& data );
	static std::string	generateName( const WgSkin_p& data );
	static std::string	generateName( const WgWidget_p& data );

	void				setResLoader( WgResLoader * pLoader );
	void				clear();

	void				clearSurfaces();
	void				clearWidgets();

	std::string			loadString( const std::string& token );

	bool				addResDb( const std::string& file, MetaData * pMetaData = 0 );
	bool				addResDb( const WgResDB_p& db, const std::string& file, MetaData * pMetaData = 0 );

	bool				addSurface( const std::string& id, const std::string& file, MetaData * pMetaData, bool bRequired );
	bool				addSurface( const std::string& id, const WgSurface_p& pSurf, const std::string& filename, MetaData * pMetaData = 0 );

	bool				addGlyphset( const std::string& id, const std::string& file, MetaData * pMetaData = 0 );
	bool				addGlyphset( const std::string& id, const WgGlyphset_p& pGlyphset, const std::string& file, MetaData * pMetaData = 0 );

	bool				addFont( const std::string& id, const WgFont_p& pFont, MetaData * pMetaData = 0 );
	bool				addGfxAnim( const std::string& id, const WgGfxAnim_p& pAnim, MetaData * pMetaData = 0 );
	bool				addCursor( const std::string& id, const WgCaret_p& pCursor, MetaData * pMetaData = 0 );
	bool				addTextprop( const std::string& id, const WgTextprop_p& pProp, MetaData * pMetaData = 0 );
	bool				addColor( const std::string& id, WgColor col, MetaData * pMetaData = 0 );
	bool				addLegoSource( const std::string& id, const std::string& surface, WgRect rect, Uint32 nStates, MetaData * pMetaData = 0 );
	bool				addSkin( const std::string& id, const WgSkin_p& pSkin, MetaData * pMetaData = 0 );
	bool				addConnect( MetaData * pMetaData );
	bool				addWidget( const std::string& id, const WgWidget_p& pWidget, MetaData * pMetaData = 0 );
	bool				addDataSet( const std::string& id, MetaData * pMetaData );

	bool				removeSurface( const std::string& id );
	bool				removeGlyphset( const std::string& id );
	bool				removeFont( const std::string& id );
	bool				removeGfxAnim( const std::string& id );
	bool				removeCursor( const std::string& id );
	bool				removeTextprop( const std::string& id );
	bool				removeColor( const std::string& id );
	bool				removeLegoSource( const std::string& id );
	bool				removeSkin( const std::string& id );
	bool				removeMenuitem( const std::string& id );
	bool				removeTab( const std::string& id );
	bool				removeConnect( const std::string& id );
	bool				removeResDb( const std::string& id );
	bool				removeWidget( const std::string& id );
	bool				removeDataSet( const std::string& id );

	bool				removeSurface( SurfaceRes * pRes );
	bool				removeGlyphset( GlyphsetRes * pRes );
	bool				removeFont( FontRes * pRes );
	bool				removeGfxAnim( GfxAnimRes * pRes );
	bool				removeCursor( CursorRes * pRes );
	bool				removeTextprop( TextpropRes * pRes );
	bool				removeColor( ColorRes * pRes );
	bool				removeLegoSource( LegoSource * pRes );
	bool				removeSkin( SkinRes * pRes );
	bool				removeConnect( ConnectRes* pRes );
	bool				removeResDb( ResDBRes* pRes );
	bool				removeWidget( WidgetRes * pRes );
	bool				removeDataSet( DataSetRes * pRes );



	WgSurface_p		getSurface( const std::string& id ) const;
	WgGlyphset_p		getGlyphset( const std::string& id ) const;
	WgFont_p			getFont( const std::string& id ) const;
	WgGfxAnim_p		getGfxAnim( const std::string& id ) const;
	WgCaret_p			getCursor( const std::string& id ) const;
	WgTextprop_p		getTextprop( const std::string& id ) const;
	WgColor				getColor( const std::string& id ) const;
	WgSkin_p			getSkin( const std::string& id ) const;
	WgWidget_p			getWidget( const std::string& id ) const;
	MetaData*			getDataSet( const std::string& id ) const;
	WgWidget_p			cloneWidget( const std::string& id ) const;


	ResDBRes *			getResDbRes( const std::string& id ) const;
	SurfaceRes *		getResSurface( const std::string& id ) const;
	GlyphsetRes *		getResGlyphset( const std::string& id ) const;
	FontRes *			getResFont( const std::string& id ) const;
	GfxAnimRes *		getResGfxAnim( const std::string& id ) const;
	CursorRes *			getResCursor( const std::string& id ) const;
	TextpropRes *		getResTextprop( const std::string& id ) const;
	ColorRes *			getResColor( const std::string& id ) const;
	LegoSource *		getLegoSource( const std::string& id ) const;
	SkinRes *			getResSkin( const std::string& id ) const;
	ConnectRes *		getResConnect( const std::string& id ) const;
	WidgetRes *			getResWidget( const std::string& id ) const;
	DataSetRes *		getResDataSet( const std::string& id ) const;

	ResDBRes *			findResDbRes( const WgResDB_p& data ) const;
	SurfaceRes *		findResSurface( const WgSurface_p& data ) const;
	GlyphsetRes *		findResGlyphset( const WgGlyphset_p& data ) const;
	FontRes *			findResFont( const WgFont_p& data ) const;
	GfxAnimRes *		findResGfxAnim( const WgGfxAnim_p& data ) const;
	CursorRes *			findResCursor( const WgCaret_p& data ) const;
	TextpropRes *		findResTextprop( const WgTextprop_p& data ) const;
	ColorRes *			findResColor( const WgColor col ) const;
	SkinRes *			findResSkin( const WgSkin_p& data ) const;
	WidgetRes *			findResWidget( const WgWidget_p& data ) const;

	std::string			findSurfaceId( const WgSurface_p& data ) const		{ SurfaceRes * r =	findResSurface(data); return r ? r->id : ""; }
	std::string			findGlyphsetId( const WgGlyphset_p& data ) const		{ GlyphsetRes * r =	findResGlyphset(data); return r ? r->id : ""; }
	std::string			findFontId( const WgFont_p& data ) const				{ FontRes *	r =		findResFont(data); return r ? r->id : ""; }
	std::string			findGfxAnimId( const WgGfxAnim_p& data ) const		{ GfxAnimRes *	r =		findResGfxAnim(data); return r ? r->id : ""; }
	std::string			findCursorId( const WgCaret_p& data ) const			{ CursorRes *	r =	findResCursor(data); return r ? r->id : ""; }
	std::string			findTextpropId( const WgTextprop_p& data ) const	{ TextpropRes *r =  findResTextprop(data); return r ? r->id : ""; }
	std::string			findColorId( const WgColor data ) const				{ ColorRes *r =		findResColor(data); return r ? r->id : ""; }
	std::string			findSkinId( const WgSkin_p& data ) const			{ SkinRes *r =		findResSkin(data); return r ? r->id : ""; }
	std::string			findWidgetId( const WgWidget_p& data ) const			{ WidgetRes *r =	findResWidget(data); return r ? r->id : ""; }

	inline ResDBRes *		getFirstResDbRes() const { return m_resDbs.first(); }
	inline SurfaceRes *		getFirstResSurface() const { return m_surfaces.first(); }
	inline GlyphsetRes *	getFirstResGlyphset() const { return m_glyphsets.first(); }
	inline FontRes *		getFirstResFont() const { return m_fonts.first(); }
	inline GfxAnimRes *		getFirstResGfxAnim() const { return m_gfxAnims.first(); }
	inline CursorRes *		getFirstResCursor() const { return m_cursors.first(); }
	inline TextpropRes *	getFirstResTextprop() const { return m_textProps.first(); }
	inline ColorRes *		getFirstResColor() const { return m_colors.first(); }
	inline SkinRes *		getFirstResSkin() const { return m_skins.first(); }
	inline ConnectRes *		getFirstResConnect() const { return m_connects.first(); }
	inline WidgetRes *		getFirstResWidget() const { return m_widgets.first(); }
	inline DataSetRes *		getFirstResDataSet() const { return m_dataSets.first(); }


private:
	WgResDB();
	virtual ~WgResDB();

	typedef std::map<std::string, ResDBRes*>		ResDBMap;
	typedef std::map<std::string, SurfaceRes*>		SurfMap;
	typedef std::map<std::string, GlyphsetRes*>		GlyphMap;
	typedef std::map<std::string, FontRes*>			FontMap;
	typedef std::map<std::string, GfxAnimRes*>		GfxAnimMap;
	typedef std::map<std::string, CursorRes*>		CursorMap;
	typedef std::map<std::string, TextpropRes*>		PropMap;
	typedef std::map<std::string, ColorRes*>		ColMap;
	typedef std::map<std::string, LegoSource*>		LegoMap;
	typedef std::map<std::string, SkinRes*>			SkinMap;
	typedef std::map<std::string, ConnectRes*>		ConnectMap;
	typedef std::map<std::string, WidgetRes*>		WidgetMap;
	typedef std::map<std::string, DataSetRes*>		DataSetMap;

	WgChain<ResDBRes>		m_resDbs;
	WgChain<SurfaceRes>		m_surfaces;
	WgChain<GlyphsetRes>	m_glyphsets;
	WgChain<FontRes>		m_fonts;
	WgChain<GfxAnimRes>		m_gfxAnims;
	WgChain<CursorRes>		m_cursors;
	WgChain<TextpropRes>	m_textProps;
	WgChain<ColorRes>		m_colors;
	WgChain<LegoSource>		m_legos;
	WgChain<SkinRes>		m_skins;
	WgChain<ConnectRes>		m_connects;
	WgChain<WidgetRes>		m_widgets;
	WgChain<DataSetRes>		m_dataSets;

	ResDBMap		m_mapResDBs;
	ColMap			m_mapColors;
	SurfMap			m_mapSurfaces;
	GlyphMap		m_mapGlyphsets;
	FontMap			m_mapFonts;
	GfxAnimMap		m_mapGfxAnims;
	CursorMap		m_mapCursors;
	PropMap			m_mapTextprops;
	LegoMap			m_mapLegoSources;
	SkinMap			m_mapSkins;
	ConnectMap		m_mapConnects;
	WidgetMap		m_mapWidgets;
	DataSetMap		m_mapDataSets;

	WgResLoader *	m_pResLoader;
};



#endif //WG_RESDB_DOT_H
