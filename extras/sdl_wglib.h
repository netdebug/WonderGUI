
#ifdef WIN32
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

class WgEventHandler;
class WgSurface;
class WgSurfaceFactory;
class WgResDB;
class WgFont;
class WgPixelFormat;

namespace sdl_wglib
{
	void 		MapKeys();
	void 		BeginEvents( WgEventHandler * pHandler );
	void 		TranslateEvent( SDL_Event& event );
	void 		EndEvents();

	WgSurface * LoadSurface( const char * path, const WgSurfaceFactory& factory );

	WgResDB *	LoadStdWidgets( const char * pImagePath, const char * pImagePathX2, const char * pImagePathX4, const WgSurfaceFactory& factory );

	WgFont *	LoadBitmapFont( const char * pImgPath, const char * pSpecPath, const WgSurfaceFactory& factory );
	
	int 		FileSize( const char * pPath );
	void * 		LoadFile( const char * pPath );

	void		ConvertPixelFormat( WgPixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat );
	
};
