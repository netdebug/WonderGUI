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

#include <wg_glgfxdevice.h>
#include <wg_glsurface.h>
#include <wg_glsurfacefactory.h>
#include <wg_base.h>
#include <assert.h>
#include <math.h>
#include <algorithm>

#ifdef __APPLE___
#	include <OpenGL/glu.h>
#endif

#ifdef WIN32
#	include <windows.h>
#	include <gl/glu.h>
#endif

using namespace std;


//____ Vertex and Fragment shaders ____________________________________________

const char fillVertexShader[] =

"#version 330 core\n"
"uniform vec2 dimensions;                                  "
"layout(location = 0) in vec2 pos;                          "
"void main()                                                "
"{                                                          "
"   gl_Position.x = pos.x*2/dimensions.x - 1.0;             "
"   gl_Position.y = pos.y*2/dimensions.y - 1.0;             "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"}                                                          ";


const char fillFragmentShader[] =

"#version 330 core\n"
"uniform vec4 color;                    "
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor = color;                   "
"}                                      ";

const char horrWaveFragmentShader[] =

"#version 330 core\n"
"layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;"
"uniform isamplerBuffer texId;          "
"uniform vec2 windowOfs;				"
"uniform vec4 topBorderColor;           "
"uniform vec4 bottomBorderColor;        "
"uniform vec4 frontColor;               "
"uniform vec4 backColor;                "
"out vec4 outColor;                     "
"void main()                            "
"{										"
"	vec2 ofs = gl_FragCoord.xy - windowOfs;"
"   int column = int(ofs.x)*9;			"
"  float aFrac = texelFetch(texId, column).r/65536.0;"
"  float aInc = texelFetch(texId, column+1).r/65536.0;"
"  float dFrac = texelFetch(texId, column+6).r/65536.0;"
"  float dInc = texelFetch(texId, column+7).r/65536.0;"

"  aFrac = clamp(aFrac + aInc*ofs.y,0.0,1.0);"
"  dFrac = clamp(dFrac + dInc*ofs.y,0.0,1.0);"

"  if (aFrac == 0.0 || dFrac == 1.0) "
"	discard; "

"  float bFrac = texelFetch(texId, column+2).r/65536.0;"
"  float bInc = texelFetch(texId, column+3).r/65536.0;"
"  float cFrac = texelFetch(texId, column+4).r/65536.0;"
"  float cInc = texelFetch(texId, column+5).r/65536.0;"
"  int bFlipped = texelFetch(texId, column+8).r;"

"  bFrac = clamp(bFrac + bInc*ofs.y,0.0,1.0);"
"  cFrac = clamp(cFrac + cInc*ofs.y,0.0,1.0);"


" aFrac -= bFrac;"
" bFrac -= cFrac;"
" cFrac -= dFrac;"
" dFrac = 0.0;"

" if(bFlipped != 0) "
" { "
"	float x = aFrac;"
"	aFrac = cFrac;"
"	cFrac = x;"
"	dFrac = bFrac;"
"	bFrac = 0.0;"
" }"

"  aFrac *= topBorderColor.a;"
"  bFrac *= frontColor.a;"
"  cFrac *= bottomBorderColor.a;"
"  dFrac *= backColor.a;"

"  float totalAlpha = aFrac + bFrac + cFrac + dFrac;"
"  aFrac /= totalAlpha; "
"  bFrac /= totalAlpha; "
"  cFrac /= totalAlpha; "
"  dFrac /= totalAlpha; "

"   outColor.a = totalAlpha;  "
"   outColor.r = aFrac*topBorderColor.r + bFrac*frontColor.r + cFrac*bottomBorderColor.r + dFrac*backColor.r;  "
"   outColor.g = aFrac*topBorderColor.g + bFrac*frontColor.g + cFrac*bottomBorderColor.g + dFrac*backColor.g;  "
"   outColor.b = aFrac*topBorderColor.b + bFrac*frontColor.b + cFrac*bottomBorderColor.b + dFrac*backColor.b;  "
"}                                      ";


/* Original, unoptimized versions
 
 const char mildSlopeFragmentShader[] =
 
 "#version 330 core\n"
 "uniform vec4 color;                    "
 "uniform vec2 start;                   "
 "uniform float slope;                   "
 "uniform float width;                   "
 "out vec4 outColor;                     "
 "void main()                            "
 "{                                      "
 "   float center = start.y - (gl_FragCoord.x - start.x) * slope;    "
 "   float edgeDist = width - abs(gl_FragCoord.y - center); "        // Distance to line edge from fragCoords perspective, += fragCoord is inside line.
 "   float a = clamp(edgeDist+0.5, 0.0, 1.0);"
 "   outColor = color * a;               "
 "}                                      ";
 
 const char steepSlopeFragmentShader[] =
 
 "#version 330 core\n"
 "uniform vec4 color;                    "
 "uniform vec2 start;                   "
 "uniform float slope;                   "
 "uniform float width;                   "
 "out vec4 outColor;                     "
 "void main()                            "
 "{                                      "
 "   float center = start.x + (start.y - gl_FragCoord.y) * slope;    "
 "   float edgeDist = width - abs(gl_FragCoord.x - center); "        // Distance to line edge from fragCoords perspective, += fragCoord is inside line.
 "   float a = clamp(edgeDist+0.5, 0.0, 1.0);"
 "   outColor = color * a;               "
 "}                                      ";
 */


const char steepSlopeFragmentShader[] =

"#version 330 core\n"
"uniform vec4 color;                    "
"uniform float s;                       "       // start.x + start.y*slope
"uniform float slope;                   "
"uniform float w;                       "       // Horizontal line segment width + 0.5
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor.rgb = color.rgb;           "
"   outColor.a = color.a * clamp(w - abs(gl_FragCoord.x - s + gl_FragCoord.y * slope), 0.0, 1.0); "
"}                                      ";


const char mildSlopeFragmentShader[] =

"#version 330 core\n"
"uniform vec4 color;                    "
"uniform float s;                       "       // start.y + start.x*slope
"uniform float slope;                   "
"uniform float w;                       "       // Vertical line segment width + 0.5
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor.rgb = color.rgb;           "
"   outColor.a = color.a * clamp(w - abs(gl_FragCoord.y - s + gl_FragCoord.x * slope), 0.0, 1.0); "
"}                                      ";


const char aaFillFragmentShader[] =

