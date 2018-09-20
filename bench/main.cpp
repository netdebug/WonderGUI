#include <cstdlib>

#include <dwmapi.h>
#undef min
#undef max


#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#	include <GL/glew.h>
#else
#	ifdef __APPLE__
#		include <SDL2/SDL.h>
#		include <SDL2_image/SDL_image.h>
#	else
#		include <SDL2/SDL.h>
#		include <SDL2/SDL_image.h>
#	endif
#endif

#include <wondergui.h>

#include <wg_glsurface.h>
#include <wg_glsurfacefactory.h>
#include <wg_glgfxdevice.h>


#include <wg_bitmapglyphs.h>
#include <wg_vectorglyphs.h>
#include <wg_knob.h>
#include <iostream>

#include <wg_surface_soft.h>
#include <wg_surfacefactory_soft.h>
#include <wg_glsurface.h>
#include <wg_gfxdevice_soft.h>
#include <wg_glgfxdevice.h>
#include <wg_surfacefactory_soft.h>
#include <sdl_wglib.h>
#include <wg_boxskin.h>
#include <wg_volumemeter.h>
#include <wg_simplevolumemeter.h>
#include <wg_chart.h>
#include <wg_scrollchart.h>
#include <wg_canvascapsule.h>

#include <wg_popupopener.h>
#include <wg_popuplayer.h>
#include <wg_blockset.h>

#include "testwidget.h"

//#define USE_OPEN_GL


WgSurfaceFactory *	g_pSurfaceFactory = nullptr;
WgGfxDevice *		g_pGfxDevice = nullptr;


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

WgScrollChart * m_pScrollChart = nullptr;
int				m_hWave1 = 0;

WgFlexHook * g_pSpriteHook = nullptr;


volatile int	m_bGotVsync = 0;

static int TestThread(void *ptr)
{
	while (true)
	{
		m_bGotVsync = 1;
		DwmFlush();
	}

	return 0;
}


//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{

	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 1000, height = 600;

	int flags = 0;



#ifdef USE_OPEN_GL
	SDL_Window * pWin = SDL_CreateWindow("WonderGUI Testbench (OpenGL)", posX, posY, width, height, SDL_WINDOW_OPENGL);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

	SDL_GLContext context = SDL_GL_CreateContext(pWin);

	SDL_GL_SetSwapInterval(1);

//	SDL_Renderer * pRenderer = SDL_CreateRenderer(pWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#ifdef WIN32  
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#endif


	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();


#else
	SDL_Window * pWin = SDL_CreateWindow("WonderGUI Testbench (Software)", posX, posY, width, height, 0);
#endif


	int res = IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );


	SDL_Thread *thread;
	int         threadReturnValue;
	thread = SDL_CreateThread(TestThread, "TestThread", (void *)NULL);


	// Init WonderGUI

	WgBase::Init();
	sdl_wglib::MapKeys();


	WgBase::InitFreeType();

	// Setup gfxdevice and gui

#ifdef USE_OPEN_GL
	g_pGfxDevice = new WgGlGfxDevice( WgSize(width,height) );
	g_pSurfaceFactory = new WgGlSurfaceFactory();
#else
	SDL_Surface * pScreen = SDL_GetWindowSurface(pWin);

	WgPixelType type = WG_PIXEL_UNKNOWN;

	if (pScreen->format->BitsPerPixel == 32)
		type = WG_PIXEL_BGRA_8;
	else if (pScreen->format->BitsPerPixel == 24)
		type = WG_PIXEL_BGR_8;


	WgSurfaceSoft * pCanvas = new WgSurfaceSoft( WgSize(width,height), type, (unsigned char *) pScreen->pixels, pScreen->pitch );
	g_pGfxDevice = new WgGfxDeviceSoft( pCanvas );

	g_pSurfaceFactory = new WgSurfaceFactorySoft();
#endif
	
	//	pGfxDevice->SetBilinearFiltering( true );



	// Load TTF-font

	WgVectorGlyphs::SetSurfaceFactory( g_pSurfaceFactory );

	char	ttfname[] = { "../resources/DroidSans.ttf" };

	int size = sdl_wglib::FileSize( ttfname );
	char * pFontFile = (char*) sdl_wglib::LoadFile( ttfname );
	WgVectorGlyphs * pGlyphs = new WgVectorGlyphs( pFontFile , size, 0 );

	WgFont * pFont = new WgFont();
	pFont->SetDefaultVectorGlyphs( pGlyphs );

	// Load bitmap font

//	WgFont * pFont = sdl_wglib::LoadBitmapFont( "../resources/anuvverbubbla_8x8.png", "../resources/anuvverbubbla_8x8.fnt", * g_pSurfaceFactory );

	// Load and setup cursor

	WgSurface * pCursorImg = sdl_wglib::LoadSurface("../resources/cursors.png", * g_pSurfaceFactory );

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
	prop.SetSize(16);

	WgBase::SetDefaultTextprop( prop.Register() );


	WgRootPanel * pRoot = setupGUI( g_pGfxDevice );
	
	

	// Setup debug overlays
/*
	WgBoxSkinPtr pOverlaySkin = WgBoxSkin::Create( WgColor(255,0,0,128), WgBorders(1), WgColor::black);
	pOverlaySkin->SetStateColor( WG_STATE_NORMAL, WgColor::transparent, WgColor::red );	
	pRoot->SetUpdatedRectOverlay( pOverlaySkin, 0 );
	*/

