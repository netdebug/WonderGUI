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
#include <wg_glsurface.h>
#include <wg_gfxdevice_soft.h>
#include <wg_glgfxdevice.h>
#include <wg_surfacefactory_soft.h>
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


WgFlexPanel * createPresetSelector();
void updateOscilloscope( WgOscilloscope * pOsc, int ofs, float width, float amplitude );


WgModalLayer * g_pModal = 0;

WgWidget * pWidgetToMove = 0;

WgValueDisplay * m_pCounter = 0;
WgValueDisplay * m_pCounter2 = 0;

WgVolumeMeter * m_pVolMeter = 0;
WgSimpleVolumeMeter * m_pSimpleVolMeter1 = 0;
WgOscilloscope * g_pOsc = 0;

float	leftPeak = 1.f, rightPeak = 0.5f, leftHold = 0.5f, rightHold = 0.5f;

bool	leftUp = true, rightUp = false;


//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{

	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 320, height = 240;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0);

	SDL_Surface * pScreen = SDL_GetWindowSurface(pWin);

	IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );

	// Init WonderGUI

	WgBase::Init();
	sdl_wglib::MapKeys();

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
//	pGfxDevice->SetBilinearFiltering( true );



//	WgGfxDeviceGL * pGfxDevice = new WgGfxDeviceGL( WgSize(640,480) );


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

	WgFont * pFont = sdl_wglib::LoadBitmapFont( "../resources/anuvverbubbla_8x8.png", "../resources/anuvverbubbla_8x8.fnt", WgSurfaceFactorySoft() );

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
	prop.SetColor( WgColor::white );
	prop.SetSize(8);

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

	// Bottom Flex

	WgFlexPanel * pBottom = new WgFlexPanel();
	pMenuLayer->SetBase( pBottom );
	pBottom->SetSkin( WgColorSkin::Create( WgColor::black ) );

	// Main Flex

	WgFlexPanel * pFlex = new WgFlexPanel();
	pBottom->AddChild( pFlex, WG_NORTHWEST, WG_SOUTHEAST, WgBorders(10) );

	// Background

	WgImage * pBackground = new WgImage();
	pBackground->SetSource( pBackBlock );

	WgFlexHook * pHook = pFlex->AddChild( pBackground );
	pHook->SetAnchored( WG_NORTHWEST, WG_SOUTHEAST );


	// LED Volume meter
/*	
	WgVolumeMeter * pMeter1 = new WgVolumeMeter();
	pHook = pFlex->AddChild( pMeter1,WgRect(10,10,40,100) );

	pMeter1->SetValue( 0.7f, 1.0f );
	pMeter1->SetDirection( WG_RIGHT );
	m_pVolMeter = pMeter1;
*/

	// Simple Volume meter
/*	
	WgSimpleVolumeMeter * pMeter2 = new WgSimpleVolumeMeter();
	pHook = pFlex->AddChild( pMeter2,WgRect(10,10,40,100) );
	pMeter2->SetValue( 0.5f, 1.0f, 0.3f, 0.9f );
	m_pSimpleVolMeter1 = pMeter2;

	pMeter2->SetColors( WgColor(0,255,0,128), WgColor(255,255,0,128), WgColor(255,0,0,128));
	pMeter2->SetSections( 0.4f, 0.3f );
*/
	// Simple movable rectangle
/*	
	WgFiller * pRect = new WgFiller();
	pRect->SetColors( WgColorset::Create( WgColor::coral ));
	
	pHook = pFlex->AddChild( pRect, WgRect(10,10,50,50) );
	
	pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pRect, 1), cbInitDrag, pRect );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonDrag(pRect, 1), cbDragWidget, pRect );
*/

	// Test widget
/*
	TestWidget * pTest = new TestWidget();
	pHook = pFlex->AddChild( pTest );
	pTest->Start();
*/


	


	// Extra flex with background
