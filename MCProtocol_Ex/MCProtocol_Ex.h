
// MCProtocol_Ex.h: MCProtocol_Ex 應用程式的主要標頭檔
//
#pragma once

#ifndef __AFXWIN_H__
	#error "對 PCH 在包含此檔案前先包含 'pch.h'"
#endif

#include "resource.h"       // 主要符號
#include "ESMV_CS_TCPIP_Client_Lib.h"

// CMCProtocolExApp:
// 查看 MCProtocol_Ex.cpp 以了解此類別的實作
//

class CMCProtocolExApp : public CWinApp
{
public:
	CMCProtocolExApp() noexcept;


// 覆寫
public:
	virtual BOOL InitInstance();

// 程式碼實作
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
	int g_nClient_Channel;
};

extern CMCProtocolExApp theApp;
