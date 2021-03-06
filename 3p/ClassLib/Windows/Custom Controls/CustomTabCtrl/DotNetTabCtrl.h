#ifndef __DOTNET_TABCTRL_H__
#define __DOTNET_TABCTRL_H__

/////////////////////////////////////////////////////////////////////////////
// CDotNetTabCtrl - Tab control derived from CCustomTabCtrl
//    meant to look like the tabs in VS.Net (MDI tabs,
//    solution explorer tabs, etc.)
// CDotNetButtonTabCtrl - Tab control derived from CCustomTabCtrl
//    meant to look like VS.Net view of HTML with the Design/HTML buttons
//  
//
// Written by Daniel Bowen (dbowen@es.com).
// Copyright (c) 2001-2004 Daniel Bowen.
//
// ClassLib version by Jan van den Baard
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//
// History (Date/Author/Description):
// ----------------------------------
//
// 2004/12/25: Jan van den Baard
// - First implementation for the ClassLib library.
// - Adjusted class names to fit the names of the classes in ClassLib. This 
//   basically means the "C" prefixes at the start of the class names are 
//   replaced by "Cls".
// - Adjusted indentation etc. a bit more to my style, This is not a snear
//   to the style of the original authors. It is just my way to force me to
//   read all the code so that I can understand it better.
// - Moved the DrawCloseButton(), DrawScrollButtons(), CalcSize_CloseButton()
//   and CalcSize_ScrollButtons() to a seperate class called 
//   "ClsCloseScrollTabCtrl" defined in CustomTabCtrl.h. This because I
//   want to re-use this code in other tab-implementations.
// - Just before an image is rendered the background color is set to CLR_NONE.
//   After the image is rendered the background color of the image list is
//   restored. Allows the usage of the system image list.
//
// 2004/12/26: Jan van den Baard
// - Selected tab background color and the inactive text color did not get 
//   updated properly on a settings change.
// - Tab labels are now rendered using end ellipsis instead of path ellipsis.
// - Added the SetSystemImageList() method. When using the system image list
//   the control will render and measure the images from the system image list.
//

#include "customtabctrl.h"

template< typename TItem = CCustomTabItem, bool t_bDotNetAdjust = true >
class ClsDotNetTabCtrl :  public ClsCloseScrollTabCtrl< TItem, t_bDotNetAdjust >
{
	_NO_COPY( ClsDotNetTabCtrl );
	typedef ClsCloseScrollTabCtrl< TItem, t_bDotNetAdjust > customTabClass;
	// Constructor
public:
	ClsDotNetTabCtrl() :
		m_clrTextInactiveTab( ::GetSysColor( COLOR_GRAYTEXT )),
		m_clrSelectedTab( ::GetSysColor( COLOR_BTNFACE )),
		m_nFontSizeTextTopOffset( 0 ),
		m_nMinWidthToDisplayText( 12 ),
		m_bUseSystemImageList( false )
	{
	}
	virtual ~ClsDotNetTabCtrl() 
	{
		// If we are using the system image list we must detach
		// it because we can not have it destroyed.
		if ( m_bUseSystemImageList )
		{
			m_imageList.Detach();
		}
	}

	// Attributes.
	ClsImageList SetSystemImageList( bool bRedraw = true ) 
	{ 
		ClsImageList imageListOld = m_imageList;
		m_bUseSystemImageList = true;
		m_imageList.Attach( ClsGetApp()->GetSystemImageList());
		if ( bRedraw )
		{
			this->UpdateLayout();
			this->Invalidate();
		}
		return imageListOld;
	}