//	WgColor lineColor = WgColor::transparent;
	WgColor lineColor = WgColor::white;

	int lineThickness = 1.f;

	int		topWave[2001];
	int		bottomWave[2001];

	WgWaveLine	topLine, bottomLine;

	topLine.color = lineColor;
	topLine.thickness = lineThickness;
	topLine.pWave = topWave;
	topLine.length = 2001;

	bottomLine.color = lineColor;
	bottomLine.thickness = lineThickness;
	bottomLine.pWave = topWave;
	bottomLine.length = 2001;

	for (int i = 0; i < 2001; i++)
	{
		topWave[i] = (int)((sin(i / 5.f) * 100) * 256);
		bottomWave[i] = topWave[i]+360;
//		bottomWave[i] = (int)((0 + sin(i / 20.0) * 6) * 256);
	}

   // program main loop

	int counter = 0;

    while (eventLoop( pRoot->EventHandler() ))
    {
		if (g_pSpriteHook)
		{
			WgRect pos = g_pSpriteHook->PointGeo();
			pos.x-= 10;
			if (pos.x + pos.w <= 0)
				pos.x = width-1;

			g_pSpriteHook->SetPointOfs(pos);
		}


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
/*
		{
			WgRect clip = { 100,100,200,200 };

			g_pGfxDevice->Fill(clip, WgColor::black);

			int lineLength = 250;
			WgCoord pos;

			pos.x = clip.x + clip.w / 2 + sin(counter / 250.0)*clip.w * 3 / 3;
			pos.y = clip.y + clip.h / 2 - lineLength / 2 + sin(counter / 150.0)*clip.h * 2 / 3;


			g_pGfxDevice->ClipDrawLine(clip, pos, WgDirection::WG_DOWN, lineLength, WgColor::white, 1.f );


//			for (int i = 0; i < 20; i++)
//				g_pGfxDevice->ClipDrawLine(clip, { clip.x + 1, clip.y + i * 15 }, WgDirection::WG_RIGHT, 50, WgColor::white, i*0.25 );

		}
*/

		// DRAWING STARTS HERE

//		pRoot->AddDirtyPatch( pRoot->Geo().Size() );

#ifndef USE_OPEN_GL
		SDL_LockSurface( pScreen );
#endif
/*
		g_pGfxDevice->BeginRender();

		g_pGfxDevice->Fill(WgSize(width,height), WgColor::black);

		g_pGfxDevice->ClipDrawHorrWave({ 10,100,380,500 }, { 0,500 }, 1900, topLine, bottomLine, WgColor::red, WgColor::red);

		//		pGfxDevice->stretchBlitSubPixelWithInvert(pMyCanvas, 0,0,400,400, 0,0, 200, 200 );
		//		pGfxDevice->blit(pMyCanvas, { 0,0,400,400 }, { 0,0 });
		//		pGfxDevice->stretchBlit(pMyCanvas, { 0,0,400,400 }, { 0,0,200,200 });
		g_pGfxDevice->EndRender();
*/

		pRoot->Render();

#ifndef USE_OPEN_GL
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

			const WgRect r = pRoot->PixelGeo();

			updatedRects[0].x = r.x;
			updatedRects[0].y = r.y;
			updatedRects[0].w = r.w;
			updatedRects[0].h = r.h;
		}

		SDL_UpdateWindowSurfaceRects(pWin, updatedRects, nUpdatedRects);
#endif
 
        // Pause for a while

//		SDL_Delay(32);

		while (!m_bGotVsync)
		{
			SDL_Delay(1);
		}
		m_bGotVsync = 0;

		DwmFlush();

//		SDL_RenderPresent(pRenderer);

		counter++;

    } // end main loop


	// Exit WonderGUI

	delete pRoot;
	delete g_pGfxDevice;

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

