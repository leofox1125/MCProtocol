
// MCProtocol_ExView.cpp: CMCProtocolExView 類別的實作
//



#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以定義在實作預覽、縮圖和搜尋篩選條件處理常式的
// ATL 專案中，並允許與該專案共用文件程式碼。
#ifndef SHARED_HANDLERS
#include "MCProtocol_Ex.h"
#endif

#include "MCProtocol_ExDoc.h"
#include "MCProtocol_ExView.h"

#include "MCProtocol_Com.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

# define BIT_REFRESH_SIZE 416
# define WORD_REFRESH_SIZE 52

#define BIT_REFRESH	1		
#define WORD_REFRESH	2	

BOOL IsBitRefreshing = FALSE;
BOOL IsWordRefreshing = FALSE;

// CMCProtocolExView

IMPLEMENT_DYNCREATE(CMCProtocolExView, CFormView)

BEGIN_MESSAGE_MAP(CMCProtocolExView, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CMCProtocolExView::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMCProtocolExView::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CMCProtocolExView::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUT_BIT_REFRESH, &CMCProtocolExView::OnBnClickedButBitRefresh)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MFCBUTTON1, &CMCProtocolExView::OnBnClickedMfcbutton1)
	ON_BN_CLICKED(IDC_BUT_ONOFF, &CMCProtocolExView::OnBnClickedButOnoff)
	ON_BN_CLICKED(IDC_MFCBUTTON2, &CMCProtocolExView::OnBnClickedMfcbutton2)
	ON_BN_CLICKED(IDC_BUT_WRITE_DATA, &CMCProtocolExView::OnBnClickedButWriteData)
	ON_BN_CLICKED(IDC_BUT_ONOFF2, &CMCProtocolExView::OnBnClickedButOnoff2)
	ON_BN_CLICKED(IDC_BUT_ONOFF3, &CMCProtocolExView::OnBnClickedButOnoff3)
END_MESSAGE_MAP()

// CMCProtocolExView 建構/解構

CMCProtocolExView::CMCProtocolExView() noexcept
	: CFormView(IDD_MCPROTOCOL_EX_FORM)
{
	// TODO: 在此加入建構程式碼
}

CMCProtocolExView::~CMCProtocolExView()
{
}

void CMCProtocolExView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BIT_VALUE, Bit_List);
	DDX_Control(pDX, IDC_LIST_WORD_VALUE, Word_List);
}

BOOL CMCProtocolExView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此經由修改 CREATESTRUCT cs 
	// 達到修改視窗類別或樣式的目的

	return CFormView::PreCreateWindow(cs);
}

void CMCProtocolExView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	char TCP_Client_Version[20];
	CString Lib_Version;
	ESMV_CS_TCPIP_Client_Alloc(TCP_Client_Version);
	Lib_Version.Format(L"TCP_Version : %s", TCP_Client_Version);
	AfxMessageBox(Lib_Version);

	int nTCPIP_Client = ESMV_CS_TCPIP_Client_Add_Client(this->GetSafeHwnd(), "192.168.100.1", 6000);;
	ESMV_CS_TCPIP_Client_Set_Window_Pos(nTCPIP_Client, 55, 30);
	Init_PLC_Com(nTCPIP_Client, TRUE);

	CString csAddress;
	Bit_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Bit_List.InsertColumn(0, L"Device Name", LVCFMT_CENTER, 120, -1);
	for (int i = 16; i > 0; i--)
	{
		csAddress.Format(L"%X", i - 1);
		Bit_List.InsertColumn(16 - i + 1, csAddress, LVCFMT_CENTER, 27, -1);
	}	

	Word_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Word_List.InsertColumn(0, L"Device Name", LVCFMT_CENTER, 120, -1);
	for (int i = 16; i > 0; i--)
	{
		csAddress.Format(L"%X", i - 1);
		Word_List.InsertColumn(16 - i + 1, csAddress, LVCFMT_CENTER, 27, -1);
	}
	Word_List.InsertColumn(17, L"String", LVCFMT_CENTER, 120, -1);
}


// CMCProtocolExView 診斷

#ifdef _DEBUG
void CMCProtocolExView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMCProtocolExView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CMCProtocolExDoc* CMCProtocolExView::GetDocument() const // 內嵌非偵錯版本
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMCProtocolExDoc)));
	return (CMCProtocolExDoc*)m_pDocument;
}
#endif //_DEBUG


// CMCProtocolExView 訊息處理常式


