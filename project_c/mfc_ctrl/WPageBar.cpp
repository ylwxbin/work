// WPageBar.cpp : 实现文件
//

#include "stdafx.h"
#include "posmanager.h"
#include "WPageBar.h"
#include "WDrawString.h"
#include <GdiPlus.h>
using namespace Gdiplus;

#define DELETE_WPAGEBAR_OBJ( class_name, p )	if(p){ delete (class_name*)p; p = NULL; }
#define GetWAValue(rgb)      (LOBYTE((rgb)>>24))
// CWPageBar

IMPLEMENT_DYNAMIC(CWPageBar, CStatic)

CWPageBar::CWPageBar() : m_nSumItem(0)
	, m_nCountPerPage(0)
	, m_nCurPage(0)
	, m_nCountDisplay(5)
	, m_crBK(0xFFFFFFFF)
	, m_crText(0xFFC0C0C0)
	, m_crCurrent(0xFFC00000)
	, m_crTextHover(0xFF000000)
	, m_nCXNormal(24)
	, m_nCXNextFirst(64)
	, m_nHoverButton(-1)
	, m_isHover(0)
	, m_nTimerID(0)
	, m_nPages(0)
	, m_crLine(0)
	, m_pf(NULL)
	, m_pContext(NULL)
{
	memset( &m_Button, 0, sizeof(m_Button) );
	wcscpy_s(m_Button[WPAGE_BTN_INDEX_NEXT].strText, sizeof(m_Button[WPAGE_BTN_INDEX_NEXT].strText) / sizeof(wchar_t), L"Next");
	wcscpy_s(m_Button[WPAGE_BTN_INDEX_PREV].strText, sizeof(m_Button[WPAGE_BTN_INDEX_PREV].strText) / sizeof(wchar_t), L"First");
}

CWPageBar::~CWPageBar()
{
	int i = 0;
	for (i = 0; i < MAX_PAGE_BUTTON_COUNT; i++)
	{
		DELETE_WPAGEBAR_OBJ(Gdiplus::Image, m_Button[i].pImg);
		DELETE_WPAGEBAR_OBJ(Gdiplus::Image, m_Button[i].pHoverImg);
	}

	DELETE_WPAGEBAR_OBJ(CWDrawString, m_pText);
	DELETE_WPAGEBAR_OBJ(CWDrawString, m_pTextBold);
}

BEGIN_MESSAGE_MAP(CWPageBar, CStatic)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

// CWPageBar 消息处理程序


BOOL CWPageBar::PreTranslateMessage(MSG* pMsg)
{
	return CStatic::PreTranslateMessage(pMsg);
}

#define VAL_DISTANCE	1
#define VAL_LEFT_BORDER 16

void CWPageBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int i = 0;
	int cx = 0;
	int nPointX = 0;
	CWDrawString* pText = NULL;
	Gdiplus::RectF rcf;
	CRect	rect;
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CMemDC dcobj(*pDC, this);
	CDC& memdc = dcobj.GetDC();
	int crDrawText = 0xFF000000;
	Gdiplus::Graphics graphics(memdc.GetSafeHdc());
	GetClientRect(&rect);
	
	do 
	{
		Gdiplus::SolidBrush linGrBrush(Gdiplus::Color(255, GetRValue(m_crBK), GetGValue(m_crBK), GetBValue(m_crBK)));
		graphics.FillRectangle(&linGrBrush, Gdiplus::Rect(0, 0, rect.Width(), rect.Height()));

		Gdiplus::Pen line_pen(Gdiplus::Color(255, GetRValue(m_crLine), GetGValue(m_crLine), GetBValue(m_crLine)), 1);
		graphics.DrawLine(&line_pen, 0, rect.bottom-1, rect.right, rect.bottom-1);
		
		if (m_nPages <= 1)	break;

		nPointX = rect.Width() - VAL_LEFT_BORDER;

		for (i = 0; i < MAX_PAGE_BUTTON_COUNT; i++)
		{
			if ( (m_Button[i].nFlag & WPAGE_BTN_FLAG_IS_VALID) == 0)	continue;

			pText = i == m_nHoverButton ? m_pTextBold : m_pText;
			m_pTextBold->GetStringRect(&graphics, m_Button[i].strText, &cx, NULL);
			m_Button[i].nWidth = cx + 5;

			rcf.X = (REAL)nPointX - m_Button[i].nWidth;
			rcf.Y = 0;
			rcf.Width = (REAL)m_Button[i].nWidth;
			rcf.Height = (REAL)m_nCXNormal;
			//graphics.DrawRectangle(&line_pen, Gdiplus::Rect(nPointX - cx, 0, cx, m_nCXNormal));
			crDrawText = i == m_nHoverButton ? m_crTextHover : m_crText;

			if (m_Button[i].nPageIndex == m_nCurPage &&
				i != WPAGE_BTN_INDEX_NEXT  &&
				i != WPAGE_BTN_INDEX_LATEST &&
				i != WPAGE_BTN_INDEX_PREV)
			{
				crDrawText = m_crCurrent;
				pText = m_pTextBold;
			}

			if (m_pText)
			{
				pText->SetColor(crDrawText);
				pText->DrawString(&graphics, &rcf, m_Button[i].strText);
			}
			nPointX -= m_Button[i].nWidth + VAL_DISTANCE;
		}

	} while (0);
}