"#version 330 core\n"
"uniform vec4 color;                    "
"uniform vec4 frame;                    "
"uniform vec4 outsideAA;                "
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   vec4 col = color;                   "
"   if( gl_FragCoord.x < frame.x )      "
"        col.a *= outsideAA.x;       "
"   if( gl_FragCoord.y > frame.y )      "
"        col.a *= outsideAA.y;       "
"   if( gl_FragCoord.x > frame.z )      "
"        col.a *= outsideAA.z;       "
"   if( gl_FragCoord.y < frame.w )      "
"        col.a *= outsideAA.w;       "
"   outColor = col;                     "
"}                                      ";



const char blitVertexShader[] =

"#version 330 core\n"
"uniform vec2 dimensions;                                   "
"layout(location = 0) in vec2 pos;                          "
"in vec2 texPos;                                             "
"out vec2 texUV;                                             "
"void main()                                                "
"{                                                          "
"   gl_Position.x = pos.x*2/dimensions.x - 1.0;             "
"   gl_Position.y = pos.y*2/dimensions.y - 1.0;             "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   texUV = texPos;                                         "
"}                                                          ";


const char blitFragmentShader[] =

"#version 330 core\n"

"uniform vec4 tint;                     "
"uniform sampler2D texId;               "
"in vec2 texUV;                         "
"out vec4 color;                        "
"void main()                            "
"{                                      "
"   color = texture(texId, texUV) * tint;     "
"}                                      ";

const char plotVertexShader[] =

"#version 330 core\n"
"uniform vec2 dimensions;                                  "
"layout(location = 0) in ivec2 pos;                         "
"in vec4 inColor;                                       "
"out vec4 color;"
"void main()                                                "
"{                                                          "
"   gl_Position.x = (pos.x+0.5)*2.0/dimensions.x - 1.0;             "
"   gl_Position.y = 1.0 -(pos.y+0.5)*2.0/dimensions.y;             "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   color = inColor;"
"}                                                          ";


const char plotFragmentShader[] =

"#version 330 core\n"
"uniform vec4 tint;                     "
"in vec4 color;                         "
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor = color * tint;            "
"}                                      ";





//____ Constructor _____________________________________________________________

WgGlGfxDevice::WgGlGfxDevice( WgSize canvas ) : WgGfxDevice(canvas)
{
    m_bRendering = false;

    _initTables();
    
    m_fillProg = _createGLProgram( fillVertexShader, fillFragmentShader );
    assert( glGetError() == 0 );
    m_fillProgColorLoc = glGetUniformLocation( m_fillProg, "color");
    
    GLint err = glGetError();
    assert( err == 0 );
    
    m_aaFillProg = _createGLProgram( fillVertexShader, aaFillFragmentShader );
    m_aaFillProgColorLoc = glGetUniformLocation( m_aaFillProg, "color");
    
    m_aaFillProgFrameLoc = glGetUniformLocation( m_aaFillProg, "frame");
    m_aaFillProgOutsideAALoc = glGetUniformLocation( m_aaFillProg, "outsideAA");
    
    m_mildSlopeProg = _createGLProgram( fillVertexShader, mildSlopeFragmentShader );
    m_mildSlopeProgColorLoc = glGetUniformLocation( m_mildSlopeProg, "color");
    m_mildSlopeProgSlopeLoc = glGetUniformLocation( m_mildSlopeProg, "slope");
    m_mildSlopeProgSLoc     = glGetUniformLocation( m_mildSlopeProg, "s");
    m_mildSlopeProgWLoc     = glGetUniformLocation( m_mildSlopeProg, "w");
    
    m_steepSlopeProg = _createGLProgram( fillVertexShader, steepSlopeFragmentShader );
    m_steepSlopeProgColorLoc = glGetUniformLocation( m_steepSlopeProg, "color");
    m_steepSlopeProgSlopeLoc = glGetUniformLocation( m_steepSlopeProg, "slope");
    m_steepSlopeProgSLoc = glGetUniformLocation( m_steepSlopeProg, "s");
    m_steepSlopeProgWLoc = glGetUniformLocation( m_steepSlopeProg, "w");
    
    m_blitProg = _createGLProgram( blitVertexShader, blitFragmentShader );
    m_blitProgTintLoc = glGetUniformLocation( m_blitProg, "tint" );
    m_blitProgTexIdLoc = glGetUniformLocation( m_blitProg, "texId" );

	m_horrWaveProg = _createGLProgram(fillVertexShader, horrWaveFragmentShader);
	m_horrWaveProgTexIdLoc = glGetUniformLocation(m_horrWaveProg, "texId");
	m_horrWaveProgWindowOfsLoc = glGetUniformLocation(m_horrWaveProg, "windowOfs");
	m_horrWaveProgTopBorderColorLoc = glGetUniformLocation(m_horrWaveProg, "topBorderColor");
	m_horrWaveProgBottomBorderColorLoc = glGetUniformLocation(m_horrWaveProg, "bottomBorderColor");
	m_horrWaveProgFrontFillLoc = glGetUniformLocation(m_horrWaveProg, "frontColor");
	m_horrWaveProgBackFillLoc = glGetUniformLocation(m_horrWaveProg, "backColor");

    m_plotProg = _createGLProgram( plotVertexShader, plotFragmentShader );
    m_plotProgTintLoc = glGetUniformLocation( m_plotProg, "tint" );
    
    glGenVertexArrays(1, &m_vertexArrayId);
    glBindVertexArray(m_vertexArrayId);
    glGenBuffers(1, &m_vertexBufferId);
    
    glGenVertexArrays(1, &m_texCoordArrayId);
    glBindVertexArray(m_texCoordArrayId);
    glGenBuffers(1, &m_texCoordBufferId);
    glBindVertexArray(0);

    glGenFramebuffers(1, &m_framebufferId);

	glGenTextures(1, &m_horrWaveBufferTexture);
	glGenBuffers(1, &m_horrWaveBufferTextureData);

    SetCanvas( canvas );
    SetTintColor( WgColor::white );
    
    assert( glGetError() == 0 );
}

//____ Destructor ______________________________________________________________

WgGlGfxDevice::~WgGlGfxDevice()
{
    assert( glGetError() == 0 );
    glDeleteBuffers(1, &m_vertexBufferId);
    glDeleteBuffers(1, &m_texCoordBufferId);
    assert( glGetError() == 0 );
    glDeleteVertexArrays(1, &m_vertexArrayId);
    assert( glGetError() == 0 );
    glDeleteVertexArrays(1, &m_texCoordArrayId);
    assert( glGetError() == 0 );
}

//____ SetViewport() ________________________________________________________________

