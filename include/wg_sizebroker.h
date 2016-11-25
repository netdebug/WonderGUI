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
#ifndef	WG_SIZEBROKER_DOT_H
#define	WG_SIZEBROKER_DOT_H


class WgSizeBrokerItem
{
public:	
	int		preferred;		// Preferred length for this item (input)
	int		min;			// Min length for this item (input)
	int		max;			// Max length for this item (input)
	float	weight;			// Weight for this item (input)
	int		output;			// Length or preferred length for this item (output)
};


class WgSizeBroker
{
public:
	virtual ~WgSizeBroker();
    
	virtual int SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const = 0;
	virtual int SetPreferredLengths( WgSizeBrokerItem * pItems, int nItems ) const = 0;
	virtual bool MayAlterPreferredLengths() const = 0;

protected:
	WgSizeBroker();
};


class WgUniformSizeBroker : public WgSizeBroker
{
public:
    WgUniformSizeBroker() {};
    ~WgUniformSizeBroker() {};
    
	int SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const;
	int SetPreferredLengths( WgSizeBrokerItem * pItems, int nItems ) const;
	bool MayAlterPreferredLengths() const;

protected:
    int     _findLongestUnified( WgSizeBrokerItem * pItems, int nItems ) const;
    void    _setOutputs( WgSizeBrokerItem * pItems, int nItems, int value ) const;

    
};


class WgScalePreferredSizeBroker : public WgSizeBroker
{
public:
    WgScalePreferredSizeBroker() {};
    ~WgScalePreferredSizeBroker() {};
    
	int SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const;
	int SetPreferredLengths( WgSizeBrokerItem * pItems, int nItems ) const;
	bool MayAlterPreferredLengths() const;
    
protected:
    
};



#endif //WG_SIZEBROKER_DOT_H