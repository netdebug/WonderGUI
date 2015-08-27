
#include <cstdlib>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <wondergui.h>

#include <wg_softsurface.h>
#include <wg_softgfxdevice.h>

using namespace wg;

void 			translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot );
WgMouseButton 	translateMouseButton( Uint8 button );
void 			updateWindowRects( const RootPanel_p& pRoot, SDL_Window * pWindow );
void 			myButtonClickCallback( const Msg_p& pMsg );
void * 			loadFile( const char * pPath );

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter );


bool	bQuit = false;


void freeSDLSurfCallback( void * pSDLSurf )
{
	SDL_FreeSurface( (SDL_Surface*) pSDLSurf );
}

InputHandler * pDebug;

//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 320, height = 240;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0);

	SDL_Surface * pWinSurf = SDL_GetWindowSurface( pWin );

	IMG_Init( IMG_INIT_JPG | IMG_INIT_PNG );

	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------

	Base::init();

	InputHandler_p pInput = InputHandler::create();
	pDebug = pInput.rawPtr();
	
	WgPixelType type = WG_PIXEL_UNKNOWN;

	if( pWinSurf->format->BitsPerPixel == 32 )
		type = WG_PIXEL_ARGB_8;
	else if( pWinSurf->format->BitsPerPixel == 24 )
		type = WG_PIXEL_RGB_8;
		
	SoftSurface_p pCanvas = SoftSurface::create( Size(pWinSurf->w,pWinSurf->h), type, (unsigned char*) pWinSurf->pixels, pWinSurf->pitch, 0 );

	SoftGfxDevice_p pGfxDevice = SoftGfxDevice::create( pCanvas );

	RootPanel_p pRoot = RootPanel::create( pGfxDevice );
	Base::msgRouter()->setRoot(pRoot);

	// 
	
	MsgLogger_p pLogger = MsgLogger::create( std::cout );
	pLogger->ignoreAllMsgs();
	pLogger->logMouseMsgs();
	
	Base::msgRouter()->broadcastTo( pLogger );


	// Init font
	
	SDL_Surface * pFontSurf = IMG_Load( "../../../resources/anuvverbubbla_8x8.png" );
	Blob_p pFontSurfBlob = Blob::create( pFontSurf, freeSDLSurfCallback );
	SoftSurface_p pFontImg = SoftSurface::create( Size(pFontSurf->w,pFontSurf->h), WG_PIXEL_ARGB_8, (unsigned char*) pFontSurf->pixels, pFontSurf->pitch, pFontSurfBlob );
		
	char * pFontSpec = (char*) loadFile( "../../../resources/anuvverbubbla_8x8.fnt" );

	BitmapGlyphs_p pGlyphs = BitmapGlyphs::create( pFontImg, pFontSpec );
	Font_p pFont = Font::create( pGlyphs, 8 );

	Textprop prop;
	prop.setFont(pFont);
	prop.setSize(8);
	Textprop_p pProp = prop.reg();

	Base::setDefaultTextprop( pProp );

	TextStyle_p pStyle = TextStyle::create();
	pStyle->setFont(pFont);
	pStyle->setSize(8);
	Base::setDefaultStyle(pStyle);

	// Init skins

	SDL_Surface * pSDLSurf = IMG_Load( "../../../resources/simple_button.bmp" );
	SoftSurface_p pButtonSurface = SoftSurface::create( Size( pSDLSurf->w, pSDLSurf->h ), WG_PIXEL_RGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, 0 );
	BlockSkin_p pSimpleButtonSkin = BlockSkin::createClickableFromSurface( pButtonSurface, 0, Border(3) );
	pSimpleButtonSkin->setContentPadding( Border(5) );

	pSDLSurf = IMG_Load( "../../../resources/state_button.bmp" );
	SoftSurface_p pStateButtonSurface = SoftSurface::create( Size( pSDLSurf->w, pSDLSurf->h ), WG_PIXEL_RGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, 0 );
	BlockSkin_p pStateButtonSkin = BlockSkin::createClickSelectableFromSurface( pStateButtonSurface, 0, Border(3) );
	pStateButtonSkin->setContentPadding( Border(5) );

	pSDLSurf = IMG_Load( "../../../resources/grey_pressable_plate.bmp" );
	SoftSurface_p pPressablePlateSurface = SoftSurface::create( Size( pSDLSurf->w, pSDLSurf->h ), WG_PIXEL_RGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, 0 );
	Skin_p pPressablePlateSkin = BlockSkin::createClickableFromSurface( pPressablePlateSurface, 0, Border(3) );
	
	pSDLSurf = IMG_Load( "../../../resources/list_entry.png" );
	SoftSurface_p pListEntrySurface = SoftSurface::create( Size( pSDLSurf->w, pSDLSurf->h ), WG_PIXEL_RGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, 0 );
	Skin_p pListEntrySkin = BlockSkin::createClickableFromSurface( pListEntrySurface, 0, Border(3) );

	//------------------------------------------------------
	// Setup a simple GUI consisting of a filled background and 
	// a button using scaled bitmaps.
	//------------------------------------------------------

	FlexPanel_p pFlexPanel = FlexPanel::create();
	pFlexPanel->setSkin( ColorSkin::create(Color::burlywood));
	pRoot->setWidget(pFlexPanel);