	ClsImageList SetImageList( HIMAGELIST hImageList )
	{
		m_bUseSystemImageList = false;
		return baseClass::SetImageList( hImageList );
	}

// Message Handling
public:
	BEGIN_MSG_MAP(customTabClass)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSettingChange)
	END_MSG_MAP()

	virtual LRESULT OnSettingChange( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
	{
		DWORD dwStyle = this->GetStyle();

		// Initialize/Reinitialize font
		// Visual Studio.Net seems to use the "icon" font for the tabs
		LOGFONT lfIcon = { 0 };
		::SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof( lfIcon ), &lfIcon, 0 );

		m_clrTextInactiveTab = ::GetSysColor( COLOR_GRAYTEXT );
		m_clrSelectedTab = ::GetSysColor( COLOR_BTNFACE );

		bool bResetFont = true;
		if( m_font.IsValid())
		{
			LOGFONT lf = { 0 };
			if( m_font.GetLogFont( &lf ))
			{
				if( _tcsicmp( lf.lfFaceName, lfIcon.lfFaceName ) == 0 &&
					      lf.lfHeight == lfIcon.lfHeight )
				{
					bResetFont = false;
				}
			}
		}

		if( bResetFont )
		{
			if( m_font.IsValid()) m_font.Delete();
			if( m_fontSel.IsValid()) m_fontSel.Delete();

			if ( m_font.CreateFontIndirect( &lfIcon ) == FALSE )
			{
				m_font.Attach( ::GetStockObject( DEFAULT_GUI_FONT ));
			}

			if( CTCS_BOLDSELECTEDTAB == ( dwStyle & CTCS_BOLDSELECTEDTAB ))
			{
				lfIcon.lfWeight = FW_BOLD;
			}

			if ( m_fontSel.CreateFontIndirect( &lfIcon ) == FALSE )
			{
				m_fontSel.Attach( ::GetStockObject( DEFAULT_GUI_FONT ));
			}
		}

		// Background brush
		if( m_hbrBackground.IsValid()) m_hbrBackground.Delete();
		ClsWindowDC dcWindow( NULL );
		int nBitsPerPixel = dcWindow.GetDeviceCaps( BITSPIXEL );
		if( nBitsPerPixel > 8 )
		{
			//COLORREF clrBtnHilite = ::GetSysColor(COLOR_BTNHILIGHT);
			//COLORREF clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
			//m_clrBackground =
			//	RGB( GetRValue(clrBtnFace) + ((GetRValue(clrBtnHilite) - GetRValue(clrBtnFace)) / 3 * 2),
			//		GetGValue(clrBtnFace) + ((GetGValue(clrBtnHilite) - GetGValue(clrBtnFace)) / 3 * 2),
			//		GetBValue(clrBtnFace) + ((GetBValue(clrBtnHilite) - GetBValue(clrBtnFace)) / 3 * 2),
			//	);
			//m_hbrBackground.CreateSolidBrush(m_clrBackground);

			COLORREF clrBtnFace = ::GetSysColor( COLOR_BTNFACE );

			// This is a brave attempt to mimic the algorithm that Visual Studio.Net
			// uses to calculate the tab's background color and inactive tab color.
			// The other colors that VS.Net uses seems to be standard ones,
			// but these two colors are calculated.
			BYTE nRed = 0, nGreen = 0, nBlue = 0, nMax = 0;

			// Early experiments seemed to reveal that the background color is dependant
			// on COLOR_BTNFACE.  The following algorithm is just an attempt
			// to match several empirical results.  I tested with 20 variations
			// on COLOR_BTNFACE and kept track of what the tab background became.
			// I then brought the numbers into Excel, and started crunching on the numbers
			// until I came up with a formula that seems to pretty well match.

			nRed = GetRValue( clrBtnFace );
			nGreen = GetGValue( clrBtnFace );
			nBlue = GetBValue( clrBtnFace );

			nMax = ( nRed > nGreen ) ? (( nRed > nBlue ) ? nRed : nBlue ) : (( nGreen > nBlue ) ? nGreen : nBlue );
			const BYTE nMagicBackgroundOffset = ( nMax > ( 0xFF - 35 )) ? ( BYTE )( 0xFF - nMax ) : ( BYTE )35;
			if( nMax == 0 )
			{
				nRed = ( BYTE )( nRed + nMagicBackgroundOffset );
				nGreen = ( BYTE )( nGreen + nMagicBackgroundOffset );
				nBlue = ( BYTE )( nBlue + nMagicBackgroundOffset );
			}
			else
			{
				nRed = ( BYTE )( nRed + ( nMagicBackgroundOffset * ( nRed / ( double )nMax ) + 0.5 ));
				nGreen = ( BYTE )( nGreen + ( nMagicBackgroundOffset * ( nGreen / ( double)nMax ) + 0.5 ));
				nBlue = ( BYTE )( nBlue + ( nMagicBackgroundOffset * ( nBlue / ( double )nMax ) + 0.5 ));
			}

			m_hbrBackground.CreateSolidBrush( RGB( nRed, nGreen, nBlue ));

			// The inactive tab color seems to be calculated in a similar way to
			// the tab background, only instead of lightening BNTFACE, it darkens GRAYTEXT.
			COLORREF clrGrayText = ::GetSysColor( COLOR_GRAYTEXT );

			nRed = GetRValue( clrGrayText );
			nGreen = GetGValue( clrGrayText );
			nBlue = GetBValue( clrGrayText );

			nMax = ( nRed > nGreen ) ? (( nRed > nBlue ) ? nRed : nBlue ) : (( nGreen > nBlue ) ? nGreen : nBlue );
			const BYTE nMagicInactiveOffset = 43;
			if( nMax != 0 )
			{
				if( nRed < nMagicInactiveOffset )
					nRed = ( BYTE )( nRed / 2 );
				else
					nRed = ( BYTE )( nRed - ( nMagicInactiveOffset * ( nRed / ( double )nMax) + 0.5 ));

				if( nGreen < nMagicInactiveOffset )
					nGreen = ( BYTE )( nGreen / 2 );
				else
					nGreen = ( BYTE )( nGreen - ( nMagicInactiveOffset * ( nGreen / ( double )nMax ) + 0.5 ));

				if( nBlue < nMagicInactiveOffset )
					nBlue = ( BYTE )( nBlue / 2 );
				else
					nBlue = ( BYTE )( nBlue - ( nMagicInactiveOffset * ( nBlue / ( double )nMax ) + 0.5 ));
			}

			m_clrTextInactiveTab = RGB( nRed, nGreen, nBlue );
		}
		else
		{
			m_hbrBackground.CreateSysColorBrush( COLOR_WINDOW );
			m_clrTextInactiveTab = ::GetSysColor( COLOR_GRAYTEXT );
		}

		m_settings.iIndent = 5;
		m_settings.iPadding = 4;
		m_settings.iMargin = 3;
		m_settings.iSelMargin = 3;

		int nHeightLogicalUnits = -lfIcon.lfHeight;
		// In MSDN for "LOGFONT", they give the following formula for calculating
		// the log font height given a point size.
		//long lfHeight = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);

		const int nNominalFontLogicalUnits = 11;	// 8 point Tahoma with 96 DPI
		m_nFontSizeTextTopOffset = ( BYTE )(( nHeightLogicalUnits - nNominalFontLogicalUnits ) / 2 );

		this->UpdateLayout();
		this->Invalidate();
		return 0;
	}