void WgGlGfxDevice::SetViewportOffset( WgCoord ofs )
{
    m_viewportOffset = ofs;
}



//____ SetCanvas() __________________________________________________________________

bool WgGlGfxDevice::SetCanvas( WgSize dimensions )
{
    m_pCanvas                   = nullptr;
    m_canvasSize                = dimensions; 
    m_defaultFramebufferSize    = dimensions;
    _updateProgramDimensions();

    if (m_bRendering)
        return _setFramebuffer();

    return true;
}

bool WgGlGfxDevice::SetCanvas( WgSurface * _pSurface )
{
    if (!_pSurface)
        return SetCanvas(m_defaultFramebufferSize);     // Revert back to default frame buffer.

    WgGlSurface * pSurface = static_cast<WgGlSurface*>(_pSurface);

    m_pCanvas       = pSurface;
    m_canvasSize    = pSurface->Size();
    _updateProgramDimensions();

    if (m_bRendering)
        return _setFramebuffer();

    return true;
}

//____ _setFramebuffer() ____________________________________________________

bool WgGlGfxDevice::_setFramebuffer()
{
    if (m_pCanvas)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, static_cast<WgGlSurface*>(m_pCanvas)->GetTexture(), 0);

        GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }
    }
    else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}


//____ _updateProgramDimensions() ____________________________________________

void WgGlGfxDevice::_updateProgramDimensions()
{
    glUseProgram(m_fillProg);
    GLint dimLoc = glGetUniformLocation(m_fillProg, "dimensions");
    glUniform2f(dimLoc, (GLfloat)m_canvasSize.w, (GLfloat)m_canvasSize.h);

    glUseProgram(m_aaFillProg);
    dimLoc = glGetUniformLocation(m_aaFillProg, "dimensions");
    glUniform2f(dimLoc, (GLfloat)m_canvasSize.w, (GLfloat)m_canvasSize.h);

    glUseProgram(m_mildSlopeProg);
    dimLoc = glGetUniformLocation(m_mildSlopeProg, "dimensions");
    glUniform2f(dimLoc, (GLfloat)m_canvasSize.w, (GLfloat)m_canvasSize.h);

    glUseProgram(m_steepSlopeProg);
    dimLoc = glGetUniformLocation(m_steepSlopeProg, "dimensions");
    glUniform2f(dimLoc, (GLfloat)m_canvasSize.w, (GLfloat)m_canvasSize.h);

    glUseProgram(m_blitProg);
    dimLoc = glGetUniformLocation(m_blitProg, "dimensions");
    glUniform2f(dimLoc, (GLfloat)m_canvasSize.w, (GLfloat)m_canvasSize.h);
    glUniform1i(m_blitProgTexIdLoc, 0);

    glUseProgram(m_plotProg);
    dimLoc = glGetUniformLocation(m_plotProg, "dimensions");
    glUniform2f(dimLoc, (GLfloat)m_canvasSize.w, (GLfloat)m_canvasSize.h);

	glUseProgram(m_horrWaveProg);
	dimLoc = glGetUniformLocation(m_horrWaveProg, "dimensions");
	glUniform2f(dimLoc, (GLfloat)m_canvasSize.w, (GLfloat)m_canvasSize.h);
}

//____ SetTintColor() __________________________________________________________

void WgGlGfxDevice::SetTintColor( WgColor color )
{
    assert( glGetError() == 0 );
    WgGfxDevice::SetTintColor(color);
    
    glUseProgram( m_blitProg );
    glUniform4f( m_blitProgTintLoc, m_tintColor.r/255.f, m_tintColor.g/255.f, m_tintColor.b/255.f, m_tintColor.a/255.f );
    glUseProgram( m_plotProg );
    glUniform4f( m_plotProgTintLoc, m_tintColor.r/255.f, m_tintColor.g/255.f, m_tintColor.b/255.f, m_tintColor.a/255.f );
    
    assert( glGetError() == 0 );
}

//____ SetBlendMode() __________________________________________________________

bool WgGlGfxDevice::SetBlendMode( WgBlendMode blendMode )
{
    assert( glGetError() == 0 );
    if( blendMode != WG_BLENDMODE_BLEND && blendMode != WG_BLENDMODE_OPAQUE &&
       blendMode != WG_BLENDMODE_ADD && blendMode != WG_BLENDMODE_MULTIPLY &&
       blendMode != WG_BLENDMODE_INVERT )
        return false;
    
    WgGfxDevice::SetBlendMode(blendMode);
    if( m_bRendering )
        _setBlendMode(blendMode);
    
    assert( glGetError() == 0 );
    return true;
}

//____ BeginRender() ___________________________________________________________

bool WgGlGfxDevice::BeginRender()
{
    assert( glGetError() == 0 );
    
    if( m_bRendering == true )
        return false;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

    // Remember GL states so we can restore in EndRender()
    
    m_glDepthTest 		= glIsEnabled(GL_DEPTH_TEST);
    m_glScissorTest 	= glIsEnabled(GL_SCISSOR_TEST);
    m_glBlendEnabled  	= glIsEnabled(GL_BLEND);
    glGetIntegerv(GL_BLEND_SRC, &m_glBlendSrc);
    glGetIntegerv(GL_BLEND_DST, &m_glBlendDst);
    glGetIntegerv(GL_VIEWPORT, m_glViewport);
    glGetIntegerv(GL_SCISSOR_BOX, m_glScissorBox);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &m_glReadFrameBuffer);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_glDrawFrameBuffer);  

    // Set correct framebuffer

    _setFramebuffer();
    
    //  Modify states
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glScissor( 0, 0, m_canvasSize.w, m_canvasSize.h );
    glViewport(m_viewportOffset.x, m_viewportOffset.y, m_canvasSize.w, m_canvasSize.h);
    
    // Set correct blend mode
    
    _setBlendMode(m_blendMode);
    
    //
    
    assert( glGetError() == 0 );
    m_bRendering = true;
    return true;
}

//____ EndRender() _____________________________________________________________

bool WgGlGfxDevice::EndRender()
{
    assert( glGetError() == 0 );
    if( m_bRendering == false )
        return false;
    
    glFlush();
    
    if( m_glDepthTest )
        glEnable(GL_DEPTH_TEST);
    
    if( !m_glScissorTest )
        glDisable(GL_SCISSOR_TEST);
    
    if( m_glBlendEnabled )
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    
    glBlendFunc( m_glBlendSrc, m_glBlendDst );
    
    glViewport( m_glViewport[0], m_glViewport[1], m_glViewport[2], m_glViewport[3] );
    glScissor( m_glScissorBox[0], m_glScissorBox[1], m_glScissorBox[2], m_glScissorBox[3] );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_glReadFrameBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_glDrawFrameBuffer);

    assert( glGetError() == 0 );
    m_bRendering = false;
    return true;
}