void CWPageBar::PreSubclassWindow()
{
	DWORD dwStyle = GetStyle();
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | SS_OWNERDRAW | SS_NOTIFY);
	EnableToolTips(TRUE);
	CStatic::PreSubclassWindow();
}


BOOL CWPageBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


void CWPageBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int i = 0;
	int nNextButtonY = 0;
	CRect BarRect;
	CRect ButtonRect;
	int nPointX = 0;
	int cx = 0;
	int isChanged = 0;

	GetClientRect(&BarRect);
	nPointX = BarRect.right - VAL_LEFT_BORDER;
	for (i = 0; i < MAX_PAGE_BUTTON_COUNT; i++)
	{
		if ((m_Button[i].nFlag & WPAGE_BTN_FLAG_IS_VALID) == 0)	continue;

		cx = m_Button[i].nWidth;

		ButtonRect.left = nPointX - cx;
		ButtonRect.top = BarRect.top;
		ButtonRect.right = ButtonRect.left + cx;
		ButtonRect.bottom = ButtonRect.top + m_nCXNormal;

		if (ButtonRect.PtInRect(point))
		{
			if (i == WPAGE_BTN_INDEX_NEXT)	
			{ 
				if (m_nCurPage < (m_nPages - 1)) { m_nCurPage++; isChanged = 1; }
			}
			else if (i == WPAGE_BTN_INDEX_LATEST)
			{ 
				if (m_nCurPage != m_nPages - 1){ m_nCurPage = m_nPages - 1; isChanged = 1; }
			}
			else if (i == WPAGE_BTN_INDEX_PREV)
			{ 
				if (m_nCurPage>0)	{ m_nCurPage--; isChanged = 1;  }
			}
			else
			{
				if (m_nCurPage != m_Button[i].nPageIndex)
				{
					isChanged = 1;
					m_nCurPage = m_Button[i].nPageIndex;
				}
			}

			if( isChanged != 0 ) SetCurPage(m_nCurPage);
			break;
		}
		nPointX -= cx + VAL_DISTANCE;
	}

	CStatic::OnLButtonDown(nFlags, point);
	if (isChanged != 0 && m_pContext && m_pf)
	{ 
		m_pf(m_pContext, m_nCurPage);
	}
}


void _stdcall MouseTimerFuncPageBar(HWND hWnd, UINT n, UINT_PTR np, DWORD dw)
{
	int i = 0;
	int nNextButtonY = 0;
	POINT CursorPos;
	RECT BarRect;
	RECT ButtonRect;
	CWPageBar* pWnd = (CWPageBar*)CWnd::FromHandle(hWnd);
	int nPointX = 0;
	int cx = 0;

	::GetCursorPos(&CursorPos);
	::ScreenToClient(hWnd, &CursorPos);
	::GetClientRect(hWnd, &BarRect);

	pWnd->m_nHoverButton = -1;

	if (!::PtInRect(&BarRect, CursorPos))
	{
		::PostMessage(hWnd, WM_MOUSELEAVE, 0, 0);
	}
	else
	{
		nPointX = BarRect.right - VAL_LEFT_BORDER;
		for (i = 0; i < MAX_PAGE_BUTTON_COUNT; i++)
		{
			if ((pWnd->m_Button[i].nFlag & WPAGE_BTN_FLAG_IS_VALID) == 0)	continue;
			cx = pWnd->m_Button[i].nWidth;

			ButtonRect.left = nPointX - cx;
			ButtonRect.top = BarRect.top;
			ButtonRect.right = ButtonRect.left + cx;
			ButtonRect.bottom = ButtonRect.top + pWnd->m_nCXNormal;

			if (::PtInRect(&ButtonRect, CursorPos))
			{
				pWnd->m_nHoverButton = i;
				break;
			}
			nPointX -= cx + VAL_DISTANCE;
		}
		::PostMessage(hWnd, WM_MOUSEHOVER, 0, 0);
	}
}

void CWPageBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_isHover)
	{
		m_nTimerID = SetTimer(1, 50, MouseTimerFuncPageBar);
		m_isHover = TRUE;
	}
	CStatic::OnMouseMove(nFlags, point);
}


