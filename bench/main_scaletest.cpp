#include <cstdlib>

#ifdef WIN32
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif


#include <wondergui.h>
#include <wg_bitmapglyphs.h>
#include <wg_vectorglyphs.h>
#include <wg_knob.h>
#include <iostream>

#include <wg_surface_soft.h>
#include <wg_surfacefactory_soft.h>
#include <wg_glsurface.h>
#include <wg_gfxdevice_soft.h>
#include <wg_glgfxdevice.h>
#include <sdl_wglib.h>
#include <wg_boxskin.h>
#include <wg_volumemeter.h>
#include <wg_simplevolumemeter.h>


#include "testwidget.h"

void DBG_ASSERT(bool x) {}


extern std::ostream cout;

bool			eventLoop( WgEventHandler * pHandler );
WgRootPanel * 		setupGUI( WgGfxDevice * pDevice );

void cbInitDrag( const WgEvent::Event* _pEvent, WgWidget * pWidget );
void cbDragWidget( const WgEvent::Event* _pEvent, WgWidget * pWidget );

void cbOpenModal( const WgEvent::Event* _pEvent, WgWidget * pWidget );
void cbCloseModal( const WgEvent::Event* _pEvent, WgWidget * pWidget );

void addResizablePanel( WgFlexPanel * pParent, WgWidget * pChild, WgEventHandler * pEventHandler );
void cbResize( const WgEvent::Event* _pEvent, void * _pFlexHook );

WgWidget * generateScalingUI( WgResDB * pDB );

WgFlexPanel * createPresetSelector();
void updateOscilloscope( WgOscilloscope * pOsc, int ofs, float width, float amplitude );


WgModalLayer * g_pModal = 0;

WgWidget * pWidgetToMove = 0;

WgValueDisplay * m_pCounter = 0;
WgValueDisplay * m_pCounter2 = 0;

WgVolumeMeter * m_pVolMeter = 0;
WgSimpleVolumeMeter * m_pSimpleVolMeter1 = 0;
WgOscilloscope * g_pOsc = 0;

WgSizeCapsule * s_pScaleSizer = 0;

const WgSize s_originalSize(400,400);


float	leftPeak = 1.f, rightPeak = 0.5f, leftHold = 0.5f, rightHold = 0.5f;

bool	leftUp = true, rightUp = false;


//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{

	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 0, posY = 0, width = 1600, height = 1000;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0);

	SDL_Surface * pScreen = SDL_GetWindowSurface(pWin);

	IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );

	// Init WonderGUI

	WgBase::Init();
	WgBase::InitFreeType();
	sdl_wglib::MapKeys();
	
	WgBase::SetMouseButtonRepeat( 150, 50 );


	WgSurface * pBackImg = sdl_wglib::LoadSurface("../resources/What-Goes-Up-3.bmp", WgSurfaceFactorySoft() );


//	WgBase::InitFreeType();

	// Setup gfxdevice and gui

	WgPixelType type = WG_PIXEL_UNKNOWN;

	if (pScreen->format->BitsPerPixel == 32)
		type = WG_PIXEL_BGRA_8;
	else if (pScreen->format->BitsPerPixel == 24)
		type = WG_PIXEL_BGR_8;


	WgSurfaceSoft * pCanvas = new WgSurfaceSoft( WgSize(width,height), type, (unsigned char *) pScreen->pixels, pScreen->pitch );
	WgGfxDeviceSoft * pGfxDevice = new WgGfxDeviceSoft( pCanvas );


	// Load TTF-font

	WgVectorGlyphs::SetSurfaceFactory( new WgSurfaceFactorySoft() );

	char	ttfname[] = { "../resources/DroidSans.ttf" };

	int size = sdl_wglib::FileSize( ttfname );
	char * pFontFile = (char*) sdl_wglib::LoadFile( ttfname );
	WgVectorGlyphs * pGlyphs = new WgVectorGlyphs( pFontFile , size, 0 );

	WgFont * pFont = new WgFont();
	pFont->SetDefaultVectorGlyphs( pGlyphs );

	// Load bitmap font