//____ Clear() _________________________________________________________________

void WgGlGfxDevice::Clear( WgColor col )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );
    glClearColor( col.r/255.f, col.g/255.f, col.b/255.f, col.a/255.f );
    glClear( GL_COLOR_BUFFER_BIT );
    assert( 0 == (err = glGetError()) );
}


//____ Fill() __________________________________________________________________

void WgGlGfxDevice::Fill( const WgRect& _rect, const WgColor& _col )
{
//    GLenum err;
    GLenum err = glGetError();
    assert(err == 0);
    
    if( _col.a  == 0 || _rect.w < 1 || _rect.h < 1 )
        return;
    
    WgColor fillColor = _col * m_tintColor;
    
    int	dx1 = _rect.x;
    int	dy1 = m_canvasSize.h - _rect.y;
    int dx2 = _rect.x + _rect.w;
    int dy2 = m_canvasSize.h - (_rect.y + _rect.h);
    
    m_vertexBufferData[0] = dx1;
    m_vertexBufferData[1] = dy1;
    m_vertexBufferData[2] = dx2;
    m_vertexBufferData[3] = dy1;
    m_vertexBufferData[4] = dx2;
    m_vertexBufferData[5] = dy2;
    m_vertexBufferData[6] = dx1;
    m_vertexBufferData[7] = dy2;
    
    glUseProgram( m_fillProg );
    glUniform4f( m_fillProgColorLoc, fillColor.r/255.f, fillColor.g/255.f, fillColor.b/255.f, fillColor.a/255.f );
    
    assert( 0 == (err = glGetError()) );
    
    glBindVertexArray(m_vertexArrayId);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
    glDisableVertexAttribArray(0);
    
    assert( 0 == (err = glGetError()) );
    return;
}

//____ Blit() __________________________________________________________________

void WgGlGfxDevice::Blit( const WgSurface * _pSrc, const WgRect& _src, int _dx, int _dy  )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );

    if( !_pSrc )
        return;
    
    float sw = (float) _pSrc->Width();
    float sh = (float) _pSrc->Height();
    
    float	sx1 = _src.x/sw;
    float	sx2 = (_src.x+_src.w)/sw;
    float	sy1 = _src.y/sh;
    float	sy2 = (_src.y+_src.h)/sh;
    
    int		dx1 = _dx;
    int		dx2 = _dx + _src.w;
    int		dy1 = m_canvasSize.h - _dy;
    int		dy2 = dy1 - _src.h;
    
    m_vertexBufferData[0] = dx1;
    m_vertexBufferData[1] = dy1;
    m_vertexBufferData[2] = dx2;
    m_vertexBufferData[3] = dy1;
    m_vertexBufferData[4] = dx2;
    m_vertexBufferData[5] = dy2;
    m_vertexBufferData[6] = dx1;
    m_vertexBufferData[7] = dy2;
    
    m_texCoordBufferData[0] = sx1;
    m_texCoordBufferData[1] = sy1;
    m_texCoordBufferData[2] = sx2;
    m_texCoordBufferData[3] = sy1;
    m_texCoordBufferData[4] = sx2;
    m_texCoordBufferData[5] = sy2;
    m_texCoordBufferData[6] = sx1;
    m_texCoordBufferData[7] = sy2;
    
    glActiveTexture(GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, ((const WgGlSurface*)_pSrc)->GetTexture());
    
    glUseProgram( m_blitProg );
    
    glBindVertexArray(m_vertexArrayId);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordArrayId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_texCoordBufferData), m_texCoordBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    assert( 0 == (err = glGetError()) );

}

//____ FillSubPixel() ______________________________________________________

void WgGlGfxDevice::FillSubPixel( const WgRectF& rect, const WgColor& col )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );

    if( col.a  == 0 )
        return;
    
    glUseProgram( m_aaFillProg );
    
    // Set color
    
    WgColor fillColor = col * m_tintColor;
    glUniform4f( m_aaFillProgColorLoc, fillColor.r/255.f, fillColor.g/255.f, fillColor.b/255.f, fillColor.a/255.f );
    
    // Calc frame coordinates and outside frame AA while we are still in normal coordinate space
    
    float frameX1 = (int)(rect.x+0.999f);
    float frameY1 = (int)(rect.y+0.999f);
    float frameX2 = (int)(rect.x + rect.w);
    float frameY2 = (int)(rect.y + rect.h);
    
    glUniform4f( m_aaFillProgOutsideAALoc, frameX1 - rect.x, frameY1 - rect.y, rect.x + rect.w - frameX2, rect.y + rect.h - frameY2 );
    
    // Set frame coords in GL coordinate space
    
    glUniform4f( m_aaFillProgFrameLoc, frameX1, m_canvasSize.h - frameY1, frameX2, m_canvasSize.h - frameY2 );
    
    
    // Convert rect to topLeft and bottomRight coordinates in GL coordinate space
    
    int	dx1 = rect.x;
    int	dy1 = m_canvasSize.h - (int)rect.y;
    int dx2 = rect.x + rect.w+0.999f;
    int dy2 = m_canvasSize.h - (int)(rect.y + rect.h + 0.999f);
    
    // Set vertices
    
    m_vertexBufferData[0] = dx1;
    m_vertexBufferData[1] = dy1;
    m_vertexBufferData[2] = dx2;
    m_vertexBufferData[3] = dy1;
    m_vertexBufferData[4] = dx2;
    m_vertexBufferData[5] = dy2;
    m_vertexBufferData[6] = dx1;
    m_vertexBufferData[7] = dy2;
    
    // Set buffer and draw
    
    glBindVertexArray(m_vertexArrayId);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
    glDisableVertexAttribArray(0);
    
    assert( 0 == (err = glGetError()) );
    return;
}


//____ StretchBlitSubPixel() ___________________________________________________

