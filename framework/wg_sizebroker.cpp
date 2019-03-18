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
#include <wg_sizebroker.h>




WgSizeBroker::WgSizeBroker()
{
}

WgSizeBroker::~WgSizeBroker()
{
}

/*
int WgSizeBroker::SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int _totalLength ) const
{
	int totalLength = 0;
	for( int i = 0 ; i < nItems ; i++ )
	{
		totalLength += pItems[i].preferred;
		pItems[i].output = pItems[i].preferred;
	}
	return totalLength;
}

int WgSizeBroker::SetPreferredLengths( WgSizeBrokerItem * pItems, int nItems ) const
{
	int totalLength = 0;
	for( int i = 0 ; i < nItems ; i++ )
	{	
		totalLength += pItems[i].preferred;
		pItems[i].output = pItems[i].preferred;
	}
	return totalLength;
}

bool WgSizeBroker::MayAlterPreferredLengths() const
{
	return false;
}
*/

int WgUniformSizeBroker::SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const
{
    if( nItems == 0 )
        return 0;
    
    // Gather some data we need
        
    int staticLength = 0;               // Total preferred length of all static (non-stretching) items
    int nUnifiedItems = 0;                // Number of static items
    float extraWeight = 0;                // Total "extra weight", of all items with weight > 1.
        
    for( int i = 0 ; i < nItems ; i++ )
    {
        if( pItems[i].weight == 0.f )
            staticLength += pItems[i].preferred;
        else if( pItems[i].weight <= 1.f )
            nUnifiedItems++;
        else
            extraWeight += pItems[i].weight-1.f;
    }
        
    // Calculate baseLength and extraLengthPerWeightUnit
        
    int unifiedLength = nUnifiedItems > 0 ? (totalLength - staticLength) / nUnifiedItems : 0;
    int paddingLength = totalLength - staticLength - unifiedLength*nUnifiedItems;
    float paddingPerWeightUnit = extraWeight > 0 ? paddingLength / extraWeight : 0.f;

    // Loop through items and set their length
        
    float paddingAcc = 0.0001f;
    int total = 0;
    int length;
        
    for( int i = 0 ; i < nItems ; i++ )
    {
        if( pItems[i].weight == 0 )
        {
            length = pItems[i].preferred;
        }
        else if( pItems[i].weight > 1 )
        {
            paddingAcc += (pItems[i].weight-1) * paddingPerWeightUnit;
            length = (int) paddingAcc;
            paddingAcc -= (int) paddingAcc;
        }
        else
            length = unifiedLength;
        
        pItems[i].output = length;
        total += length;
    }
    return total;
}

int WgUniformSizeBroker::SetPreferredLengths( WgSizeBrokerItem * pItems, int nItems ) const
{
    if( nItems == 0 )
        return 0;
    
    int unifiedLength = _findLongestUnified(pItems, nItems);
    int total = 0;
    int length;
    
    for( int i = 0 ; i < nItems ; i++ )
    {
        if( pItems[i].weight == 0.f )
            length = pItems[i].preferred;
        else if( pItems[i].weight > 1.f )
            length = 0;
        else
            length = unifiedLength;
        
        pItems[i].output = length;
        total += length;
    }
    
    return total;
}

bool WgUniformSizeBroker::MayAlterPreferredLengths() const
{
    return false; //true;
}

int WgUniformSizeBroker::_findLongestUnified( WgSizeBrokerItem * pItems, int nItems ) const
{
    int longest = 0;
    for( int i = 0 ; i < nItems ; i++ )
        if( pItems[i].preferred > longest && pItems[i].weight == 1.f )
            longest = pItems[i].preferred;
    return longest;
}


int WgScalePreferredSizeBroker::SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const
{
    return SetPreferredLengths( pItems, nItems );
}

int WgScalePreferredSizeBroker::SetPreferredLengths( WgSizeBrokerItem * pItems, int nItems ) const
{
    int total = 0;
    for( int i = 0 ; i < nItems ; i++ )
    {
        int def = (int) (pItems[i].preferred * pItems[i].weight);
        pItems[i].output = def;
        total += def;
    }
    return total;
}

bool  WgScalePreferredSizeBroker::MayAlterPreferredLengths() const
{
    return true;
}

int WgScaleWeightSizeBroker::SetItemLengths(WgSizeBrokerItem * pItems, int nItems, int totalLength) const
{
	if (nItems == 0)
		return 0;

	// Gather some data we need

	int staticLength = 0;               // Total preferred length of all static (non-stretching) items
	float totalWeight = 0;                // Total weight of all items.

	for (int i = 0; i < nItems; i++)
	{
		if (pItems[i].weight <= 0.f)
			staticLength += pItems[i].preferred;
		else
			totalWeight += pItems[i].weight;
	}

	// Calculate baseLength and extraLengthPerWeightUnit

	float lengthPerWeightUnit = totalWeight > 0 ? (totalLength - staticLength) / totalWeight : 0;

	if (lengthPerWeightUnit < 0.f)
		lengthPerWeightUnit = 0;


	// Loop through items and set their length

	int total = 0;
	int length;

	for (int i = 0; i < nItems; i++)
	{
		if (pItems[i].weight <= 0)
			length = pItems[i].preferred;
		else
			length = pItems[i].weight * lengthPerWeightUnit;

		pItems[i].output = length;
		total += length;
	}
	return total;
}

int WgScaleWeightSizeBroker::SetPreferredLengths(WgSizeBrokerItem * pItems, int nItems) const
{
	if (nItems == 0)
		return 0;

	float lengthPerWeight = _findNeededLengthPerWeight(pItems, nItems);
	int total = 0;
	int length;

	for (int i = 0; i < nItems; i++)
	{
		if (pItems[i].weight <= 0.f)
			length = pItems[i].preferred;
		else
			length = int(lengthPerWeight*pItems[i].weight);

		pItems[i].output = length;
		total += length;
	}

	return total;
}

bool WgScaleWeightSizeBroker::MayAlterPreferredLengths() const
{
	return false; //true;
}

float WgScaleWeightSizeBroker::_findNeededLengthPerWeight(WgSizeBrokerItem * pItems, int nItems) const
{
	float largest = 0;
	for (int i = 0; i < nItems; i++)
	{
		if (pItems[i].weight > 0)
		{
			int unifiedLengthNeeded = pItems[i].preferred / pItems[i].weight;
			if (unifiedLengthNeeded  > largest)
				largest = unifiedLengthNeeded;
		}
	}
	return largest;
}
