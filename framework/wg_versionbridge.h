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

#pragma once

#include <assert.h>

#include <wg_types.h>
#include <wg3_types.h>
#include <wg_geo.h>
#include <wg3_geo.h>


inline WgCoord _convert(const wg::Coord& r) { return WgCoord(r.x, r.y); }
inline wg::Coord _convert(const WgCoord& r) { return wg::Coord(r.x, r.y); }

inline WgSize _convert(const wg::Size& r) { return WgSize(r.w, r.h); }
inline wg::Size _convert(const WgSize& r) { return wg::Size(r.w, r.h); }

inline WgBorders _convert(const wg::Border& r) { return WgBorders( (Uint8)r.left, (Uint8)r.top, (Uint8)r.right, (Uint8)r.bottom); }
inline wg::Border _convert(const WgBorders& r) { return wg::Border(r.top,r.right,r.bottom,r.left); }

inline WgRect _convert(const wg::Rect& r) { return WgRect(r.x, r.y, r.w, r.h); }
inline wg::Rect _convert(const WgRect& r) { return wg::Rect(r.x, r.y, r.w, r.h); }

inline WgRectF _convert(const wg::RectF& r) { return WgRectF(r.x, r.y, r.w, r.h); }
inline wg::RectF _convert(const WgRectF& r) { return wg::RectF(r.x, r.y, r.w, r.h); }


inline WgColor _convert(wg::Color c) { return WgColor(c.argb); }
inline wg::Color _convert(WgColor c) { return wg::Color(c.argb); }

inline WgDirection _convert(wg::Direction dir )
{
	switch (dir)
	{
	case wg::Direction::Up:
		return WG_UP;
	case wg::Direction::Right:
		return WG_RIGHT;
	case wg::Direction::Down:
		return WG_DOWN;
	case wg::Direction::Left:
		return WG_LEFT;
	}
}

inline wg::Direction _convert(WgDirection dir)
{
	switch (dir)
	{
	case WG_UP:
		return wg::Direction::Up;
	case WG_RIGHT:
		return wg::Direction::Right;
	case WG_DOWN:
		return wg::Direction::Down;
	case WG_LEFT:
		return wg::Direction::Left;
	}

	assert(false);									// Should never get here
	return wg::Direction::Down;						// Just avoiding compiler warnings
}

inline wg::BlendMode _convert(WgBlendMode m)
{
	switch (m)
	{
	case WG_BLENDMODE_OPAQUE:
		return wg::BlendMode::Replace;
	case WG_BLENDMODE_BLEND:
		return wg::BlendMode::Blend;
	case WG_BLENDMODE_ADD:
		return wg::BlendMode::Add;
	case WG_BLENDMODE_MULTIPLY:
		return wg::BlendMode::Multiply;
	case WG_BLENDMODE_INVERT:
		return wg::BlendMode::Invert;
	}

	assert(false);									// Should never get here
	return wg::BlendMode::Blend;						// Just avoiding compiler warnings
}

inline WgBlendMode _convert(wg::BlendMode m)
{
	switch (m)
	{
	case wg::BlendMode::Replace:
		return WG_BLENDMODE_OPAQUE;
	case wg::BlendMode::Blend:
		return WG_BLENDMODE_BLEND;
	case wg::BlendMode::Add:
		return WG_BLENDMODE_ADD;
	case wg::BlendMode::Multiply:
		return WG_BLENDMODE_MULTIPLY;
	case wg::BlendMode::Invert:
		return WG_BLENDMODE_INVERT;
    default:
		assert(false);                                      // Undefined, Ingore and Subtract doesn't exist in WG2, so let's catch them and see where we went wrong.
		return WG_BLENDMODE_BLEND;
	}
}