/*	
	{
	
		WgFlexPanel * pExtraFlex = new WgFlexPanel();
		pExtraFlex->SetSkin( WgColorSkin::Create( WgColor(255,0,0,255)));
		
		pFlex->AddChild( pExtraFlex, WgRect(20,20,100,100) );
		
		WgImage * pPlate = (WgImage*) pDB->CloneWidget( "plate" );
		pExtraFlex->AddChild( pPlate, WG_NORTHWEST, WG_SOUTHEAST, WgBorders(30) );
		
	}
*/
	// Test transparency issue

	
	{

/*		
		m_pCounter = new WgValueDisplay();
		m_pCounter->SetValue(123);
		m_pCounter->SetRange(0, 100000);
		pFlex->AddChild( m_pCounter );
		m_pCounter = 0;
		m_pCounter2 = 0;
*/

/*
		WgFlexPanel * pExtraFlex = new WgFlexPanel();
		pExtraFlex->SetSkin( WgColorSkin::Create( WgColor(0,0,0,128)));


		WgFlexPanel * pSel = createPresetSelector();
		pFlex->AddChild(pSel);



//		pFlex->AddChild( pExtraFlex );

		WgTextDisplay * pText = new WgTextDisplay();
		pText->SetText( "THIS IS SOME TEST TEXT" );
		
				
		pExtraFlex->AddChild( pText, WgRect( 10,10,100,100) );

		addResizablePanel( pFlex, pExtraFlex, pEventHandler );

		((WgFlexHook*)pExtraFlex->Hook())->SetSize( WgSize(200,150));
		((WgFlexHook*)pExtraFlex->Hook())->SetOfs( WgCoord(50,50));


		pWidgetToMove = pExtraFlex;
*/
	}






/*
	{
		WgShaderCapsule * pShader = new WgShaderCapsule();
		pShader->SetBlendMode(WG_BLENDMODE_MULTIPLY);
		pShader->SetColor( WgColor(0xFF0000FF) );

		WgStackPanel * pStack = new WgStackPanel();
		pShader->SetChild( pStack );

		WgImage * pBg = (WgImage*) pDB->CloneWidget( "plate" );
		pStack->AddChild( pBg );

		WgShaderCapsule * pShader2 = new WgShaderCapsule();
		pShader2->SetBlendMode(WG_BLENDMODE_ADD);
		pShader2->SetColor( WgColor(0xFFFFFFFF) );
		WgStackHook * pHook = pStack->AddChild( pShader2 );
		pHook->SetSizePolicy( WgStackHook::SCALE );
		pHook->SetOrigo( WG_CENTER );
		pHook->SetPadding( WgBorders(2) );


		WgImage * pSplash= new WgImage();
		pSplash->SetSource( pSplashBlock );
		pShader2->SetChild( pSplash );

		addResizablePanel( pFlex, pShader, pEventHandler );
	}
*/

/*
	{
		WgStackPanel * pStack = new WgStackPanel();

		WgImage * pBg = (WgImage*) pDB->CloneWidget( "plate" );
		pStack->AddChild( pBg );

		WgListPanel * pVBox = new WgListPanel();
		WgStackHook * pHook = pStack->AddChild( pVBox );
		pHook->SetPadding( WgBorders(4) );

		for( int i = 0 ; i < 10 ; i++ )
		{
			WgValueDisplay * pV = new WgValueDisplay();
			pV->SetValue( 1000 + i );
			pVBox->AddChild( pV );
		}
		

//		WgValueEditor * pValue = new WgValueEditor();
//		pVBox->AddChild( pValue );

//		WgWidgetSlider * pSlider = (WgWidgetSlider*) pDB->CloneWidget( "hslider" );
//		pSlider->SetSliderTarget(pValue);
//		pVBox->AddChild( pSlider );

		addResizablePanel( pFlex, pStack, pEventHandler );

//		pValue->SetValue( 100 );
//		pValue->GrabFocus();
	}
*/
/*
	{
		WgKnob * pKnob = new WgKnob();
		addResizablePanel( pFlex, pKnob, pEventHandler );
	}
*/
	// Test menus and MenuPanel.
