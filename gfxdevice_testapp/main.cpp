// gfxdevice_testapp.cpp : Defines the entry point for the console application.
//

#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#else
#	include <SDL2/SDL.h>
#	include <SDL2/SDL_image.h>
#endif

#include <wondergui.h>
#include <fonts/freetype/wg_freetypefont.h>
#include <gfxdevices/software/wg_softgfxdevice.h>

using namespace wg;


bool		init_system( Rect windowGeo );
void		exit_system();

Surface_p	loadSurface(const char * pPath);
bool		process_system_events(const RootPanel_p& pRoot);
int64_t		getSystemTicks();
void		update_window_rects( const Rect * pRects, int nRects );

bool		init_wondergui();
void		exit_wondergui();

bool		setup_chrome();
void		teardown_chrome();



MouseButton translateSDLMouseButton(Uint8 button);
void convertSDLFormat(PixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat);


SDL_Window *	g_pSDLWindow = nullptr;

Surface_p		g_pWindowSurface = nullptr;				// Set by init_system()
GfxDevice_p		g_pBaseGfxDevice = nullptr;
SurfaceFactory_p	g_pBaseSurfaceFactory = nullptr;
RootPanel_p		g_pRoot = nullptr;



//____ main() _________________________________________________________________

int main(int argc, char *argv[] )
{

	if (!init_system({ 20,20,1400, 900 }))
		return -1;
	
	if (!init_wondergui() )
		return -1;

	if (!setup_chrome())
		return -1;


	bool bContinue = true;

	int64_t prevTicks = getSystemTicks();

	while (bContinue)
	{
		// Handle system events

		bContinue = process_system_events(g_pRoot);
		Base::msgRouter()->dispatch();

		// Render

		g_pRoot->render();

		update_window_rects(g_pRoot->firstUpdatedRect(), g_pRoot->nbUpdatedRects());

		//

		SDL_Delay(10);

		// Update time

		int64_t ticks = getSystemTicks();
		Base::msgRouter()->post(TickMsg::create(ticks, (int)(ticks-prevTicks)));
		prevTicks = ticks;
	}

	teardown_chrome();
	exit_wondergui();
	exit_system();
	return 0;
}

//____ load_surface() _________________________________________________________

Surface_p load_surface(const char * pPath)
{
	PixelFormat format;

	auto pSDLSurf = IMG_Load(pPath);
	convertSDLFormat(&format, pSDLSurf->format);
	auto pSurface = g_pBaseSurfaceFactory->createSurface(Size(pSDLSurf->w, pSDLSurf->h), PixelType::BGR_8, (unsigned char*)pSDLSurf->pixels, pSDLSurf->pitch, &format);
	SDL_FreeSurface(pSDLSurf);
	return pSurface;
}


//____ update_window_rects() __________________________________________________

void update_window_rects(const Rect * pRects, int nRects)
{
	if (nRects == 0)
		return;

	std::vector<SDL_Rect>	rects;

	for (int i = 0; i < nRects; i++)
	{
		SDL_Rect r = { pRects[i].x, pRects[i].y, pRects[i].w, pRects[i].h };
		rects.push_back(r);
	}

	SDL_UpdateWindowSurfaceRects(g_pSDLWindow, &rects.front(), nRects);
}


//____ setup_chrome() _________________________________________________________

bool setup_chrome()
{
	auto pPlateSurface = load_surface("../resources/grey_plate.bmp");
	BlockSkin_p pPlateSkin = BlockSkin::createStaticFromSurface(pPlateSurface, Border(3));
	pPlateSkin->setContentPadding(Border(3));

	auto pPressablePlateSurface = load_surface("../resources/grey_pressable_plate.bmp");
	BlockSkin_p pPressablePlateSkin = BlockSkin::createClickableFromSurface(pPressablePlateSurface, 0, Border(3));
	pPressablePlateSkin->setContentPadding(Border(3));

	auto pButtonSurface = load_surface("../resources/simple_button.bmp");
	BlockSkin_p pSimpleButtonSkin = BlockSkin::createClickableFromSurface(pButtonSurface, 0, Border(3));
	pSimpleButtonSkin->setContentPadding(Border(5));

	auto pLayerStack = StackPanel::create();
	pLayerStack->setSkin(ColorSkin::create(Color::AntiqueWhite));
	g_pRoot->child = pLayerStack;

	auto pMidSection = PackPanel::create();
	pMidSection->setOrientation(Orientation::Horizontal);
	pMidSection->setSizeBroker(UniformSizeBroker::create());
	auto it = pLayerStack->children << pMidSection;
	pLayerStack->children.setSizePolicy(it, SizePolicy2D::Stretch);

	auto pSidebar = Button::create();
	pSidebar->setSkin(pPlateSkin);

	auto pCanvas = ScrollPanel::create();
	pCanvas->setSkin( ColorSkin::create(Color::Black) );

	pMidSection->children << pSidebar;
	pMidSection->children << pCanvas;

	pMidSection->children.setWeight(0, 0.f);
	pMidSection->children.setWeight(1, 1.f);




	return true;
}

//____ teardown_chrome() ______________________________________________________

void teardown_chrome()
{
}


