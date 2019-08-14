// WBListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "WBListCtrl.h"
#include <GdiPlus.h>
using namespace Gdiplus;

#define IDC_EDIT_WBLISTCTRL			80000+1
#define IDC_LBOX_WBLISTCTRL			80000+2
#define WM_USER_WBLIST_EDIT_END		WM_USER + 205
#define LIST_UPDATE_TIMER			220
#define EDIT_UPDATE_TIMER			220

IMPLEMENT_DYNAMIC(CWBHeaderCtrl, CHeaderCtrl)

void CWBHeaderCtrl::OnFillBackground(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
}

CWBHeaderCtrl::CWBHeaderCtrl()
{
}

CWBHeaderCtrl::~CWBHeaderCtrl()
{
}

BOOL CWBHeaderCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BEGIN_MESSAGE_MAP(CWBHeaderCtrl, CHeaderCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void CWBHeaderCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CHeaderCtrl::OnKillFocus(pNewWnd);
	Invalidate(FALSE);
}


void CWBHeaderCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CHeaderCtrl::OnSetFocus(pOldWnd);
}

// CWBListCtrl

IMPLEMENT_DYNAMIC(CWBListCtrl, CListCtrl)

CWBListCtrl::CWBListCtrl():	m_crListBK1(0x00F6F4F4)
		, m_crListBK2(0x00EFEBEB)
		, m_crBK(0x00F5F3F3)
		, m_crListBKSelect(RGB(49, 106, 197))
		, m_crText(0x00333333)
		, m_crTextEdit(0xFF555555)
		, m_nCount(0)
		, m_isHDNTrack(0)
		, m_pItemColorList(NULL)
		, m_nEditItem(-1)
		, m_nEditSubItem(-1)
		, m_nRowHeight(12)
		, m_dwEditChangeTick(0)
		, m_nColUpdateList(-1)
		, m_pfDbClickFunc(NULL)
		, m_isEnableStatus(1)
		, m_isCheck(0)
{
	memset(m_item, 0, sizeof(m_item));
	m_cFont.CreateStockObject(DEFAULT_GUI_FONT);
}

CWBListCtrl::~CWBListCtrl()
{
	m_cFont.DeleteObject();
}


BEGIN_MESSAGE_MAP(CWBListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(HDN_BEGINTRACKA, 0, &CWBListCtrl::OnHdnBegintrack)
	ON_NOTIFY(HDN_BEGINTRACKW, 0, &CWBListCtrl::OnHdnBegintrack)
	ON_NOTIFY(HDN_ENDTRACKW, 0, &CWBListCtrl::OnHdnEndtrack)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CWBListCtrl::OnCustomDraw)
	ON_EN_KILLFOCUS(IDC_EDIT_WBLISTCTRL, &CWBListCtrl::OnEnKillfocusEdit)
	ON_EN_CHANGE(IDC_EDIT_WBLISTCTRL, &CWBListCtrl::OnEnChangeEdit)
	ON_LBN_DBLCLK(IDC_LBOX_WBLISTCTRL, &CWBListCtrl::OnLbnDblclkListBox)
	ON_MESSAGE(WM_USER_WBLIST_EDIT_END, CWBListCtrl::OnEditEnd)
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_NOTIFY(HDN_ITEMCLICKA, 0, &CWBListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, &CWBListCtrl::OnHdnItemclick)
END_MESSAGE_MAP()


void CWBListCtrl::SetRowHeight(int nHeight)
{
	m_nRowHeight = nHeight;
	m_images.Create(1, nHeight, TRUE | ILC_COLOR32, 1, 0);
	SetImageList(&m_images, LVSIL_SMALL);
}

int	CWBListCtrl::InsertColumnEx(int nCol, LPCTSTR lpszColumnHeading, int nWidth, int isResize, int nFormat)
{
	int nRet = -1;
	if (nCol < WBLIST_MAX_WLIST_COL)
	{
		nRet = InsertColumn(nCol, lpszColumnHeading, nFormat, nWidth);
		m_item[nCol].nWidth = nWidth;
		m_nCount++;
		if (isResize != 0)	m_item[nCol].nFlag |= WBLIST_COL_RESIZE;
	}

	return nRet;
}

