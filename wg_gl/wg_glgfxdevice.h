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
#ifndef WG_GLGFXDEVICE_DOT_H
#define WG_GLGFXDEVICE_DOT_H

#ifndef WG_GEO_DOT_H
#	include <wg_geo.h>
#endif

#ifdef WIN32
#	include <gl/glew.h>
#	include <gl/gl.h>
#else
#	ifdef __APPLE__
#		include <OpenGL/gl3.h>
#	else
#		include <GLES3/gl3.h>
#	endif
#endif

#ifndef	WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif


class WgGlGfxDevice : public WgGfxDevice
{
public:
    WgGlGfxDevice( WgSize canvas );
    ~WgGlGfxDevice();

	void    SetViewportOffset( WgCoord ofs );
    bool    SetCanvas( WgSize dimensions );
    bool    SetCanvas( WgSurface * pCanvas );

    void	SetTintColor( WgColor color );
    bool	SetBlendMode( WgBlendMode blendMode );

    //

    bool	BeginRender();
    bool	EndRender();

	void    Clear( WgColor col );
	
    void	Fill( const WgRect& rect, const WgColor& col );

    void	ClipDrawHorrLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col );
    void	ClipDrawVertLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col );
    void	ClipPlotSoftPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor& col, float thickness );

    
    void    PlotPixels( int nCoords, const WgCoord * pCoords, const WgColor * pColors);
    void    ClipPlotPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor * pColors);
	
    void	DrawLine( WgCoord begin, WgCoord end, WgColor color, float thickness = 1.f );
    void	ClipDrawLine( const WgRect& clip, WgCoord begin, WgCoord end, WgColor color, float thickness = 1.f );

	void	ClipDrawHorrWave(const WgRect& clip, WgCoord begin, int length, const WgWaveLine& topLine, const WgWaveLine& bottomLine, WgColor front, WgColor back);

    void	DrawArcNE( const WgRect& rect, WgColor color );
    void	DrawElipse( const WgRect& rect, WgColor color );
    void	DrawFilledElipse( const WgRect& rect, WgColor color );

    void	ClipDrawArcNE( const WgRect& clip, const WgRect& rect, WgColor color );
    void	ClipDrawElipse( const WgRect& clip, const WgRect& rect, WgColor color );
    void	ClipDrawFilledElipse( const WgRect& clip, const WgRect& rect, WgColor color );




    void	Blit( const WgSurface * src, const WgRect& srcrect, int dx, int dy  );

    void	StretchBlitSubPixel( const WgSurface * pSrc, float sx, float sy, float sw, float sh,
                                 float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias );




    void	FillSubPixel( const WgRectF& rect, const WgColor& col );

    void    StretchBlitSubPixelWithInvert(const WgSurface * pSrc, float sx, float sy, float sw, float sh,
            float dx, float dy, float dw, float dh);

protected:

    void	_initTables();
    void	_setBlendMode( WgBlendMode blendMode );

    GLuint  _createGLProgram( const char * pVertexShader, const char * pFragmentShader );
    void    _updateProgramDimensions();
    bool    _setFramebuffer();   
    
    bool	m_bRendering;

    float	m_lineThicknessTable[17];

    GLuint  m_framebufferId;

    WgSize  m_defaultFramebufferSize;

    // Device programs
    
    GLuint  m_fillProg;
    GLint   m_fillProgColorLoc;

    GLuint  m_aaFillProg;
    GLint   m_aaFillProgColorLoc;
    GLint   m_aaFillProgFrameLoc;
    GLint   m_aaFillProgOutsideAALoc;

    GLuint  m_blitProg;
    GLint   m_blitProgTintLoc;
    GLint   m_blitProgTexIdLoc;
    
    GLuint  m_plotProg;
    GLint   m_plotProgTintLoc;

    GLuint  m_mildSlopeProg;
    GLint   m_mildSlopeProgColorLoc;
    GLint   m_mildSlopeProgSLoc;
    GLint   m_mildSlopeProgWLoc;
    GLint   m_mildSlopeProgSlopeLoc;

    GLuint  m_steepSlopeProg;
    GLint   m_steepSlopeProgColorLoc;
    GLint   m_steepSlopeProgSLoc;
    GLint   m_steepSlopeProgWLoc;
    GLint   m_steepSlopeProgSlopeLoc;
    
	GLuint  m_horrWaveProg;
	GLuint	m_horrWaveBufferTexture;
	GLuint	m_horrWaveBufferTextureData;
	GLint	m_horrWaveProgTexIdLoc;
	GLint	m_horrWaveProgWindowOfsLoc;
	GLint	m_horrWaveProgTopBorderColorLoc;
	GLint	m_horrWaveProgBottomBorderColorLoc;
	GLint	m_horrWaveProgFrontFillLoc;
	GLint	m_horrWaveProgBackFillLoc;

    GLuint  m_vertexArrayId;
    GLuint  m_vertexBufferId;
    GLfloat m_vertexBufferData[8];         // Space to store a quad (through triangle strip)
    
    GLuint  m_texCoordArrayId;
    GLuint  m_texCoordBufferId;
    GLfloat m_texCoordBufferData[8];         // Space to store UV for a quad
    
    
    // GL states saved between BeginRender() and EndRender().

    GLboolean	m_glDepthTest;
    GLboolean   m_glScissorTest;
    GLboolean	m_glBlendEnabled;
    GLint		m_glBlendSrc;
    GLint		m_glBlendDst;
	GLint		m_glViewport[4];
	GLint		m_glScissorBox[4];
    GLint       m_glReadFrameBuffer;
    GLint       m_glDrawFrameBuffer;

	//
	
	WgSize		m_size;
	WgCoord     m_viewportOffset;
	
};

#endif //WG_GLGFXDEVICE_DOT_H