void CWPageBar::OnMouseHover(UINT nFlags, CPoint point)
{
	Invalidate();
	CStatic::OnMouseHover(nFlags, point);
}


void CWPageBar::OnMouseLeave()
{
	KillTimer(m_nTimerID);
	m_isHover = 0;
	Invalidate();
	CStatic::OnMouseLeave();
}

#define CNT_CUR_PAGE_	2		// 当前页前面的页码数量
/*
	nCurPage 从0 开始计算， 0 是第一页
*/
void CWPageBar::SetCurPage(int nCurPage)
{
	int i = 0;
	int j = 0;
	int nShengyu = 0;
	int	nQian = 0;
	int nPageCount = 0;
	int nButtonIndex = 3;	// button的index， 0 和1用作下一个按钮和最后一页按钮
	
	do 
	{
		if (nCurPage < 0 )	break;
		m_nCurPage = nCurPage;

		for (i = 0; i < MAX_PAGE_BUTTON_COUNT; i++)
		{
			m_Button[i].nFlag &= ~WPAGE_BTN_FLAG_IS_VALID;
		}
		
		m_Button[WPAGE_BTN_INDEX_NEXT].nFlag |= WPAGE_BTN_FLAG_IS_VALID;
		m_Button[WPAGE_BTN_INDEX_PREV].nFlag |= WPAGE_BTN_FLAG_IS_VALID;

		if (m_nSumItem <= m_nCountPerPage)	break;	// 仅有1页
		nPageCount = m_nPages;

		// 下一页按钮是否 disable
		if (m_nCurPage == nPageCount - 1)	m_Button[WPAGE_BTN_INDEX_NEXT].nFlag |= WPAGE_BTN_FLAG_IS_DISABLE;
		// 上一页按钮 是否 disable
		if (m_nCurPage == 0) m_Button[WPAGE_BTN_INDEX_PREV].nFlag |= WPAGE_BTN_FLAG_IS_DISABLE;

		if (nPageCount > m_nCountDisplay)
		{
			// 最后一页按钮是否显示以及最后一页按钮的文字	ok
			if (m_nCurPage + (m_nCountDisplay - 1) / 2 < nPageCount - 2)
			{
				m_Button[WPAGE_BTN_INDEX_LATEST].nFlag |= WPAGE_BTN_FLAG_IS_VALID;
				m_Button[WPAGE_BTN_INDEX_LATEST].nPageIndex = nPageCount - 1;
				swprintf_s(m_Button[WPAGE_BTN_INDEX_LATEST].strText, L"···%d", nPageCount);
				nShengyu = 2;
			}
			else if (m_nCurPage + (m_nCountDisplay - 1) / 2 == nPageCount - 2)
			{
				nShengyu = 3;
			}
			else// if (m_nCurPage + (m_nCountDisplay - 1) / 2 > nPageCount - 2)
			{
				nShengyu = nPageCount - (m_nCurPage + 1);
			}

			for (i = nShengyu; i > 0; i--)
			{
				if ((m_nCurPage + i) > (nPageCount - 1))	break;
				m_Button[nButtonIndex].nFlag |= WPAGE_BTN_FLAG_IS_VALID;
				m_Button[nButtonIndex].nPageIndex = m_nCurPage + i;
				swprintf_s(m_Button[nButtonIndex].strText, L"%d", m_nCurPage + i + 1);
				nButtonIndex++;
			}

			// 当前页码
			m_Button[nButtonIndex].nFlag |= WPAGE_BTN_FLAG_IS_VALID;
			m_Button[nButtonIndex].nPageIndex = m_nCurPage;
			swprintf_s(m_Button[nButtonIndex].strText, L"%d", m_nCurPage + 1);
			nButtonIndex++;

			if (m_nCurPage > (m_nCountDisplay - 1) / 2 + 1 )
			{
				m_Button[WPAGE_BTN_INDEX_FIRST].nFlag |= WPAGE_BTN_FLAG_IS_VALID;
				m_Button[WPAGE_BTN_INDEX_FIRST].nPageIndex = 0;
				swprintf_s(m_Button[WPAGE_BTN_INDEX_FIRST].strText, L"1···");
				nShengyu = 2;

				nQian = (m_nCountDisplay - 1) / 2;
			}
			else if (m_nCurPage == (m_nCountDisplay - 1) / 2 + 1)
			{
				nQian = 3;
			}
			else
			{
				nQian = m_nCurPage;
			}

			// 当前页码前面的页码
			for (int i = nQian; i > 0; i--)
			{
				if ((m_nCurPage - j - 1) < 0)	break;
				m_Button[nButtonIndex].nFlag |= WPAGE_BTN_FLAG_IS_VALID;
				m_Button[nButtonIndex].nPageIndex = m_nCurPage - j - 1;
				swprintf_s(m_Button[nButtonIndex].strText, L"%d", m_nCurPage - j);
				nButtonIndex++;
				j++;
			}
		}
		else
		{
			for (int i = nPageCount; i> 0; i--)
			{
				m_Button[nButtonIndex].nFlag |= WPAGE_BTN_FLAG_IS_VALID;
				m_Button[nButtonIndex].nPageIndex = i-1;
				swprintf_s(m_Button[nButtonIndex].strText, L"%d", i);
				nButtonIndex++;
			}
		}		

	} while (0);
}