//	WgFont * pFont = sdl_wglib::LoadBitmapFont( "../resources/anuvverbubbla_8x8.png", "../resources/anuvverbubbla_8x8.fnt", WgSurfaceFactorySoft() );

	// Load and setup cursor

	WgSurface * pCursorImg = sdl_wglib::LoadSurface("../resources/cursors.png", WgSurfaceFactorySoft() );

	WgGfxAnim * pCursorEOL = new WgGfxAnim();
	pCursorEOL->SetSize( WgSize(8,8) );
	pCursorEOL->AddFrames(pCursorImg, WgCoord(0,0), WgSize(2,1), 200 );
	pCursorEOL->SetPlayMode( WG_FORWARD_LOOPING );

	WgGfxAnim * pCursorINS = new WgGfxAnim();
	pCursorINS->SetSize( WgSize(8,8) );
	pCursorINS->AddFrames( pCursorImg, WgCoord(0,8), WgSize(2,1), 200 );
	pCursorINS->SetPlayMode( WG_FORWARD_LOOPING );

	WgCursor * pCursor = new WgCursor();
	pCursor->SetAnim(WgCursor::EOL, pCursorEOL);
	pCursor->SetAnim(WgCursor::INS, pCursorINS);
	pCursor->SetAnim(WgCursor::OVR, pCursorEOL);
	pCursor->SetBearing(WgCursor::EOL, WgCoord(0,-8));
	pCursor->SetBearing(WgCursor::INS, WgCoord(0,-8));
	pCursor->SetBearing(WgCursor::OVR, WgCoord(0,-8));

	WgBase::SetDefaultCursor( pCursor );

	// Set default textprop

	WgTextprop prop;

	prop.SetFont(pFont);
	prop.SetColor( WgColor::black );
	prop.SetSize(12);

	WgBase::SetDefaultTextprop( prop.Register() );


	WgRootPanel * pRoot = setupGUI( pGfxDevice );
	
	
	// Setup debug overlays
/*	
	WgBoxSkinPtr pOverlaySkin = WgBoxSkin::Create( WgColor(255,0,0,128), WgBorders(1), WgColor::black);
	pOverlaySkin->SetStateColor( WG_STATE_NORMAL, WgColor::transparent, WgColor::red );	
	pRoot->SetUpdatedRectOverlay( pOverlaySkin, 0 );
*/	


   // program main loop

	int counter = 0;

    while (eventLoop( pRoot->EventHandler() ))
    {
		if( m_pCounter )
			m_pCounter->IncValue();
		
		if( m_pCounter2 )
			m_pCounter2->IncValue();
		
		
		if( m_pVolMeter )
			m_pVolMeter->SetValue( (counter%100)/100.f, (counter%1000)/1000.f ); 
			
		if( m_pSimpleVolMeter1 )
		{
			if( leftUp )
				leftPeak += 0.01;
			else
				leftPeak -= 0.01;
				
			if( leftPeak >= 1.f )
				leftUp = false;
				
			if( leftPeak <= 0.f )
				leftUp = true;
				
			if( leftPeak > leftHold )
				leftHold = leftPeak;
			else
				leftHold -= 0.003;
				

			if( rightUp )
				rightPeak += 0.01;
			else
				rightPeak -= 0.01;
				
			if( rightPeak >= 1.f )
				rightUp = false;
				
			if( rightPeak <= 0.f )
				rightUp = true;
				
			if( rightPeak > rightHold )
				rightHold = rightPeak;
			else
				rightHold -= 0.003;

				
			m_pSimpleVolMeter1->SetValue( leftPeak, leftHold, rightPeak, rightHold );
		}
			
			
		if( g_pOsc )
		{
			const int freq_max = 500;
			
			float freq;
			
			int x = counter % freq_max;
			if( x <= freq_max/2 )
				freq = 0.0005f * x;
			else
				freq = 0.0005f * (freq_max - x); 
			
			
			const int amp_mod_speed = 600;
			
			float amp;
			
			int y = counter % amp_mod_speed;
			if( y <= amp_mod_speed/2 )
				amp = 1.f * y / (amp_mod_speed/2);
			else
				amp = 1.f * (amp_mod_speed - y) / (amp_mod_speed/2);
			
			
			updateOscilloscope( g_pOsc, counter, freq, amp );

		}	
			
		// DRAWING STARTS HERE

//		pRoot->AddDirtyPatch( pRoot->Geo().Size() );

		SDL_LockSurface( pScreen );
		pRoot->Render();
		SDL_UnlockSurface( pScreen );

        // DRAWING ENDS HERE

        // finally, update the screen :)

		// GET DIRTY RECTS

		int nUpdatedRects;
		SDL_Rect	updatedRects[100];

		if( pRoot->NbUpdatedRects() <= 100 )
		{
			nUpdatedRects = pRoot->NbUpdatedRects();
			for( int i = 0 ; i < nUpdatedRects ; i++ )
			{
				const WgRect * pR = pRoot->FirstUpdatedRect() + i;

				updatedRects[i].x = pR->x;
				updatedRects[i].y = pR->y;
				updatedRects[i].w = pR->w;
				updatedRects[i].h = pR->h;
			}
		}
		else
		{
			nUpdatedRects = 1;

			const WgRect r = pRoot->Geo();

			updatedRects[0].x = r.x;
			updatedRects[0].y = r.y;
			updatedRects[0].w = r.w;
			updatedRects[0].h = r.h;
		}

		SDL_UpdateWindowSurfaceRects(pWin, updatedRects, nUpdatedRects);

 
//		SDL_GL_SwapBuffers();
        // Pause for a while

        SDL_Delay(50);
			 
		counter++;

    } // end main loop


	// Exit WonderGUI

	delete pRoot;
	delete pGfxDevice;

	WgBase::Exit();

	IMG_Quit();

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}