void CMCProtocolExView::OnBnClickedButton1()
{
	BOOL Signal[17];
	char Data[5];

	BOOL State = FALSE;

	//State = ReadBitDevice(B, "DF5", 9, Signal);
	//State = ReadWordDevice(W, "100", 11, Data);
	for (int i = 0; i < 5; i++)
	{
		Data[i] = 97 + i;		
	}

	State = WriteWordDevice(W, "7F1", 5, Data);

	CString Res;
	Res.Format(L"%d", State);
	AfxMessageBox(Res);
}

void CMCProtocolExView::OnBnClickedButton2()
{}

void CMCProtocolExView::OnBnClickedButton4()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CMCProtocolExView::OnBnClickedButBitRefresh()
{
	
}

void CMCProtocolExView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值
	CString csAddress;
	char charAddress[6];
	char charTmpAddress[6];
	Device_Header Device_Type;
	
	if (nIDEvent == BIT_REFRESH)
	{
		BOOL* Result;
		GetDlgItem(IDC_EDIT_BITDEVICE)->GetWindowText(csAddress);
		sprintf_s(charAddress, "%ls", csAddress);

		if (charAddress[0] == 'B')
			Device_Type = B;
		else
		{
			IsBitRefreshing = FALSE;
			GetDlgItem(IDC_EDIT_BITDEVICE)->EnableWindow(!IsBitRefreshing);
			((CMFCButton*)GetDlgItem(IDC_MFCBUTTON1))->SetFaceColor(RGB(243, 243, 243));
			KillTimer(BIT_REFRESH);
			AfxMessageBox(L"Device is not applicable!!!");
			return;
		}

		int AddressLen = 0;
		while (charAddress[AddressLen + 1] != '\0')
			AddressLen++;

		for (int i = 1; i < AddressLen + 1; i++) //6A
		{
			if (charAddress[i] == '0' ||
				charAddress[i] == '1' ||
				charAddress[i] == '2' ||
				charAddress[i] == '3' ||
				charAddress[i] == '4' ||
				charAddress[i] == '5' ||
				charAddress[i] == '6' ||
				charAddress[i] == '7' ||
				charAddress[i] == '8' ||
				charAddress[i] == '9')
			{
				charTmpAddress[i - 1] = charAddress[i] - 48;
			}

			if (charAddress[i] == 'A' ||
				charAddress[i] == 'B' ||
				charAddress[i] == 'C' ||
				charAddress[i] == 'D' ||
				charAddress[i] == 'E' ||
				charAddress[i] == 'F')
			{
				charTmpAddress[i - 1] = charAddress[i] - 55;
			}
		}

		double DecNumDouble = 0;

		for (int i = 0; i < AddressLen; i++)
			DecNumDouble += pow(16, AddressLen - 1 - i) * charTmpAddress[i];

		if (DecNumDouble > 0x1FFF)
		{
			IsBitRefreshing = FALSE;
			GetDlgItem(IDC_EDIT_BITDEVICE)->EnableWindow(!IsBitRefreshing);
			((CMFCButton*)GetDlgItem(IDC_MFCBUTTON1))->SetFaceColor(RGB(243, 243, 243));
			KillTimer(BIT_REFRESH);
			AfxMessageBox(L"Memory is out of range!!!");
			return;
		}

		int Refresh_Size = 0;
		if ((DecNumDouble + BIT_REFRESH_SIZE - 1) > 0x1FFF) //B000~B1FFF
			Refresh_Size = 0x1FFF - DecNumDouble + 1;
		else
			Refresh_Size = BIT_REFRESH_SIZE;

		Result = new BOOL[Refresh_Size];

		if (ReadBitDevice(Device_Type, &charAddress[1], Refresh_Size, Result))
		{
			Bit_List.DeleteAllItems();
			int Row_Size = 0;
			if (Refresh_Size % 16 > 0)
				Row_Size = Refresh_Size / 16 + 1;
			else
				Row_Size = Refresh_Size / 16;

			CString Signal;
			int Row_Refresh_Count = 0;
			for (int i = 0; i < Row_Size; i++)
			{
				csAddress.Format(L"B%X", int(DecNumDouble));
				Bit_List.InsertItem(i + 1, csAddress);
				DecNumDouble += 16;

				if ((Refresh_Size - 16 * i) / 16 > 0)
					Row_Refresh_Count = 16;
				else
					Row_Refresh_Count = Refresh_Size - 16 * i;

				for (int j = 0; j < Row_Refresh_Count; j++)
				{
					Signal.Format(L"%d", Result[16 * i + j]);
					Bit_List.SetItemText(i, 16 - j, Signal);
				}
			}				

			if (Result != NULL)
			{
				delete[] Result;
				Result = NULL;
			}
		}
		else
		{
			if (Result != NULL)
			{
				delete[] Result;
				Result = NULL;
			}
			IsBitRefreshing = FALSE;
			GetDlgItem(IDC_EDIT_BITDEVICE)->EnableWindow(!IsBitRefreshing);
			((CMFCButton*)GetDlgItem(IDC_MFCBUTTON1))->SetFaceColor(RGB(243, 243, 243));
			KillTimer(BIT_REFRESH);
			return;
		}
	}

	if (nIDEvent == WORD_REFRESH)
	{
		char* Result;
		GetDlgItem(IDC_EDIT_WORDDEVICE)->GetWindowText(csAddress);
		sprintf_s(charAddress, "%ls", csAddress);

		if (charAddress[0] == 'W')
			Device_Type = W;
		else
		{
			IsWordRefreshing = FALSE;
			GetDlgItem(IDC_EDIT_WORDDEVICE)->EnableWindow(!IsWordRefreshing);
			((CMFCButton*)GetDlgItem(IDC_MFCBUTTON2))->SetFaceColor(RGB(243, 243, 243));
			KillTimer(WORD_REFRESH);
			AfxMessageBox(L"Device is not applicable!!!");
			return;
		}

		int AddressLen = 0;
		while (charAddress[AddressLen + 1] != '\0')
			AddressLen++;

		for (int i = 1; i < AddressLen + 1; i++) //6A
		{
			if (charAddress[i] == '0' ||
				charAddress[i] == '1' ||
				charAddress[i] == '2' ||
				charAddress[i] == '3' ||
				charAddress[i] == '4' ||
				charAddress[i] == '5' ||
				charAddress[i] == '6' ||
				charAddress[i] == '7' ||
				charAddress[i] == '8' ||
				charAddress[i] == '9')
			{
				charTmpAddress[i - 1] = charAddress[i] - 48;
			}

			if (charAddress[i] == 'A' ||
				charAddress[i] == 'B' ||
				charAddress[i] == 'C' ||
				charAddress[i] == 'D' ||
				charAddress[i] == 'E' ||
				charAddress[i] == 'F')
			{
				charTmpAddress[i - 1] = charAddress[i] - 55;
			}
		}

		double DecNumDouble = 0;

		for (int i = 0; i < AddressLen; i++)
			DecNumDouble += pow(16, AddressLen - 1 - i) * charTmpAddress[i];

		if (DecNumDouble > 0x1FFF)
		{
			IsWordRefreshing = FALSE;
			GetDlgItem(IDC_EDIT_WORDDEVICE)->EnableWindow(!IsWordRefreshing);
			((CMFCButton*)GetDlgItem(IDC_MFCBUTTON2))->SetFaceColor(RGB(243, 243, 243));
			KillTimer(WORD_REFRESH);
			AfxMessageBox(L"Memory is out of range!!!");
			return;
		}

		int Refresh_Size = 0;
		if ((DecNumDouble + WORD_REFRESH_SIZE/2 - 1) > 0x1FFF) //B000~B1FFF
			Refresh_Size = (0x1FFF - DecNumDouble + 1)*2;
		else
			Refresh_Size = WORD_REFRESH_SIZE;

		Result = new char[Refresh_Size];

		if (ReadWordDevice(Device_Type, &charAddress[1], Refresh_Size, Result))
		{
			Word_List.DeleteAllItems();
			int Row_Size = 0;
			if (Refresh_Size % 2 > 0)
				Row_Size = Refresh_Size / 2 + 1;
			else
				Row_Size = Refresh_Size / 2;

			CString Signal;
			char Mask = 0x01;

			for (int i = 0; i < Row_Size; i++)
			{
				csAddress.Format(L"W%X", int(DecNumDouble));
				Word_List.InsertItem(i + 1, csAddress);
				DecNumDouble += 1;

				if (Refresh_Size % 2 > 0 && i == (Row_Size - 1))
					Signal.Format(L"%c", Result[2 * i]);
				else
					Signal.Format(L"%c%c", Result[2 * i], Result[2 * i + 1]);
				Word_List.SetItemText(i, 17, Signal);

				Mask = 0x01;
				for (int j = 0; j < 8; j++)
				{
					if (Result[2 * i] & Mask)
						Signal.Format(L"%d", 1);
					else
						Signal.Format(L"%d", 0);
					Word_List.SetItemText(i, 16 - j, Signal);
					Mask = Mask << 1;
				}

				if (Refresh_Size % 2 == 0 || (Refresh_Size % 2 > 0 && i != Row_Size - 1))
				{
					Mask = 0x01;
					for (int j = 8; j < 16; j++)
					{
						if (Result[2 * i + 1] & Mask)
							Signal.Format(L"%d", 1);
						else
							Signal.Format(L"%d", 0);
						Word_List.SetItemText(i, 16 - j, Signal);
						Mask = Mask << 1;
					}
				}
			}			

			if (Result != NULL)
			{
				delete[] Result;
				Result = NULL;
			}
		}
		else
		{
			if (Result != NULL)
			{
				delete[] Result;
				Result = NULL;
			}

			IsWordRefreshing = FALSE;
			GetDlgItem(IDC_EDIT_WORDDEVICE)->EnableWindow(!IsWordRefreshing);
			((CMFCButton*)GetDlgItem(IDC_MFCBUTTON2))->SetFaceColor(RGB(243, 243, 243));
			KillTimer(WORD_REFRESH);
			return;
		}
	}

	CFormView::OnTimer(nIDEvent);
}


