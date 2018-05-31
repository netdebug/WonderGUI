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


#include <wg_gfxdevice_soft.h>
#include <wg_surface_soft.h>
#include <wg_base.h>


#include <wg_userdefines.h>

#include <wg3_softgfxdevice.h>


#ifdef SOFTUBE_USE_PACE_FUSION
#include "PaceFusion.h"
PACE_FUSION_EXCLUDE_USER_CALLBACKS
#endif

//____ Constructor _____________________________________________________________

WgGfxDeviceSoft::WgGfxDeviceSoft()
{
	m_pRealDevice = wg::SoftGfxDevice::create();
}
 
WgGfxDeviceSoft::WgGfxDeviceSoft( WgSurfaceSoft * pCanvas )
{
	m_pRealDevice = wg::SoftGfxDevice::create( (wg::SoftSurface*) pCanvas->m_pRealSurface.rawPtr() );
    m_pCanvas = pCanvas;
}

//____ Destructor ______________________________________________________________

WgGfxDeviceSoft::~WgGfxDeviceSoft()
{
}