//____ updateOscilloscope()_____________________________________________________

void updateOscilloscope( WgOscilloscope * pOsc, int ofs, float freq, float amplitude )
{
		float points[256];

		for( int i = 0 ; i < 256 ; i++ )
		{
			points[i] = (float) sin((i + ofs)*freq)*amplitude;
		}

		pOsc->SetLinePoints(256,points);
}


void resizeButtonClicked( const WgEvent::Event * pEvent)
{
	WgWidget * p = pEvent->Widget();

	int scale = p->Id();
	
	s_pScaleSizer->SetPreferredSize( s_originalSize*scale / WG_SCALE_BASE );	
	s_pScaleSizer->SetScale(scale);
}

void incButtonPressed( const WgEvent::Event * pEvent)
{
	int scale = s_pScaleSizer->Scale();
	scale += WG_SCALE_BASE >> 6;
	
	s_pScaleSizer->SetPreferredSize( s_originalSize*scale / WG_SCALE_BASE );	
	s_pScaleSizer->SetScale(scale);
}

void decButtonPressed( const WgEvent::Event * pEvent)
{
	int scale = s_pScaleSizer->Scale();
	scale -= WG_SCALE_BASE >> 6;

	if( scale == 0  )
		scale = WG_SCALE_BASE >> 6;
	
	s_pScaleSizer->SetPreferredSize( s_originalSize*scale / WG_SCALE_BASE );	
	s_pScaleSizer->SetScale(scale);
}


//____ setupGUI() ______________________________________________________________

