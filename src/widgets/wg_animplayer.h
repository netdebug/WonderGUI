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

#ifndef	WG_ANIMPLAYER_DOT_H
#define	WG_ANIMPLAYER_DOT_H


#ifndef	WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef	WG_GFXANIM_DOT_H
#	include <wg_gfxanim.h>
#endif

namespace wg 
{
	
	class WgAnimPlayer;
	typedef	WgStrongPtr<WgAnimPlayer,WgWidget_p>		WgAnimPlayer_p;
	typedef	WgWeakPtr<WgAnimPlayer,WgWidget_wp>	WgAnimPlayer_wp;
	
	
	class WgAnimPlayer:public WgWidget
	{
	public:
		static WgAnimPlayer_p	create() { return WgAnimPlayer_p(new WgAnimPlayer()); }
	
		bool		isInstanceOf( const char * pClassName ) const;
		const char *className( void ) const;
		static const char	CLASSNAME[];
		static WgAnimPlayer_p	cast( const WgObject_p& pObject );
	
		//____ Methods __________________________________________
	
		bool			setAnimation( const WgGfxAnim_p& pAnim );
		WgGfxAnim_p		animation() const { return m_pAnim; }
			
		int				playPos();										/// Returns play position in ticks.
		bool			setPlayPos( int ticks );						/// Position in ticks for next update.
		bool			setPlayPosFractional( float fraction );			/// Position in fractions of duration.
		
		bool			rewind( int ticks );
		bool			fastForward( int ticks );
	
		int				duration();										/// Returns duration of animation (one-shot-through, no looping).
		int				durationScaled();								/// Returns duration of animation, scaled by speed.
	
		float			speed();
		bool			setSpeed( float speed );
	
		bool			play();
		bool			stop();
		bool			isPlaying() { return m_bPlaying; };
	
		WgSize			preferredSize() const;
	
	protected:
		WgAnimPlayer();
		virtual ~WgAnimPlayer();
		virtual WgWidget* _newOfMyType() const { return new WgAnimPlayer(); };
	
		void			_onCloneContent( const WgWidget * _pOrg );
		void			_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
		void			_onRefresh();
		void			_onMsg( const WgMsg_p& pMsg );
		bool			_onAlphaTest( const WgCoord& ofs, const WgSize& sz );
		void			_onStateChanged( WgState oldState );
	
		void			_playPosUpdated();
	
	private:
	
		WgGfxAnim_p	m_pAnim;
		WgGfxFrame *	m_pAnimFrame;			// Frame currently used by animation.
		WgRouteId		m_tickRouteId;
	
		bool			m_bPlaying;
		double			m_playPos;
		float			m_speed;
	};
	
	

} // namespace wg
#endif //	WG_ANIMPLAYER_DOT_H
