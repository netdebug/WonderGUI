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
    
    m_plotProg = _createGLProgram( plotVertexShader, plotFragmentShader );
    m_plotProgTintLoc = glGetUniformLocation( m_plotProg, "tint" );
    
    glGenVertexArrays(1, &m_vertexArrayId);
    glBindVertexArray(m_vertexArrayId);
    glGenBuffers(1, &m_vertexBufferId);
    
    glGenVertexArrays(1, &m_texCoordArrayId);
    glBindVertexArray(m_texCoordArrayId);
    glGenBuffers(1, &m_texCoordBufferId);
    glBindVertexArray(0);
    
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

void WgGlGfxDevice::SetCanvas( WgSize canvas )
{
    m_canvasSize 	= canvas;
    assert( glGetError() == 0 );
    
    glUseProgram( m_fillProg );
    GLint dimLoc = glGetUniformLocation( m_fillProg, "dimensions");
    glUniform2f(dimLoc, canvas.w, canvas.h);
    
    glUseProgram( m_aaFillProg );
    dimLoc = glGetUniformLocation( m_aaFillProg, "dimensions");
    glUniform2f(dimLoc, canvas.w, canvas.h);
    
    glUseProgram( m_mildSlopeProg );
    dimLoc = glGetUniformLocation( m_mildSlopeProg, "dimensions");
    glUniform2f(dimLoc, canvas.w, canvas.h);
    
    glUseProgram( m_steepSlopeProg );
    dimLoc = glGetUniformLocation( m_steepSlopeProg, "dimensions");
    glUniform2f(dimLoc, canvas.w, canvas.h);
    
    glUseProgram( m_blitProg );
    dimLoc = glGetUniformLocation( m_blitProg, "dimensions");
    glUniform2f(dimLoc, canvas.w, canvas.h);
    glUniform1i( m_blitProgTexIdLoc, 0 );
    
    glUseProgram( m_plotProg );
    dimLoc = glGetUniformLocation( m_plotProg, "dimensions");
    glUniform2f(dimLoc, canvas.w, canvas.h);
    assert( glGetError() == 0 );
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
    
    // Remember GL states so we can restore in EndRender()
    
    m_glDepthTest 		= glIsEnabled(GL_DEPTH_TEST);
    m_glScissorTest 	= glIsEnabled(GL_SCISSOR_TEST);
    m_glBlendEnabled  	= glIsEnabled(GL_BLEND);
    glGetIntegerv(GL_BLEND_SRC, &m_glBlendSrc);
    glGetIntegerv(GL_BLEND_DST, &m_glBlendDst);
    glGetIntegerv(GL_VIEWPORT, m_glViewport);
    glGetIntegerv(GL_SCISSOR_BOX, m_glScissorBox);
    
    
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
    GLenum err;
    assert( (err = glGetError()) == 0 );
    
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    assert( (err = glGetError()) == 0 );
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    assert( (err = glGetError()) == 0 );
    glShaderSource(vertexShaderID, 1, &pVertexShader, NULL);
    assert( (err = glGetError()) == 0 );
    glCompileShader(vertexShaderID);
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


