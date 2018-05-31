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
#ifndef WG_GFXDEVICE_SOFT_DOT_H
#define WG_GFXDEVICE_SOFT_DOT_H

#ifndef WG_GEO_DOT_H
#	include <wg_geo.h>
#endif

#ifndef	WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif


class WgSurfaceSoft;
/*
struct WgSegmentEdge
{
	int			begin;				// Pixeloffset, 24.8 format.
	int			end;				// Pixeloffset, 24.8 format. First pixel after edge (segment after the edge has 100% coverage)
	int			coverage;			// 0-65536
	int			coverageInc;		// 24.8 format
};
*/


class WgGfxDeviceSoft : public WgGfxDevice
{
public:
	WgGfxDeviceSoft();
	WgGfxDeviceSoft( WgSurfaceSoft * pCanvas );
	~WgGfxDeviceSoft();

};

#endif //WG_GFXDEVICE_SOFT_DOT_H