void WgGlGfxDevice::StretchBlitSubPixel( const WgSurface * pSrc, float sx, float sy,
                                        float sw, float sh,
                                        float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );

    if( pSrc->scaleMode() == WG_SCALEMODE_INTERPOLATE )
    {
        if( sw < dw )
            sx += 0.5f;
        
        if( sh < dh )
            sy += 0.5f;
    }
    
    if( !pSrc )
        return;
    
    float tw = (float) pSrc->Width();
    float th = (float) pSrc->Height();
    
    float	sx1 = sx/tw;
    float	sx2 = (sx+sw)/tw;
    float	sy1 = sy/th;
    float	sy2 = (sy+sh)/th;
    
    float	dx1 = dx;
    float	dx2 = dx + dw;
    float	dy1 = m_canvasSize.h - dy;
    float	dy2 = dy1 - dh;
    
    m_vertexBufferData[0] = dx1;
    m_vertexBufferData[1] = dy1;
    m_vertexBufferData[2] = dx2;
    m_vertexBufferData[3] = dy1;
    m_vertexBufferData[4] = dx2;
    m_vertexBufferData[5] = dy2;
    m_vertexBufferData[6] = dx1;
    m_vertexBufferData[7] = dy2;
    
    m_texCoordBufferData[0] = sx1;
    m_texCoordBufferData[1] = sy1;
    m_texCoordBufferData[2] = sx2;
    m_texCoordBufferData[3] = sy1;
    m_texCoordBufferData[4] = sx2;
    m_texCoordBufferData[5] = sy2;
    m_texCoordBufferData[6] = sx1;
    m_texCoordBufferData[7] = sy2;
    
    glActiveTexture(GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, ((const WgGlSurface*)pSrc)->GetTexture());
    
    glUseProgram( m_blitProg );
    
    glBindVertexArray(m_vertexArrayId);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordArrayId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_texCoordBufferData), m_texCoordBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    assert( 0 == (err = glGetError()) );
}

//____ ClipBlitFromCanvas() _______________________________________________________

void WgGlGfxDevice::ClipBlitFromCanvas(const WgRect& clip, const WgSurface* pSrc, const WgRect& srcRect, int dx, int dy)
{
	if ((clip.x <= dx) && (clip.x + clip.w > dx + srcRect.w) &&
		(clip.y <= dy) && (clip.y + clip.h > dy + srcRect.h))
	{
		StretchBlitSubPixelWithInvert(pSrc, srcRect.x, srcRect.y, srcRect.w, srcRect.h, dx, dy, srcRect.w, srcRect.h); // Totally inside clip-rect.
		return;
	}

	if ((clip.x > dx + srcRect.w) || (clip.x + clip.w < dx) ||
		(clip.y > dy + srcRect.h) || (clip.y + clip.h < dy))
		return;																						// Totally outside clip-rect.

																									// Do Clipping

	WgRect	newSrc = srcRect;

	if (dx < clip.x)
	{
		newSrc.w -= clip.x - dx;
		newSrc.x += clip.x - dx;
		dx = clip.x;
	}

	if (dy < clip.y)
	{
		newSrc.h -= clip.y - dy;
		newSrc.y += clip.y - dy;
		dy = clip.y;
	}

	if (dx + newSrc.w > clip.x + clip.w)
		newSrc.w = (clip.x + clip.w) - dx;

	if (dy + newSrc.h > clip.y + clip.h)
		newSrc.h = (clip.y + clip.h) - dy;

	StretchBlitSubPixelWithInvert(pSrc, newSrc.x, newSrc.y, newSrc.w, newSrc.h, dx, dy, newSrc.w, newSrc.h); 
}


//____ StretchBlitSubPixelWithInvert() ___________________________________________________

void WgGlGfxDevice::StretchBlitSubPixelWithInvert(const WgSurface * pSrc, float sx, float sy, float sw, float sh,
	float dx, float dy, float dw, float dh)
{
    GLenum err;
    assert( 0 == (err = glGetError()) );

    if( pSrc->scaleMode() == WG_SCALEMODE_INTERPOLATE )
    {
        if( sw < dw )
            sx += 0.5f;
        
        if( sh < dh )
            sy += 0.5f;
    }
    
    if( !pSrc )
        return;
    
    float tw = (float) pSrc->Width();
    float th = (float) pSrc->Height();
    
    float   sx1 = sx/tw;
    float   sx2 = (sx+sw)/tw;
    float   sy1 = 1.f - (sy/th);
    float   sy2 = 1.f - (sy+sh)/th;
    
    float   dx1 = dx;
    float   dx2 = dx + dw;
    float   dy1 = m_canvasSize.h - dy;
    float   dy2 = dy1 - dh;
    
    m_vertexBufferData[0] = dx1;
    m_vertexBufferData[1] = dy1;
    m_vertexBufferData[2] = dx2;
    m_vertexBufferData[3] = dy1;
    m_vertexBufferData[4] = dx2;
    m_vertexBufferData[5] = dy2;
    m_vertexBufferData[6] = dx1;
    m_vertexBufferData[7] = dy2;
    
    m_texCoordBufferData[0] = sx1;
    m_texCoordBufferData[1] = sy1;
    m_texCoordBufferData[2] = sx2;
    m_texCoordBufferData[3] = sy1;
    m_texCoordBufferData[4] = sx2;
    m_texCoordBufferData[5] = sy2;
    m_texCoordBufferData[6] = sx1;
    m_texCoordBufferData[7] = sy2;
    
    glActiveTexture(GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, ((const WgGlSurface*)pSrc)->GetTexture());
    
    glUseProgram( m_blitProg );
    
    glBindVertexArray(m_vertexArrayId);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordArrayId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_texCoordBufferData), m_texCoordBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    assert( 0 == (err = glGetError()) );
}


//____ _setBlendMode() _________________________________________________________

void WgGlGfxDevice::_setBlendMode( WgBlendMode blendMode )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );

    switch( blendMode )
    {
        case WG_BLENDMODE_OPAQUE:
            glBlendEquation( GL_FUNC_ADD );
            glDisable(GL_BLEND);
            break;
            
        case WG_BLENDMODE_BLEND:
            glBlendEquation( GL_FUNC_ADD );
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            break;
            
        case WG_BLENDMODE_ADD:
            glBlendEquation( GL_FUNC_ADD );
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
            
        case WG_BLENDMODE_MULTIPLY:
            glBlendEquation( GL_FUNC_ADD );
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
            
        case WG_BLENDMODE_INVERT:
            glBlendEquation( GL_FUNC_ADD );
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
            break;
            
        default:
            break;
    }
    assert( 0 == (err = glGetError()) );
}


//____ _createGlProgram() ___________________________________________________