/*
	{
		WgMenu * pSubMenu1_1 = (WgMenu*) pDB->CloneWidget( "menu" );
		pSubMenu1_1->AddItem( new WgMenuEntry( WgString("ENTRY 11"), WgString("Help text for entry 11"), WgBlocksetPtr(), 0 ));
		pSubMenu1_1->AddItem( new WgMenuEntry( WgString("ENTRY 12"), WgString("Help text for entry 12"), WgBlocksetPtr(), 0 ));
		
		WgMenu * pSubMenu1 = (WgMenu*) pDB->CloneWidget( "menu" );
		pSubMenu1->AddItem( new WgMenuEntry( WgString("ENTRY 7"), WgString("Help text for entry 7"), WgBlocksetPtr(), 0 ));
		pSubMenu1->AddItem( new WgMenuEntry( WgString("ENTRY 8"), WgString("Help text for entry 8"), WgBlocksetPtr(), 0 ));
		pSubMenu1->AddItem( new WgMenuSeparator() );
		pSubMenu1->AddItem( new WgMenuSubMenu( WgString("SUBMENU 1_1"), WgString("Help text for submenu 1_1"), WgBlocksetPtr(), 0, pSubMenu1_1 ) );

		WgMenu * pSubMenu2 = (WgMenu*) pDB->CloneWidget( "menu" );
		pSubMenu2->AddItem( new WgMenuEntry( WgString("ENTRY 9"), WgString("Help text for entry 9"), WgBlocksetPtr(), 0 ));
		pSubMenu2->AddItem( new WgMenuSeparator() );
		pSubMenu2->AddItem( new WgMenuEntry( WgString("ENTRY 10"), WgString("Help text for entry 10"), WgBlocksetPtr(), 0 ));

		WgMenu * pMenu = (WgMenu*) pDB->CloneWidget( "menu" );
		pMenu->AddItem( new WgMenuEntry( WgString("ENTRY 1"), WgString("Help text for entry 1"), WgBlocksetPtr(), 0 ));
		pMenu->AddItem( new WgMenuSeparator() );
		pMenu->AddItem( new WgMenuEntry( WgString("ENTRY 2"), WgString("Help text for entry 2"), WgBlocksetPtr(), 0 ));
		pMenu->AddItem( new WgMenuEntry( WgString("ENTRY 3"), WgString("Help text for entry 3"), WgBlocksetPtr(), 0 ));
		pMenu->AddItem( new WgMenuEntry( WgString("ENTRY 4"), WgString("Help text for entry 4"), WgBlocksetPtr(), 0 ));
		pMenu->AddItem( new WgMenuEntry( WgString("ENTRY 5"), WgString("Help text for entry 5"), WgBlocksetPtr(), 0 ));
		pMenu->AddItem( new WgMenuEntry( WgString("ENTRY 6"), WgString("Help text for entry 6"), WgBlocksetPtr(), 0 ));

		pMenu->AddItem( new WgMenuSubMenu( WgString("SUBMENU 1"), WgString("Help text for submenu 1"), WgBlocksetPtr(), 0, pSubMenu1 ) );
		pMenu->AddItem( new WgMenuSubMenu( WgString("SUBMENU 2"), WgString("Help text for submenu 2"), WgBlocksetPtr(), 0, pSubMenu2 ) );

		addResizablePanel( pFlex, pMenu, pEventHandler );
		pMenu->GrabFocus();

//		pMenuPanel->OpenMenu( pMenu, WgRect(10,10,100,10), WG_SOUTHWEST );
	}
*/
	// Test menubar and menus