WgRootPanel * setupGUI( WgGfxDevice * pDevice )
{
	WgResDB * pDB = sdl_wglib::LoadStdWidgets( "../resources/blocks.png", WgSurfaceFactorySoft() );
	if( !pDB )
		return 0;

	WgRootPanel * pRoot = new WgRootPanel( pDevice );

	WgEventHandler * pEventHandler = pRoot->EventHandler();
		
	WgEventLogger * pEventLogger = new WgEventLogger( std::cout );
	pEventLogger->IgnoreEvent( WG_EVENT_MOUSE_POSITION );
	pEventLogger->IgnoreEvent( WG_EVENT_MOUSEBUTTON_REPEAT );
	pEventLogger->IgnoreEvent( WG_EVENT_BUTTON_PRESS );
//	pEventLogger->IgnoreAllEvents();
//	pEventLogger->LogMouseButtonEvents();
	pEventHandler->AddCallback( pEventLogger );


	// Load images and specify blocks

	WgSurface * pBackImg = sdl_wglib::LoadSurface("../resources/What-Goes-Up-3.bmp", WgSurfaceFactorySoft() );
	WgBlocksetPtr pBackBlock = WgBlockset::CreateFromSurface(pBackImg, WG_TILE_ALL );

	WgSurface * pFlagImg = sdl_wglib::LoadSurface("cb2.bmp", WgSurfaceFactorySoft() );
	WgBlocksetPtr pFlagBlock = WgBlockset::CreateFromSurface( pFlagImg );

	WgSurface * pSplashImg = sdl_wglib::LoadSurface("../resources/splash.png", WgSurfaceFactorySoft() );
	WgBlocksetPtr pSplashBlock = WgBlockset::CreateFromSurface( pSplashImg );

	WgSurface * pBigImg = sdl_wglib::LoadSurface("../resources/frog.jpg", WgSurfaceFactorySoft() );
	WgBlocksetPtr pBigBlock = WgBlockset::CreateFromSurface( pBigImg );

	// MenuPanel

	WgMenuLayer * pMenuLayer = new WgMenuLayer();
	pRoot->SetChild( pMenuLayer );

	// Base Flex

	WgFlexPanel * pBase = new WgFlexPanel();
	pMenuLayer->SetBase( pBase );
	pBase->SetSkin( WgColorSkin::Create( WgColor::black ) );

	const int sidepanelWidth = 420;

	// Add side panel
	
	WgPackPanel * pSidePanel = new WgPackPanel();

	pSidePanel->SetSkin( WgColorSkin::Create(WgColor::wheat) );
	pSidePanel->SetOrientation( WgOrientation::WG_VERTICAL );

	WgFlexHook * pHook = pBase->AddChild( pSidePanel );
	pHook->SetAnchored( WG_NORTHWEST, pBase->AddAnchor(0.f,1.f, WgCoord(sidepanelWidth,0)) );


	// Add Button panel to SidePanel
	
	{
		WgPackPanel * pButtons = new WgPackPanel();
		pButtons->SetOrientation( WgOrientation::WG_HORIZONTAL );

		WgButton * p100 = (WgButton*) pDB->CloneWidget("button");
		p100->SetText( "100%");
		p100->SetId( WG_SCALE_BASE );
		pButtons->AddChild(p100)->SetPadding(2);

		WgButton * p150 = (WgButton*) pDB->CloneWidget("button");
		p150->SetText( "150%");
		p150->SetId( (int) WG_SCALE_BASE*1.5f );
		pButtons->AddChild(p150)->SetPadding(2);
		
		WgButton * p200 = (WgButton*) pDB->CloneWidget("button");
		p200->SetText( "200%");
		p200->SetId( WG_SCALE_BASE*2 );
		pButtons->AddChild(p200)->SetPadding(2);

		WgButton * p250 = (WgButton*) pDB->CloneWidget("button");
		p250->SetText( "250%");
		p250->SetId( (int) WG_SCALE_BASE*2.5f );
		pButtons->AddChild(p250)->SetPadding(2);
		
		WgButton * p300 = (WgButton*) pDB->CloneWidget("button");
		p300->SetText( "300%");
		p300->SetId( (int) WG_SCALE_BASE*3.f );
		pButtons->AddChild(p300)->SetPadding(2);

		WgButton * p400 = (WgButton*) pDB->CloneWidget("button");
		p400->SetText( "400%");
		p400->SetId( (int) WG_SCALE_BASE*4.f );
		pButtons->AddChild(p400)->SetPadding(2);

		WgButton * pInc = (WgButton*) pDB->CloneWidget("button");
		pInc->SetText( "+");
		pButtons->AddChild(pInc)->SetPadding(2);

		WgButton * pDec = (WgButton*) pDB->CloneWidget("button");
		pDec->SetText( "-");
		pButtons->AddChild(pDec)->SetPadding(2);


		pSidePanel->AddChild( pButtons );
	

		pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(p100,1), resizeButtonClicked );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(p150,1), resizeButtonClicked );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(p200,1), resizeButtonClicked );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(p250,1), resizeButtonClicked );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(p300,1), resizeButtonClicked );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(p400,1), resizeButtonClicked );

		pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pInc,1), incButtonPressed );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pDec,1), decButtonPressed );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonRepeat(pInc,1), incButtonPressed );
		pEventHandler->AddCallback( WgEventFilter::MouseButtonRepeat(pDec,1), decButtonPressed );
	
	}

	// Add original  to SidePanel
	
	{
		WgPackPanel * pRow = new WgPackPanel();
		

		WgStackPanel * pBg = new WgStackPanel();
		pBg->SetSkin( WgBoxSkin::Create( WgColor::white, WgBorders(1), WgColor::black) );

		WgSizeCapsule * pSizer = new WgSizeCapsule();
		
		pSizer->SetPreferredSize( s_originalSize );
		pSizer->SetMaxSize( s_originalSize );
		pSizer->SetChild( generateScalingUI( pDB ) );
		

		pBg->AddChild( pSizer )->SetPadding( WgBorders(1) );
				
		pRow->AddChild(pBg);
				
		pSidePanel->AddChild( pRow )->SetPadding( WgBorders(2) );
	}
	

	// Add scaled copy 
	
	{
		WgScrollPanel * pView = (WgScrollPanel*) pDB->CloneWidget( "view" );

		WgSizeCapsule * pSizer = new WgSizeCapsule();
		
		pSizer->SetPreferredSize( s_originalSize*2 );
		pSizer->SetChild( generateScalingUI( pDB ) );
		pSizer->SetScale( WG_SCALE_BASE*2 );

		s_pScaleSizer = pSizer;

		pView->SetContent( pSizer );
		
		int anchor = pBase->AddAnchor( 0,0, WgCoord( sidepanelWidth, 0) );		
		pBase->AddChild( pView, anchor, WG_SOUTHEAST );
	}


	return pRoot;
}