WgRootPanel * setupGUI(WgGfxDevice * pDevice)
{
	WgResDB * pDB = sdl_wglib::LoadStdWidgets("../resources/blocks.png", "../resources/blocks_x2.png", "../resources/blocks_x4.png", *g_pSurfaceFactory);
	if (!pDB)
		return 0;

	WgRootPanel * pRoot = new WgRootPanel(pDevice);

	pRoot->SetScale(8192);

	WgEventHandler * pEventHandler = pRoot->EventHandler();

	WgEventLogger * pEventLogger = new WgEventLogger(std::cout);
	pEventLogger->LogAllEvents();
	pEventLogger->IgnoreEvent(WG_EVENT_TICK);
	pEventLogger->IgnoreEvent(WG_EVENT_MOUSE_POSITION);
	pEventLogger->IgnoreEvent(WG_EVENT_MOUSE_MOVE);
	pEventLogger->IgnoreEvent(WG_EVENT_MOUSEBUTTON_REPEAT);

	//	pEventLogger->IgnoreEvent( WG_EVENT_MOUSE_POSITION );
	//	pEventLogger->IgnoreEvent( WG_EVENT_MOUSEBUTTON_REPEAT );
	//	pEventLogger->IgnoreEvent( WG_EVENT_BUTTON_PRESS );
	//	pEventLogger->IgnoreAllEvents();
	//	pEventLogger->LogMouseButtonEvents();
	pEventHandler->AddCallback(pEventLogger);


	// Load images and specify blocks

	WgSurface * pBackImg = sdl_wglib::LoadSurface("../resources/What-Goes-Up-3.bmp", *g_pSurfaceFactory);
	WgBlocksetPtr pBackBlock = WgBlockset::CreateFromSurface(pBackImg, WG_TILE_ALL);

	WgSurface * pFlagImg = sdl_wglib::LoadSurface("cb2.bmp", *g_pSurfaceFactory);
	WgBlocksetPtr pFlagBlock = WgBlockset::CreateFromSurface(pFlagImg);

	WgSurface * pSplashImg = sdl_wglib::LoadSurface("../resources/splash.png", *g_pSurfaceFactory);
	WgBlocksetPtr pSplashBlock = WgBlockset::CreateFromSurface(pSplashImg);

	WgSurface * pBigImg = sdl_wglib::LoadSurface("../resources/frog.jpg", *g_pSurfaceFactory);
	WgBlocksetPtr pBigBlock = WgBlockset::CreateFromSurface(pBigImg);

	WgSurface * pPlateImg = sdl_wglib::LoadSurface("../resources/grey_pressable_plate.bmp", *g_pSurfaceFactory);

	WgSurface * pAnimSurf = sdl_wglib::LoadSurface("../resources/dummy_anim.png", *g_pSurfaceFactory);

	WgSurface * pPlateImg_x2 = sdl_wglib::LoadSurface("../resources/grey_pressable_plate_x2.bmp", *g_pSurfaceFactory);
	pPlateImg_x2->SetScaleFactor(4096 * 2);

	auto pPressablePlateSkin = WgBlockSkin::CreateClickableFromSurface(pPlateImg, 0, WgBorders(3));

//	WgSurface * pInjectWidget = sdl_wglib::LoadSurface("../resources/IDR_MOD_INJECT_WIDGET_CHROME.2x.png", *g_pSurfaceFactory);
//	pInjectWidget->SetScaleFactor(4096 * 2);

	

//	WgBlockSkinPtr pTagSkin = WgBlockSkin::Create();


	// Popup Layer

	WgPopupLayer * pPopupLayer = new WgPopupLayer();
	pRoot->SetChild(pPopupLayer);

	// Bottom Flex

	WgFlexPanel * pBottom = new WgFlexPanel();
	pPopupLayer->SetBase(pBottom); 
//	pRoot->SetChild(pBottom);
	pBottom->SetSkin(WgColorSkin::Create({60,60,60,255} ));
//	pBottom->SetSkin(WgColorSkin::Create({ 255,255,255,255 }));

	// Main Flex

	WgFlexPanel * pFlex = new WgFlexPanel();
	pBottom->AddChild(pFlex, WG_NORTHWEST, WG_SOUTHEAST, WgBorders(10));

	// Background
/*
	WgImage * pBackground = new WgImage();
	pBackground->SetSource(pBackBlock);
	WgFlexHook * pHook = pFlex->AddChild(pBackground);
	pHook->SetAnchored(WG_NORTHWEST, WG_SOUTHEAST);
*/

	// BlendTest
/*
	{
		auto pSkin = WgBlockSkin::CreateStaticFromSurface(pInjectWidget);

		WgImage * pImage = new WgImage();
		pImage->SetSkin(pSkin);

		WgShaderCapsule * pShader = new WgShaderCapsule();
		pShader->SetChild(pImage);
//		pShader->SetBlendMode(WgBlendMode::WG_BLENDMODE_OPAQUE);

		WgCanvasCapsule * pCapsule = new WgCanvasCapsule();
		pCapsule->SetSurfaceFactory(g_pSurfaceFactory);
		pCapsule->SetChild(pShader);




		WgFlexHook * pHook = pFlex->AddChild(pCapsule, WgRect( 20,20,366/2,178/2 ) );


		WgImage * pFacitImage = new WgImage();
		pFacitImage->SetSkin(pSkin);
		WgFlexHook * pHook2 = pFlex->AddChild(pFacitImage, WgRect(240, 20, 366 / 2, 178 / 2));

	}
*/



	// Animplayer test
/*
	{
		auto pPlayerFrame = new WgFlexPanel();

		auto pBgSkin = WgBoxSkin::Create(WgColor::white, 2, WgColor::black);
		pBgSkin->SetContentPadding(2);

		pPlayerFrame->SetSkin( pBgSkin );


		auto pPlayer = new WgAnimPlayer();
		pPlayerFrame->AddChild(pPlayer, { 2,2,100,100 });

		auto pBtn1 = (WgButton*)pDB->CloneWidget("button");
		pPlayerFrame->AddChild(pBtn1, { 2,102,30,30 } );


		auto pBtn2 = (WgButton*)pDB->CloneWidget("button");
		pPlayerFrame->AddChild(pBtn2, { 32,102,30,30 });

		auto pBtn3 = (WgButton*)pDB->CloneWidget("button");
		pPlayerFrame->AddChild(pBtn3, { 62,102,30,30 });

		pFlex->AddChild(pPlayerFrame, { 20,20,104,154 });



		auto pGfxAnim = new WgGfxAnim({ 100,100 });
		pGfxAnim->AddFrames(pAnimSurf, 500, 5);

		pPlayer->SetAnimation(pGfxAnim);


		pEventHandler->AddCallback(WgEventFilter::ButtonPress(pBtn1), [](const WgEvent::Event * pEvent, WgWidget *pPlayer)
		{
			static_cast<WgAnimPlayer*>(pPlayer)->PlayToFractional(0.1f);
		}, pPlayer);

		pEventHandler->AddCallback(WgEventFilter::ButtonPress(pBtn2), [](const WgEvent::Event * pEvent, WgWidget *pPlayer)
		{
			static_cast<WgAnimPlayer*>(pPlayer)->PlayToFractional(0.5f);
		}, pPlayer);

		pEventHandler->AddCallback(WgEventFilter::ButtonPress(pBtn3), [](const WgEvent::Event * pEvent, WgWidget *pPlayer)
		{
			static_cast<WgAnimPlayer*>(pPlayer)->PlayToFractional(0.9f);
		}, pPlayer);


//		pPlayer->Play();
	}
*/


	// Test for mouse leave on parent while button pressed and leaving child
/*
	{
		auto pSkin = WgBoxSkin::Create(WgColor::cornsilk, 2, WgColor::deeppink);
		pSkin->SetStateColor(WG_STATE_HOVERED, WgColor::white, WgColor::red );
		pSkin->SetStateColor(WG_STATE_PRESSED, WgColor::yellow, WgColor::red);
		pSkin->SetContentPadding(3 );
		pSkin->SetContentShift(WG_STATE_PRESSED, { 2,2 });

		auto pButton = new WgButton();
		pButton->SetSkin(pSkin);
//		pButton->SetText("TEST");
		pFlex->AddChild(pButton, { 50,50,200,100 } );
	}
*/

	// Setup moving sprite
/*
	{
		WgFiller * pSprite = new WgFiller();
		pSprite->SetColors(WgColorset::Create(WgColor::red));

		g_pSpriteHook = pFlex->AddChild(pSprite, WgRect(0, 100, 100, 100));
	}
*/
	// PackPanel padding test
/*
	{
		auto pPack = new WgPackPanel();
		pPack->SetSkin(WgColorSkin::Create(WgColor::blue));
		pFlex->AddChild(pPack, { 0,0,300,100 });
		pPack->SetOrientation(WG_VERTICAL);

		for (int i = 0; i < 3; i++)
		{
			auto pFiller = new WgFiller();
			pFiller->SetColors(WgColorset::Create(WgColor::red));
			pFiller->SetPreferredPointSize({ 10,10 });

			pPack->AddChild(pFiller)->SetPadding(4);
		}

	}
*/


	// Esc callback test
/*
	{
		auto p = new WgLineEditor();
		p->SetSkin(WgColorSkin::Create(WgColor::aliceblue));

		pEventHandler->AddCallback(WgEventFilter::KeyPress(p),
			[](const WgEvent::Event * pEvent) {

			const WgEvent::KeyPress * p = static_cast<const WgEvent::KeyPress*>(pEvent);
			int keyCode = p->NativeKeyCode();
		}

			);

		WgFlexHook * pHook = pFlex->AddChild(p, { 10,10,200,100 });
	}
*/
	// Flow Panel Test
/*
	{
		WgFlowPanel * pFlow = new WgFlowPanel();

		pFlow->SetSkin(WgColorSkin::Create(WgColor::white));

		WgFlexHook * pHook = pFlex->AddChild(pFlow, { 10,10,400,300 });


		WgSize fillers[10]{ {10,10}, {20,20}, {30,15}, {40,20}, {80,20}, {160,20}, {320,20}, {160,20}, {80,20}, {40,20} };

		WgColorsetPtr colors[2];

		colors[0] = WgColorset::Create(WgColor::blue);
		colors[1] = WgColorset::Create(WgColor::red);

		for (int i = 0; i < 10; i++)
		{
			auto p = new WgFiller();

			p->SetPreferredPointSize(fillers[i]);
			p->SetColors(colors[i % 2]);
			WgFlowHook * pH = pFlow->AddChild(p);
			pH->SetPadding(2);
		}

		WgLineEditor * pLE = new WgLineEditor();
		pLE->SetText("ABC");
		pLE->SetSkin(WgColorSkin::Create(WgColor::yellow));
		pFlow->AddChild(pLE);
	}
*/

	// Volume meter direction test
/*
	for (int i = 0; i < 4; i++)
	{
		auto p = new WgSimpleVolumeMeter();

		p->SetValue(0.8f, 1.0f, 0.7f, 0.8f);

		p->SetDirection((WgDirection) (WgDirection::WG_UP + i));

		pFlex->AddChild(p, { 10 + i * 110, 10, 100, 100 });
	}
*/

	// Popup Layer Test
/*
	{
		auto pOpener = new WgPopupOpener();

		pOpener->SetSkin(pPressablePlateSkin);
		pOpener->SetText("OPEN");

//		pOpener->setPointerStyle(PointerStyle::Crosshair);

		pFlex->AddChild(pOpener, WgRect(30, 30, 100, 100) );



		auto pMenu = new WgPackPanel();
		pMenu->SetOrientation(WG_VERTICAL);
//		pMenu->setSelectable(false);

		auto pSkin = WgBoxSkin::Create(WgColor::red, WgBorders(0), WgColor::red );
		pSkin->SetContentPadding(12);
		pMenu->SetSkin(pSkin);

		auto pEntry1 = new WgTextDisplay();
		pEntry1->SetText("Entry One");
		pEntry1->SetSkin(pPressablePlateSkin);
		pMenu->AddChild(pEntry1);

		auto pEntry2 = new WgTextDisplay();
		pEntry2->SetText("Entry Two");
		pEntry2->SetSkin(pPressablePlateSkin);
		pMenu->AddChild(pEntry2);

		auto pEntry3 = new WgTextDisplay();
		pEntry3->SetText("Entry Three");
		pEntry3->SetSkin(pPressablePlateSkin);
		pMenu->AddChild(pEntry3);

		//---

		auto pSubMenuOpener = new WgPopupOpener();
		pSubMenuOpener->SetText("Sub Menu");
		pSubMenuOpener->SetSkin(pPressablePlateSkin);
		pSubMenuOpener->SetOpenOnHover(true);
		pSubMenuOpener->SetAttachPoint(WG_NORTHEAST);
		pSubMenuOpener->SetPopupOffset({ -10,5 });
		pMenu->AddChild(pSubMenuOpener);


		auto pSubMenu = new WgPackPanel();
		pSubMenu->SetOrientation(WG_VERTICAL);
		pSubMenu->SetSkin(pSkin);

		auto pSubEntry1 = new WgTextDisplay();
		pSubEntry1->SetText("Subentry One");
		pSubEntry1->SetSkin(pPressablePlateSkin);
		pSubMenu->AddChild(pSubEntry1);

		auto pSubEntry2 = new WgTextDisplay();
		pSubEntry2->SetText("Subentry Two");
		pSubEntry2->SetSkin(pPressablePlateSkin);
		pSubMenu->AddChild(pSubEntry2);

		auto pSubEntry3 = new WgTextDisplay();
		pSubEntry3->SetText("Subentry Three");
		pSubEntry3->SetSkin(pPressablePlateSkin);
		pSubMenu->AddChild(pSubEntry3);

		pSubMenuOpener->SetPopup(pSubMenu);

		//---

		auto pSubMenuOpener2 = new WgPopupOpener();
		pSubMenuOpener2->SetText("Sub Menu 2");
		pSubMenuOpener2->SetSkin(pPressablePlateSkin);
		pSubMenuOpener2->SetOpenOnHover(true);
		pSubMenuOpener2->SetAttachPoint(WG_NORTHEAST);
		pMenu->AddChild(pSubMenuOpener2);


		auto pSubMenu2 = new WgPackPanel();
		pSubMenu2->SetOrientation(WG_VERTICAL);
		pSubMenu2->SetSkin(pSkin);

		auto pSubEntry21 = new WgTextDisplay();
		pSubEntry21->SetText("Subentry A");
		pSubEntry21->SetSkin(pPressablePlateSkin);
		pSubMenu2->AddChild(pSubEntry21);

		auto pSubEntry22 = new WgTextDisplay();
		pSubEntry22->SetText("Subentry B");
		pSubEntry22->SetSkin(pPressablePlateSkin);
		pSubMenu2->AddChild(pSubEntry22);

		auto pSubEntry23 = new WgTextDisplay();
		pSubEntry23->SetText("Subentry C");
		pSubEntry23->SetSkin(pPressablePlateSkin);
		pSubMenu2->AddChild(pSubEntry23);

		pSubMenuOpener2->SetPopup(pSubMenu2);

		//---

		auto pButtons = new WgPackPanel();
		
		auto pBtn1 = new WgButton();
		pBtn1->SetSkin(pPressablePlateSkin);
		pBtn1->SetText("Button 1");
		pButtons->AddChild(pBtn1);

		auto pBtn2 = new WgButton();
		pBtn2->SetSkin(pPressablePlateSkin);
		pBtn2->SetText("Button 2");
		pButtons->AddChild(pBtn2);

		pMenu->AddChild(pButtons);

		pOpener->SetPopup( pMenu );

	}
*/


	// Button skin test
/*
	pFlex->SetScale(4096 * 8);

	auto pSkin = WgMultiBlockSkin::Create({ 10,10 }, WgBorders(2));

	int hLayer = pSkin->AddLayer(pPlateImg_x2, { 0,0 }, { 10,0 }, { WG_STATE_NORMAL } );
	pSkin->SetLayerTint(hLayer, { { WG_STATE_HOVERED, WgColor::red}, { WG_STATE_PRESSED, WgColor( 0,255,0,128 ) } } );

	hLayer = pSkin->AddLayer(pPlateImg_x2, { 5,0 } );
	pSkin->SetLayerTint(hLayer, { { WG_STATE_NORMAL, WgColor::transparent },{ WG_STATE_PRESSED, WgColor(255,0,0,128) } });
	pSkin->SetLayerBlendMode(hLayer, WgBlendMode::WG_BLENDMODE_ADD);

	auto pSkin2 = WgBlockSkin::CreateClickable( pPlateImg_x2, { 10,10 }, { 0,0 }, { 10,0 }, WgBorders(2) );
//	pSkin->SetContentPadding(3);
//	pSkin->SetContentShift(WG_STATE_PRESSED, { 2,2 });

	auto pButton = new WgButton();
	pButton->SetSkin(pSkin);
//	pButton->SetText("    ");
	pFlex->AddChild(pButton, WgCoord(20,20) );
*/
	// TextDisplay skin test

//	auto pText = new WgTextDisplay();
//	pText->SetSkin(pSkin);
//	pText->SetText("THIS IS THE TEST TEXT");
//	pFlex->AddChild(pText, WgCoord(100,100));


// PackPanel Scale Test
/*
	auto pMyFlex = new WgFlexPanel();
	pFlex->AddChild(pMyFlex, WG_NORTHWEST, WG_SOUTHEAST );
	pMyFlex->SetScale(WG_SCALE_BASE * 2);

	auto p1 = new WgPackPanel();

	WgColor zebra[2] = { WgColor::red, WgColor::blue };



	for (int i = 0; i < 4; i++)
	{
		auto pFiller = new WgFiller();
		pFiller->SetColors(WgColorset::Create(zebra[i % 2]));
		pFiller->SetPreferredPointSize({ 100,100 });
		p1->AddChild(pFiller);

		if( i == 0 )
			pMyFlex->AddChild(p1, WgSize(800, 200));
	}

	p1->SetSkin(WgColorSkin::Create(WgColor::beige));
*/



// Text InputFocus test

/*
	WgBlockSkinPtr pSkin = WgBlockSkin::CreateStatic(pPlateImg, { 0,0,10,10 }, 3);
	pSkin->SetStateBlock(WG_STATE_FOCUSED, { 10,0 });

//	WgBoxSkinPtr pSkin = WgBoxSkin::Create(WgColor::antiquewhite, 1, WgColor::black );
//	pSkin->SetStateColor(WG_STATE_FOCUSED, WgColor::seagreen, WgColor::red);


	WgTextDisplay * pDisplay1 = new WgTextDisplay();
	pDisplay1->SetText("DISPLAY 1 WITH A LONG LINE THAT NEEDS TO AUTOWRAP SOONER OR LATER");
	pDisplay1->SetEditMode(WG_TEXT_EDITABLE);
	pDisplay1->SetSkin(pSkin);

	WgTextprop selectionProp;
	selectionProp.SetBgColor(WgColor::red);



	pDisplay1->SetSelectionProperties(selectionProp.Register());


	auto pCanvas = new WgCanvasCapsule();
	pCanvas->SetChild(pDisplay1);
	pCanvas->SetSurfaceFactory(g_pSurfaceFactory);
	pCanvas->SetColor(WgColor::transparent);

	pCanvas->StartFade(WgColor::white, 3000);

	pFlex->AddChild(pCanvas, WgRect(0, 0, 200, 100));


	WgScrollPanel * pScroll = new WgScrollPanel();
	pScroll->SetContentSizePolicy(WG_BOUND, WG_DEFAULT);
	pFlex->AddChild(pScroll, WgRect(0, 110, 200, 100));

	WgTextDisplay * pDisplay2 = new WgTextDisplay();
	pDisplay2->SetText("DISPLAY WITH MANY LINES\n");

	for (int i = 0; i < 20; i++)
	{
		pDisplay2->AddText("Line X\nAnother Line\n");
	}

	pDisplay2->SetEditMode(WG_TEXT_EDITABLE);
	pDisplay2->SetSkin(pSkin);
	pDisplay2->SetSelectionProperties(selectionProp.Register());

	pScroll->SetContent(pDisplay2);

	WgVSlider * pSlider = (WgVSlider*) pDB->CloneWidget("vslider");
	pScroll->SetVSlider(pSlider);
*/

//	pFlex->SetScale(WG_SCALE_BASE * 2);


	// Cursor belong to widget as long as button pressed




	// Multislider widget


	WgBoxSkinPtr pSliderBgSkin = WgBoxSkin::Create(WgColor::grey, 1, WgColor::grey);
	pSliderBgSkin->SetStateColor(WG_STATE_HOVERED, WgColor::blue);
	pSliderBgSkin->SetStateColor(WG_STATE_SELECTED, WgColor::black, WgColor::black);


	WgBoxSkinPtr pSliderHandleSkin = WgBoxSkin::Create(WgColor::white, 1, WgColor::red);
	pSliderHandleSkin->SetContentPadding(10);
	pSliderHandleSkin->SetStateColor(WG_STATE_HOVERED, WgColor::blue);
	pSliderHandleSkin->SetStateColor(WG_STATE_PRESSED, WgColor::red);


	auto pMultiSlider = new WgMultiSlider();

//	pMultiSlider->SetSkin(WgBoxSkin::Create(WgColor::white, { 1 }, WgColor::green));

	pMultiSlider->SetDefaults(pSliderBgSkin, pSliderHandleSkin);

	pMultiSlider->SetPassive(true);

	pMultiSlider->SetCallback([pMultiSlider](int sliderId, float value, float value2) 
	{ 
		pMultiSlider->SetSliderValue(sliderId, value, value2); 
	}
	);

	pMultiSlider->AddSlider2D(0, WG_NORTHWEST, 
						[](WgMultiSlider::SetGeoVisitor& visitor) {
							return WgRectF( 0.1f, 0.1f, 0.8f, 0.1f ); 
						},
						0.00f, 0.50f, -3.5f, 2.1f, 5, 0.f, 1.f, 0,
						nullptr
						);

	pMultiSlider->AddSlider(1, WG_UP,
						[](WgMultiSlider::SetGeoVisitor& visitor) { 
			
							WgRectF r = visitor.geo(0);
							WgCoordF c = visitor.handlePos(0);
							return WgRectF( r.x + r.w*c.x, r.y + r.h*c.y + 0.1f, 0.0f, 1.f - r.y -0.2f ); 
						},
						0.25f, 0.f, 1.f, 0,
						[](WgMultiSlider::SetValueVisitor& visitor) {

							static float prevValue = -1.f;

							float	value = visitor.value();
							if (value < 0.001f)
								value = 0.001f;

							if (prevValue == -1.f)
								prevValue = visitor.value();

							float	scaleFactor = value/prevValue;


							prevValue = value;

							visitor.setValue(2, visitor.value(2)*scaleFactor);
							visitor.setValue(3, visitor.value(3)*scaleFactor);

							return visitor.value();
						});

	pMultiSlider->AddSlider(2, WG_UP,
						[](WgMultiSlider::SetGeoVisitor& visitor) { 
	
							WgRectF r = visitor.geo(1);
							return WgRectF(r.x-0.1f, r.y + 0.1f, r.w, r.h - 0.2f); 
						},
						0.25f, 0.f, 1.f, 0,
						[](WgMultiSlider::SetValueVisitor& visitor) {
							return visitor.value(); 
						});
	
	pMultiSlider->AddSlider(3, WG_UP,
						[](WgMultiSlider::SetGeoVisitor& visitor) {

							WgRectF r = visitor.geo(1);
							return WgRectF(r.x + 0.1f, r.y + 0.1f, r.w, r.h - 0.2f );
						},
						0.75f, 0.f, 1.f, 0,
						[](WgMultiSlider::SetValueVisitor& visitor) { 
							return visitor.value(); 
						}, nullptr, nullptr, { 1.f,1.f }, WgBorders(10) );


	WgButton * pBlockedButton = new WgButton();
	pBlockedButton->SetSkin(pPressablePlateSkin);

	pFlex->AddChild(pBlockedButton, WgRect(0, 0, 450, 300));
	auto pMyHook = pFlex->AddChild(pMultiSlider, WgRect(0, 0, 450, 300));

	pFlex->SetScale(WG_SCALE_BASE * 2);


	pMultiSlider->SetPressMode(WgMultiSlider::PressMode::MultiSetValue);

	pMultiSlider->SetSkin(WgColorSkin::Create(WgColor::blanchedalmond));

	auto pEvH = pRoot->EventHandler();

	pEvH->AddCallback(WgEventFilter::Type(WG_EVENT_SLIDER_PRESS), [](const WgEvent::Event * _pEv, void * pParam) 
	{
		auto pEv = ((WgEvent::SliderPressed *)_pEv);

		int id = pEv->id();
		int button = pEv->button();
		WgOrigo side = pEv->sideOfHandle();


	}, nullptr );


	// Scroll chart widget

/*
	m_pScrollChart = new WgScrollChart();

	WgBoxSkinPtr pChartSkin = WgBoxSkin::Create(WgColor::white, WgBorders(1), WgColor::black);
	pChartSkin->SetContentPadding({ 50,2,2,2});
	m_pScrollChart->SetSkin(pChartSkin);

//	m_pScrollChart->SetValueRange(10, -10);
	m_pScrollChart->SetDynamicValueRange(false);

	m_pScrollChart->SetSurfaceFactory(g_pSurfaceFactory);

//	m_hWave1 = m_pScrollChart->StartSimpleWave(0.f, 0.f, 5.f, WgColor::black, 1.f, WgColor::red, WgColor::grey, WgColor::darkred);

	m_hWave1 = m_pScrollChart->StartSimpleWave(0.f, 0.f, 0.8f, WgColor::black, 1.f, WgColor::red, WgColor::grey, WgColor::darkred);
	m_pScrollChart->Start(2000);

	WgScrollChart::GridLine valueGrid[3]{ { 0.25f,1.f,WgColor::red,"0.25" },{ 0.5f,1.f,WgColor::red,"0.5" },{ -0.5f,1.f,WgColor::red,"-0.5" } };
	WgBoxSkinPtr pLabelSkin = WgBoxSkin::Create(WgColor::antiquewhite, WgBorders(1), WgColor::black);
	pLabelSkin->SetContentPadding(3);

	m_pScrollChart->SetValueGridLines(3, valueGrid);
	m_pScrollChart->SetValueLabelStyle(WG_NORTHWEST, { -10,0 }, pLabelSkin, 0);
//	m_pScrollChart->SetCanvasPadding(WgBorders(50, 0, 0, 0));




	auto pWindowContent = new WgPackPanel();
	pWindowContent->SetOrientation(WG_VERTICAL);
	pWindowContent->SetSizeBroker(new WgUniformSizeBroker());

	pWindowContent->AddChild(m_pScrollChart)->SetWeight(2);


	// Canvas Capsule

//	auto pWindow = pWindowContent;

	auto pWindow = new WgCanvasCapsule();
	pWindow->SetChild(pWindowContent);
	pWindow->SetSurfaceFactory(g_pSurfaceFactory);

	pWindow->SetColor(0xFFFFFFFF);
//	pWindow->StartFade(0x00FFFFFF, 4000);

	auto pButtonBar = new WgPackPanel();
	pButtonBar->SetSizeBroker(new WgUniformSizeBroker());

	auto pFiller = new WgFiller();
	pFiller->SetColors(WgColorset::Create(WgColor::grey));


	auto pValueScaleButton = (WgButton*)pDB->CloneWidget("button");
	pValueScaleButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pValueScaleButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();

		WgChart * pChart = static_cast<WgChart*>(pWin);

		float first = pChart->ValueRangeStart() - drag.x / 5.f;
		float last = pChart->ValueRangeEnd() + drag.x / 5.f;

		m_pScrollChart->SetValueRange(first, last);
	}, m_pScrollChart);


	auto pZoomButton = (WgButton*)pDB->CloneWidget("button");
	pZoomButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pZoomButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();

		WgChart * pChart = static_cast<WgChart*>(pWin);

		float first = pChart->FirstSample();
		float last = pChart->LastSample() + drag.x / 5.f;

		pChart->SetFixedSampleRange(first, last);
	}, pWindow);


	auto pScrollButton = (WgButton*)pDB->CloneWidget("button");
	pScrollButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pScrollButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();

		WgChart * pChart = static_cast<WgChart*>(pWin);

		float first = pChart->FirstSample() + drag.x / 10.f;
		float last = pChart->LastSample() + drag.x / 10.f;

		pChart->SetFixedSampleRange(first, last);
	}, pWindow);




	auto pResizeButton = (WgButton*)pDB->CloneWidget("button");
	pResizeButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pResizeButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();
		static_cast<WgFlexHook*>(pWin->Hook())->SetPointSize(WgSize(pWin->PointSize() + WgSize(drag.x, drag.y)) / 1);
	}, pWindow);

	pButtonBar->AddChild(pFiller)->SetWeight(2);
	pButtonBar->AddChild(pValueScaleButton)->SetWeight(0);
//	pButtonBar->AddChild(pZoomButton)->SetWeight(0);
//	pButtonBar->AddChild(pScrollButton)->SetWeight(0);
	pButtonBar->AddChild(pResizeButton)->SetWeight(0);
	pWindowContent->AddChild(pButtonBar)->SetWeight(0);



	pHook = pFlex->AddChild(pWindow, WgRect(0, 0, 500, 300));

//	pHook->SetScaleGeo(true);

//	pFlex->SetScale(WG_SCALE_BASE * 2);

*/
	// Chart widget
/*
	WgChart * pChart = new WgChart();

	float topSamples[10] = { 1.f,0.75f,0.5f,1.f,1.f,0.8f,0.8f,1.f,0.1f,0.1f };
	float bottomSamples[10] = { 0.f,0.f,-1.f,-1.f,-1.f,-0.5f,-0.5f, -0.5f, 0.f,0.f };

	float topSamples2[3] = { 2.f,1.f,0.5f };
	float bottomSamples2[3] = { 2.f,1.f,0.5f };

	float	topWave[2001];
	float		bottomWave[2001];

	WgWaveLine	topLine, bottomLine;

	for (int i = 0; i < 2001; i++)
	{
		topWave[i] = (sin(i / 10.0) * 80);
		bottomWave[i] = (-100 + sin(i / 20.0) * 6);
	}



	WgBoxSkinPtr pChartSkin = WgBoxSkin::Create(WgColor::white, WgBorders(0), WgColor::red);
//	pChartSkin->SetContentPadding({ 20 });

	int iWave = pChart->AddWave();
	pChart->SetWaveSamples(iWave, 0, 1001, topWave);
	pChart->SetWaveStyle(iWave, WgColor::antiquewhite, WgColor::brown, 2.f, WgColor::black, 8.f, WgColor::black);

//	int iWave2 = pChart->AddWave();
//	pChart->SetWaveSamples(iWave2, 2, 3, topSamples2, bottomSamples2, 0);


	pChart->SetSkin( pChartSkin );
	pChart->SetCanvasPadding({ 10 });

//	pChart->SetFixedValueRange(pChart->ValueRangeStart()+20, pChart->ValueRangeEnd()-20);

	WgChart::GridLine valueGrid[3]{ {0.f,1.f,WgColor::red,"0"},{ 100.f,1.f,WgColor::red,"100" },{ -100.f,1.f,WgColor::red,"-100" } };

	WgChart::GridLine sampleGrid[2]{ { 0.f,1.f,WgColor::grey,"0" },{ 100.f,1.f,WgColor::grey,"100" } };


	WgBoxSkinPtr pLabelSkin = WgBoxSkin::Create(WgColor::antiquewhite, WgBorders(1), WgColor::black);
	pLabelSkin->SetContentPadding(3);

	pChart->SetValueGridLines(3, valueGrid);
	pChart->SetSampleGridLines(2, sampleGrid);
	pChart->SetSampleLabelStyle(WG_CENTER, { 0,0 }, pLabelSkin, 0 );

	pChart->SetValueLabelStyle(WG_SOUTHEAST, { 0,0 }, pLabelSkin, 0);


//	pChart->SetFixedValueRange(-1, 1);


	auto pWindow = new WgPackPanel();
	pWindow->SetOrientation(WG_VERTICAL);
	pWindow->SetSizeBroker(new WgUniformSizeBroker());

	pWindow->AddChild(pChart)->SetWeight(2);


	auto pButtonBar = new WgPackPanel();
	pButtonBar->SetSizeBroker(new WgUniformSizeBroker());

	auto pFiller = new WgFiller();
	pFiller->SetColors(WgColorset::Create(WgColor::grey));


	auto pValueScaleButton = (WgButton*)pDB->CloneWidget("button");
	pValueScaleButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pValueScaleButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();

		WgChart * pChart = static_cast<WgChart*>(pWin);

		float first = pChart->ValueRangeStart() - drag.x / 5.f;
		float last = pChart->ValueRangeEnd() + drag.x / 5.f;

		pChart->SetFixedValueRange(first, last);
	}, pChart);


	auto pZoomButton = (WgButton*)pDB->CloneWidget("button");
	pZoomButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pZoomButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();

		WgChart * pChart = static_cast<WgChart*>(pWin);

		float first = pChart->FirstSample();
		float last = pChart->LastSample() + drag.x / 5.f;

		pChart->SetFixedSampleRange(first, last);
	}, pChart);


	auto pScrollButton = (WgButton*)pDB->CloneWidget("button");
	pScrollButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pScrollButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();

		WgChart * pChart = static_cast<WgChart*>(pWin);

		float first = pChart->FirstSample() + drag.x / 10.f;
		float last = pChart->LastSample() + drag.x / 10.f;

		pChart->SetFixedSampleRange(first,last);
	}, pChart);


	auto pResizeButton = (WgButton*)pDB->CloneWidget("button");
	pResizeButton->SetText(" ");
	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pResizeButton, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
	{ 
		WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();
		static_cast<WgFlexHook*>(pWin->Hook())->SetPointSize(WgSize(pWin->PointSize() + WgSize(drag.x, drag.y))/2); 
	}, pWindow);

	pButtonBar->AddChild(pFiller)->SetWeight(2);
	pButtonBar->AddChild(pValueScaleButton)->SetWeight(0);
	pButtonBar->AddChild(pZoomButton)->SetWeight(0);
	pButtonBar->AddChild(pScrollButton)->SetWeight(0);
	pButtonBar->AddChild(pResizeButton)->SetWeight(0);
	pWindow->AddChild(pButtonBar)->SetWeight(0);



	pHook = pFlex->AddChild(pWindow, WgRect(10, 10, 500, 300) );
*/
//	pHook->SetScaleGeo(true);

//	pFlex->SetScale(WG_SCALE_BASE*2);


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
/*
	{
		WgSurface * pImg = sdl_wglib::LoadSurface("../resources/blocks.png", g_pSurfaceFactory );

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
*/
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
/*
	// Test view

	{
		WgScrollPanel * pView = (WgScrollPanel*) pDB->CloneWidget( "view" );

		WgImage * pImage = new WgImage();
		pImage->SetSource( pBigBlock );
		pView->SetContent( pImage );
		addResizablePanel( pFlex, pView, pEventHandler );
	}
*/

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
	if (m_pScrollChart)
	{
		int ticks = SDL_GetTicks();
		

		m_pScrollChart->FeedSample(m_hWave1, sin(ticks / 100.0)*0.9 );
	}
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


	dragStartPos = pHook->FloatPointOfs();
	printf( "DRAG START!\n" );
}