/*
	{
		WgMenubar * pMenubar = WgCast<WgMenubar>(pDB->CloneWidget("menubar"));

		WgMenu * pFileMenu = (WgMenu*) pDB->CloneWidget( "menu" );
		pFileMenu->AddItem( new WgMenuEntry( WgString("Open..."), WgString("Open a file"), WgBlocksetPtr(), 0 ));
		pFileMenu->AddItem( new WgMenuEntry( WgString("Save"), WgString("Save the file"), WgBlocksetPtr(), 0 ));
		pFileMenu->AddItem( new WgMenuSeparator() );
		pFileMenu->AddItem( new WgMenuEntry( WgString("Quit"), WgString("Quit this program"), WgBlocksetPtr(), 0 ));
		pMenubar->AddMenu( "FILE", pFileMenu );
	
		WgMenu * pHelpMenu = (WgMenu*) pDB->CloneWidget( "menu" );
		pHelpMenu->AddItem( new WgMenuEntry( WgString("Help"), WgString("Get some help"), WgBlocksetPtr(), 0 ));
		pHelpMenu->AddItem( new WgMenuSeparator() );
		pHelpMenu->AddItem( new WgMenuEntry( WgString("About"), WgString("About this program"), WgBlocksetPtr(), 0 ));
		pMenubar->AddMenu( "HELP", pHelpMenu );

		addResizablePanel( pFlex, pMenubar, pEventHandler );
	}
*/

	// Test oscilloscope

	{
		WgSurface * pImg = sdl_wglib::LoadSurface("../resources/blocks.png", WgSurfaceFactorySoft() );

		WgBlocksetPtr pMarkerBlock = WgBlockset::CreateFromRect( pImg, WgRect(1,120,8,8) );

		WgOscilloscope * pOsc = new WgOscilloscope();

		pOsc->SetBackground( WgColorSkin::Create( WgColor(0,0,96) ) );

		float grid[] = {-1.f,-0.5f,0.f,0.5f,1.f};
		pOsc->SetHGridLines( 5, grid );
		pOsc->SetVGridLines( 5, grid );
		pOsc->SetGridColor( WgColor(64,64,64) );
		pOsc->SetRenderSegments( 20 );

//		pOsc->SetMarkerGfx( pMarkerBlock );
//		pOsc->AddMarker( 30, 0.f );

		addResizablePanel( pFlex, pOsc, pEventHandler );
		
		g_pOsc = pOsc;
	}

	// Test combobox
/*
	{
		WgCombobox * pCombobox = WgCast<WgCombobox>(pDB->CloneWidget("combobox"));

		WgMenu * pComboMenu = (WgMenu*) pDB->CloneWidget( "menu" );
		pComboMenu->AddItem( new WgMenuEntry( 1, WgString("Alfa"), WgString("First"), WgBlocksetPtr(), 0 ) );
		pComboMenu->AddItem( new WgMenuEntry( 2, WgString("Beta"), WgString("Second"), WgBlocksetPtr(), 0 ) );
		pCombobox->SetMenu( pComboMenu );

		addResizablePanel( pFlex, pCombobox, pEventHandler );

	}
*/

	// Test view

	{
		WgScrollPanel * pView = (WgScrollPanel*) pDB->CloneWidget( "view" );

		WgImage * pImage = new WgImage();
		pImage->SetSource( pBigBlock );
		pView->SetContent( pImage );
		addResizablePanel( pFlex, pView, pEventHandler );
	}


	// Test overlapping non-visible
/*
	{
		WgFiller * pCovered = new WgFiller();
		pCovered->SetColors( WgColorset::Create( WgColor::aliceblue ) );
		pFlex->AddChild( pCovered, WgRect( 10,10,20,20 ) );

		g_pCovered = pCovered;

		WgFiller * pCovering = new WgFiller();
		pCovering->SetColors( WgColorset::Create( WgColor::wheat ) );
		WgFlexHook * pHook = pFlex->AddChild( pCovering, WgRect( 0,0,50,50 ) );
		pHook->SetVisible(false);
	}
*/

	// Modal panel