// Overrideables
	virtual void DrawBackground( RECT rcClient, LPNMCTCCUSTOMDRAW lpNMCustomDraw )
	{
		ClsDC dc( lpNMCustomDraw->nmcd.hdc );

		// Set up the text color and background mode
		dc.SetTextColor( lpNMCustomDraw->clrBtnText );
		dc.SetBkMode( TRANSPARENT );

		RECT rcClip = { 0 };
		dc.GetClipBox( &rcClip );

		if( ::EqualRect( &rcClip, &m_rcCloseButton ) ||
		    ::EqualRect( &rcClip, &m_rcScrollRight ) ||
		    ::EqualRect( &rcClip, &m_rcScrollLeft ))
		{
			// Paint needed in only "other button" area

			HGDIOBJ hOldBrush = dc.SelectObject(( HGDIOBJ )lpNMCustomDraw->hBrushBackground );
			dc.PatBlt( rcClip.left, rcClip.top, rcClip.right - rcClip.left, rcClip.bottom - rcClip.top, PATCOPY );
			dc.SelectObject( hOldBrush );
		}
		else
		{
			// Paint needed in tab item area or more

			// Erase Background
			//  (do it here instead of a handler for WM_ERASEBKGND
			//   so that we can do flicker-free drawing with the help
			//   of COffscreenDrawRect that's in the base class)

			// TODO: Don't "erase" entire client area.
			//  Do a smarter erase of just what needs it

			RECT rc = rcClient;

			HGDIOBJ hOldBrush = dc.SelectObject( lpNMCustomDraw->hBrushBackground );
			dc.PatBlt( rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY );
			dc.SelectObject( hOldBrush );

			// Connect with the client area.
			DWORD dwStyle = this->GetStyle();

			if( CTCS_BOTTOM == ( dwStyle & CTCS_BOTTOM ))
			{
				rc.bottom = rc.top + 3;
				dc.FillSolidRect( &rc, lpNMCustomDraw->clrBtnFace );

				ClsPen penText;
				penText.CreatePen(PS_SOLID, 1, lpNMCustomDraw->clrBtnText );
				HGDIOBJ penOld = dc.SelectObject( penText );

				dc.MoveTo( rc.left, rc.bottom );
				dc.LineTo( rc.right, rc.bottom );

				dc.SelectObject( penOld );
			}
			else
			{
				int nOrigTop = rc.top;
				rc.top = rc.bottom - 2;
				dc.FillSolidRect( &rc, lpNMCustomDraw->clrBtnFace );

				ClsPen penHilight;
				penHilight.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrBtnHighlight );
				HGDIOBJ penOld = dc.SelectObject( penHilight );

				dc.MoveTo( rc.left, rc.top - 1 );
				dc.LineTo( rc.right, rc.top - 1 );

				rc.top = nOrigTop;

				ClsPen penShadow, pen3D;
				penShadow.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrBtnShadow );
				pen3D.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrBtnFace );
				dc.SelectObject( penShadow );

				dc.MoveTo( rc.left, rc.bottom );
				dc.LineTo( rc.left, rc.top );
				dc.LineTo( rc.right - 1, rc.top );
						
				if( 0 == ( dwStyle & CTCS_FLATEDGE ))
				{
					dc.SelectObject( penHilight );
				}
				dc.LineTo( rc.right - 1, rc.bottom );

				dc.SelectObject( pen3D );
				dc.MoveTo( rc.right - 2, rc.bottom - 3 );
				dc.LineTo( rc.right - 2, rc.top );
				dc.MoveTo( rc.left + 1, rc.bottom - 3 );
				dc.LineTo( rc.left + 1, rc.top );

				dc.SelectObject( penOld );
			}
		}
	}

	virtual void DrawItem_InitBounds( DWORD dwStyle, RECT rcItem, RECT& rcTab, RECT& rcText, int& nIconVerticalCenter )
	{
		if( CTCS_BOTTOM == ( dwStyle & CTCS_BOTTOM ))
		{
			rcTab.top += 3;
			rcTab.bottom -= 2;

			rcText.top = rcTab.top+1 + m_nFontSizeTextTopOffset;
			rcText.bottom = rcItem.bottom;
			//nIconVerticalCenter = rcTab.top + (rc.bottom - rcTab.top) / 2;
			//nIconVerticalCenter = rcTab.top + rcText.Height() / 2;
			nIconVerticalCenter = ( rcItem.bottom + rcItem.top ) / 2 + rcTab.top / 2;
		}
		else
		{
			rcTab.top += 3;
			rcTab.bottom -= 2;

			rcText.top = rcItem.top+1 + m_nFontSizeTextTopOffset;
			rcText.bottom = rcItem.bottom;
			nIconVerticalCenter = ( rcItem.bottom + rcItem.top ) / 2 + rcTab.top / 2;
		}
	}

	virtual void DrawItem_TabSelected( DWORD dwStyle, LPNMCTCCUSTOMDRAW lpNMCustomDraw, RECT& rcTab )
	{
		// Tab is selected, so paint tab folder

		bool bHighlighted = ( CDIS_MARKED == ( lpNMCustomDraw->nmcd.uItemState & CDIS_MARKED ));

		ClsDC dc( lpNMCustomDraw->nmcd.hdc );

		rcTab.right--;
		if( bHighlighted )
		{
			dc.FillSolidRect( &rcTab, lpNMCustomDraw->clrHighlight );
		}
		else
		{
			dc.FillSolidRect( &rcTab, lpNMCustomDraw->clrSelectedTab );
		}

		ClsPen penText, penHilight;
		penText.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrBtnText );
		penHilight.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrBtnHighlight );

		if( CTCS_BOTTOM == ( dwStyle & CTCS_BOTTOM ))
		{
			HGDIOBJ penOld = dc.SelectObject( penText );

			dc.MoveTo( rcTab.right, rcTab.top );
			dc.LineTo( rcTab.right, rcTab.bottom );
			dc.LineTo( rcTab.left, rcTab.bottom );
			dc.SelectObject( penHilight );
			dc.LineTo( rcTab.left, rcTab.top - 1 );

			dc.SelectObject( penOld );
		}
		else
		{
			HGDIOBJ penOld = dc.SelectObject( penHilight );

			dc.MoveTo( rcTab.left, rcTab.bottom - 1 );
			dc.LineTo( rcTab.left, rcTab.top );
			dc.LineTo( rcTab.right, rcTab.top );
			dc.SelectObject( penText );
			dc.LineTo( rcTab.right, rcTab.bottom );

			dc.SelectObject( penOld );
		}
	}

	virtual void DrawItem_TabInactive( DWORD dwStyle, LPNMCTCCUSTOMDRAW lpNMCustomDraw, RECT& rcTab )
	{
		// Tab is not selected

		bool bHighlighted = ( CDIS_MARKED == ( lpNMCustomDraw->nmcd.uItemState & CDIS_MARKED ));

		int nItem = ( int )lpNMCustomDraw->nmcd.dwItemSpec;
		ClsDC dc( lpNMCustomDraw->nmcd.hdc );

		if( bHighlighted )
		{
			if( CTCS_BOTTOM == ( dwStyle & CTCS_BOTTOM ))
			{
				RECT rcHighlight = { rcTab.left + 1, rcTab.top + 3, rcTab.right - 2, rcTab.bottom - 1 };
				if( nItem - 1 == m_iCurSel ) rcHighlight.left += 1;  // Item to the right of the selected tab
				dc.FillSolidRect( &rcHighlight, lpNMCustomDraw->clrHighlight );
			}
			else
			{
				RECT rcHighlight = { rcTab.left + 1, rcTab.top + 2, rcTab.right - 2, rcTab.bottom - 2 };
				if( nItem - 1 == m_iCurSel) rcHighlight.left += 1;  // Item to the right of the selected tab
				dc.FillSolidRect( &rcHighlight, lpNMCustomDraw->clrHighlight );
			}
		}

		// Draw division line on right, unless we're the item
		// on the left of the selected tab
		if( nItem + 1 == m_iCurSel )
		{
			// Item just left of selected tab
		}
		else
		{
			ClsPen pen;
			pen.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrBtnShadow );
			HGDIOBJ penOld = dc.SelectObject( pen );
			if( CTCS_BOTTOM == ( dwStyle & CTCS_BOTTOM ))
			{
				// Important!  Be sure and keep within "our" tab area horizontally
				dc.MoveTo( rcTab.right - 1, rcTab.top + 3 );
				dc.LineTo( rcTab.right - 1, rcTab.bottom - 1 );
			}
			else
			{
				// Important!  Be sure and keep within "our" tab area horizontally
				dc.MoveTo( rcTab.right - 1, rcTab.top + 2 );
				dc.LineTo( rcTab.right - 1, rcTab.bottom - 2 );
			}
			dc.SelectObject( penOld );
		}
	}

	virtual void DrawItem_ImageAndText( DWORD /*dwStyle*/, LPNMCTCCUSTOMDRAW lpNMCustomDraw, int nIconVerticalCenter, RECT& rcTab, RECT& rcText )
	{
		ClsDC dc( lpNMCustomDraw->nmcd.hdc );
		bool bHighlighted = ( CDIS_MARKED == ( lpNMCustomDraw->nmcd.uItemState & CDIS_MARKED ));
		bool bSelected = ( CDIS_SELECTED == ( lpNMCustomDraw->nmcd.uItemState & CDIS_SELECTED ));
		bool bHot = ( CDIS_HOT == ( lpNMCustomDraw->nmcd.uItemState & CDIS_HOT ));
		int nItem = ( int )lpNMCustomDraw->nmcd.dwItemSpec;

		TItem* pItem = this->GetItem( nItem );

		HGDIOBJ hOldFont = NULL;
		if( bSelected )
		{
			hOldFont = dc.SelectObject(( HGDIOBJ )lpNMCustomDraw->hFontSelected );
		}
		else
		{
			hOldFont = dc.SelectObject(( HGDIOBJ )lpNMCustomDraw->hFontInactive );
		}

		COLORREF crPrevious = 0;
		if( bHighlighted )
		{
			crPrevious = dc.SetTextColor( lpNMCustomDraw->clrHighlightText );
		}
		else if( bHot )
		{
			crPrevious = dc.SetTextColor( lpNMCustomDraw->clrHighlightHotTrack );
		}
		else if( bSelected )
		{
			crPrevious = dc.SetTextColor( lpNMCustomDraw->clrTextSelected );
		}
		else
		{
			crPrevious = dc.SetTextColor( lpNMCustomDraw->clrTextInactive );
		}

		//--------------------------------------------
		// This is how CDotNetTabCtrlImpl interprets padding, margin, etc.:
		//
		//  M - Margin
		//  P - Padding
		//  I - Image
		//  Text - Tab Text
		//
		// With image:
		//     __________________________
		//
		//    | M | I | P | Text | P | M |
		//     --------------------------
		//
		// Without image:
		//     ______________________
		//
		//    | M | P | Text | P | M |
		//     ----------------------

		//rcText.left += (bSelected ? m_settings.iSelMargin : m_settings.iMargin);
		rcText.left += m_settings.iMargin;
		rcText.right -= m_settings.iMargin;
		if ( pItem->UsingImage() && (( HIMAGELIST )m_imageList ))
		{
			// Draw the image.
			IMAGEINFO ii = { 0 };
			int nImageIndex = pItem->GetImageIndex();

			m_imageList.GetImageInfo( nImageIndex, &ii );

			if(( ii.rcImage.right - ii.rcImage.left ) < ( rcTab.right - rcTab.left ))
			{
				int nImageHalfHeight = ( ii.rcImage.bottom - ii.rcImage.top ) / 2;
				COLORREF crOldBkColor = m_imageList.SetBkColor( CLR_NONE );
				m_imageList.Draw( nImageIndex, dc, rcText.left, nIconVerticalCenter - nImageHalfHeight + m_nFontSizeTextTopOffset, ILD_NORMAL );
				m_imageList.SetBkColor( crOldBkColor );
			}

			// Offset on the right of the image.
			rcText.left += ( ii.rcImage.right - ii.rcImage.left );
		}

		if ( rcText.left + m_nMinWidthToDisplayText < rcText.right )
		{
			::InflateRect( &rcText, -m_settings.iPadding, 0 );

			ClsString sText = pItem->GetText();
			dc.DrawText( sText, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS );
		}

		dc.SetTextColor( crPrevious );
		dc.SelectObject( hOldFont );
	}