//____ init_wondergui() _______________________________________________________

bool init_wondergui()
{
	Base::init();
	FreeTypeFont::init(SoftSurfaceFactory::create());

	g_pBaseGfxDevice = SoftGfxDevice::create( SoftSurface::cast(g_pWindowSurface));
	g_pRoot = RootPanel::create(g_pBaseGfxDevice);

	g_pBaseSurfaceFactory = SoftSurfaceFactory::create();


	return true;
}

//____ exit_wondergui() _______________________________________________________

void exit_wondergui()
{
	FreeTypeFont::exit();
	Base::exit();
}


//____ init_system() _______________________________________________________

bool init_system( Rect windowGeo )
{
	// initialize SDL video
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		return false;
	}

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	SDL_Window * pWin = SDL_CreateWindow("GfxDevice TestApp", windowGeo.x, windowGeo.y, windowGeo.w, windowGeo.h, SDL_WINDOW_ALLOW_HIGHDPI);
	if( pWin == nullptr )
	{
		printf("Unable to create SDL window: %s\n", SDL_GetError());
		return false;
	}


	SDL_Surface * pWinSurf = SDL_GetWindowSurface(pWin);
	if (pWinSurf == nullptr)
	{
		printf("Unable to get window SDL Surface: %s\n", SDL_GetError());
		return false;
	}

	PixelType type = PixelType::Unknown;

	switch (pWinSurf->format->BitsPerPixel)
	{
		case 32:
			type = PixelType::BGRA_8;
			break;
		case 24:
			type = PixelType::BGR_8;
			break;
		default:
		{
			printf("Unsupported pixelformat of SDL Surface!\n");
			return false;
		}
	}

	g_pSDLWindow = pWin;

	Blob_p pCanvasBlob = Blob::create(pWinSurf->pixels, 0);
	g_pWindowSurface = SoftSurface::create(Size(pWinSurf->w, pWinSurf->h), type, pCanvasBlob, pWinSurf->pitch);

	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	return true;
}

//____ exit_system() __________________________________________________________

void exit_system()
{
	IMG_Quit();
}

//____ getSystemTicks() ________________________________________________________

int64_t getSystemTicks()
{
	return SDL_GetPerformanceCounter() * 1000 / SDL_GetPerformanceFrequency();
}


//____ process_system_events() ________________________________________________

bool process_system_events(const RootPanel_p& pRoot)
{
	// Process all the SDL events in a loop

	InputHandler_p pInput = Base::inputHandler();

	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		switch (e.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_MOUSEMOTION:
			pInput->setPointer(pRoot, Coord(e.motion.x, e.motion.y));
			break;

		case SDL_MOUSEBUTTONDOWN:
			pInput->setButton(translateSDLMouseButton(e.button.button), true);
			break;

		case SDL_MOUSEBUTTONUP:
			pInput->setButton(translateSDLMouseButton(e.button.button), false);
			break;

		case SDL_MOUSEWHEEL:
		{
			Coord distance(e.wheel.x, e.wheel.y);
			if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
				distance *= -1;

			pInput->setWheelRoll(1, distance);
			break;
		}

		case SDL_KEYDOWN:
		{
			pInput->setKey(e.key.keysym.sym, true);
			break;
		}

		case SDL_KEYUP:
		{
			pInput->setKey(e.key.keysym.sym, false);
			break;
		}

		case SDL_TEXTINPUT:
			pInput->putText(e.text.text);
			break;


		default:
			break;
		}
	}

	return true;
}

//____ translateSDLMouseButton() __________________________________________________
//
// Translate SDL mouse button enums to WonderGUI equivalents.
//
MouseButton translateSDLMouseButton(Uint8 button)
{
	switch (button)
	{
	default:
	case SDL_BUTTON_LEFT:
		return MouseButton::Left;
	case SDL_BUTTON_MIDDLE:
		return MouseButton::Middle;
	case SDL_BUTTON_RIGHT:
		return MouseButton::Right;
	case SDL_BUTTON_X1:
		return MouseButton::X1;
	case SDL_BUTTON_X2:
		return MouseButton::X2;
	}
}

//____ convertSDLFormat() ______________________________________________________

void convertSDLFormat(PixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat)
{
	pWGFormat->type = PixelType::Custom;
	pWGFormat->bits = pSDLFormat->BitsPerPixel;

	pWGFormat->R_mask = pSDLFormat->Rmask;
	pWGFormat->G_mask = pSDLFormat->Gmask;
	pWGFormat->B_mask = pSDLFormat->Bmask;
	pWGFormat->A_mask = pSDLFormat->Amask;

	pWGFormat->R_shift = pSDLFormat->Rshift;
	pWGFormat->G_shift = pSDLFormat->Gshift;
	pWGFormat->B_shift = pSDLFormat->Bshift;
	pWGFormat->A_shift = pSDLFormat->Ashift;

	pWGFormat->R_bits = 8 - pSDLFormat->Rloss;
	pWGFormat->G_bits = 8 - pSDLFormat->Gloss;
	pWGFormat->B_bits = 8 - pSDLFormat->Bloss;
	pWGFormat->A_bits = 8 - pSDLFormat->Aloss;

}