/*
	g_pModal = new WgWidgetModalLayer();
	g_pModal->SetBase( pFlex );

	pRoot->SetChild(g_pModal);


	// Modal button

	WgButton * pModalButton = (WgButton*) pDB->CloneWidget( "button" );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(pModalButton, 1), cbCloseModal, pModalButton );

	//

	WgButton * pButton = (WgButton*) pDB->CloneWidget( "button" );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pButton, 1), cbOpenModal, pModalButton );

	pHook = pFlex->AddChild( pButton, WgRect(0,0,100,100), WG_NORTHWEST );

	//

	WgImage * pFlag1= new WgImage();
	pFlag1->SetSource( pSplashBlock );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pFlag1, 1), cbInitDrag, pFlag1 );

	pHook = pFlex->AddChild( pFlag1, WgCoord(0,0), WG_CENTER );



	WgImage * pFlag2= new WgImage();
	pFlag2->SetSource( pFlagBlock );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonDrag(pFlag1, 1), cbDragWidget, pFlag1 );

	pHook = pFlex->AddChild( pFlag2, WgCoord(100,100), WG_CENTER );

	//


	WgListPanel * pVBox = new WgListPanel();
//	pFlex->AddChild( pVBox, WgCoord(50,50), WG_NORTHWEST );


	WgImage * pFlag3 = new WgImage();
	pFlag3->SetSource( pFlagBlock );
	WgImage * pFlag4= new WgImage();
	pFlag4->SetSource( pFlagBlock );

	WgButton * pButton2 = (WgButton*) pDB->CloneWidget( "button" );
	pButton2->SetText( "BUTTON TEXT" );

	pVBox->AddChild(pButton2);

	pVBox->AddChild(pFlag3);
//	pVBox->AddChild(pFlag4);

	pHook = pFlex->AddChild( pVBox, WgCoord(50,50), WG_NORTHWEST );
	pHook->SetMaxSize( WgSize(120, INT_MAX) );

	//

	WgWidgetTabOrder * pTabOrder = new WgWidgetTabOrder();
	pVBox->AddChild(pTabOrder);

	WgListPanel * pTabBox = new WgListPanel();
	pTabOrder->SetChild(pTabBox);
//	pVBox->AddChild(pTabBox);

	WgTextDisplay * pText1 = new WgTextDisplay();
	pText1->SetText("TEXTA1");
	pText1->SetEditMode(WG_TEXT_EDITABLE);
	pTabBox->AddChild(pText1);

	WgTextDisplay * pText2 = new WgTextDisplay();
	pText2->SetText("TEXTB234ABC sajfas kjfaljsras kjasdfkasd kajfd fkajfa fkdjfa dfasfda asdkfj");
	pText2->SetEditMode(WG_TEXT_EDITABLE);
	pTabBox->AddChild(pText2);

	pText1->GrabFocus();

//	pTabOrder->AddToTabOrder(pText1);
//	pTabOrder->AddToTabOrder(pText2);

	// Radiobuttons test

	pVBox->AddChild( pDB->CloneWidget( "radiobutton" ) );
	pVBox->AddChild( pDB->CloneWidget( "radiobutton" ) );

	pFlex->AddChild( pDB->CloneWidget( "radiobutton" ) );
	pFlex->AddChild( pDB->CloneWidget( "radiobutton" ) );

	pVBox->SetRadioGroup(true);
*/
	return pRoot;
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




WgFlexPanel * createPresetSelector()
{
	WgFlexPanel * pBottom = new WgFlexPanel();
	
    WgBoxSkinPtr pSkin = WgBoxSkin::Create(WgColor(0, 0, 0, 255), WgBorders(0), WgColor(0, 0, 0, 255));

	pBottom->SetSkin(pSkin);
/*
		WgButton * pButton = new WgButton();
		pButton->SetText( "BUTTON" );
		pBottom->AddChild( pButton, WgRect(4,75,100,15) );
*/

    for(int i = 0; i< 10; i++)
    {	
		WgButton * pButton = new WgButton();
		pButton->SetText( "BUTTON" );
		pBottom->AddChild( pButton, WgRect(4,i*15,100,15) );
    }
	
	return pBottom;
}