GLuint WgGlGfxDevice::_createGLProgram( const char * pVertexShader, const char * pFragmentShader )
{
	char log[1024];
	GLsizei logLen;
	
	GLenum err;
    assert( (err = glGetError()) == 0 );
    
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    assert( (err = glGetError()) == 0 );
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    assert( (err = glGetError()) == 0 );
    glShaderSource(vertexShaderID, 1, &pVertexShader, NULL);
    assert( (err = glGetError()) == 0 );
    glCompileShader(vertexShaderID);

	glGetShaderInfoLog(fragmentShaderID, 1023, &logLen, log);

	assert( (err = glGetError()) == 0 );
    
    glShaderSource(fragmentShaderID, 1, &pFragmentShader, NULL);
    assert( (err = glGetError()) == 0 );
    glCompileShader(fragmentShaderID);
    assert( (err = glGetError()) == 0 );
    
    GLuint  programID = glCreateProgram();
    glAttachShader( programID, vertexShaderID );
    assert( (err = glGetError()) == 0 );
    glAttachShader( programID, fragmentShaderID );
    assert( (err = glGetError()) == 0 );
    glLinkProgram( programID );
    
    // glLinkProgram doesn't use glGetError
    int mess = 0;
    glGetProgramiv( programID, GL_LINK_STATUS, &mess);
    assert( mess == GL_TRUE );
    
    assert( (err = glGetError()) == 0 );
    
    glDetachShader( programID, vertexShaderID );
    assert( (err = glGetError()) == 0 );
    glDetachShader( programID, fragmentShaderID );
    assert( (err = glGetError()) == 0 );
    
    glDeleteShader( vertexShaderID );
    assert( (err = glGetError()) == 0 );
    glDeleteShader( fragmentShaderID );
    assert( (err = glGetError()) == 0 );
    
    return programID;
}




void WgGlGfxDevice::ClipDrawHorrLine( const WgRect& clip, const WgCoord& _start, int length, const WgColor& col )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );

    if( col.a  == 0 || _start.y < clip.y || _start.y >= clip.y + clip.h )
        return;
    
    WgCoord start = _start;
    
    if( start.x < clip.x )
    {
        length -= clip.x - start.x;
        start.x = clip.x;
    }
    
    if( start.x + length > clip.x + clip.w )
        length = clip.x + clip.w - start.x;
    
    WgColor fillColor = col * m_tintColor;
    
    float	dx1 = start.x + 0.5f;
    float	dy = m_canvasSize.h - (start.y +0.5f);
    float   dx2 = start.x + length +0.5f;
    
    m_vertexBufferData[0] = dx1;
    m_vertexBufferData[1] = dy;
    m_vertexBufferData[2] = dx2;
    m_vertexBufferData[3] = dy;
    
    glUseProgram( m_fillProg );
    glUniform4f( m_fillProgColorLoc, fillColor.r/255.f, fillColor.g/255.f, fillColor.b/255.f, fillColor.a/255.f );
    
    glBindVertexArray(m_vertexArrayId);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glDrawArrays(GL_LINES, 0, 2); // Starting from vertex 0; 2 vertices total -> 1 line
    glDisableVertexAttribArray(0);

    assert( 0 == (err = glGetError()) );
    return;
    
}

void WgGlGfxDevice::ClipDrawVertLine( const WgRect& clip, const WgCoord& _start, int length, const WgColor& col )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );

    if( col.a  == 0 || _start.x < clip.x || _start.x >= clip.x + clip.w )
        return;
    
    WgCoord start = _start;
    
    if( start.y < clip.y )
    {
        length -= clip.y - start.y;
        start.y = clip.y;
    }
    
    if( start.y + length > clip.y + clip.h )
        length = clip.y + clip.h - start.y;
    
    WgColor fillColor = col * m_tintColor;
    
    float	dx = start.x + 0.5f;
    float	dy1 = m_canvasSize.h - (start.y +0.5f);
    float   dy2 = m_canvasSize.h - (start.y + length +0.5f);
    
    m_vertexBufferData[0] = dx;
    m_vertexBufferData[1] = dy1;
    m_vertexBufferData[2] = dx;
    m_vertexBufferData[3] = dy2;
    
    glUseProgram( m_fillProg );
    glUniform4f( m_fillProgColorLoc, fillColor.r/255.f, fillColor.g/255.f, fillColor.b/255.f, fillColor.a/255.f );
    
    glBindVertexArray(m_vertexArrayId);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glDrawArrays(GL_LINES, 0, 2); // Starting from vertex 0; 2 vertices total -> 1 line
    glDisableVertexAttribArray(0);
    assert( 0 == (err = glGetError()) );
    return;
}

void WgGlGfxDevice::ClipPlotSoftPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor& col, float thickness )
{
    
}

void WgGlGfxDevice::ClipPlotPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor * pColors)
{
    GLenum err;
    assert( 0 == (err = glGetError()) );
    glScissor( clip.x, m_canvasSize.h - clip.y - clip.h, clip.w, clip.h );
    PlotPixels( nCoords, pCoords, pColors );
    glScissor( 0, 0, m_canvasSize.w, m_canvasSize.h );
    assert( 0 == (err = glGetError()) );
}


void WgGlGfxDevice::PlotPixels( int nCoords, const WgCoord * pCoords, const WgColor * pColors)
{
    GLenum err;
    assert( 0 == (err = glGetError()) );
    if( nCoords == 0 )
        return;
    
    glUseProgram( m_plotProg );
    
    glBindVertexArray(m_vertexArrayId);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WgCoord)*nCoords, pCoords, GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_INT,             // type
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordArrayId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WgColor)*nCoords, pColors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          GL_BGRA,            // size
                          GL_UNSIGNED_BYTE,   // type
                          GL_TRUE,            // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_POINTS, 0, nCoords); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    assert( 0 == (err = glGetError()) );
}

