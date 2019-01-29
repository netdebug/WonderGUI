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

#ifndef WG_CHAIN_DOT_H
#	include <wg_chain.h>
#endif

class WgMemStack
{
public:
	WgMemStack( int blockSize );
	~WgMemStack() {}

	char *  Alloc( int bytes );
	void	Release( int bytes );
	void	Clear();

private:

	class BlockHeader : public WgLink
	{
	public:
		BlockHeader(int bytes);
		~BlockHeader();

		char *	pBlock;
		int		size;
		int		allocated;
	};

	int						m_blockSize;
	WgChain<BlockHeader>	m_blocks;
};