/*

SectionPresetSelector::SectionPresetSelector(float fCenterX, float fTopY, float fScale, CSoftubeGUIModule pkModule, WgEventHandler pHandler, WgTextpropPtr pWgProp, WgTextManager pTextManager, WgFlexPanel pkRootPanel, int iParam, std::vector<SectionPreset> pPresets) : WgFlexPanel(),
m_iParam(iParam),
m_pPresets(pPresets),
m_bIsVisible(false)
{
    //Coordinates and sizes are in 1150 px Width to match the rest of the GUI placement
    int iRows = pPresets.size();
    
    float fHalfWidth  = 0.045f;
    if(iParam == eParamReverbPreset)
        fHalfWidth = 0.06f;
    
    float fStepHeight = 0.0232f;
    float fHalfHeightOffset = 0.0125f;
    
    int iTextHeight = (int)floor(14.0f*fScale + .5f);
    int iTextWidth  = (int)floor(90.0f*fScale + .5f);
    if(iParam == eParamReverbPreset)
        iTextWidth  = (int)floor(120.0f*fScale + .5f);
        
    int iBorder     = (int)floor(10.0f*fScale + .5f);
    int iSpacing    = (int)floor(2.0f*fScale + .5f);
    
    //WgBoxSkinPtr pSkin = WgBoxSkin::Create(WgColor(0, 0, 0, 255), WgBorders(0), WgColor(0, 0, 0, 255));
    WgBoxSkinPtr pTextSkin = WgBoxSkin::Create(WgColor(0, 0, 0, 0), WgBorders(0), WgColor(0, 0, 0, 0));

 //SetSkin(pSkin);

    //Place Preset Selection window on root panel
    int iTopLeft = pkRootPanel->AddAnchor(fCenterX-fHalfWidth, fTopY);
    int iBottomRight = pkRootPanel->AddAnchor(fCenterX + fHalfWidth, fTopY + fStepHeight*iRows + 2.0f*fHalfHeightOffset);
    m_pHook = pkRootPanel->AddChild(this, iTopLeft, iBottomRight);
    //m_pHook = pkRootPanel->AddChild(this, WgRect(500, 500, 100, 200), WG_WEST);
    DBG_ASSERT(m_pHook != NULL);

    // Create background image
 WgImage* pImage = new WgImage();
 WgSurface* pSurface = pkModule->GetGui()->GetSurface(IDR_BACKGROUND_PRESET);

 WgFlexHook * pkHook = this->AddChild( pImage, WgRect( 0, 0, pSurface->Width(), pSurface->Height()) );

 if( pSurface )
  pImage->SetSource( WgBlockset::CreateFromSurface( pSurface ) );

 pImage->SetMarkOpacity(0);
 pkHook->SetVisible(true);

    //Add text button and callback for all presets
    for(int i = 0; i<pPresets.size(); i++)
    {
        WgGizmoTextButton* pkTextButton;
        pkTextButton = pkModule->AddTextButton(pPresets[i].GetPresetName(), pWgProp, WgCoord(iBorder, i*(iTextHeight+iSpacing)+iBorder), WgCoord(iTextWidth, iTextHeight), i, WG_NORTHWEST, this);
        
        pkTextButton->SetSkin(pTextSkin);
        pkTextButton->SetHoverSkin(pTextSkin);
        pkTextButton->SetSelectedSkin(pTextSkin);
        pkTextButton->SetSelectedHoverSkin(pTextSkin);
        pkTextButton->SetDisabledSkin(pTextSkin);
        pkTextButton->SetTextManager(pTextManager);
        pkTextButton->SetTextAlignment(WG_WEST);
        pkTextButton->SetParam(pPresets[i].GetPresetParam());
        m_pTextButtons.push_back(pkTextButton);
        
        pHandler->AddCallback( WgEventFilter::KnobTurn(pkTextButton), cbPresetSelectionCallback, pkTextButton);
    }
    m_pHook->SetVisible(m_bIsVisible);
}

*/