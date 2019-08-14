#pragma once

// CWHeaderCtrl
class CWBHeaderCtrl : public CHeaderCtrl
{
	DECLARE_DYNAMIC(CWBHeaderCtrl)

public:
	CWBHeaderCtrl();
	virtual ~CWBHeaderCtrl();
	virtual void OnFillBackground(CDC* pDC);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};

// CWBListCtrl
#define WBLIST_MAX_WLIST_COL			16
#define WBLIST_COL_RESIZE				0x1

#define ITEM_EDIT_CTRL_MODE_EDIT		1		// 编辑框仅显示		edit box
#define ITEM_EDIT_CTRL_MODE_LIST		2		// 编辑框显示		 list


typedef int(*PFuncWBFillList)(const WCHAR* str, CListBox* pList, int* nCountRes );	// 弹出edit 控件后，输入字符串，通过字符串获取搜索结果后填到list box里
typedef int(*PFuncListSelItem)(int nMode, int nListItem, int nSubItem, unsigned int nID, void* pContext);		// 双击list box的选择项。

typedef struct tagWBListItemData
{
	unsigned char	nFlag;
	int				nWidth;
	int				nMode;
	PFuncWBFillList pf;
}WBLIST_ITEM_DATA;

class CWBListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CWBListCtrl)

public:
	CWBListCtrl();
	virtual ~CWBListCtrl();

protected:
	DECLARE_MESSAGE_MAP()

public:
	WBLIST_ITEM_DATA		m_item[WBLIST_MAX_WLIST_COL];		// 记录每列的标识，是否支持列宽自动缩放等 WLIST_COL_RESIZE
	int						m_nCount;							// col的数量
	int						m_isHDNTrack;
	unsigned int			m_crListBK1;
	unsigned int			m_crListBK2;
	unsigned int			m_crListBKSelect;
	unsigned int			m_crText;
	unsigned int			m_crTextEdit;
	unsigned int			m_crBK;

	CWBHeaderCtrl			m_wndHeader;
	CImageList				m_images;

	int*					m_pItemColorList;
	CFont					m_cFont;
	int						m_nEditItem;
	int						m_nEditSubItem;
	CEdit					m_edit;
	CListBox				m_list;
	int						m_nRowHeight;
	DWORD					m_dwEditChangeTick;
	int						m_nColUpdateList;
	CString					m_lastEditTxt;
	RECT					m_rcList;
	RECT					m_rcEdit;
	PFuncListSelItem		m_pfDbClickFunc;
	void*					m_pFuncContext;
	int						m_isEnableStatus;
	char					m_isCheck;
public:
	void	SetCBFunc(PFuncListSelItem p, void* pContext) { m_pfDbClickFunc = p; m_pFuncContext = pContext; }
	void	SetRowHeight(int nHeight);
	void	SetRowColor(int cr1, int cr2) { m_crListBK1 = cr1; m_crListBK2 = cr2; }
	void	SetRowColorSelect(int cr) { m_crListBKSelect = cr; }
	void	SetBK(int cr) { m_crBK = cr; }
	void	SetTextCr(int cr) { m_crText = cr; }
	void	SetEditTextCr(int cr) { m_crTextEdit = cr; }
	int		InsertColumnEx(int nCol, LPCTSTR lpszColumnHeading, int nWidth, int isResize = 0, int nFormat = LVCFMT_CENTER);
	void	SetEditColumn(int nCol, int nMode, PFuncWBFillList p = NULL) { m_item[nCol].nMode = nMode; m_item[nCol].pf = p; }	// 设置当前列的item 可编辑，通过自定义控件来编辑
	void	ResetWList();
	void	SetSelectItem( int nItem );
	void    SetColorList(int* p) { m_pItemColorList = p; }
	void	ShowEdit(int nMode, int nItem, int nSubItem, CRect& rcCtrl);
	void	HideEdit();
	void    SetEditStatus(int isEnable = 1) { m_isEnableStatus = isEnable;  }
	int		GetFirstSelectItem();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnEndtrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEnKillfocusEdit();
	afx_msg void OnEnChangeEdit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLbnDblclkListBox();
	afx_msg void OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnEditEnd(WPARAM wParam, LPARAM lParam);
};


