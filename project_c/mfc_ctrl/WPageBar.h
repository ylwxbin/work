#pragma once


#define MAX_PAGE_BUTTON_COUNT					16
#define WPAGE_BTN_FLAG_IS_VALID					0x1
#define WPAGE_BTN_FLAG_IS_HOVER					0x2
#define WPAGE_BTN_FLAG_IS_DISABLE				0x4


// ��һҳ ��һҳ��ť ��������û������λ�ò���䣬�����û���
// 1 ... 5 6 7 8 9  ...15 ��һҳ ��һҳ
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
	void* pHoverImg;// ��ťͼƬ������ƶ���ȥʱ
	wchar_t	strText[16];
}WPAGE_BUTTON;


class CWPageBar : public CStatic
{
	DECLARE_DYNAMIC(CWPageBar)

public:
	CWPageBar();
	virtual ~CWPageBar();

public:
	void	SetDisplayPageCount(int n) { if( n>5 && n<12) m_nCountDisplay = n; }	// bar����ʾҳ���������  ��һҳ��1��2��3��4��5 ......8 ��һҳ���������nΪ5��Ҫ���ó�����
	
	void	SetVal(int nSum, int nCntPerPage);
	
	void	SetPF(CBPageChanged f, void* p) { m_pf = f; m_pContext = p; }
	void	SetCurPage(int nPage);
	int		GetCurPage() { return m_nCurPage;  }
	void	SetLineColor(int cr) { m_crLine = cr; }
	void	SetColor(int cBK, int cText, int cTextHover, int crCurrent) { m_crBK = cBK; m_crText = cText; m_crTextHover = cTextHover; m_crCurrent = crCurrent; }
	void	SetButtonImage(wchar_t* strNext, wchar_t* strNextHover, wchar_t* strPrev, wchar_t* strPrevHover );
	void	SetButtonText(wchar_t* strNext, wchar_t* strFirst);	// ���õ�һҳ�Լ���һҳ��ť�����֣���Ҫ��Ϊ�˶����Կ���
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

	int					m_nSumItem;			// �ܹ�����Ŀ��
	int					m_nCountPerPage;	// ÿҳ��ʾ��������
	int					m_nCurPage;			// ��ǰҳ
	int					m_nPages;			// �ܹ��ж���ҳ

	int					m_nCountDisplay;	// ��ʾ����ҳ��

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