// Overrides for painting from CCustomTabCtrl
	virtual void InitializeDrawStruct( LPNMCTCCUSTOMDRAW lpNMCustomDraw )
	{
		//DWORD dwStyle = this->GetStyle();

		lpNMCustomDraw->hFontInactive = m_font;
		lpNMCustomDraw->hFontSelected = m_fontSel;
		lpNMCustomDraw->hBrushBackground = m_hbrBackground;
		lpNMCustomDraw->clrTextSelected = ::GetSysColor( COLOR_BTNTEXT );
		lpNMCustomDraw->clrTextInactive = m_clrTextInactiveTab;
		lpNMCustomDraw->clrSelectedTab = m_clrSelectedTab;
		lpNMCustomDraw->clrBtnFace = ::GetSysColor( COLOR_BTNFACE );
		lpNMCustomDraw->clrBtnShadow = ::GetSysColor( COLOR_BTNSHADOW );
		lpNMCustomDraw->clrBtnHighlight = ::GetSysColor( COLOR_BTNHIGHLIGHT );
		lpNMCustomDraw->clrBtnText = ::GetSysColor( COLOR_BTNTEXT );
		lpNMCustomDraw->clrHighlight = ::GetSysColor( COLOR_HIGHLIGHT );
#if WINVER >= 0x0500 || _WIN32_WINNT >= 0x0500
		lpNMCustomDraw->clrHighlightHotTrack = ::GetSysColor( COLOR_HOTLIGHT );
#else
		lpNMCustomDraw->clrHighlightHotTrack = ::GetSysColor( COLOR_HIGHLIGHT );
#endif
		lpNMCustomDraw->clrHighlightText = ::GetSysColor( COLOR_HIGHLIGHTTEXT );
	}

	virtual void DoPrePaint( RECT rcClient, LPNMCTCCUSTOMDRAW lpNMCustomDraw )
	{
		this->DrawBackground( rcClient, lpNMCustomDraw );
	}

	virtual void DoItemPaint( LPNMCTCCUSTOMDRAW lpNMCustomDraw )
	{
		bool bSelected = ( CDIS_SELECTED == ( lpNMCustomDraw->nmcd.uItemState & CDIS_SELECTED ));
		// NOTE: lpNMCustomDraw->nmcd.rc is in logical coordinates
		RECT &rcItem = lpNMCustomDraw->nmcd.rc;

		DWORD dwStyle = this->GetStyle();
		RECT rcTab = rcItem;
		RECT rcText = rcItem;
		int nIconVerticalCenter = 0;

		this->DrawItem_InitBounds( dwStyle, rcItem, rcTab, rcText, nIconVerticalCenter );

		if( bSelected )
		{
			this->DrawItem_TabSelected( dwStyle, lpNMCustomDraw, rcTab );
		}
		else
		{
			this->DrawItem_TabInactive( dwStyle, lpNMCustomDraw, rcTab );
		}

		this->DrawItem_ImageAndText( dwStyle, lpNMCustomDraw, nIconVerticalCenter, rcTab, rcText );
	}

	virtual void DoPostPaint( RECT /*rcClient*/, LPNMCTCCUSTOMDRAW lpNMCustomDraw )
	{
		DWORD dwStyle = this->GetStyle();

		if( 0 == ( dwStyle & ( CTCS_CLOSEBUTTON | CTCS_SCROLL )))
		{
			return;
		}

		// Close Button
		if( CTCS_CLOSEBUTTON == ( dwStyle & CTCS_CLOSEBUTTON ))
		{
			if(( m_iCurSel >= 0 ) && (( size_t )m_iCurSel < ( size_t )m_Items.GetSize()))
			{
				TItem* pItem = m_Items[ m_iCurSel ];
				_ASSERT( pItem != NULL );
				if(( pItem != NULL ) && pItem->CanClose())
				{
					this->DrawCloseButton( lpNMCustomDraw );
				}
			}
		}

		// Scroll Buttons
		if( CTCS_SCROLL == ( dwStyle & CTCS_SCROLL ))
		{
			this->DrawScrollButtons( lpNMCustomDraw );
		}
	}