inline WgPixelType _convert(wg::PixelFormat t)
{
	switch (t)
	{
	case wg::PixelFormat::Unknown:
		return WG_PIXEL_UNKNOWN;
	case wg::PixelFormat::Custom:
		return WG_PIXEL_CUSTOM;
	case wg::PixelFormat::BGRA_8:
		return WG_PIXEL_BGRA_8;
	case wg::PixelFormat::BGR_8:
		return WG_PIXEL_BGR_8;
    case wg::PixelFormat::BGRX_8:
        return WG_PIXEL_BGRX_8;
    case wg::PixelFormat::BGR_565:
        return WG_PIXEL_BGR_565;
    case wg::PixelFormat::BGRA_4:
        return WG_PIXEL_BGRA_4;
    case wg::PixelFormat::I8:
        return WG_PIXEL_I8;
    case wg::PixelFormat::A8:
        return WG_PIXEL_A8;
	}
	assert(false);									// Should never get here
	return WG_PIXEL_UNKNOWN;						// Just avoiding compiler warnings
}

inline wg::PixelFormat _convert(WgPixelType t)
{
	switch (t)
	{
	case WG_PIXEL_UNKNOWN:
		return wg::PixelFormat::Unknown;
	case WG_PIXEL_CUSTOM:
		return wg::PixelFormat::Custom;
	case WG_PIXEL_BGRA_8:
		return wg::PixelFormat::BGRA_8;
	case WG_PIXEL_BGR_8:
		return wg::PixelFormat::BGR_8;
    case WG_PIXEL_BGRX_8:
        return wg::PixelFormat::BGRX_8;
    case WG_PIXEL_BGR_565:
        return wg::PixelFormat::BGR_565;
    case WG_PIXEL_BGRA_4:
        return wg::PixelFormat::BGRA_4;
    case WG_PIXEL_I8:
        return wg::PixelFormat::I8;
    case WG_PIXEL_A8:
        return wg::PixelFormat::A8;
	}

	assert(false);									// Should never get here
	return wg::PixelFormat::Unknown;					// Just avoiding compiler warnings
}

inline void _convert(const wg::PixelDescription& f, WgPixelFormat& out )
{
	out.bits = f.bits;
	out.type = _convert(f.format);

	out.R_mask = f.R_mask;
	out.R_shift = f.R_shift;
	out.R_bits = f.R_bits;

	out.G_mask = f.G_mask;
	out.G_shift = f.G_shift;
	out.G_bits = f.G_bits;

	out.B_mask = f.B_mask;
	out.B_shift = f.B_shift;
	out.B_bits = f.B_bits;

	out.A_mask = f.A_mask;
	out.A_shift = f.A_shift;
	out.A_bits = f.A_bits;

}

inline void _convert(const WgPixelFormat& f, wg::PixelDescription& out )
{
	out.bits = f.bits;
	out.format = _convert(f.type);

	out.R_mask = f.R_mask;
	out.R_shift = f.R_shift;
	out.R_bits = f.R_bits;

	out.G_mask = f.G_mask;
	out.G_shift = f.G_shift;
	out.G_bits = f.G_bits;

	out.B_mask = f.B_mask;
	out.B_shift = f.B_shift;
	out.B_bits = f.B_bits;

	out.A_mask = f.A_mask;
	out.A_shift = f.A_shift;
	out.A_bits = f.A_bits;
}

inline WgAccessMode _convert(wg::AccessMode m)
{
	switch (m)
	{
	case wg::AccessMode::None:
		return WG_NO_ACCESS;
	case wg::AccessMode::ReadOnly:
		return WG_READ_ONLY;
	case wg::AccessMode::WriteOnly:
		return WG_WRITE_ONLY;
	case wg::AccessMode::ReadWrite:
		return WG_READ_WRITE;
	}
	assert(false);									// Should never get here
	return WG_NO_ACCESS;							// Just avoiding compiler warnings
}

inline wg::AccessMode _convert(WgAccessMode m)
{
	switch (m)
	{
	case WG_NO_ACCESS:
		return wg::AccessMode::None;
	case WG_READ_ONLY:
		return wg::AccessMode::ReadOnly;
	case WG_WRITE_ONLY:
		return wg::AccessMode::WriteOnly;
	case WG_READ_WRITE:
		return wg::AccessMode::ReadWrite;
	}
	assert(false);									// Should never get here
	return wg::AccessMode::None;					// Just avoiding compiler warnings
}

