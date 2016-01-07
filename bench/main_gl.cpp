#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef WIN32
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#endif


#include <wondergui.h>
#include <wg_bitmapglyphs.h>
#include <wg_vectorglyphs.h>
#include <wg_knob.h>
#include <iostream>

#include <wg_surface_soft.h>
#include <wg_surface_gl.h>
#include <wg_gfxdevice_soft.h>
#include <wg_gfxdevice_gl.h>
#include <sdl_wglib.h>
#include <wg_boxskin.h>

#include "testwidget.h"

void DBG_ASSERT(bool x) {}


extern std::ostream cout;

SDL_Surface *	initSDL( int w, int h );
bool			eventLoop( WgEventHandler * pHandler );
WgRootPanel * 		setupGUI( WgGfxDevice * pDevice );



WgFlexPanel * createPresetSelector();


WgModalLayer * g_pModal = 0;


WgValueDisplay * m_pCounter = 0;
WgValueDisplay * m_pCounter2 = 0;


//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
		
	// Init SDL

	SDL_Surface * pScreen = initSDL(640,480);
	if(!pScreen )
		return 1;

	IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );

	// Init WonderGUI

	WgBase::Init();
	sdl_wglib::MapKeys();


//	WgBase::InitFreeType();

	// Setup gfxdevice and gui

	WgGfxDeviceGL * pGfxDevice = new WgGfxDeviceGL( WgSize(640,480) );


	// Load TTF-font
/*
	WgVectorGlyphs::SetSurfaceFactory( new WgSurfaceFactorySDL() );

	char	ttfname[] = { "a.ttf" };

	int size = fileSize( ttfname );
	char * pFontFile = (char*) loadFile( ttfname );
	WgVectorGlyphs * pGlyphs = new WgVectorGlyphs( pFontFile , size, 0 );

	WgFont * pFont = new WgFont();
	pFont->SetDefaultVectorGlyphs( pGlyphs );
*/
	// Load bitmap font

	WgFont * pFont = sdl_wglib::LoadBitmapFont( "../resources/anuvverbubbla_8x8.png", "../resources/anuvverbubbla_8x8.fnt", WgSurfaceFactoryGL() );

	// Load and setup cursor

	WgSurface * pCursorImg = sdl_wglib::LoadSurface("../resources/cursors.png", WgSurfaceFactoryGL() );

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
	prop.SetColor( WgColor::white );
	prop.SetSize(8);

	WgBase::SetDefaultTextprop( prop.Register() );


	WgRootPanel * pRoot = setupGUI( pGfxDevice );

	pRoot->FindWidget( WgCoord(10,10), WG_SEARCH_ACTION_TARGET );

   // program main loop

	int moveCounter = 0;

    while (eventLoop( pRoot->EventHandler() ))
    {
		if( m_pCounter )
			m_pCounter->IncValue();
		
		if( m_pCounter2 )
			m_pCounter2->IncValue();
		
		// DRAWING STARTS HERE

		pRoot->AddDirtyPatch( pRoot->Geo().Size() );

		pRoot->Render();

        // DRAWING ENDS HERE

        // finally, update the screen :)

 
		SDL_GL_SwapBuffers();

        // Pause for a while

        SDL_Delay(10);
					 

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


//____ setupGUI() ______________________________________________________________

WgRootPanel * setupGUI( WgGfxDevice * pDevice )
{
	WgResDB * pDB = sdl_wglib::LoadStdWidgets( "../resources/blocks.png", WgSurfaceFactoryGL() );
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

	WgSurface * pBackImg = sdl_wglib::LoadSurface("../resources/What-Goes-Up-3.bmp", WgSurfaceFactoryGL() );
	WgBlocksetPtr pBackBlock = WgBlockset::CreateFromSurface(pBackImg, WG_TILE_ALL );

	WgSurface * pFlagImg = sdl_wglib::LoadSurface("cb2.bmp", WgSurfaceFactoryGL() );
	WgBlocksetPtr pFlagBlock = WgBlockset::CreateFromSurface( pFlagImg );

	WgSurface * pSplashImg = sdl_wglib::LoadSurface("../resources/splash.png", WgSurfaceFactoryGL() );
	WgBlocksetPtr pSplashBlock = WgBlockset::CreateFromSurface( pSplashImg );

	WgSurface * pBigImg = sdl_wglib::LoadSurface("../resources/frog.jpg", WgSurfaceFactoryGL() );
	WgBlocksetPtr pBigBlock = WgBlockset::CreateFromSurface( pBigImg );


	// Bottom Flex

	WgFlexPanel * pBottom = new WgFlexPanel();
	pRoot->SetChild( pBottom );
	pBottom->SetSkin( WgColorSkin::Create( WgColor::black ) );

	// Background

	WgImage * pBackground = new WgImage();
	pBackground->SetSource( pBackBlock );

	WgFlexHook * pHook = pBottom->AddChild( pBackground );
	pHook->SetAnchored( WG_NORTHWEST, WG_SOUTHEAST );


	// Test Widget
	
	TestWidget * pTest = new TestWidget();
	pBottom->AddChild( pTest, WG_NORTHWEST, WG_SOUTHEAST, WgBorders(10) );



	// Test oscilloscope
/*
	{
		WgSurface * pImg = sdl_wglib::LoadSurface("../resources/blocks.png", WgSurfaceFactorySoft() );

		WgBlocksetPtr pMarkerBlock = WgBlockset::CreateFromRect( pImg, WgRect(1,120,8,8) );

		WgOscilloscope * pOsc = new WgOscilloscope();

		pOsc->SetBackground( WgColorSkin::Create( WgColor(0,0,96) ) );

		float grid[] = {-1.f,-0.5f,0.f,0.5f,1.f};
		pOsc->SetHGridLines( 5, grid );
		pOsc->SetVGridLines( 5, grid );
		pOsc->SetGridColor( WgColor(64,64,64) );

		pOsc->SetMarkerGfx( pMarkerBlock );
		pOsc->AddMarker( 30, 0.f );


		float points[256];

		for( int i = 0 ; i < 256 ; i++ )
		{
			points[i] = (float) sin(i/25.f)*0.90f;
		}

		pOsc->SetLinePoints(256,points);


		addResizablePanel( pFlex, pOsc, pEventHandler );
	}
*/
	return pRoot;
}

//____ initSDL() ______________________________________________________________

SDL_Surface * initSDL( int w, int h )
{
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 0;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

 
	// OpenGL preparations

   const SDL_VideoInfo* info = SDL_GetVideoInfo( );
    int bpp = info->vfmt->BitsPerPixel;
 
//    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
//    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
//    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
//    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );



    // create a new window
    SDL_Surface* pScreen = SDL_SetVideoMode(w, h, bpp, SDL_OPENGL);
    if ( !pScreen )
    {
        printf("Unable to set %dx%d video: %s\n", w, h, SDL_GetError());
        return 0;
    }

	SDL_EnableUNICODE(true);

	return pScreen;
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