// Overrides from CCustomTabCtrl
	virtual void CalcSize_NonClient( LPRECT prcTabItemArea )
	{
		// account for "non-client" areas
		// TODO: For the short term, we will use this
		//  for the non-client areas on the left and right.
		//  The drawing code for the tabs already accounts
		//  for the "non-client" areas on the top and bottom, and
		//  would need to be updated if we account for it here.
		//  Tab item rect methods also would need to be
		//  updated to account for the non-client areas
		//  on top and bottom (and effected drawing code
		//  would need to be updated).
		DWORD dwStyle = this->GetStyle();

		if( CTCS_BOTTOM == ( dwStyle & CTCS_BOTTOM ))
		{
			// TODO: Update to actually specify the
			//  non-client areas, and adjust all of the
			//  effected drawing code, as well as
			//  tab item rect related things
			//prcTabItemArea->top += 3;
		}
		else
		{
			prcTabItemArea->left += 2;
			prcTabItemArea->right -= 2;

			// TODO: Update to actually specify the top and bottom
			//  non-client areas, and adjust all of the
			//  effected drawing code, as well as
			//  tab item rect related things
			//prcTabItemArea->top += 1;
			//// We would have bottom as 3, but we want the
			//// selected tab to actually paint over highlight part
			//prcTabItemArea->bottom -= 2;
		}
	}

	virtual void UpdateLayout_Default( RECT rcTabItemArea )
	{
		long nMinInactiveWidth = 0x7FFFFFFF;
		long nMaxInactiveWidth = 0;

		//DWORD dwStyle = this->GetStyle();

		ClsGetDC dc( this );
		//HFONT hOldFont = dc.SelectFont(lpNMCustomDraw->hFontInactive);
		HGDIOBJ hOldFont = dc.SelectObject( m_font );

		LONG nTabAreaWidth = ( rcTabItemArea.right - rcTabItemArea.left );

		RECT rcItem = rcTabItemArea;
		// rcItem.top and rcItem.bottom aren't really going to change

		// Recalculate tab positions and widths
		// See DrawItem_ImageAndText for a discussion of how CDotNetTabCtrlImpl
		//  interprets margin, padding, etc.
		size_t nCount = ( size_t )m_Items.GetSize();
		int xpos = m_settings.iIndent;
		HGDIOBJ hRestoreNormalFont = NULL;

		size_t i;
		for( i = 0; i < nCount; ++i )
		{
			bool bSelected = (( int )i == m_iCurSel );
			if( bSelected )
			{
				//hRestoreNormalFont = dc.SelectFont(lpNMCustomDraw->hFontSelected);
				hRestoreNormalFont = dc.SelectObject( m_fontSel );
			}

			TItem* pItem = m_Items[ ( int )i ];
			_ASSERT( pItem != NULL );
			rcItem.left = rcItem.right = xpos;
			//rcItem.right += ((bSelected ? m_settings.iSelMargin : m_settings.iMargin));
			rcItem.right += m_settings.iMargin;
			if( pItem->UsingImage() && (( HIMAGELIST )m_imageList ))
			{
				IMAGEINFO ii = { 0 };
				int nImageIndex = pItem->GetImageIndex();

				m_imageList.GetImageInfo( nImageIndex, &ii );
				rcItem.right += ( ii.rcImage.right - ii.rcImage.left );
			}
			if( pItem->UsingText())
			{
				RECT rcText = { 0 };
				ClsString sText = pItem->GetText();
				dc.DrawText( sText, &rcText, DT_SINGLELINE | DT_CALCRECT );
				rcItem.right += ( rcText.right - rcText.left ) + ( m_settings.iPadding * 2 );
			}
			rcItem.right += m_settings.iMargin;
			pItem->SetRect( rcItem );
			xpos += ( rcItem.right - rcItem.left );

			if( hRestoreNormalFont != NULL )
			{
				dc.SelectObject( hRestoreNormalFont );
				hRestoreNormalFont = NULL;
			}

			if( ! bSelected )
			{
				if(( rcItem.right - rcItem.left ) < nMinInactiveWidth )
				{
					nMinInactiveWidth = ( rcItem.right - rcItem.left );
				}
				if(( rcItem.right - rcItem.left ) > nMaxInactiveWidth )
				{
					nMaxInactiveWidth = ( rcItem.right - rcItem.left );
				}
			}
		}
		xpos += m_settings.iIndent;

		if( xpos > nTabAreaWidth && nCount > 0 && m_iCurSel >= 0 )
		{
			// Our desired widths are more than the width of the client area.
			// We need to have some or all of the tabs give up some real estate

			// We'll try to let the selected tab have its fully desired width.
			// If it can't, we'll make all the tabs the same width.

			RECT rcSelected = m_Items[ m_iCurSel ]->GetRect();
			LONG nSelectedWidth = ( rcSelected.right - rcSelected.left );

			long cxClientInactiveTabs = nTabAreaWidth - ( m_settings.iIndent * 2 ) - nSelectedWidth;
			long cxDesiredInactiveTabs = xpos - ( m_settings.iIndent * 2 ) - nSelectedWidth;

			double nRatioWithSelectionFullSize = 0.0;
			if( cxDesiredInactiveTabs != 0 )
			{
				nRatioWithSelectionFullSize = ( double )( cxClientInactiveTabs ) / ( double )( cxDesiredInactiveTabs );
			}

			long nInactiveSameSizeWidth = ( m_nMinWidthToDisplayText + ( m_settings.iMargin * 2 ) + ( m_settings.iPadding ));

			if( cxClientInactiveTabs > ( nInactiveSameSizeWidth * ( long )( nCount - 1 )))
			{
				//  There should be enough room to display the entire contents of
				//  the selected tab plus something for the inactive tabs

				bool bMakeInactiveSameSize = (( nMinInactiveWidth * nRatioWithSelectionFullSize ) < nInactiveSameSizeWidth );

				xpos = m_settings.iIndent;
				for( i = 0; i < nCount; ++i )
				{
					TItem* pItem = m_Items[ ( int )i ];
					_ASSERT( pItem != NULL );
					RECT rcItemDesired = pItem->GetRect();
					rcItem.left = rcItem.right = xpos;
					if(( int )i == m_iCurSel)
					{
						rcItem.right += ( rcItemDesired.right - rcItemDesired.left );
					}
					else
					{
						if( bMakeInactiveSameSize && ( nCount != 1 ))
						{
							rcItem.right += (long)(( cxClientInactiveTabs / ( nCount - 1 )) + 0.5 );
						}
						else
						{
							rcItem.right += ( long )((( rcItemDesired.right - rcItemDesired.left ) * nRatioWithSelectionFullSize ) + 0.5 );
						}
					}
					pItem->SetRect( rcItem );
					xpos += ( rcItem.right - rcItem.left );
				}
			}
			else
			{
				// We're down pretty small, so just make all the tabs the same width
				int cxItem = ( nTabAreaWidth - ( m_settings.iIndent * 2 )) / ( int )nCount;

				xpos = m_settings.iIndent;

				for( i = 0; i < nCount; ++i )
				{
					rcItem.left = rcItem.right = xpos;
					rcItem.right += cxItem;
					m_Items[ ( int )i ]->SetRect( rcItem );
					xpos += ( rcItem.right-rcItem.left );
				}
			}
		}

		dc.SelectObject( hOldFont );
	}

	void UpdateLayout_ScrollToFit( RECT rcTabItemArea )
	{
		//DWORD dwStyle = this->GetStyle();

		// When we scroll to fit, we ignore what's passed in for the
		// tab item area rect, and use the client rect instead
		RECT rcClient;
		::GetClientRect( m_hWnd, &rcClient );

		ClsGetDC dc( this );
		//HFONT hOldFont = dc.SelectFont(lpNMCustomDraw->hFontInactive);
		HGDIOBJ hOldFont = dc.SelectObject( m_font );

		RECT rcItem = rcClient;
		// rcItem.top and rcItem.bottom aren't really going to change

		// Recalculate tab positions and widths
		// See DrawItem_ImageAndText for a discussion of how CDotNetTabCtrlImpl
		//  interprets margin, padding, etc.
		size_t nCount = ( size_t )m_Items.GetSize();
		int xpos = m_settings.iIndent;
		HGDIOBJ hRestoreNormalFont = NULL;

		for( size_t i = 0; i < nCount; ++i )
		{
			bool bSelected = (( int )i == m_iCurSel );
			if( bSelected )
			{
				//hRestoreNormalFont = dc.SelectFont(lpNMCustomDraw->hFontSelected);
				hRestoreNormalFont = dc.SelectObject( m_fontSel );
			}

			TItem* pItem = m_Items[ ( int )i ];
			_ASSERT( pItem != NULL );
			rcItem.left = rcItem.right = xpos;
			//rcItem.right += ((bSelected ? m_settings.iSelMargin : m_settings.iMargin));
			rcItem.right += m_settings.iMargin;
			if( pItem->UsingImage() && ((( HIMAGELIST )m_imageList ) || m_bUseSystemImageList ))
			{
				IMAGEINFO ii = { 0 };
				int nImageIndex = pItem->GetImageIndex();
				m_imageList.GetImageInfo( nImageIndex, &ii );
				rcItem.right += ( ii.rcImage.right - ii.rcImage.left );
			}
			if( pItem->UsingText())
			{
				RECT rcText = { 0 };
				ClsString sText = pItem->GetText();
				dc.DrawText( sText, &rcText, DT_SINGLELINE | DT_CALCRECT );
				rcItem.right += ( rcText.right - rcText.left ) + ( m_settings.iPadding * 2 );
			}
			rcItem.right += m_settings.iMargin;
			pItem->SetRect( rcItem );
			xpos += ( rcItem.right - rcItem.left );

			if( hRestoreNormalFont != NULL )
			{
				dc.SelectObject( hRestoreNormalFont );
				hRestoreNormalFont = NULL;
			}
		}
		xpos += m_settings.iIndent;

		// If we've been scrolled to the left, and resize so
		// there's more client area to the right, adjust the
		// scroll offset accordingly.
		if(( xpos + m_iScrollOffset ) < rcTabItemArea.right )
		{
			m_iScrollOffset = ( rcTabItemArea.right - xpos );
		}

		dc.SelectObject( hOldFont );
	}