void CWBListCtrl::ResetWList()
{
	m_nCount = 0;
	memset(m_item, 0, sizeof(m_item));
}


void CWBListCtrl::OnSize(UINT nType, int cx, int cy)
{
	int i = 0;
	int nWidth = 0;
	int	nSumWidthAll = 0;
	int nSumWidthResize = 0;	// 不需要调整列宽的列，宽度和，用来计算缩放比例
	float fRatio = 0;

	int nColWidth = 0;
	int nLatestVal = 0;
	int nUseVal = 0;
	SCROLLBARINFO sbi;
	CString str;

	do
	{
		CListCtrl::OnSize(nType, cx, cy);
		if (m_isHDNTrack != 0)	break;

		m_item[m_nCount - 1].nFlag |= WBLIST_COL_RESIZE;
		nWidth = cx;

		if (GetScrollBarInfo(OBJID_VSCROLL, &sbi))
		{
			nWidth -= sbi.rcScrollBar.right - sbi.rcScrollBar.left - 5;
		}

		for (i = 0; i < m_nCount; i++)
		{
			nSumWidthAll += m_item[i].nWidth;
			if ((m_item[i].nFlag & WBLIST_COL_RESIZE) == 0)	continue;

			nSumWidthResize += m_item[i].nWidth;
		}
		nUseVal = nSumWidthAll - nSumWidthResize;
		fRatio = (float)(nWidth - nUseVal) / nSumWidthResize;

		for (i = 0; i < m_nCount - 1; i++)
		{
			if (m_item[i].nFlag & WBLIST_COL_RESIZE)
			{
				nColWidth = (int)(m_item[i].nWidth*fRatio);
				nWidth = (nColWidth > m_item[i].nWidth) ? nColWidth : m_item[i].nWidth;
				SetColumnWidth(i, nWidth);
				nUseVal += nWidth;
			}
		}
		nColWidth = cx - nUseVal;
		SetColumnWidth(m_nCount - 1, nColWidth);
	} while (0);
}

void CWBListCtrl::OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	m_isHDNTrack = 1;
	*pResult = 0;
}


void CWBListCtrl::OnHdnEndtrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	m_isHDNTrack = 0;
	*pResult = 0;
}


void CWBListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE(pNMHDR != NULL);
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	int iColumn = lplvcd->iSubItem;
	int iRow = (int)lplvcd->nmcd.dwItemSpec;
	POSITION pos = NULL;
	int index = -1;

	*pResult = CDRF_DODEFAULT;

	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		pos = GetFirstSelectedItemPosition();
		index = GetNextSelectedItem(pos);

		if (m_pItemColorList == NULL)
		{
			if (index == iRow)//如果要刷新的项为当前选择的项，则将文字设为白色，背景色设为蓝色
			{
				lplvcd->clrText = RGB(255, 255, 255);					  //Set the text to white
				lplvcd->clrTextBk = m_crListBKSelect & 0x00FFFFFF;        //Set the background color to blue
			}
			else
			{
				lplvcd->clrTextBk = (iRow % 2 == 0) ? (m_crListBK1 & 0x00FFFFFF) : (m_crListBK2 & 0x00FFFFFF);

				if (m_item[iColumn].nMode == ITEM_EDIT_CTRL_MODE_EDIT)
				{
					lplvcd->clrText = m_crTextEdit & 0x00FFFFFF;
				}
				else
				{
					lplvcd->clrText = m_crText & 0x00FFFFFF;
				}
			}
		}
		else
		{
			lplvcd->clrTextBk = m_pItemColorList[iRow] & 0x00FFFFFF;
			lplvcd->clrText = m_crText & 0x00FFFFFF;
		}
		
		*pResult = CDRF_DODEFAULT;
		break;
	case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CWBListCtrl::SetSelectItem(int nItem)
{
	POSITION posSel = NULL;
	int nSelItem = -1;

	posSel = GetFirstSelectedItemPosition();
	if (posSel != NULL)
	{
		nSelItem = GetNextSelectedItem(posSel);
	}

	if (nSelItem != -1)	SetItemState(nSelItem, 0, LVNI_FOCUSED | LVNI_SELECTED);
	if (nItem != -1)	SetItemState(nItem, LVNI_FOCUSED | LVNI_SELECTED, LVNI_FOCUSED | LVNI_SELECTED);
	//EnsureVisible(nItem, FALSE);
}