/*	Filler_p pBackground = Filler::create();
	pBackground->setSkin( ColorSkin::create(Color::aqua) );
	pFlexPanel->addWidget(pBackground, WG_NORTHWEST, Coord(), WG_SOUTHEAST, Coord());
*/

	Image_p pImage = Image::create();
	pImage->setSkin( pSimpleButtonSkin );
	pFlexPanel->addWidget( pImage, Rect(0,0,80,33), WG_CENTER );


//	pRoot->msgRouter()->AddCallback( MsgFilter::select(), pButton, myButtonClickCallback );


	// Test transparency issue
	
	{
		FlexPanel_p pExtraFlex = FlexPanel::create();
		pExtraFlex->setSkin( ColorSkin::create( Color(0,0,0,128)));

		TextDisplay_p pText = TextDisplay::create();
		pText->text.set( "THIS IS SOME TEST TEXT" );
		
		pExtraFlex->addWidget( pText, Rect( 10,10,100,100) );

		addResizablePanel( pFlexPanel, pExtraFlex, Base::msgRouter() );
	}



/*
	SizeCapsule_p pCapsule = SizeCapsule::create();
	pCapsule->setMaxSize( Size(100,1000));
	pFlexPanel->addWidget( pCapsule );

	StackPanel_p pStack = StackPanel::create();
	pCapsule->setWidget( pStack );


	TextDisplay_p pText = TextDisplay::create();
	pText->text()->set( "THIS IS THE LONG TEXT THAT SHOULD WRAP AND BE FULLY DISPLAYED." );
	pStack->addWidget(pText);
*/
	
/*
	CheckBoxPtr pCheckbox = CheckBox::create();
	pCheckbox->Label()->set( "CHECKBOX" );
	pCheckbox->Icon()->set( pCheckboxSkin );
*/

/*
	ToggleGroup_p pGroup = ToggleGroup::create();
	

	for( int i = 0 ; i < 4 ; i++ )
	{
		ToggleButton_p pCheckbox = ToggleButton::create();
		pCheckbox->label.set( "CHECKBOX" );
		pCheckbox->setSkin( pStateButtonSkin );
		pFlexPanel->addWidget( pCheckbox, Coord(10,20*i) );
		pGroup->add( pCheckbox );
	}
*/

/*
	TextDisplay_p pText = TextDisplay::create();
	pText->text.set( "THIS IS THE TEXT\nLINE TWO.\nAN EXTRA LONG LONG LONG LINE TO TEST THE CLIPPING AND RAND BEHAVIOURS." );
	pFlexPanel->addWidget( pText, WG_NORTHWEST, WG_EAST );
	

	StandardPresenter_p pPresenter = StandardPresenter::create();
	pPresenter->setAlignment( WG_CENTER );
	pText->text.setPresenter( pPresenter );


	ValueDisplay_p pValue = ValueDisplay::create();
	pValue->value.setFormatter( TimeFormatter::create("%2H:%2M:%2S"));
	pValue->value.set(3600+60+12);
	pValue->value.setPresenter( pPresenter );
	pFlexPanel->addWidget( pValue, WG_WEST, WG_SOUTHEAST );
*/

	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	while( !bQuit ) 
	{
		translateEvents( pInput, pRoot );

		SDL_LockSurface(pWinSurf);
		pRoot->render();
		SDL_UnlockSurface(pWinSurf);

		updateWindowRects( pRoot, pWin );

		SDL_Delay(20);
    }

	// Exit WonderGUI

	Base::exit();

	// Exit SDL

	IMG_Quit();
	SDL_Quit();

    return 0;
}

//____ translateEvents() ___________________________________________________________

void translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot )
{
	// WonderGUI needs Tick-events to keep track of time passed for things such
	// key-repeat, double-click detection, animations etc.  So we create one
	// and put it on the event queue.
	
	static unsigned int oldTicks = 0;
	
	unsigned int ticks = SDL_GetTicks();
	int tickDiff;

	if( oldTicks == 0 )
		tickDiff = 0;
	else
		tickDiff = (int) (ticks - oldTicks);		
	oldTicks = ticks;

	Base::msgRouter()->post( TickMsg::create(tickDiff) );

	// Process all the SDL events in a loop

	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		
		switch( e.type )
		{
			case SDL_QUIT:
				bQuit = true;
				break;
				
			case SDL_MOUSEMOTION:
				pInput->setPointer( pRoot, Coord(e.motion.x,e.motion.y) );
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				pInput->setButton( translateMouseButton(e.button.button), true );
				break;

			case SDL_MOUSEBUTTONUP:
				pInput->setButton( translateMouseButton(e.button.button), false );
				break;
				
			default:
				break;
		}
	}
	
	Base::msgRouter()->dispatch();	
}