void WgGlGfxDevice::DrawLine( WgCoord beg, WgCoord end, WgColor color, float thickness )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );
    int 	length;
    float   width;
    float	slope;
    
    if( abs(beg.x-end.x) > abs(beg.y-end.y) )
    {
        // Prepare mainly horizontal line segment
        
        if( beg.x > end.x )
            swap( beg, end );
        
        length = end.x - beg.x;
        slope = ((float)(end.y - beg.y)) / length;
        
        width = (thickness*m_lineThicknessTable[abs((int)(slope*16))]);
        
        
        glUseProgram( m_mildSlopeProg );
        
        WgColor fillColor = color * m_tintColor;
        glUniform4f( m_mildSlopeProgColorLoc, fillColor.r/255.f, fillColor.g/255.f, fillColor.b/255.f, fillColor.a/255.f );
        //            glUniform2f( m_mildSlopeProgStartLoc, beg.x + 0.5, m_canvasSize.h - (beg.y + 0.5));
        glUniform1f( m_mildSlopeProgSLoc, (beg.x + 0.5)*slope + (m_canvasSize.h - (beg.y + 0.5)));
        glUniform1f( m_mildSlopeProgWLoc, width/2 + 0.5 );
        glUniform1f( m_mildSlopeProgSlopeLoc, slope );
        
        
        float   x1 = beg.x;
        float   y1 = m_canvasSize.h - (beg.y - width/2);
        float   x2 = end.x;
        float   y2 = m_canvasSize.h - (end.y - width/2);
        
        m_vertexBufferData[0] = x1;
        m_vertexBufferData[1] = y1 +1;
        m_vertexBufferData[2] = x2;
        m_vertexBufferData[3] = y2 +1;
        m_vertexBufferData[4] = x2;
        m_vertexBufferData[5] = y2 - width -2;
        m_vertexBufferData[6] = x1;
        m_vertexBufferData[7] = y1 - width -2;
    }
    else
    {
        // Prepare mainly vertical line segment
        
        if( beg.y > end.y )
            swap( beg, end );
        
        length = end.y - beg.y;
        if( length == 0 )
            return;											// TODO: Should stil draw the caps!
        
        slope = ((float)(end.x - beg.x)) / length;
        width = (thickness*m_lineThicknessTable[abs((int)(slope*16))]);
        
        glUseProgram( m_steepSlopeProg );
        
        WgColor fillColor = color * m_tintColor;
        glUniform4f( m_steepSlopeProgColorLoc, fillColor.r/255.f, fillColor.g/255.f, fillColor.b/255.f, fillColor.a/255.f );
        //            glUniform2f( m_steepSlopeProgStartLoc, beg.x + 0.5, m_canvasSize.h - (beg.y + 0.5));
        glUniform1f( m_steepSlopeProgSLoc, (beg.x + 0.5) + (m_canvasSize.h - (beg.y + 0.5))*slope );
        glUniform1f( m_steepSlopeProgWLoc, width/2 + 0.5f );
        glUniform1f( m_steepSlopeProgSlopeLoc, slope );
        
        
        float   x1 = beg.x - width/2;
        float   y1 = m_canvasSize.h - beg.y;
        float   x2 = end.x - width/2;
        float   y2 = m_canvasSize.h - end.y;
        
        m_vertexBufferData[0] = x1 -1;
        m_vertexBufferData[1] = y1;
        m_vertexBufferData[2] = x1 + width +2;
        m_vertexBufferData[3] = y1;
        m_vertexBufferData[4] = x2 + width +2;
        m_vertexBufferData[5] = y2;
        m_vertexBufferData[6] = x2 -1;
        m_vertexBufferData[7] = y2;
    }
    
    
    
    
    // Set buffer and draw
    
    
    glBindVertexArray(m_vertexArrayId);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
    glDisableVertexAttribArray(0);
    
    assert( 0 == (err = glGetError()) );
}

void WgGlGfxDevice::ClipDrawLine( const WgRect& clip, WgCoord begin, WgCoord end, WgColor color, float thickness )
{
    GLenum err;
    assert( 0 == (err = glGetError()) );
    glScissor( clip.x, m_canvasSize.h - clip.y - clip.h, clip.w, clip.h );
    DrawLine( begin, end, color, thickness );
    glScissor( 0, 0, m_canvasSize.w, m_canvasSize.h );
    assert( 0 == (err = glGetError()) );
}

//____ ClipDrawHorrWave() _____________________________________________________