//____ generateScalingUI() ____________________________________________________________

WgWidget * generateScalingUI( WgResDB * pDB )
{
	WgSurface * pFrogImg = sdl_wglib::LoadSurface("../resources/frog.jpg", WgSurfaceFactorySoft() );
	WgBlocksetPtr pFrogBlock = WgBlockset::CreateFromSurface( pFrogImg );

	WgSurface * pSplashImg = sdl_wglib::LoadSurface("../resources/splash.png", WgSurfaceFactorySoft() );
	WgBlocksetPtr pSplashBlock = WgBlockset::CreateFromSurface( pSplashImg );


	WgFlexPanel * pBack = new WgFlexPanel();
	
	pBack->SetSkin( WgColorSkin::Create(WgColor::blue));

	
	WgImage * pImg1 = new WgImage();
	pImg1->SetSource( pFrogBlock );
	pBack->AddChild( pImg1, WgRect(10,10,100,100) );

	WgImage * pImg2 = new WgImage();
	pImg2->SetSource( pSplashBlock );
	pBack->AddChild( pImg2, WgCoord(100,100) );

	WgButton * pBtn1 = (WgButton*) pDB->CloneWidget( "button" );
	pBtn1->SetText( "Button Label" );
	pBack->AddChild( pBtn1, WgCoord(5,150) );

	WgButton * pBtn2 = (WgButton*) pDB->CloneWidget( "button" );
	pBtn2->SetText( "Button Label" );
	pBack->AddChild( pBtn2, WgRect(5,200,100,50) );


	return pBack;
}




//____ eventLoop() ____________________________________________________________

bool eventLoop( WgEventHandler * pHandler )
{
	sdl_wglib::BeginEvents( pHandler );

   // message processing loop
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// check for messages
		switch (event.type)
		{
			// exit if the window is closed
			case SDL_QUIT:
				return false;

			// check for keypresses
			case SDL_KEYDOWN:
			{
				// exit if ESCAPE is pressed
//				if (event.key.keysym.sym == SDLK_ESCAPE)
//					return false;
			}
		}
		sdl_wglib::TranslateEvent( event );
	}

	sdl_wglib::EndEvents();

	return true;
}



WgCoord dragStartPos;

//____ cbInitDrag() ___________________________________________________________

void cbInitDrag( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
	WgFlexHook * pHook = static_cast<WgFlexHook*>(pWidget->Hook());


	dragStartPos = pHook->FloatOfs();
	printf( "DRAG START!\n" );
}

//____ cbDragWidget() __________________________________________________________

void cbDragWidget( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
	if( _pEvent->Type() != WG_EVENT_MOUSEBUTTON_DRAG || !pWidget->Parent() )
		return;

	const WgEvent::MouseButtonDrag* pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

	WgCoord	dragDistance = pEvent->DraggedTotal();

	WgCoord	ofs = dragStartPos + dragDistance;

//	printf( "AccDistance: %d, %d\n", dragDistance.x, dragDistance.y );
	printf( "ofs: %d, %d   start: %d %d   distance: %d, %d\n", ofs.x, ofs.y, dragStartPos.x, dragStartPos.y, dragDistance.x, dragDistance.y );

	WgFlexHook * pHook = static_cast<WgFlexHook*>(pWidget->Hook());
	pHook->SetOfs(dragStartPos+dragDistance);
}

//____ cbOpenModal() __________________________________________________________

void cbOpenModal( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
	g_pModal->AddModal( pWidget, WgCoord(), WG_SOUTHEAST );
}

//____ cbCloseModal() __________________________________________________________

void cbCloseModal( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
	g_pModal->ReleaseChild(pWidget);
}

//____ cbResizeWidget() _________________________________________________________

void cbResize( const WgEvent::Event* _pEvent, void * _pFlexHook )
{
	WgFlexHook * pHook = static_cast<WgFlexHook*>(_pFlexHook);
	const WgEvent::MouseButtonDrag* pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

	WgCoord dragged = pEvent->DraggedNow();

	pHook->SetSize( pHook->Size() + WgSize(dragged.x,dragged.y) );
}



//____ addResizablePanel() _________________________________________________

void addResizablePanel( WgFlexPanel * pParent, WgWidget * pChild, WgEventHandler * pEventHandler )
{
	WgHook * pHook = pParent->AddChild( pChild );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonDrag(pChild, 3), cbResize, pHook );


	pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pChild, 1), cbInitDrag, pChild );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonDrag(pChild, 1), cbDragWidget, pChild );
}