//____ translateMouseButton() __________________________________________________
//
// Translate SDL mouse button enums to WonderGUI equivalents.
//
WgMouseButton translateMouseButton( Uint8 button )
{
	switch( button )
	{
		default:
		case SDL_BUTTON_LEFT:
			return WG_BUTTON_LEFT;
		case SDL_BUTTON_MIDDLE:
			return WG_BUTTON_MIDDLE;
		case SDL_BUTTON_RIGHT:
			return WG_BUTTON_RIGHT;
		case SDL_BUTTON_X1:
			return WG_BUTTON_X1;
		case SDL_BUTTON_X2:
			return WG_BUTTON_X2;
	}
}

//____ updateWindowRects() _____________________________________________________
//
// Updates the rectangles of the SDL Window that WonderGUI has modified.
//
void updateWindowRects( const RootPanel_p& pRoot, SDL_Window * pWindow )
{	
	int nRects = pRoot->nbUpdatedRects();
	if( nRects == 0 )
		return;
	
	const Rect * pUpdatedRects = pRoot->firstUpdatedRect();
	SDL_Rect * pSDLRects = (SDL_Rect*) Base::memStackAlloc( sizeof(SDL_Rect) * nRects );

	for( int i = 0 ; i < nRects ; i++ )
	{
		pSDLRects[i].x = pUpdatedRects[i].x;
		pSDLRects[i].y = pUpdatedRects[i].y;
		pSDLRects[i].w = pUpdatedRects[i].w;
		pSDLRects[i].h = pUpdatedRects[i].h;
	}

	SDL_UpdateWindowSurfaceRects( pWindow, pSDLRects, nRects );

	Base::memStackRelease( sizeof(SDL_Rect) * nRects );
}

//____ myButtonClickCallback() _________________________________________________

void myButtonClickCallback( const Msg_p& pMsg )
{
	bQuit = true;
}


//____ loadFile() _____________________________________________________________

void * loadFile( const char * pPath )
{
	FILE * fp = fopen( pPath, "rb" );
	if( !fp )
		return 0;

	fseek( fp, 0, SEEK_END );
	int size = ftell(fp);
	fseek( fp, 0, SEEK_SET );

	char * pMem = (char*) malloc( size+1 );
	pMem[size] = 0;
	int nRead = fread( pMem, 1, size, fp );
	fclose( fp );

	if( nRead < size )
	{
		free( pMem );
		return 0;
	}

	return pMem;

}

Coord dragStartPos;

//____ cbInitDrag() ___________________________________________________________

void cbInitDrag( const Msg_p& _pMsg, const Object_p& pObject )
{
	Widget_p pWidget = Widget::cast(pObject);

	FlexHook_p pHook = FlexHook::cast(pWidget->hook());


	dragStartPos = pHook->floatOfs();
	printf( "DRAG START!\n" );
}

//____ cbDragWidget() __________________________________________________________

void cbDragWidget( const Msg_p& _pMsg, const Object_p& pObject )
{
	Widget_p pWidget = Widget::cast(pObject);
	
	if( _pMsg->type() != WG_MSG_MOUSE_DRAG || !pWidget->parent() )
		return;

	const MouseDragMsg_p pMsg = MouseDragMsg::cast(_pMsg);



	Coord	dragDistance = pMsg->draggedTotal();

	Coord	ofs = dragStartPos + dragDistance;

//	printf( "AccDistance: %d, %d\n", dragDistance.x, dragDistance.y );
	printf( "ofs: %d, %d   start: %d %d   distance: %d, %d\n", ofs.x, ofs.y, dragStartPos.x, dragStartPos.y, dragDistance.x, dragDistance.y );

	FlexHook_p pHook = FlexHook::cast(pWidget->hook());
	pHook->setOfs(dragStartPos+dragDistance);
}


//____ cbResizeWidget() _________________________________________________________
/*
void cbResize( const Msg_p _pMsg, Object_p _pFlexHook )
{
	FlexHook * pHook = static_cast<FlexHook*>(_pFlexHook);
	const Msg::MouseButtonDrag* pMsg = static_cast<const Msg::MouseButtonDrag*>(_pMsg);

	Coord dragged = pMsg->draggedNow();

	pHook->setSize( pHook->size() + Size(dragged.x,dragged.y) );
}
*/



//____ addResizablePanel() _________________________________________________

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter )
{
	Hook * pHook = pParent->addWidget( pChild );
//	pMsgRouter->AddCallback( MsgFilter::MouseButtonDrag(pChild, 3), cbResize, pHook );

	pMsgRouter->addRoute( MsgFilter::mousePress(WG_BUTTON_LEFT), pChild, MsgFunc::create(cbInitDrag, pChild) );
	pMsgRouter->addRoute( MsgFilter::mouseDrag(WG_BUTTON_LEFT), pChild, MsgFunc::create(cbDragWidget, pChild) );
}