protected:
	ClsBrush		m_hbrBackground;
	COLORREF		m_clrTextInactiveTab, m_clrSelectedTab;
	signed char		m_nFontSizeTextTopOffset;
	signed char		m_nMinWidthToDisplayText;
	bool			m_bUseSystemImageList;
};
typedef ClsDotNetTabCtrl< ClsCustomTabItem, true >	ClsDotNetTabControl;

template< typename TItem = ClsCustomTabItem >
class ClsDotNetButtonTabCtrl : public ClsDotNetTabCtrl< TItem, false >
{
	_NO_COPY( ClsDotNetButtonTabCtrl );
protected:
	typedef ClsDotNetTabCtrl< TItem, false > baseClass;

// Constructor
public:
	ClsDotNetButtonTabCtrl()
	{
		// We can't use a member initialization list to initialize
		// members of our base class, so do it explictly by assignment here.
		m_clrTextInactiveTab = ::GetSysColor( COLOR_BTNTEXT );
		m_clrSelectedTab = ::GetSysColor( COLOR_WINDOW );
	}
	virtual ~ClsDotNetButtonTabCtrl() {;}

// Message Handling
	BEGIN_MSG_MAP( baseClass )
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSettingChange)
	END_MSG_MAP()

	virtual LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DWORD dwStyle = this->GetStyle();

		// Initialize/Reinitialize font
		// Visual Studio.Net seems to use the "icon" font for the tabs
		LOGFONT lfIcon = { 0 };
		::SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof( lfIcon ), &lfIcon, 0 );

		m_clrTextInactiveTab = ::GetSysColor( COLOR_BTNTEXT );
		m_clrSelectedTab = ::GetSysColor( COLOR_WINDOW );

		bool bResetFont = true;
		if( m_font.IsValid())
		{
			LOGFONT lf = { 0 };
			if( m_font.GetLogFont( &lf ))
			{
				if( _tcsicmp( lf.lfFaceName, lfIcon.lfFaceName ) == 0 &&
					      lf.lfHeight == lfIcon.lfHeight )
				{
					bResetFont = false;
				}
			}
		}

		if( bResetFont )
		{
			if( m_font.IsValid()) m_font.Delete();
			if( m_fontSel.IsValid()) m_fontSel.Delete();

			if ( ! m_font.CreateFontIndirect( &lfIcon ))
			{
				m_font.Attach( ::GetStockObject( DEFAULT_GUI_FONT ));
			}

			if( CTCS_BOLDSELECTEDTAB == ( dwStyle & CTCS_BOLDSELECTEDTAB ))
			{
				lfIcon.lfWeight = FW_BOLD;
			}

			if ( m_fontSel.CreateFontIndirect(&lfIcon) == FALSE )
			{
				m_fontSel.Attach( ::GetStockObject( DEFAULT_GUI_FONT ));
			}
		}

		// Background brush
		if( m_hbrBackground.IsValid()) m_hbrBackground.Delete();

		m_hbrBackground.CreateSysColorBrush( COLOR_BTNFACE );

		m_settings.iIndent = 5;
		m_settings.iPadding = 4;
		m_settings.iMargin = 3;
		m_settings.iSelMargin = 3;

		this->UpdateLayout();
		this->Invalidate();
		return 0;
	}

