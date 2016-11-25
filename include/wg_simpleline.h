//
//  wg_simpleline.h
//
//  Created by Patrik Holmström on 11/17/15.
//
//
#ifndef WG_SIMPLELINE_DOT_H
#define WG_SIMPLELINE_DOT_H

#include <math.h>


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif


// Class
#include <vector>

class WgSurfaceSoft;

class WgSimpleLine : public WgWidget
{
public:
    WgSimpleLine();
    virtual ~WgSimpleLine();
    
    virtual const char *Type( void ) const;
    static const char * GetClass();
    virtual WgWidget * NewOfMyType() const { return new WgSimpleLine(); };
    
    void	SetLine( int x1, int y1, int x2, int y2 );
    void    RemoveLine();
    void	SetColor( WgColor color );
    //void    SetLineWidth( float fWidth );
    
    void SetSurfaceSize( int iWidth, int iHeight );

    void drawLine(const WgRect& _clip, bool erase=false);
    
    WgSize	PreferredSize() const;

    
protected:
    
    void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
    void	_onCloneContent( const WgWidget * _pOrg );
    bool	_onAlphaTest( const WgCoord& ofs );
    
    void	plot(int x, int y, double alpha, const WgRect& _clip);
    
    
private:
    float		m_lineThickness;
    
    WgColor     m_kColor;
    
    WgSurfaceSoft * m_pAngleLineSurf;
    WgRect m_src;	// xywh of gfx source rectangle.
    
    std::vector<int> m_pixelX;
    std::vector<int> m_pixelY;
    std::vector<Uint32> m_pixelC;
    std::vector<Uint32> m_erasePixelC;
    
    int m_iLen;
    int m_iAlloc;
    
    int m_iX1, m_iY1;
    int m_iX2, m_iY2;
    
};


#endif  // WG_SIMPLELINE_DOT_H
