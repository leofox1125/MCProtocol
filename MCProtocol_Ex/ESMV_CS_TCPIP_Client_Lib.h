#ifndef DLLEXPORT
	#define DLLEXPORT extern "C" __declspec(dllexport)
#endif


#ifndef ESMV_CS_TCPIP_CLIENT_LIB_DEF

	#define ESMV_CS_TCPIP_CLIENT_LIB_DEF

	#define ESMV_CS_TCPIP_CLIENT_UPDATE_DATE		"2012/03/02"

	#define ESMV_CS_TCPIP_CLIENT_FAIL				-1
	#define ESMV_CS_TCPIP_CLIENT_SUCCESS			1

	#define ESMV_CS_TCPIP_CLIENT_DLG_WIDTH			100
	#define ESMV_CS_TCPIP_CLIENT_DLG_HEIGHT			101

	#define ESMV_CS_TCPIP_CLIENT_BASE				20000

	//#define	ESMV_CS_TCPIP_CLIENT_KEEP_FISRT_MSG		200
	//#define	ESMV_CS_TCPIP_CLIENT_KEEP_LAST_MSG		201


	#define ESMV_CS_TCPIP_CLIENT_ALL_STATUS			0
	#define ESMV_CS_TCPIP_CLIENT_ONLY_ONNET			1


	#ifdef __cplusplus
	extern "C"{
	#endif

		//有關於資源配置, 設定的函式
		//DLLEXPORT void	ESMV_CS_TCPIP_Client_Alloc(int Msg_Setting);
		DLLEXPORT void	ESMV_CS_TCPIP_Client_Alloc(char* TCP_Client_Version);
		DLLEXPORT void	ESMV_CS_TCPIP_Client_Close();
		DLLEXPORT int	ESMV_CS_TCPIP_Client_Add_Client(HWND hDispWnd, char* IP_Add, int   PortNo);

		DLLEXPORT int	ESMV_CS_TCPIP_Client_Free_Channel(int ClientIndex);


		DLLEXPORT BOOL	ESMV_CS_TCPIP_Client_ReConnect(int ClientIndex);
		DLLEXPORT BOOL	ESMV_CS_TCPIP_Client_Is_Connect(int ClientIndex);
		DLLEXPORT BOOL	ESMV_CS_TCPIP_Client_Check_New_Msg(int ClientIndex);
		DLLEXPORT int	ESMV_CS_TCPIP_Client_Send_Msg(int ClientIndex, char* Send_Msg, int AssignStringLen);
		DLLEXPORT int	ESMV_CS_TCPIP_Client_Receive_Msg(int ClientIndex, char* Recv_Msg);


		DLLEXPORT void	ESMV_CS_TCPIP_Client_Change_Disp_Mode(int ChannelIndex, int Mode_Index);
		DLLEXPORT void	ESMV_CS_TCPIP_Client_Change_Parent(int ChannelIndex, HWND hDispWnd);

		DLLEXPORT void	ESMV_CS_TCPIP_Client_Show_Window(int ClientIndex, BOOL bShow);
		DLLEXPORT void	ESMV_CS_TCPIP_Client_Set_Window_Pos(int ClientIndex, int XOffset, int YOffset);
		DLLEXPORT int	ESMV_CS_TCPIP_Client_Get_Dlg_Size(int ClientIndex, int Index);


#ifdef __cplusplus
	}
	#endif
#endif