// Overrides for painting from CDotNetTabCtrlImpl
	virtual void DrawBackground( RECT rcClient, LPNMCTCCUSTOMDRAW lpNMCustomDraw )
	{
		ClsDC dc( lpNMCustomDraw->nmcd.hdc );

		// Set up the text color and background mode
		dc.SetTextColor( lpNMCustomDraw->clrBtnText );
		dc.SetBkMode( TRANSPARENT );

		// Erase Background
		//  (do it here instead of a handler for WM_ERASEBKGND
		//   so that we can do flicker-free drawing with the help
		//   of COffscreenDrawRect that's in the base class)
		// Note: Because the "erase" part is very simple, and only coloring
		//  it with the background color, we can do a smarter erase.
		//  Instead of erasing the whole client area (which might be clipped),
		//  We'll just ask the HDC for the clip box.

		RECT rc = { 0 };
		//GetClientRect(&rc);
		dc.GetClipBox( &rc );

		HGDIOBJ hOldBrush = dc.SelectObject( lpNMCustomDraw->hBrushBackground );
		dc.PatBlt( rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY );
		dc.SelectObject( hOldBrush );
	}

	virtual void DrawItem_InitBounds( DWORD dwStyle, RECT rcItem, RECT& rcTab, RECT& rcText, int& nIconVerticalCenter )
	{
		rcTab.top += 3;
		rcTab.bottom -= 3;
		nIconVerticalCenter = ( rcTab.bottom + rcTab.top ) / 2;
	}

	virtual void DrawItem_TabSelected( DWORD dwStyle, LPNMCTCCUSTOMDRAW lpNMCustomDraw, RECT& rcTab )
	{
		// Tab is selected, so paint as select
		bool bHighlighted = ( CDIS_MARKED == ( lpNMCustomDraw->nmcd.uItemState & CDIS_MARKED ));

		ClsDC dc( lpNMCustomDraw->nmcd.hdc );

		ClsPen penOutline;
		ClsBrush brushSelected;
		if( bHighlighted )
		{
			penOutline.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrBtnHighlight );
			brushSelected.CreateSolidBrush( lpNMCustomDraw->clrHighlight );
		}
		else
		{
			penOutline.CreatePen( PS_SOLID, 1, lpNMCustomDraw->clrHighlight );
			brushSelected.CreateSolidBrush( lpNMCustomDraw->clrSelectedTab );
		}

		HGDIOBJ hOldPen = dc.SelectObject( penOutline );
		HGDIOBJ hOldBrush = dc.SelectObject( brushSelected );

		dc.Rectangle( &rcTab );

		dc.SelectObject( hOldPen );
		dc.SelectObject( hOldBrush );
	}
};
typedef ClsDotNetButtonTabCtrl< ClsCustomTabItem >	ClsDotNetButtonTabControl;

#endif // __DOTNET_TABCTRL_H__