//____ cbDragWidget() __________________________________________________________

void cbDragWidget( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
	if( _pEvent->Type() != WG_EVENT_MOUSEBUTTON_DRAG || !pWidget->Parent() )
		return;

	const WgEvent::MouseButtonDrag* pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

	WgCoord	dragDistance = pEvent->DraggedTotalPixels();

	WgCoord	ofs = dragStartPos + dragDistance;

//	printf( "AccDistance: %d, %d\n", dragDistance.x, dragDistance.y );
	printf( "ofs: %d, %d   start: %d %d   distance: %d, %d\n", ofs.x, ofs.y, dragStartPos.x, dragStartPos.y, dragDistance.x, dragDistance.y );

	WgFlexHook * pHook = static_cast<WgFlexHook*>(pWidget->Hook());
	pHook->SetPointOfs(dragStartPos+dragDistance);
}

//____ cbOpenModal() __________________________________________________________

void cbOpenModal( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
//	g_pModal->AddModal( pWidget, WgCoord(), WG_SOUTHEAST );
}

//____ cbCloseModal() __________________________________________________________

void cbCloseModal( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
//	g_pModal->ReleaseChild(pWidget);
}

//____ cbResizeWidget() _________________________________________________________

void cbResize( const WgEvent::Event* _pEvent, void * _pFlexHook )
{
	WgFlexHook * pHook = static_cast<WgFlexHook*>(_pFlexHook);
	const WgEvent::MouseButtonDrag* pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

	WgCoord dragged = pEvent->DraggedNowPixels();

	pHook->SetPointSize( pHook->PixelSize() + WgSize(dragged.x,dragged.y) );
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