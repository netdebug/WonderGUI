#ifndef	WgGizmoProfiler_DOT_H
#define WgGizmoProfiler_DOT_H

#ifndef	WG_GIZMO_DOT_H
#	include <wg_gizmo.h>
#endif

#include <wg_textprop.h>

class WgFont;


class WgGizmoProfiler:public WgGizmo
{
	public:
		WgGizmoProfiler();
		virtual ~WgGizmoProfiler();
		virtual const char * Type() const;
		static const char * GetMyType();
		


		void SetTextProperties( WgTextPropPtr& pProp );

	protected:

		void	_onCloneContent( const WgGizmo * _pOrg );
		void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip, Uint8 _layer );
		void	_onUpdate( const WgUpdateInfo& _updateInfo );
		bool	_onAlphaTest( const WgCoord& ofs );


	private:

		WgTextPropPtr	m_pProp;
};



#endif //WG_GIZMO_PROFILER_DOT_H