void CWBListCtrl::OnDestroy()
{
	KillTimer(1);
	KillTimer(2);
	CListCtrl::OnDestroy();
}


void CWBListCtrl::ShowEdit(int nMode, int nItem, int nSubItem, CRect& rcCtrl)
{
	CString strItem;
	m_nEditItem = nItem;
	m_nEditSubItem = nSubItem;

	m_rcEdit = rcCtrl;
	
	if (m_isEnableStatus && (nMode &ITEM_EDIT_CTRL_MODE_EDIT))
	{
		if (m_edit.m_hWnd == NULL)
		{
			m_edit.Create(ES_AUTOHSCROLL | WS_CHILD | ES_CENTER | ES_WANTRETURN | WS_BORDER, CRect(0, 0, 0, 0), this, IDC_EDIT_WBLISTCTRL);
			m_edit.ShowWindow(SW_HIDE);
			m_edit.SetFont(&m_cFont);
		}
		SetTimer(2, EDIT_UPDATE_TIMER, NULL);
		strItem = CListCtrl::GetItemText(nItem, nSubItem);
		m_edit.MoveWindow(&rcCtrl);
		m_edit.ShowWindow(SW_SHOW);
		m_edit.SetWindowText(strItem);
		m_edit.SetFocus();
		m_edit.SetSel(-1);
	}

	if (m_isEnableStatus && ( nMode &ITEM_EDIT_CTRL_MODE_LIST) )
	{
		m_nColUpdateList = nSubItem;
		rcCtrl.top = rcCtrl.bottom;
		rcCtrl.bottom = rcCtrl.top + 2;
		m_rcList = rcCtrl;
		if (m_list.m_hWnd == NULL)
		{
			m_list.Create(WS_CHILD | WS_TABSTOP | WS_BORDER | LBS_NOTIFY, CRect(0, 0, 0, 0), this, IDC_LBOX_WBLISTCTRL);
			m_list.ShowWindow(SW_HIDE);
			m_list.SetFont(&m_cFont);
			//m_list.SetItemHeight()
		}
		SetTimer(1, LIST_UPDATE_TIMER, NULL);
		m_list.MoveWindow(&rcCtrl);
		m_list.ShowWindow(SW_SHOW);
	}
}

void CWBListCtrl::HideEdit()
{
	if( m_edit.m_hWnd != NULL )	m_edit.ShowWindow(SW_HIDE);
	if( m_list.m_hWnd != NULL )	m_list.ShowWindow(SW_HIDE);
	m_lastEditTxt.Empty();
	m_nColUpdateList = -1;
}

void CWBListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CRect rect;
	int nItem = 0, nSubItem = 0;
	LVHITTESTINFO lvhti;

	do 
	{
		lvhti.pt = point;
		nItem = CListCtrl::SubItemHitTest(&lvhti);
		if (nItem == -1) break;
		nSubItem = lvhti.iSubItem;
		if (nSubItem < 0 || nSubItem  > WBLIST_MAX_WLIST_COL)	break;
		
		GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rect);
		ShowEdit( m_item[nSubItem].nMode, nItem, nSubItem, rect);
	} while (0);
	
	CListCtrl::OnLButtonDblClk(nFlags, point);
}

LRESULT CWBListCtrl::OnEditEnd(WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	CString strText;
	CString strListItem;
	unsigned int nID = wParam;

	if (m_edit.m_hWnd)
	{
		KillTimer(1);
		KillTimer(2);

		m_edit.GetWindowText(strText);
		if (m_nEditItem != -1 && m_nEditSubItem != -1)
		{
			if (lParam == 0)
			{
				if ((m_item[m_nEditSubItem].nMode&ITEM_EDIT_CTRL_MODE_LIST) == 0)
				{
					SetItemText(m_nEditItem, m_nEditSubItem, strText);	// 判断是否显示出来list box。如果显示且list box有值，没有选择的话用第一个填写
				}
				else
				{
					if (m_list.GetCount() > 0)
					{
						nID = m_list.GetItemData(0);
						SetItemData(m_nEditItem, m_list.GetItemData(0));
					}
				}
			}
			else
			{
				SetItemData(m_nEditItem, nID);
			}

			if (m_pfDbClickFunc != NULL)
			{
				m_pfDbClickFunc(m_item[m_nEditSubItem].nMode, m_nEditItem, m_nEditSubItem, nID, m_pFuncContext);
			}
		}
		HideEdit();
		m_nEditItem = -1;
		m_nEditSubItem = -1;
	}
	
	return 0;
}