void CWPageBar::SetButtonImage(wchar_t* strNext, wchar_t* strNextHover, wchar_t* strPrev, wchar_t* strPrevHover)
{
	if (strNext == NULL || strNextHover == NULL || strPrev == NULL || strPrevHover == NULL)	return;

	m_Button[WPAGE_BTN_INDEX_PREV].pImg = new Gdiplus::Image(strPrev, FALSE);
	m_Button[WPAGE_BTN_INDEX_PREV].pHoverImg = new Gdiplus::Image(strPrevHover, FALSE);

	m_Button[WPAGE_BTN_INDEX_NEXT].pImg = new Gdiplus::Image(strNext, FALSE);
	m_Button[WPAGE_BTN_INDEX_NEXT].pHoverImg = new Gdiplus::Image(strNextHover, FALSE);
}

void CWPageBar::SetButtonText(wchar_t* strNext, wchar_t* strFirst)
{
	if (strNext)	wcscpy_s(m_Button[WPAGE_BTN_INDEX_NEXT].strText, sizeof(m_Button[WPAGE_BTN_INDEX_NEXT].strText) / sizeof(wchar_t), strNext); 
	if (strFirst)	wcscpy_s(m_Button[WPAGE_BTN_INDEX_PREV].strText, sizeof(m_Button[WPAGE_BTN_INDEX_PREV].strText) / sizeof(wchar_t), strFirst);
}

void CWPageBar::SetTextSize(int nSize)
{
	if (m_pText == NULL)
	{
		m_pText = new CWDrawString(m_crText, nSize);
		if (m_pText)	{ m_pText->SetHAlignment(1); m_pText->SetVAlignment(2);  }
	}

	if (m_pTextBold == NULL)
	{
		m_pTextBold = new CWDrawString(m_crText, nSize, 1);
		if (m_pTextBold)	{ m_pTextBold->SetHAlignment(1); m_pTextBold->SetVAlignment(2); }
	}
}

void CWPageBar::SetVal(int nSum, int nCntPerPage)
{
	m_nSumItem = nSum;
	m_nCountPerPage = nCntPerPage;
	if (nSum <= nCntPerPage)
	{
		m_nPages = 1;
	}
	else
	{
		m_nPages = (int)ceil((float)m_nSumItem / m_nCountPerPage);
	}
	Invalidate();
}


void CWPageBar::GoNextPage()
{
	if (m_nCurPage < (m_nPages - 1))
	{
		m_nCurPage++;
		SetCurPage(m_nCurPage);
		if (m_pContext && m_pf)
		{
			m_pf(m_pContext, m_nCurPage);
		}
	}
}

void CWPageBar::GoPrevPage()
{
	if (m_nCurPage > 0)
	{
		m_nCurPage--;
		SetCurPage(m_nCurPage);
		if (m_pContext && m_pf)
		{
			m_pf(m_pContext, m_nCurPage);
		}
	}
}

BOOL CWPageBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int i = 0;
	POINT CursorPos;
	RECT BarRect;
	RECT ButtonRect;
	int nPointX = 0;
	int cx = 0;
	HCURSOR hCur = NULL;

	::GetCursorPos(&CursorPos);
	::ScreenToClient(pWnd->GetSafeHwnd(), &CursorPos);
	::GetClientRect(pWnd->GetSafeHwnd(), &BarRect);

	nPointX = BarRect.right - VAL_LEFT_BORDER;
	for (i = 0; i < MAX_PAGE_BUTTON_COUNT; i++)
	{
		if ((m_Button[i].nFlag & WPAGE_BTN_FLAG_IS_VALID) == 0)	continue;
		cx = m_Button[i].nWidth;

		ButtonRect.left = nPointX - cx;
		ButtonRect.top = BarRect.top;
		ButtonRect.right = ButtonRect.left + cx;
		ButtonRect.bottom = ButtonRect.top + m_nCXNormal;

		if (::PtInRect(&ButtonRect, CursorPos))
		{
			hCur = LoadCursor(NULL, IDC_HAND);
			break;
		}
		nPointX -= cx + VAL_DISTANCE;
	}

	if (hCur)
	{
		::SetCursor(hCur);
		return TRUE;
	}
	else
	{
		return CStatic::OnSetCursor(pWnd, nHitTest, message);
	}
}
