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


#ifndef	WG_VALUEFORMAT_DOT_H
#define WG_VALUEFORMAT_DOT_H

#ifndef	WG_TYPES_DOT_H
#	include <wg_types.h>
#endif

#ifndef WG_TEXTPROP_DOT_H
#	include <wg_textprop.h>
#endif

#ifndef WG_CHARSEQ_DOT_H
#	include <wg_charseq.h>
#endif

#ifndef WG_STRING_DOT_H
#	include <wg_string.h>
#endif

#ifndef WG_POINTERS_DOT_H
#	include <wg_pointers.h>
#endif

namespace wg 
{
	
	class ValueFormat;
	typedef	WgStrongPtr<ValueFormat,Object_p>		ValueFormat_p;
	typedef	WgWeakPtr<ValueFormat,Object_wp>	ValueFormat_wp;
	
	//____ ValueFormat __________________________________________________________
	
	class ValueFormat : public Object
	{
		friend class LegacyTextField;
		public:
			static ValueFormat_p	create() { return ValueFormat_p(new ValueFormat()); }
			static ValueFormat_p	create( const CharSeq& format )  { return ValueFormat_p(new ValueFormat(format)); }
			static ValueFormat_p	create( const ValueFormat_p& pIn ) { return ValueFormat_p(new ValueFormat(pIn)); }
			static ValueFormat_p	create(	int nInt, int nDec, int grouping = 0, bool bPlus = false,
							Uint16 separator = 0xA0 /*0xA0=NO_BREAK_SPACE*/, Uint16 period = 0x2e, bool bForcePeriod = false, const char * pPrefix = 0, const char * pSuffix = 0 )
			{ return ValueFormat_p(new ValueFormat(nInt,nDec,grouping,bPlus,separator,period,bForcePeriod,pPrefix,pSuffix)); }
	
			bool		isInstanceOf( const char * pClassName ) const;
			const char *className( void ) const;
			static const char	CLASSNAME[];
			static ValueFormat_p	cast( const Object_p& pObject );
	
			void setFormat( const CharSeq& format );
			void setFormat( const ValueFormat_p& pFormat );
	
			void setFormat( int nInt, int nDec, int grouping, bool bPlus = false,
							Uint16 _separator = 0, Uint16 period = 0, bool bForcePeriod = false );
	
			void setPrefix( const String& str );
			void setPrefix( const CharSeq& seq );
			inline const String& getPrefix() const { return prefix; }
	
			void setSuffix( const String& str );
			void setSuffix( const CharSeq& seq );
			inline const String& getSuffix() const { return suffix; }
	
			inline void setTextProperties( Textprop_p& _pProperties ) { pTextProperties = _pProperties; bSetTextprop = true; }
			inline void clearTextProperties() { bSetTextprop = false; }
			inline Textprop_p getTextProperties() const { return pTextProperties; }
	
			inline void setNoDecimalThreshold( int maxValue ) { noDecimalThreshold = maxValue; }
			inline void clearNoDecimalThreshold() { noDecimalThreshold = 0; }
			inline int  getNoDecimalThreshold() const { return noDecimalThreshold; }
	
			inline void setForceDecimals( bool bForce ) { bForceDecimals = bForce; }
			inline bool	getForceDecimals() const { return bForceDecimals; }
	
			inline void setZeroIsNegative( bool bNegative ) { bZeroIsNegative = bNegative; }
			inline bool	getZeroIsNegative() const { return bZeroIsNegative; }
	
			inline void		setPeriod(Uint16 _period) { period = _period; }
			inline Uint16	getPeriod() const { return period; }
	
			inline void		setForcePeriod(bool _bForcePeriod) { bForcePeriod = _bForcePeriod; }
			inline bool		getForcePeriod() const { return bForcePeriod; }
	
			inline void		setIntegers(int nInt) { integers = nInt; }
			inline Uint8	getIntegers() const { return integers; }
	
			inline void		setDecimals(int nDec) { decimals= nDec; }
			inline Uint8	getDecimals() const { return decimals; }
	
			inline void		setGrouping(int _grouping) { grouping = _grouping; }
			inline Uint8	getGrouping() const { return grouping; }
	
			inline void		setPlus(bool _bPlus) { bPlus = _bPlus; }
			inline bool		getPlus() const { return bPlus; }
	
			inline void		setSeparator(Uint16 _separator) { separator = _separator; }
			inline Uint16	getSeparator() const { return separator; }
	
			inline int		_getScale() const { return scale; }
	
	protected:
			ValueFormat();
			ValueFormat( const CharSeq& format );
			ValueFormat( const ValueFormat_p& pIn );
			ValueFormat(	int nInt, int nDec, int grouping = 0, bool bPlus = false,
							Uint16 _separator = 0xA0 /*0xA0=NO_BREAK_SPACE*/, Uint16 period = 0x2e, bool bForcePeriod = false, const char * pPrefix = 0, const char * pSuffix = 0 );
	
	
			Uint8		integers;			/// Lowest number of integers to display, padded with zeroes.
			Uint8		decimals;			/// Number of decimals to display.
			Uint8		grouping;			/// Number of integers to group together. 0 = no grouping
			Uint16		separator;			/// Character to separate integer groups with.
			Uint16		period;				/// Character used for separating integer and decimal part
			String	prefix;				/// Characters preceding the value, like $, £ or similar.
			String	suffix;				/// Characters following the value.
			bool		bPlus;				/// Set if a plus sign should be preceding positive value.
			bool		bZeroIsNegative;	/// Set if zero value should be deemed negative and be preceeded by minus.
			bool		bForcePeriod;		/// Display period even if there are no decimals?
		Textprop_p 	pTextProperties;	/// Text properties for generated text.
			bool		bSetTextprop;		/// Set if properties should be set for text.
			bool		bForceDecimals;		/// Display decimals even if they are 0
			int			noDecimalThreshold;	/// If the value is >= this, no decimals will be displayed.
			int			scale;				/// Value is scaled by this value before being formatted.
	};
	
	//____ ValueFormatter _______________________________________________________
	
	/*
		Ideas:
	
				* Use textprops on format-string.
							(props for minus determines props for negative values,
							(props for leading '1' determines props for positive values,
							 props for first decimal determines props for decimals.
							 
							 Prop override order for number portion:
								base from '1'.
								override with decimalprops (if decimals).
								override with minusprop (if negative value).
	
							Prop override order for prefix/subfix:
								base from individual prefix/subfix characters.
								override with minusprop (if negative value).
							 )
	
			    * Position of minus-sign (before or after prefix).
				* Support hex-values (choose upper/lower).
				* Setting for round off or truncate decimals.
	
				* Skip bForcePeriod, bZeroIsNegative, bSetTextprop and pTextProperties.
				* Replace noDecimalTreshold with "value digits"?
	*/
	
	/*
	class ValueFormatter
	{
	public:
		ValueFormatter();
		ValueFormatter( const CharSeq& format );
		~ValueFormatter();
	
		void setFormat( const CharSeq& format );
	
		String prefix() const;
		String suffix() const;
		int		 decimals() const { return m_format.decimals; }
	
		String format( Sint64 value ) const;
		String formatNoPreSuffix( Sint64 value ) const;
	
	private:
	
		void	_formatValue( CharBuffer * pBuffer, Sint64 value ) const;
	
		ValueFormat	m_format;
	};
	
	*/
	

} // namespace wg
#endif