void CWBListCtrl::OnEnKillfocusEdit()
{
	do 
	{
		if (m_nEditSubItem < 0 || m_nEditSubItem  > WBLIST_MAX_WLIST_COL)	break;
		if (m_item[m_nEditSubItem].nMode & ITEM_EDIT_CTRL_MODE_LIST)	break;
		::PostMessage(this->GetSafeHwnd(), WM_USER_WBLIST_EDIT_END, 0, 0);
	} while (0);
}

void CWBListCtrl::OnTimer(UINT_PTR nIDEvent)
{
	CWnd* pFocusWnd = NULL;
	CString str;
	DWORD dwTick = GetTickCount();
	int nHeight = 0;
	int nResCount = 0;
	int nRet = 0;
	RECT rcList = m_rcList;
	RECT rc;

	GetClientRect(&rc);
	switch (nIDEvent)
	{
	case 1:
		if (m_nColUpdateList > WBLIST_MAX_WLIST_COL || m_nColUpdateList < 0)	break;
		m_edit.GetWindowText(str);
		if (str.GetLength() == 0)	break;
		if (m_lastEditTxt == str)	break;
		if (m_item[m_nColUpdateList].pf == NULL)	break;
		if (dwTick - m_dwEditChangeTick > (LIST_UPDATE_TIMER * 2 - 5))
		{
			m_lastEditTxt = str;
			m_list.ResetContent();
			nRet = m_item[m_nColUpdateList].pf(str, &m_list, &nResCount);
			nHeight = m_list.GetItemHeight(0)*m_list.GetCount();

			if (m_rcList.top + nHeight > rc.bottom)
			{
				rcList.bottom = m_rcEdit.top;
				rcList.top = m_rcEdit.top - nHeight;
			}
			else
			{
				rcList.bottom = rcList.top + nHeight + 18;
			}
			
			m_list.MoveWindow(&rcList);
		}
		break;
	case 2:
		pFocusWnd = GetFocus();
		if (pFocusWnd == &m_edit || pFocusWnd == &m_list)	break;
		::PostMessage(this->GetSafeHwnd(), WM_USER_WBLIST_EDIT_END, 0, 0);
		break;
	default:
		break;
	}
	//CListCtrl::OnTimer(nIDEvent);
}

void CWBListCtrl::OnEnChangeEdit()
{
	m_dwEditChangeTick = GetTickCount();
}

void CWBListCtrl::OnLbnDblclkListBox()
{
	int nItem = 0;
	unsigned int nID = 0;

	nItem = m_list.GetCurSel();
	nID = m_list.GetItemData(nItem);
	::PostMessage(this->GetSafeHwnd(), WM_USER_WBLIST_EDIT_END, nID, 1);
}

int	CWBListCtrl::GetFirstSelectItem()
{
	int nSel = -1;
	POSITION posSel = NULL;

	posSel = GetFirstSelectedItemPosition();
	if (posSel != NULL)
	{
		nSel = GetNextSelectedItem(posSel);
	}
	return nSel;
}

BOOL CWBListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->hwnd == m_edit.m_hWnd)
	{
		if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		{
			::PostMessage(this->GetSafeHwnd(), WM_USER_WBLIST_EDIT_END, 0, 0);
		}
	}
	return CListCtrl::PreTranslateMessage(pMsg);
}


void CWBListCtrl::OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	int i = 0;
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	HD_NOTIFY *phdn = (HD_NOTIFY *)pNMHDR;
	if (phdn->iItem == 0)	// first column
	{
		if (this->GetExtendedStyle() & LVS_EX_CHECKBOXES)
		{
			m_isCheck = m_isCheck ? 0 : 1;
			for (i = 0; i < GetItemCount(); i++)
			{
				SetCheck(i, m_isCheck);
			}
		}
	}
	*pResult = 0;
}
