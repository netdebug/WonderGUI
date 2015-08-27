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

#include <stdlib.h>
#include <wg_resdb.h>
//#include <wg_resources_xml.h>
#include <wg_resloader.h>
#include <assert.h>
#include <wg_font.h>
#include <wg_surface.h>

namespace wg 
{
	
	const char ResDB::CLASSNAME[] = {"ResDB"};
	
	
	//____ () _________________________________________________________
	
	ResDB::ResDB() :
		m_pResLoader(0)
	{
	}
	
	//____ () _________________________________________________________
	
	ResDB::~ResDB()
	{
		clear();
	}
	
	//____ isInstanceOf() _________________________________________________________
	
	bool ResDB::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;
	
		return Object::isInstanceOf(pClassName);
	}
	
	//____ className() ____________________________________________________________
	
	const char * ResDB::className( void ) const
	{
		return CLASSNAME;
	}
	
	//____ cast() _________________________________________________________________
	
	ResDB_p ResDB::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ResDB_p( static_cast<ResDB*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ () _________________________________________________________
	
	void ResDB::clear()
	{
		// Clear the quick-reference maps
	
		m_mapColors.clear();
		m_mapSurfaces.clear();
		m_mapGlyphsets.clear();
		m_mapFonts.clear();
		m_mapGfxAnims.clear();
		m_mapCursors.clear();
		m_mapColors.clear();
		m_mapTextprops.clear();
		m_mapLegoSources.clear();
		m_mapSkins.clear();
		m_mapWidgets.clear();
		m_mapConnects.clear();
		m_mapResDBs.clear();
		m_mapDataSets.clear();
	
		// Clear the linked lists, this will also delete the ResWrapper objects
		// along with their meta-data but NOT the resources themselves.
		// This will cause memory leaks until all resources are properly equipped
		// with smartpointers.
	
		m_surfaces.clear();
		m_glyphsets.clear();
		m_fonts.clear();
		m_gfxAnims.clear();
		m_cursors.clear();
		m_textProps.clear();
		m_colors.clear();
		m_legos.clear();
		m_skins.clear();
		m_widgets.clear();
		m_connects.clear();
		m_resDbs.clear();
		m_dataSets.clear();
	}
	
	void ResDB::clearSurfaces()
	{
		m_mapSurfaces.clear();
		m_surfaces.clear();
	}
	
	void ResDB::clearWidgets()
	{
		m_mapWidgets.clear();
		m_widgets.clear();
	}
	
	//____ () _________________________________________________________
	
	std::string	ResDB::generateName( const Surface_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_surf__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const Glyphset_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_glyphset__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const Font_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_font__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const GfxAnim_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_anim__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const Caret_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_cursor__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const Color data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_color__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const Textprop_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_textprop__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const Skin_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_skin__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	std::string	ResDB::generateName( const Widget_p& data )
	{
		static int nGenerated = 0;
		char pBuf[100];
		return std::string("_widget__") + TextTool::itoa(++nGenerated, pBuf, 10);
	}
	
	void ResDB::setResLoader( ResLoader * pLoader )
	{
		m_pResLoader = pLoader;
	}
	
	//____ () _________________________________________________________
	bool ResDB::addResDb( const std::string& file, MetaData * pMetaData )
	{
	//	assert(getResDbRes(file) == 0);
	
		if(getResDbRes(file) == 0 && m_pResLoader)
		{
			ResDB_p pDb = m_pResLoader->loadDb( file, this );
			// store resource even if load failed. could be an optional include
			ResDBRes* p = new ResDBRes(file, pDb, file, pMetaData);
			m_resDbs.pushBack(p);
			m_mapResDBs[file] = p;
			return true;
		}
		return false;
	}
	
	bool ResDB::addResDb( const ResDB_p& db, const std::string& file, MetaData * pMetaData )
	{
		assert(getResDbRes(file) == 0);
	
		if( !db )
			return false;
	
		if(getResDbRes(file) == 0)
		{
			ResDBRes* p = new ResDBRes(file, db, file, pMetaData);
			m_resDbs.pushBack(p);
			if(file.size())
				m_mapResDBs[file] = p;
			return true;
		}
		return false;
	}
	
	bool ResDB::addSurface( const std::string& id, const std::string& file, MetaData * pMetaData, bool bRequired )
	{
		assert(m_mapSurfaces.find(id) == m_mapSurfaces.end());
	
		if(m_mapSurfaces.find(id) == m_mapSurfaces.end() && m_pResLoader)
		{
			Surface_p pSurface = m_pResLoader->loadSurface( file, bRequired );
			if( !pSurface )
				return false;
	
			SurfaceRes* p = new SurfaceRes(id, pSurface, file, pMetaData);
			m_surfaces.pushBack(p);
			if(id.size())
				m_mapSurfaces[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addSurface( const std::string& id, const Surface_p& pSurface, const std::string& filename, MetaData * pMetaData )
	{
		assert(m_mapSurfaces.find(id) == m_mapSurfaces.end());
		if(m_mapSurfaces.find(id) == m_mapSurfaces.end())
		{
			SurfaceRes* p = new SurfaceRes(id, pSurface, filename, pMetaData);
			m_surfaces.pushBack(p);
			if(id.size())
				m_mapSurfaces[id] = p;
			return true;
		}
		return false;
	}
	
	
	//____ () _________________________________________________________
	
	bool ResDB::addGlyphset( const std::string& id, const std::string& file, MetaData * pMetaData )
	{
		assert(m_mapGlyphsets.find(id) == m_mapGlyphsets.end());
	
		if(m_mapGlyphsets.find(id) == m_mapGlyphsets.end() && m_pResLoader)
		{
			Glyphset_p pGlyphset = m_pResLoader->loadGlyphset( file );
			if( !pGlyphset )
				return false;
	
			GlyphsetRes* p = new GlyphsetRes(id, pGlyphset, file, pMetaData);
			m_glyphsets.pushBack(p);
			if(id.size())
				m_mapGlyphsets[id] = p;
			return true;
		}
		return false;
	}
	
	bool ResDB::addGlyphset( const std::string& id, const Glyphset_p& pGlyphset, const std::string& file, MetaData * pMetaData )
	{
		assert(m_mapGlyphsets.find(id) == m_mapGlyphsets.end());
		if(m_mapGlyphsets.find(id) == m_mapGlyphsets.end())
		{
			GlyphsetRes* p = new GlyphsetRes(id, pGlyphset, file, pMetaData);
			m_glyphsets.pushBack(p);
			if(id.size())
				m_mapGlyphsets[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addFont( const std::string& id, const Font_p& pFont, MetaData * pMetaData )
	{
		assert(m_mapFonts.find(id) == m_mapFonts.end());
		if(m_mapFonts.find(id) == m_mapFonts.end())
		{
			FontRes* p = new FontRes(id, pFont, pMetaData);
			m_fonts.pushBack(p);
			if(id.size())
				m_mapFonts[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addGfxAnim( const std::string& id, const GfxAnim_p& pAnim, MetaData * pMetaData )
	{
		assert(m_mapGfxAnims.find(id) == m_mapGfxAnims.end());
		if(m_mapGfxAnims.find(id) == m_mapGfxAnims.end())
		{
			GfxAnimRes* p = new GfxAnimRes(id, pAnim, pMetaData);
			m_gfxAnims.pushBack(p);
			if(id.size())
				m_mapGfxAnims[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addCursor( const std::string& id, const Caret_p& pCursor, MetaData * pMetaData )
	{
		assert(m_mapCursors.find(id) == m_mapCursors.end());
		if(m_mapCursors.find(id) == m_mapCursors.end())
		{
			CursorRes* p = new CursorRes(id, pCursor, pMetaData);
			m_cursors.pushBack(p);
			if(id.size())
				m_mapCursors[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addTextprop( const std::string& id, const Textprop_p& pProp, MetaData * pMetaData )
	{
		//assert(m_mapTextprops.find(id) == m_mapTextprops.end());
		if(m_mapTextprops.find(id) == m_mapTextprops.end())
		{
			TextpropRes* p = new TextpropRes(id, pProp, pMetaData);
			m_textProps.pushBack(p);
			if(id.size())
				m_mapTextprops[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addColor( const std::string& id, Color col, MetaData * pMetaData )
	{
		assert(m_mapColors.find(id) == m_mapColors.end());
		if(m_mapColors.find(id) == m_mapColors.end())
		{
			ColorRes* p = new ColorRes(id, col, pMetaData);
			m_colors.pushBack(p);
			if(id.size())
				m_mapColors[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addLegoSource( const std::string& id, const std::string& surface, Rect rect, Uint32 nStates, MetaData * pMetaData )
	{
		assert(m_mapLegoSources.find(id) == m_mapLegoSources.end());
		if(m_mapLegoSources.find(id) == m_mapLegoSources.end())
		{
			LegoSource* p = new LegoSource(id, surface, rect, nStates, pMetaData);
			m_legos.pushBack(p);
			if(id.size())
				m_mapLegoSources[id] = p;
			return true;
		}
		return false;
	}
	
	std::string ResDB::loadString( const std::string& token )
	{
		return m_pResLoader->loadString(token);
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addSkin( const std::string& id, const Skin_p& pSkin, MetaData * pMetaData )
	{
		assert(m_mapSkins.find(id) == m_mapSkins.end());
		if(m_mapSkins.find(id) == m_mapSkins.end())
		{
			SkinRes* p = new SkinRes(id, pSkin, pMetaData);
			m_skins.pushBack(p);
			if(id.size())
				m_mapSkins[id] = p;
			return true;
		}
		return false;
	}
	
	//____ () _________________________________________________________
	
	bool ResDB::addDataSet( const std::string& id, MetaData * pMetaData )
	{
		assert(m_mapDataSets.find(id) == m_mapDataSets.end());
		if(m_mapDataSets.find(id) == m_mapDataSets.end())
		{
			DataSetRes* p = new DataSetRes(id, 0, pMetaData);
			m_dataSets.pushBack(p);
			if(id.size())
				m_mapDataSets[id] = p;
			return true;
		}
		return false;
	}
	
	
	//____ () _________________________________________________________
	
	bool ResDB::addWidget( const std::string& id, const Widget_p& pWidget, MetaData * pMetaData )
	{
		assert(m_mapWidgets.find(id) == m_mapWidgets.end());
		if(m_mapWidgets.find(id) == m_mapWidgets.end())
		{
			WidgetRes* p = new WidgetRes(id, pWidget, pMetaData);
			m_widgets.pushBack(p);
			if(id.size())
				m_mapWidgets[id] = p;
			return true;
		}
		return false;
	}
	
	
	//____ () _________________________________________________________
	
	bool ResDB::addConnect( MetaData * pMetaData )
	{
		ConnectRes* p = new ConnectRes(pMetaData);
		m_connects.pushBack(p);
		return true;
	}
	
	//____ () _________________________________________________________
	
	Surface_p ResDB::getSurface( const std::string& id ) const
	{
		SurfaceRes* surfRes = getResSurface(id);
		return surfRes ? surfRes->res : Surface_p();
	}
	
	//____ () _________________________________________________________
	
	Glyphset_p ResDB::getGlyphset( const std::string& id ) const
	{
		GlyphsetRes* glyphRes = getResGlyphset(id);
		return glyphRes ? glyphRes->res : Glyphset_p();
	}
	
	//____ () _________________________________________________________
	
	Font_p ResDB::getFont( const std::string& id ) const
	{
		FontRes* fontRes = getResFont(id);
		return fontRes ? fontRes->res : Font_p();
	}
	
	//____ () _________________________________________________________
	
	GfxAnim_p ResDB::getGfxAnim( const std::string& id ) const
	{
		GfxAnimRes* animRes = getResGfxAnim(id);
		return animRes ? animRes->res : GfxAnim_p();
	}
	
	//____ () _________________________________________________________
	
	Caret_p ResDB::getCursor( const std::string& id ) const
	{
		CursorRes* cursorRes = getResCursor(id);
		return cursorRes ? cursorRes->res : Caret_p();
	}
	
	//____ () _________________________________________________________
	
	Textprop_p ResDB::getTextprop( const std::string& id ) const
	{
		TextpropRes* propRes = getResTextprop(id);
		return propRes ? propRes->res : Textprop_p();
	}
	
	//____ () _________________________________________________________
	
	Color ResDB::getColor( const std::string& id ) const
	{
		ColorRes* colorRes = getResColor(id);
		return colorRes ? colorRes->res : Color::black;
	}
	
	//____ () _________________________________________________________
	
	Skin_p ResDB::getSkin( const std::string& id ) const
	{
		SkinRes* skinRes = getResSkin(id);
		return skinRes ? skinRes->res : Skin_p();
	}
	
	//____ () _________________________________________________________
	
	ResDB::MetaData * ResDB::getDataSet( const std::string& id ) const
	{
		DataSetRes* dataSetRes = getResDataSet(id);
		return dataSetRes ? dataSetRes->meta : 0;
	}
	
	//____ () _________________________________________________________
	
	Widget_p ResDB::getWidget( const std::string& id ) const
	{
		WidgetRes* widgetRes = getResWidget(id);
		return widgetRes ? widgetRes->res : Widget_p();
	}
	
	//____ () _________________________________________________________
	
	Widget_p ResDB::cloneWidget( const std::string& id ) const
	{
		WidgetRes* widgetRes = getResWidget(id);
	
		if( !widgetRes )
			return 0;
	
		Widget_p pWidget = widgetRes->res;
	
		Widget_p pClone = pWidget->newOfMyType();
		pClone->cloneContent(pWidget.rawPtr());
		return pClone;
	}
	
	//____ () _________________________________________________________
	
	ResDB::ResDBRes * ResDB::getResDbRes( const std::string& id ) const
	{
		for(ResDBRes* res = getFirstResDbRes(); res; res = res->next())
		{
			if(res->file == id)
				return res;
			if(res->res)
			{
				ResDB::ResDBRes * pDb = res->res->getResDbRes(id);
				if(pDb)
					return pDb;
			}
		}
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::SurfaceRes * ResDB::getResSurface( const std::string& id ) const
	{
		SurfaceRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResSurface(id)))
					return res;
			}
		}
		SurfMap::const_iterator it = m_mapSurfaces.find(id);
		return it == m_mapSurfaces.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::GlyphsetRes * ResDB::getResGlyphset( const std::string& id ) const
	{
		GlyphsetRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResGlyphset(id)))
					return res;
			}
		}
		GlyphMap::const_iterator it = m_mapGlyphsets.find(id);
		return it == m_mapGlyphsets.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::FontRes * ResDB::getResFont( const std::string& id ) const
	{
		FontRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResFont(id)))
					return res;
			}
		}
		FontMap::const_iterator it = m_mapFonts.find(id);
		return it == m_mapFonts.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::GfxAnimRes * ResDB::getResGfxAnim( const std::string& id ) const
	{
		GfxAnimRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResGfxAnim(id)))
					return res;
			}
		}
		GfxAnimMap::const_iterator it = m_mapGfxAnims.find(id);
		return it == m_mapGfxAnims.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::CursorRes * ResDB::getResCursor( const std::string& id ) const
	{
		CursorRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResCursor(id)))
					return res;
			}
		}
		CursorMap::const_iterator it = m_mapCursors.find(id);
		return it == m_mapCursors.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::TextpropRes * ResDB::getResTextprop( const std::string& id ) const
	{
		TextpropRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResTextprop(id)))
					return res;
			}
		}
		PropMap::const_iterator it = m_mapTextprops.find(id);
		return it == m_mapTextprops.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::ColorRes * ResDB::getResColor( const std::string& id ) const
	{
		ColorRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResColor(id)))
					return res;
			}
		}
		ColMap::const_iterator it = m_mapColors.find(id);
		return it == m_mapColors.end() ? 0 : it->second;
	}
	
	
	//____ () _________________________________________________________
	
	ResDB::LegoSource * ResDB::getLegoSource( const std::string& id ) const
	{
		LegoSource* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getLegoSource(id)))
					return res;
			}
		}
		LegoMap::const_iterator it = m_mapLegoSources.find(id);
		return it == m_mapLegoSources.end() ? 0 : it->second;
	}
	
	ResDB::SkinRes * ResDB::getResSkin( const std::string& id ) const
	{
		SkinRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResSkin(id)))
					return res;
			}
		}
		SkinMap::const_iterator it = m_mapSkins.find(id);
		return it == m_mapSkins.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::WidgetRes * ResDB::getResWidget( const std::string& id ) const
	{
		WidgetRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResWidget(id)))
					return res;
			}
		}
		WidgetMap::const_iterator it = m_mapWidgets.find(id);
		return it == m_mapWidgets.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::DataSetRes * ResDB::getResDataSet( const std::string& id ) const
	{
		DataSetRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResDataSet(id)))
					return res;
			}
		}
		DataSetMap::const_iterator it = m_mapDataSets.find(id);
		return it == m_mapDataSets.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::ConnectRes * ResDB::getResConnect( const std::string& id ) const
	{
		ConnectRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->getResConnect(id)))
					return res;
			}
		}
		ConnectMap::const_iterator it = m_mapConnects.find(id);
		return it == m_mapConnects.end() ? 0 : it->second;
	}
	
	//____ () _________________________________________________________
	
	ResDB::ResDBRes * ResDB::findResDbRes( const ResDB_p& data ) const
	{
		for(ResDBRes* res = getFirstResDbRes(); res; res = res->next())
		{
			if(res->res == data)
				return res;
			if(res->res)
			{
				ResDBRes* res2 = res->res->findResDbRes(data);
				if(res2)
					return res2;
			}
		}
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::SurfaceRes* ResDB::findResSurface( const Surface_p& surf ) const
	{
		SurfaceRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResSurface(surf)))
					return res;
			}
		}
		for(res = getFirstResSurface(); res; res = res->next())
			if(res->res == surf)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::GlyphsetRes* ResDB::findResGlyphset( const Glyphset_p& meta ) const
	{
		GlyphsetRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResGlyphset(meta)))
					return res;
			}
		}
		for(res = getFirstResGlyphset(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::FontRes* ResDB::findResFont( const Font_p& meta ) const
	{
		FontRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResFont(meta)))
					return res;
			}
		}
		for(res = getFirstResFont(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::GfxAnimRes* ResDB::findResGfxAnim( const GfxAnim_p& meta ) const
	{
		GfxAnimRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResGfxAnim(meta)))
					return res;
			}
		}
		for(res = getFirstResGfxAnim(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::CursorRes* ResDB::findResCursor( const Caret_p& meta ) const
	{
		CursorRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResCursor(meta)))
					return res;
			}
		}
		for(res = getFirstResCursor(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::TextpropRes* ResDB::findResTextprop( const Textprop_p& meta ) const
	{
		TextpropRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResTextprop(meta)))
					return res;
			}
		}
		for(res = getFirstResTextprop(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::ColorRes* ResDB::findResColor( const Color meta ) const
	{
		ColorRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResColor(meta)))
					return res;
			}
		}
		for(res = getFirstResColor(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::SkinRes* ResDB::findResSkin( const Skin_p& meta ) const
	{
		SkinRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResSkin(meta)))
					return res;
			}
		}
		for(res = getFirstResSkin(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	//____ () _________________________________________________________
	
	ResDB::WidgetRes* ResDB::findResWidget( const Widget_p& meta ) const
	{
		WidgetRes* res = 0;
		for(ResDBRes* db = getFirstResDbRes(); db; db = db->next())
		{
			if(db->res)
			{
				if((res = db->res->findResWidget(meta)))
					return res;
			}
		}
		for(res = getFirstResWidget(); res; res = res->next())
			if(res->res == meta)
				return res;
		return 0;
	}
	
	
	//____ removeSurface() ________________________________________________________
	
	bool ResDB::removeSurface( const std::string& id )
	{
		SurfMap::iterator it = m_mapSurfaces.find( id );
	
		if( it == m_mapSurfaces.end() )
			return false;
	
		SurfaceRes * pRes = it->second;
		m_mapSurfaces.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeSurface( ResDB::SurfaceRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			SurfMap::iterator it = m_mapSurfaces.find( pRes->id );
			assert( it != m_mapSurfaces.end() );
			m_mapSurfaces.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ removeGlyphset() _______________________________________________________
	
	bool ResDB::removeGlyphset( const std::string& id )
	{
		GlyphMap::iterator it = m_mapGlyphsets.find( id );
	
		if( it == m_mapGlyphsets.end() )
			return false;
	
		GlyphsetRes * pRes = it->second;
		m_mapGlyphsets.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeGlyphset( ResDB::GlyphsetRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			GlyphMap::iterator it = m_mapGlyphsets.find( pRes->id );
			assert( it != m_mapGlyphsets.end() );
			m_mapGlyphsets.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ removeFont() ___________________________________________________________
	
	bool ResDB::removeFont( const std::string& id )
	{
		FontMap::iterator it = m_mapFonts.find( id );
	
		if( it == m_mapFonts.end() )
			return false;
	
		FontRes * pRes = it->second;
		m_mapFonts.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeFont( ResDB::FontRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			FontMap::iterator it = m_mapFonts.find( pRes->id );
			assert( it != m_mapFonts.end() );
			m_mapFonts.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ RemoveAnim() ___________________________________________________________
	
	bool ResDB::removeGfxAnim( const std::string& id )
	{
		GfxAnimMap::iterator it = m_mapGfxAnims.find( id );
	
		if( it == m_mapGfxAnims.end() )
			return false;
	
		GfxAnimRes * pRes = it->second;
		m_mapGfxAnims.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeGfxAnim( ResDB::GfxAnimRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			GfxAnimMap::iterator it = m_mapGfxAnims.find( pRes->id );
			assert( it != m_mapGfxAnims.end() );
			m_mapGfxAnims.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ removeCursor() ___________________________________________________________
	
	bool ResDB::removeCursor( const std::string& id )
	{
		CursorMap::iterator it = m_mapCursors.find( id );
	
		if( it == m_mapCursors.end() )
			return false;
	
		CursorRes * pRes = it->second;
		m_mapCursors.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeCursor( ResDB::CursorRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			CursorMap::iterator it = m_mapCursors.find( pRes->id );
			assert( it != m_mapCursors.end() );
			m_mapCursors.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ removeTextprop() _______________________________________________________
	
	bool ResDB::removeTextprop( const std::string& id )
	{
		PropMap::iterator it = m_mapTextprops.find( id );
	
		if( it == m_mapTextprops.end() )
			return false;
	
		TextpropRes * pRes = it->second;
		m_mapTextprops.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeTextprop( ResDB::TextpropRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			PropMap::iterator it = m_mapTextprops.find( pRes->id );
			assert( it != m_mapTextprops.end() );
			m_mapTextprops.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ removeColor() __________________________________________________________
	
	bool ResDB::removeColor( const std::string& id )
	{
		ColMap::iterator it = m_mapColors.find( id );
	
		if( it == m_mapColors.end() )
			return false;
	
		ColorRes * pRes = it->second;
		m_mapColors.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeColor( ResDB::ColorRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			ColMap::iterator it = m_mapColors.find( pRes->id );
			assert( it != m_mapColors.end() );
			m_mapColors.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	
	//____ removeLegoSource() _____________________________________________________
	
	bool ResDB::removeLegoSource( const std::string& id )
	{
		LegoMap::iterator it = m_mapLegoSources.find( id );
	
		if( it == m_mapLegoSources.end() )
			return false;
	
		LegoSource * pRes = it->second;
		m_mapLegoSources.erase(it);
		delete pRes;
		return true;
	}
	
	bool ResDB::removeLegoSource( ResDB::LegoSource * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			LegoMap::iterator it = m_mapLegoSources.find( pRes->id );
			assert( it != m_mapLegoSources.end() );
			m_mapLegoSources.erase(it);
		}
		delete pRes;
		return true;
	}
	
	//____ removeSkin() _______________________________________________________
	
	bool ResDB::removeSkin( const std::string& id )
	{
		SkinMap::iterator it = m_mapSkins.find( id );
	
		if( it == m_mapSkins.end() )
			return false;
	
		SkinRes * pRes = it->second;
		m_mapSkins.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeSkin( ResDB::SkinRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			SkinMap::iterator it = m_mapSkins.find( pRes->id );
			assert( it != m_mapSkins.end() );
			m_mapSkins.erase(it);
		}
		delete pRes;
		return true;
	}
	
	//____ removeDataSet() _________________________________________________________
	
	bool ResDB::removeDataSet( const std::string& id )
	{
		DataSetMap::iterator it = m_mapDataSets.find( id );
	
		if( it == m_mapDataSets.end() )
			return false;
	
		DataSetRes * pRes = it->second;
		m_mapDataSets.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeDataSet( ResDB::DataSetRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			DataSetMap::iterator it = m_mapDataSets.find( pRes->id );
			assert( it != m_mapDataSets.end() );
			m_mapDataSets.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ removeWidget() _________________________________________________________
	
	bool ResDB::removeWidget( const std::string& id )
	{
		WidgetMap::iterator it = m_mapWidgets.find( id );
	
		if( it == m_mapWidgets.end() )
			return false;
	
		WidgetRes * pRes = it->second;
		m_mapWidgets.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeWidget( ResDB::WidgetRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			WidgetMap::iterator it = m_mapWidgets.find( pRes->id );
			assert( it != m_mapWidgets.end() );
			m_mapWidgets.erase(it);
		}
		delete pRes;
		return true;
	}
	
	
	//____ removeConnect() _______________________________________________________
	
	bool ResDB::removeConnect( const std::string& id )
	{
		ConnectMap::iterator it = m_mapConnects.find( id );
	
		if( it == m_mapConnects.end() )
			return false;
	
		ConnectRes * pRes = it->second;
		m_mapConnects.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeConnect( ResDB::ConnectRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			ConnectMap::iterator it = m_mapConnects.find( pRes->id );
			assert( it != m_mapConnects.end() );
			m_mapConnects.erase(it);
		}
		delete pRes;
		return true;
	}
	
	//____ removeResDb() _______________________________________________________
	
	bool ResDB::removeResDb( const std::string& id )
	{
		ResDBMap::iterator it = m_mapResDBs.find( id );
	
		if( it == m_mapResDBs.end() )
			return false;
	
		ResDBRes * pRes = it->second;
		m_mapResDBs.erase(it);
		delete pRes;
	
		return true;
	}
	
	bool ResDB::removeResDb( ResDB::ResDBRes * pRes )
	{
		if( !pRes )
			return false;
	
		if( pRes->id.length() > 0 )
		{
			ResDBMap::iterator it = m_mapResDBs.find( pRes->id );
			assert( it != m_mapResDBs.end() );
			m_mapResDBs.erase(it);
		}
		delete pRes;
		return true;
	}

} // namespace wg