void CMCProtocolExView::OnBnClickedMfcbutton1()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	if (IsBitRefreshing)
	{
		IsBitRefreshing = FALSE;
		KillTimer(BIT_REFRESH);
		((CMFCButton*)GetDlgItem(IDC_MFCBUTTON1))->SetFaceColor(RGB(243, 243, 243));		
	}
	else
	{
		IsBitRefreshing = TRUE;		
		SetTimer(BIT_REFRESH, 100, NULL);
		((CMFCButton*)GetDlgItem(IDC_MFCBUTTON1))->SetFaceColor(RGB(0, 255, 0));
	}

	GetDlgItem(IDC_EDIT_BITDEVICE)->EnableWindow(!IsBitRefreshing);
}

void CMCProtocolExView::OnBnClickedButOnoff()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString csAddress;
	char charAddress[6];
	BOOL Signal[1] = {TRUE};

	GetDlgItem(IDC_EDIT_BITS_ADDRESS)->GetWindowText(csAddress);
	sprintf_s(charAddress, "%ls", csAddress);	

	if (!WriteBitDevice(B, charAddress, 1, Signal))
		AfxMessageBox(L"Write Error!!!");	
}




void CMCProtocolExView::OnBnClickedMfcbutton2()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	if (IsWordRefreshing)
	{
		IsWordRefreshing = FALSE;
		KillTimer(WORD_REFRESH);
		((CMFCButton*)GetDlgItem(IDC_MFCBUTTON2))->SetFaceColor(RGB(243, 243, 243));
	}
	else
	{
		IsWordRefreshing = TRUE;
		SetTimer(WORD_REFRESH, 100, NULL);
		((CMFCButton*)GetDlgItem(IDC_MFCBUTTON2))->SetFaceColor(RGB(0, 255, 0));
	}

	GetDlgItem(IDC_EDIT_WORDDEVICE)->EnableWindow(!IsWordRefreshing);
}


void CMCProtocolExView::OnBnClickedButWriteData()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString csAddress;
	char charAddress[6];
	CString csData;
	char charData[200];

	memset(charData, '\0', 200);

	GetDlgItem(IDC_EDIT_WORD_ADDRESS)->GetWindowText(csAddress);
	sprintf_s(charAddress, "%ls", csAddress);
	GetDlgItem(IDC_EDIT4)->GetWindowText(csData);
	sprintf_s(charData, "%ls", csData);

	int StrLength = strlen(charData);

	if(!WriteWordDevice(W, charAddress, StrLength, charData))
		AfxMessageBox(L"Write Error!!!");
}


void CMCProtocolExView::OnBnClickedButOnoff2()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	
}


void CMCProtocolExView::OnBnClickedButOnoff3()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CString csAddress;
	char charAddress[6];
	BOOL Signal[1] = { FALSE };

	GetDlgItem(IDC_EDIT_BITS_ADDRESS)->GetWindowText(csAddress);
	sprintf_s(charAddress, "%ls", csAddress);

	if (!WriteBitDevice(B, charAddress, 1, Signal))
		AfxMessageBox(L"Write Error!!!");
}
