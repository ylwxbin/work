#pragma once


#define MAX_PAGE_BUTTON_COUNT					16
#define WPAGE_BTN_FLAG_IS_VALID					0x1
#define WPAGE_BTN_FLAG_IS_HOVER					0x2
#define WPAGE_BTN_FLAG_IS_DISABLE				0x4


// 上一页 下一页按钮 放在最后，用户点击后位置不会变，方便用户点
// 1 ... 5 6 7 8 9  ...15 上一页 下一页
#define WPAGE_BTN_INDEX_NEXT					0
#define WPAGE_BTN_INDEX_PREV					1
#define WPAGE_BTN_INDEX_LATEST					2
#define WPAGE_BTN_INDEX_FIRST					MAX_PAGE_BUTTON_COUNT-1


typedef int (*CBPageChanged)(void* pContext, int nPageIndex );

class CWDrawString;
typedef struct tagWPageButton
{
	int nFlag;		// bit
	int nWidth;
	int	nPageIndex;	// -1:next; -2:latest; -3 first
	void* pImg;		// 
	void* pHoverImg;// 按钮图片，鼠标移动上去时
	wchar_t	strText[16];
}WPAGE_BUTTON;


class CWPageBar : public CStatic
{
	DECLARE_DYNAMIC(CWPageBar)

public:
	CWPageBar();
	virtual ~CWPageBar();

public:
	void	SetDisplayPageCount(int n) { if( n>5 && n<12) m_nCountDisplay = n; }	// bar里显示页码的数量。  第一页，1，2，3，4，5 ......8 下一页。这种情况n为5，要设置成奇数
	
	void	SetVal(int nSum, int nCntPerPage);
	
	void	SetPF(CBPageChanged f, void* p) { m_pf = f; m_pContext = p; }
	void	SetCurPage(int nPage);
	int		GetCurPage() { return m_nCurPage;  }
	void	SetLineColor(int cr) { m_crLine = cr; }
	void	SetColor(int cBK, int cText, int cTextHover, int crCurrent) { m_crBK = cBK; m_crText = cText; m_crTextHover = cTextHover; m_crCurrent = crCurrent; }
	void	SetButtonImage(wchar_t* strNext, wchar_t* strNextHover, wchar_t* strPrev, wchar_t* strPrevHover );
	void	SetButtonText(wchar_t* strNext, wchar_t* strFirst);	// 设置第一页以及下一页按钮的文字，主要是为了多语言考虑
	void	SetButtonCX(int nNormal, int nNextButton) { m_nCXNormal = nNormal; m_nCXNextFirst = nNextButton; }
	void	SetTextSize( int nSize );

	void	GoNextPage();
	void	GoPrevPage();
	int     GetPageCount() { return m_nPages;  }
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void PreSubclassWindow();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();


public:
	
	int		m_isHover;
	UINT	m_nTimerID;
	int		m_nHoverButton;

	WPAGE_BUTTON		m_Button[MAX_PAGE_BUTTON_COUNT];

	int					m_nSumItem;			// 总共的条目数
	int					m_nCountPerPage;	// 每页显示几条数据
	int					m_nCurPage;			// 当前页
	int					m_nPages;			// 总共有多少页

	int					m_nCountDisplay;	// 显示几个页码

	int					m_crLine;
	int					m_crBK;
	int					m_crText;
	int					m_crTextHover;
	int					m_crCurrent;

	int					m_nCXNormal;
	int					m_nCXNextFirst;

	CWDrawString*		m_pText;
	CWDrawString*		m_pTextBold;
	CBPageChanged		m_pf;
	void*				m_pContext;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};


