
#pragma once

#include <wg_base.h>

namespace wg
{
	class Base
	{
	public:
		static char *		memStackAlloc(int bytes) { return WgBase::MemStackAlloc(bytes); }
		static void			memStackRelease(int bytes) { WgBase::MemStackRelease(bytes); }
	};
}