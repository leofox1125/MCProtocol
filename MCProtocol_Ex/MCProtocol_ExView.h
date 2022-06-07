
// MCProtocol_ExView.h: CMCProtocolExView 類別的介面
//

#pragma once


class CMCProtocolExView : public CFormView
{
protected: // 僅從序列化建立
	CMCProtocolExView() noexcept;
	DECLARE_DYNCREATE(CMCProtocolExView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_MCPROTOCOL_EX_FORM };
#endif

// 屬性
public:
	CMCProtocolExDoc* GetDocument() const;

// 作業
public:

// 覆寫
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	virtual void OnInitialUpdate(); // 建構後第一次呼叫

// 程式碼實作
public:
	virtual ~CMCProtocolExView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 產生的訊息對應函式
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButBitRefresh();
	CListCtrl Bit_List;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedMfcbutton1();
	afx_msg void OnBnClickedButOnoff();
	afx_msg void OnBnClickedMfcbutton2();
	CListCtrl Word_List;
	afx_msg void OnBnClickedButWriteData();
	afx_msg void OnBnClickedButOnoff2();
	afx_msg void OnBnClickedButOnoff3();
};

#ifndef _DEBUG  // 對 MCProtocol_ExView.cpp 中的版本進行偵錯
inline CMCProtocolExDoc* CMCProtocolExView::GetDocument() const
   { return reinterpret_cast<CMCProtocolExDoc*>(m_pDocument); }
#endif