void WgGlGfxDevice::ClipDrawHorrWave(const WgRect& clip, WgCoord begin, int length, const WgWaveLine& topBorder, const WgWaveLine& bottomBorder, WgColor frontFill, WgColor backFill)
{
	// Do early rough X-clipping with margin (need to trace lines with margin of thickest line).

	int ofs = 0;
	if (clip.x > begin.x || clip.x + clip.w < begin.x + length)
	{
		int margin = (int)(max(topBorder.thickness, bottomBorder.thickness) / 2 + 0.99);

		if (clip.x > begin.x + margin)
		{
			ofs = clip.x - begin.x - margin;
			begin.x += ofs;
			length -= ofs;
		}

		if (begin.x + length - margin > clip.x + clip.w)
			length = clip.x + clip.w - begin.x + margin;

		if (length <= 0)
			return;
	}

	// Generate line traces

	int	traceBufferSize = (length + 1) * 2 * sizeof(int) * 2;	// length+1 * values per point * sizeof(int) * 2 separate traces.
	char * pTraceBuffer = WgBase::MemStackAlloc(traceBufferSize);
	int * pTopBorderTrace = (int*)pTraceBuffer;
	int * pBottomBorderTrace = (int*)(pTraceBuffer + traceBufferSize / 2);

	_traceLine(pTopBorderTrace, length+1, topBorder, ofs);
	_traceLine(pBottomBorderTrace, length+1, bottomBorder, ofs);



	// Do proper X-clipping

	int startColumn = 0;
	if (begin.x < clip.x)
	{
		startColumn = clip.x - begin.x;
		length -= startColumn;
		begin.x += startColumn;
	}

	if (begin.x + length > clip.x + clip.w)
		length = clip.x + clip.w - begin.x;

	// Box in drawing area

	WgRect box;
	box.x = begin.x;
	box.w = length;

	int top = INT_MAX;
	int bottom = INT_MIN;

	for (int i = startColumn; i <= length + startColumn; i++)
	{
		int t = pTopBorderTrace[i * 2] < pBottomBorderTrace[i * 2] ? pTopBorderTrace[i * 2] : pBottomBorderTrace[i * 2];
		int b = pTopBorderTrace[i * 2 + 1] > pBottomBorderTrace[i * 2 + 1] ? pTopBorderTrace[i * 2 + 1] : pBottomBorderTrace[i * 2 + 1];

		if (t < top) top = t;
		if (b > bottom) bottom = b;
	}

	top = begin.y + (top >> 8);
	bottom = begin.y + ((bottom + 255) >> 8);


	box.y = top > clip.y ? top : clip.y;
	box.h = bottom < (clip.y + clip.h) ? bottom - box.y : clip.y + clip.h - box.y;

	if (box.w <= 0 || box.h <= 0)
		return;

	// Render columns

	int pos[2][4];						// Startpositions for the 4 fields of the column (topBorder, fill, bottomBorder, line end) for left and right edge of pixel column. 16 binals.

	int		textureBufferDataSize = (length) * sizeof(int) * 9;
	int * pTextureBufferData = (int*)WgBase::MemStackAlloc(textureBufferDataSize);
	int * wpBuffer = pTextureBufferData;
	bool  bFlipped = false;

	for (int i = startColumn; i <= length + startColumn; i++)
	{
		// Old right pos becomes new left pos and old left pos will be reused for new right pos

		int * pLeftPos = pos[i % 2];
		int * pRightPos = pos[(i + 1) % 2];

		// Check if lines have intersected and in that case swap top and bottom lines and colors

		if (pTopBorderTrace[i * 2] > pBottomBorderTrace[i * 2])
		{
			swap(pTopBorderTrace, pBottomBorderTrace);
			bFlipped = !bFlipped;

			// We need to regenerate leftpos since we now have swapped top and bottom line.

			if (i > startColumn)
			{
				int j = i - 1;
				pLeftPos[0] = pTopBorderTrace[j * 2] << 8;
				pLeftPos[1] = pTopBorderTrace[j * 2 + 1] << 8;

				pLeftPos[2] = pBottomBorderTrace[j * 2] << 8;
				pLeftPos[3] = pBottomBorderTrace[j * 2 + 1] << 8;

				if (pLeftPos[2] < pLeftPos[1])
				{
					pLeftPos[2] = pLeftPos[1];
					if (pLeftPos[3] < pLeftPos[2])
						pLeftPos[3] = pLeftPos[2];
				}
			}
		}

		// Generate new rightpos table

		pRightPos[0] = pTopBorderTrace[i * 2] << 8;
		pRightPos[1] = pTopBorderTrace[i * 2 + 1] << 8;

		pRightPos[2] = pBottomBorderTrace[i * 2] << 8;
		pRightPos[3] = pBottomBorderTrace[i * 2 + 1] << 8;


		if (pRightPos[2] < pRightPos[1])
		{
			pRightPos[2] = pRightPos[1];
			if (pRightPos[3] < pRightPos[2])
				pRightPos[3] = pRightPos[2];
		}

		// Render the column

		if (i > startColumn)
		{
			// Calculate start amount and increment for our 4 fields

			for (int i = 0; i < 4; i++)
			{
				int yBeg;
				int64_t xInc;

				if (pLeftPos[i] < pRightPos[i])
				{
					yBeg = pLeftPos[i];
					xInc = (int64_t)65536 * 65536 / (pRightPos[i] - pLeftPos[i] + 1);
				}
				else
				{
					yBeg = pRightPos[i];
					xInc = (int64_t)65536 * 65536 / (pLeftPos[i] - pRightPos[i] + 1);
				}

				WG_LIMIT(xInc, (int64_t)0, (int64_t)65536);

				int64_t startAmount = -((xInc * yBeg) >> 16);

				*wpBuffer++ = (int)startAmount;
				*wpBuffer++ = (int)xInc;
			}
			*wpBuffer++ = (int)bFlipped;
		}
	}

	// Now we have the data generated, setup GL to operate on it

	glBindBuffer(GL_TEXTURE_BUFFER, m_horrWaveBufferTextureData);
	glBufferData(GL_TEXTURE_BUFFER, textureBufferDataSize, pTextureBufferData, GL_STREAM_DRAW);

	int	dx1 = box.x;
	int	dy1 = m_canvasSize.h - box.y;
	int dx2 = box.x + box.w;
	int dy2 = m_canvasSize.h - (box.y + box.h);

	m_vertexBufferData[0] = (GLfloat)dx1;
	m_vertexBufferData[1] = (GLfloat)dy1;
	m_vertexBufferData[2] = (GLfloat)dx2;
	m_vertexBufferData[3] = (GLfloat)dy1;
	m_vertexBufferData[4] = (GLfloat)dx2;
	m_vertexBufferData[5] = (GLfloat)dy2;
	m_vertexBufferData[6] = (GLfloat)dx1;
	m_vertexBufferData[7] = (GLfloat)dy2;

	glUseProgram(m_horrWaveProg);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, m_horrWaveBufferTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, m_horrWaveBufferTextureData);
	glUniform1i(m_horrWaveProgTexIdLoc, 0);
	glUniform2f(m_horrWaveProgWindowOfsLoc, begin.x, begin.y);
	glUniform4f(m_horrWaveProgTopBorderColorLoc, topBorder.color.r / 255.f, topBorder.color.g / 255.f, topBorder.color.b / 255.f, topBorder.color.a / 255.f);
	glUniform4f(m_horrWaveProgBottomBorderColorLoc, bottomBorder.color.r / 255.f, bottomBorder.color.g / 255.f, bottomBorder.color.b / 255.f, bottomBorder.color.a / 255.f);
	glUniform4f(m_horrWaveProgFrontFillLoc, frontFill.r / 255.f, frontFill.g / 255.f, frontFill.b / 255.f, frontFill.a / 255.f);
	glUniform4f(m_horrWaveProgBackFillLoc, backFill.r / 255.f, backFill.g / 255.f, backFill.b / 255.f, backFill.a / 255.f);

	glBindVertexArray(m_vertexArrayId);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexBufferData), m_vertexBufferData, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 2 triangles in the strip
	glDisableVertexAttribArray(0);

	// Free temporary work memory

	WgBase::MemStackRelease(textureBufferDataSize);
	WgBase::MemStackRelease(traceBufferSize);
}


void WgGlGfxDevice::DrawArcNE( const WgRect& rect, WgColor color )
{
}

void WgGlGfxDevice::DrawElipse( const WgRect& rect, WgColor color )
{
    
}

void WgGlGfxDevice::DrawFilledElipse( const WgRect& rect, WgColor color )
{
    
}

void WgGlGfxDevice::ClipDrawArcNE( const WgRect& clip, const WgRect& rect, WgColor color )
{
}

void WgGlGfxDevice::ClipDrawElipse( const WgRect& clip, const WgRect& rect, WgColor color )
{
}

void WgGlGfxDevice::ClipDrawFilledElipse( const WgRect& clip, const WgRect& rect, WgColor color )
{
    
}

//____ _initTables() ___________________________________________________________

void WgGlGfxDevice::_initTables()
{
    // Init lineThicknessTable
    
    for( int i = 0 ; i < 17 ; i++ )
    {
        double b = i/16.0;
        m_lineThicknessTable[i] = (float) sqrt( 1.0 + b*b );
    }
}


