#include "pch.h"   //放到第一行,否則編譯過程會跳出C3861 error:找不到ESMV_CS_TCPIP_Client_Send_Msg識別項
#include <ws2tcpip.h> 
#include "MCProtocol_Com.h"

#define BUF_SIZE 2000

int ClientIndex_Plc = 0;
BOOL IsLogOn = FALSE;


BOOL Init_PLC_Com(int ClientIndex, BOOL CreateLog)
{
	ClientIndex_Plc = ClientIndex;
	IsLogOn = CreateLog;

	if (!ESMV_CS_TCPIP_Client_Is_Connect(ClientIndex_Plc))
	{
		ESMV_CS_TCPIP_Client_Free_Channel(ClientIndex_Plc);
		return FALSE;
	}

	return TRUE;
};

int HexStr2DecNum(char* HexStr) 
{
	char TmpChar;
	int Binary[8] = {0};
	char Mask = 0x01;

	for (int i = 0; i < 2; i++) //6A
	{		
		if (HexStr[i] == '0' ||
			HexStr[i] == '1' ||
			HexStr[i] == '2' ||
			HexStr[i] == '3' ||
			HexStr[i] == '4' ||
			HexStr[i] == '5' ||
			HexStr[i] == '6' ||
			HexStr[i] == '7' ||
			HexStr[i] == '8' ||
			HexStr[i] == '9')
		{
			HexStr[i] -= 48;
		} 

		if (HexStr[i] == 'A' ||
			HexStr[i] == 'B' ||
			HexStr[i] == 'C' ||
			HexStr[i] == 'D' ||
			HexStr[i] == 'E' ||
			HexStr[i] == 'F')
		{
			HexStr[i] -= 55;
		}

		Mask = 0x08;
		for (int j = 0; j < 4; j++) //0110 1010  
		{
			if (HexStr[i] & Mask)
				Binary[4 * i + j] = 1;
			else
				Binary[4 * i + j] = 0;

			Mask = Mask >> 1;
		}
	}

	/*CString Res;
	Res.Format(L"%d%d%d%d%d%d%d%d", Binary[0], Binary[1], Binary[2], Binary[3], Binary[4], Binary[5], Binary[6], Binary[7]);
	AfxMessageBox(Res);*/
	
	double DecNumDouble = 0;

	for (int i = 0; i < 8; i++)
		DecNumDouble += pow(2, 7-i)* Binary[i]; //此處不可用int接double資料,會損失精度

	return (int)DecNumDouble;
};

void SetCommonMsg(char* MsgPacket)
{
	//Subheader
	MsgPacket[0] = 0x50;
	MsgPacket[1] = 0x00;

	//Network No.
	MsgPacket[2] = 0x00;

	//PC No.
	MsgPacket[3] = 0xFF;

	//Request destination module I/O No.
	MsgPacket[4] = 0xFF;
	MsgPacket[5] = 0x03;

	//Request destination module station No.
	MsgPacket[6] = 0x00;

	//Monitoring timer
	MsgPacket[9] = 0x01;
	MsgPacket[10] = 0x00;	
};
void LogSocketMsg(char* Msg, int MsgLen, BOOL IsReceive) //true receive, false send
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	FILE* fp1;
	char FileName[200];
	sprintf(FileName, "D://MC_Raw_Log%d%d%d.txt", st.wYear, st.wMonth, st.wDay);
	fopen_s(&fp1, FileName, "a");

	if(IsReceive)
		fprintf(fp1, "%02d:%02d:%02d:%03d  Receive: ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);		
	else
		fprintf(fp1, "%02d:%02d:%02d:%03d  Send: ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);		

	char Mask1 = 0b00000001;
	double DecNumDouble = 0;
	for (int i = 0; i < MsgLen; i++) //6A
	{
		DecNumDouble = 0;
		Mask1 = 0b00000001;
		for (int j = 0; j < 8; j++)
		{
			if (Msg[i] & Mask1)
				DecNumDouble += pow(2, j);
			Mask1 = Mask1 << 1;
		}

		if(DecNumDouble < 16)
			fprintf(fp1, "0%X ", (int)DecNumDouble);
		else
			fprintf(fp1, "%X ", (int)DecNumDouble);
	}
	fprintf(fp1, "\n");
	fclose(fp1);
}

BOOL ReadBitDevice(Device_Header Device_Type, char* Start_Index, int BitLength, BOOL* Result)
{
	char Receive_Msg[BUF_SIZE];

	while (ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc))
	{
		ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, Receive_Msg);
		Sleep(100);
	}

#pragma region Send Message
	char SendMsg[BUF_SIZE];
	int Command_Len = 21;
	char TmpChar[] = "000000";
	int TmpDataLength = 0;
	int StrLen = 0;

	SetCommonMsg(SendMsg);
	
	//-----Request data length-----
	SendMsg[7] = 0x0C;
	SendMsg[8] = 0x00;

	//-----Command-----
	SendMsg[11] = 0x01;
	SendMsg[12] = 0x04;

	//-----Subcommand-----
	SendMsg[13] = 0x01;
	SendMsg[14] = 0x00;
	
	//-----Device number-----, B,X,Y->hex other->dec 16A    1-> 0001, 6A-> 0110 1010-> 106
	if (Device_Type == B)
	{
		while (Start_Index[StrLen] != 0x00 || StrLen > 6)
			StrLen++;

		if (StrLen > 6 || StrLen < 1)
			return FALSE;

		for (int i = 0; i < StrLen; i++)
			TmpChar[6 - StrLen + i] = Start_Index[i];

		for (int i = 0; i < 3; i++)
			SendMsg[15 + i] = HexStr2DecNum(TmpChar + 4 - 2 * i);
	}
	else
		return FALSE;

	//-----Device code-----
	CString Res;	
	if(Device_Type == B)
		SendMsg[18] = 0xA0;

	//-----Number of device points-----	
	if (BitLength > 0xFFFF)
		return FALSE;
	//10進位 > 16進位字串 > HexStr2DecNum
	for (int i = 0; i < 6; i++)
		TmpChar[i] = '\0';
	sprintf_s(TmpChar, "%X", BitLength); //10進位 > 16進位字串	

	StrLen = 0;
	while (TmpChar[StrLen] != 0x00)
		StrLen++;

	for (int i = StrLen - 1; i > -1; i--) //靠右對齊再順序顛倒	
		TmpChar[i + 4 - StrLen] = TmpChar[i];

	for (int i = 0; i < 4-StrLen; i++) //補0
		TmpChar[i] = '0';

	for (int i = 0; i < 2; i++)
		SendMsg[19 + i] = HexStr2DecNum(TmpChar + 2 - 2 * i);

	if (ESMV_CS_TCPIP_Client_Send_Msg(ClientIndex_Plc, SendMsg, Command_Len) != -1)
	{
		if (IsLogOn)
			LogSocketMsg(SendMsg, Command_Len, FALSE);
	}
	else
		return FALSE;
#pragma endregion

#pragma region Receive Message
	int TimeOut = 0;
	//Sleep(2000);

	do 
	{
		TimeOut ++ ;
		Sleep(10);
	} while (TimeOut <= 200 && !ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc));

	int RecvMsgLen = 0;
	char RecvMsg[BUF_SIZE];

	//memset(RecvMsg, '\0', BUF_SIZE);

	if (TimeOut > 200)
		return FALSE;
	else
	{
		RecvMsgLen = ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, RecvMsg);
		if (IsLogOn)
			LogSocketMsg(RecvMsg, RecvMsgLen, TRUE);
	}

	if (RecvMsg[9] != 0x00 || RecvMsg[10] != 0x00) //End code必須為0,傳回的資料才是正確的
		return FALSE;

	double RecvBitLen = 0;

	char Mask = 0x01;
	for (int i = 7; i < 9; i++) //將收到的bit數量,由2進位轉10進位
	{
		Mask = 0x01;
		for (int j = 0; j < 8; j++)
		{
			if (RecvMsg[i] & Mask)
				RecvBitLen += pow(2, j + 8*(i-7));

			Mask = Mask << 1;
		}		
	}

	RecvBitLen -= 2;

	RecvBitLen *= 2;

	/*Res.Format(L"MsgLen = %lf", RecvBitLen);
	AfxMessageBox(Res);*/

	if (BitLength % 2 == 1)
	{
		if (RecvBitLen != BitLength + 1)
			return FALSE;
	}
	else
	{
		if (RecvBitLen != BitLength)
			return FALSE;
	}

	RecvBitLen /= 2;

	for (int i = 11; i < 11 + RecvBitLen; i++)
	{
		Mask = 0b00010000;
		if(Mask & RecvMsg[i])
			Result[2*(i - 11)] = TRUE;
		else
			Result[2 * (i - 11)] = FALSE;

		if (BitLength % 2 == 1 && i == 11 + RecvBitLen - 1)
			break;

		Mask = 0b00000001;
		if(Mask & RecvMsg[i])
			Result[2*(i - 11) + 1] = TRUE;
		else
			Result[2 * (i - 11) + 1] = FALSE;
	}

	return TRUE;
#pragma endregion
};

BOOL ReadWordDevice(Device_Header Device_Type, char* Start_Index, int CharLength, char* Result)
{
	char Receive_Msg[BUF_SIZE];

	while (ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc))
	{
		ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, Receive_Msg);
		Sleep(100);
	}

#pragma region Send Message
	char SendMsg[BUF_SIZE];
	int Command_Len = 21;
	char TmpChar[] = "000000";
	int TmpDataLength = 0;
	int StrLen = 0;

	SetCommonMsg(SendMsg);

	//-----Request data length-----
	SendMsg[7] = 0x0C;
	SendMsg[8] = 0x00;

	//-----Command-----
	SendMsg[11] = 0x01;
	SendMsg[12] = 0x04;

	//-----Subcommand-----
	SendMsg[13] = 0x00;
	SendMsg[14] = 0x00;

	//-----Device number-----
	if (Device_Type == W)
	{
		while (Start_Index[StrLen] != 0x00 || StrLen > 6)
			StrLen++;

		if (StrLen > 6 || StrLen < 1)
			return FALSE;

		for (int i = 0; i < StrLen; i++)
			TmpChar[6 - StrLen + i] = Start_Index[i];

		for (int i = 0; i < 3; i++)
			SendMsg[15 + i] = HexStr2DecNum(TmpChar + 4 - 2 * i);
	}
	else
		return FALSE;

	//CString Res;	

	//-----Device code-----
	if (Device_Type == W)
		SendMsg[18] = 0xB4;

	//-----Number of device points-----
	if (CharLength > 0xFFFF)
		return FALSE;
	//10進位 > 16進位字串 > HexStr2DecNum
	int TmpCharLength = CharLength / 2;
	if (CharLength % 2 == 1) //湊成偶數
		TmpCharLength += 1;
	for (int i = 0; i < 6; i++)
		TmpChar[i] = '\0';
	sprintf_s(TmpChar, "%X", TmpCharLength); //10進位 > 16進位字串

	/*Res.Format(L"%s", TmpChar);
	AfxMessageBox(Res);*/

	StrLen = 0;
	while (TmpChar[StrLen] != 0x00)
		StrLen++;

	for (int i = StrLen - 1; i > -1; i--) //靠右對齊	
		TmpChar[i + 4 - StrLen] = TmpChar[i];

	/*Res.Format(L"%s", TmpChar);
	AfxMessageBox(Res);*/

	for (int i = 0; i < 4 - StrLen; i++) //補0
		TmpChar[i] = '0';

	/*Res.Format(L"%s", TmpChar);
	AfxMessageBox(Res);*/

	for (int i = 0; i < 2; i++)
		SendMsg[19 + i] = HexStr2DecNum(TmpChar + 2 - 2 * i);

	if (ESMV_CS_TCPIP_Client_Send_Msg(ClientIndex_Plc, SendMsg, Command_Len) != -1)
	{
		if (IsLogOn)
			LogSocketMsg(SendMsg, Command_Len, FALSE);
	}
	else
		return FALSE;
#pragma endregion

#pragma region Receive Message
	int TimeOut = 0;

	do
	{
		TimeOut++;
		Sleep(10);
	} while (TimeOut <= 200 && !ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc));

	int RecvMsgLen = 0;
	char RecvMsg[BUF_SIZE];

	if (TimeOut > 200)
		return FALSE;
	else
	{
		RecvMsgLen = ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, RecvMsg);
		if (IsLogOn)
			LogSocketMsg(RecvMsg, RecvMsgLen, TRUE);
	}

	/*Res.Format(L"MsgLen = %X", RecvMsg);
	AfxMessageBox(Res);*/

	if (RecvMsg[9] != 0x00 || RecvMsg[10] != 0x00) //End code必須為0,傳回的資料才是正確的
		return FALSE;

	double RecvCharLen = 0;

	char Mask = 0x01;
	for (int i = 7; i < 9; i++) //將收到的word數量,由2進位轉10進位
	{
		Mask = 0x01;
		for (int j = 0; j < 8; j++)
		{
			if (RecvMsg[i] & Mask)
				RecvCharLen += pow(2, j + 8 * (i - 7));

			Mask = Mask << 1;
		}
	}

	RecvCharLen -= 2;

	/*Res.Format(L"MsgLen = %lf", RecvBitLen);
	AfxMessageBox(Res);*/

	if (CharLength % 2 == 1)
	{
		if (RecvCharLen != CharLength + 1)
			return FALSE;
	}
	else
	{
		if (RecvCharLen != CharLength)
			return FALSE;
	}

	memcpy(Result, &RecvMsg[11], CharLength);

	return TRUE;
#pragma endregion
};

BOOL WriteBitDevice(Device_Header Device_Type, char* Start_Index, int BitLength, BOOL* Data)
{
	char Receive_Msg[BUF_SIZE];

	while (ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc))
	{
		ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, Receive_Msg);
		Sleep(100);
	}

#pragma region Send Message
	char SendMsg[BUF_SIZE];
	int Command_Len = 21;
	char TmpChar[] = "000000";
	int TmpDataLength = 0;
	int StrLen = 0;	

	SetCommonMsg(SendMsg);

	//-----Request data length-----
	int ReqDataLen = 0;
	if (BitLength % 2 == 1)
		Command_Len = 21 + BitLength / 2 + 1;
	else
		Command_Len = 21 + BitLength / 2;

	ReqDataLen = Command_Len - 9;

	for (int i = 0; i < 6; i++)
		TmpChar[i] = '\0';
	sprintf_s(TmpChar, "%X", ReqDataLen);

	StrLen = 0;
	while (TmpChar[StrLen] != 0x00)
		StrLen++;	

	for (int i = StrLen - 1; i > -1; i--) //靠右對齊	
		TmpChar[i + (6-StrLen)] = TmpChar[i];
	
	for (int i = 0; i < 6 - StrLen; i++) //補0
		TmpChar[i] = '0';	

	for (int i = 0; i < 2; i++)
		SendMsg[7 + i] = HexStr2DecNum(TmpChar + 4 - 2 * i);	

	//-----Command-----
	SendMsg[11] = 0x01;
	SendMsg[12] = 0x14;

	//-----Subcommand-----
	SendMsg[13] = 0x01;
	SendMsg[14] = 0x00;

	//-----Device number-----
	if (Device_Type == B)
	{
		while (Start_Index[StrLen] != 0x00 || StrLen > 6)
			StrLen++;

		if (StrLen > 6 || StrLen < 1)
			return FALSE;

		for (int i = 0; i < StrLen; i++)
			TmpChar[6 - StrLen + i] = Start_Index[i];

		for (int i = 0; i < 3; i++)
			SendMsg[15 + i] = HexStr2DecNum(TmpChar + 4 - 2 * i);
	}
	else
		return FALSE;	

	//-----Device code-----
	if (Device_Type == B)
		SendMsg[18] = 0xA0;

	//-----Number of device points	-----
	if (BitLength > 0xFFFF)
		return FALSE;
	//10進位 > 16進位字串 > HexStr2DecNum
	for (int i = 0; i < 6; i++)
		TmpChar[i] = '\0';
	sprintf_s(TmpChar, "%X", BitLength); //10進位 > 16進位字串

	StrLen = 0;
	while (TmpChar[StrLen] != 0x00)
		StrLen++;

	for (int i = StrLen - 1; i > -1; i--) //靠右對齊	
		TmpChar[i + 4 - StrLen] = TmpChar[i];

	for (int i = 0; i < 4 - StrLen; i++) //補0
		TmpChar[i] = '0';

	for (int i = 0; i < 2; i++)
		SendMsg[19 + i] = HexStr2DecNum(TmpChar + 2 - 2 * i);

	//-----Data to write-----
	for (int i = 0; i < Command_Len-21; i++)
	{
		if (Data[2 * i] == TRUE)
		{
			if(BitLength%2 == 1 && i == BitLength / 2 + 1)
				SendMsg[21 + i] = 0b00010000;
			else
			{
				if (Data[2 * i + 1] == FALSE)
					SendMsg[21 + i] = 0b00010000;
				else
					SendMsg[21 + i] = 0b00010001;
			}
		}
		if (Data[2 * i] == FALSE)
		{
			if (BitLength % 2 == 1 && i == BitLength / 2 + 1)
				SendMsg[21 + i] = 0b00000000;
			else
			{
				if (Data[2 * i + 1] == FALSE)
					SendMsg[21 + i] = 0b00000000;
				else
					SendMsg[21 + i] = 0b00000001;
			}
		}
	}

	if (ESMV_CS_TCPIP_Client_Send_Msg(ClientIndex_Plc, SendMsg, Command_Len) != -1)
	{
		if (IsLogOn)
			LogSocketMsg(SendMsg, Command_Len, FALSE);
	}	
	else
		return FALSE;
#pragma endregion

#pragma region Receive Message
	int TimeOut = 0;
	int RecvMsgLen = 0;
	char RecvMsg[BUF_SIZE];

	//Sleep(100);

	do
	{
		TimeOut++;
		Sleep(10);
	} while (TimeOut <= 200 && !ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc));

	if (TimeOut > 200)
		return FALSE;
	else
	{
		RecvMsgLen = ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, RecvMsg);
		if (IsLogOn)
			LogSocketMsg(RecvMsg, RecvMsgLen, TRUE);
	}

	//確認回覆結果是否正確
	if (RecvMsgLen != 11)
		return FALSE;

	int TmpValue = 0;
	int Mask = 0b00000001;
	for (int i = 0; i < 10; i++)
	{
		TmpValue = 0;
		Mask = 0b00000001;
		for (int j = 0; j < 8; j++)
		{
			if (RecvMsg[i] & Mask)
				TmpValue += pow(2, j);
			Mask = Mask << 1;
		}

		if (i == 0 && TmpValue != 0xD0)
			return FALSE;
		if (i == 1 && TmpValue != 0x00)
			return FALSE;
		if (i == 2 && TmpValue != 0x00)
			return FALSE;
		if (i == 3 && TmpValue != 0xFF)
			return FALSE;
		if (i == 4 && TmpValue != 0xFF)
			return FALSE;
		if (i == 5 && TmpValue != 0x03)
			return FALSE;
		if (i == 6 && TmpValue != 0x00)
			return FALSE;
		if (i == 7 && TmpValue != 0x02)
			return FALSE;
		if (i == 8 && TmpValue != 0x00)
			return FALSE;
		if (i == 9 && TmpValue != 0x00)
			return FALSE;
		if (i == 10 && TmpValue != 0x00)
			return FALSE;
	}

	return TRUE;
#pragma endregion
};

BOOL WriteWordDevice(Device_Header Device_Type, char* Start_Index, int CharLength, char* Data)
{
	
	char Receive_Msg[BUF_SIZE];

	while (ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc))
	{
		ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, Receive_Msg);
		Sleep(100);
	};

#pragma region Send Message
	char SendMsg[BUF_SIZE];
	int Command_Len = 0;
	char TmpChar[] = "000000";
	int TmpDataLength = 0;
	int StrLen = 0;
	CString Res;

	SetCommonMsg(SendMsg);

	//-----Request data length-----
	int ReqDataLen = 0;
	
	if (CharLength % 2 == 1)		
		Command_Len = 21 + CharLength + 1;	
	else
		Command_Len = 21 + CharLength;
	
	ReqDataLen = Command_Len - 9;

	for (int i = 0; i < 6; i++)
		TmpChar[i] = '\0';
	sprintf_s(TmpChar, "%X", ReqDataLen);

	StrLen = 0;
	while (TmpChar[StrLen] != 0x00)
		StrLen++;	

	for (int i = StrLen - 1; i > -1; i--) //靠右對齊	
		TmpChar[i + (6 - StrLen)] = TmpChar[i];	

	for (int i = 0; i < 6 - StrLen; i++) //補0
		TmpChar[i] = '0';	

	for (int i = 0; i < 2; i++)
		SendMsg[7 + i] = HexStr2DecNum(TmpChar + 4 - 2 * i);

	//-----Command-----
	SendMsg[11] = 0x01;
	SendMsg[12] = 0x14;

	//-----Subcommand-----
	SendMsg[13] = 0x00;
	SendMsg[14] = 0x00;

	//-----Device number-----
	if (Device_Type == W)
	{
		while (Start_Index[StrLen] != 0x00 || StrLen > 6)
			StrLen++;

		if (StrLen > 6 || StrLen < 1)
			return FALSE;

		for (int i = 0; i < StrLen; i++)
			TmpChar[6 - StrLen + i] = Start_Index[i];

		for (int i = 0; i < 3; i++)
			SendMsg[15 + i] = HexStr2DecNum(TmpChar + 4 - 2 * i);
	}
	else
		return FALSE;	

	//-----Device code-----
	if (Device_Type == W)
		SendMsg[18] = 0xB4;

	//-----Number of device points	-----
	if (CharLength > 0xFFFF)
		return FALSE;
	//10進位 > 16進位字串 > HexStr2DecNum
	for (int i = 0; i < 6; i++)
		TmpChar[i] = '\0';
	if(CharLength%2 == 1)
		sprintf_s(TmpChar, "%X", CharLength/2 + 1); //10進位 > 16進位字串
	else
		sprintf_s(TmpChar, "%X", CharLength / 2);
	
	StrLen = 0;
	while (TmpChar[StrLen] != 0x00)
		StrLen++;

	for (int i = StrLen - 1; i > -1; i--) //靠右對齊	
		TmpChar[i + 4 - StrLen] = TmpChar[i];
	
	for (int i = 0; i < 4 - StrLen; i++) //補0
		TmpChar[i] = '0';	

	for (int i = 0; i < 2; i++)
		SendMsg[19 + i] = HexStr2DecNum(TmpChar + 2 - 2 * i);

	//-----Data to write-----
	for (int i = 0; i < CharLength; i++)
		SendMsg[21 + i] = Data[i];	

	if (CharLength % 2 == 1)
	{
		/*char* Original_Data =  new char[CharLength + 1];
				
		if (ReadWordDevice(W, Start_Index, CharLength + 1, Original_Data))
		{
			SendMsg[21 + CharLength] = Original_Data[CharLength];
			
			if (Original_Data != NULL)
			{
				delete[]Original_Data;
				Original_Data = NULL;
			}
		}
		else
		{
			if (Original_Data != NULL)
			{
				delete[]Original_Data;
				Original_Data = NULL;
			}
			return FALSE;
		}		*/

		AfxMessageBox(L"Even number of char is allowed !!!");
		return FALSE;
	}	

	if (ESMV_CS_TCPIP_Client_Send_Msg(ClientIndex_Plc, SendMsg, Command_Len) != -1)
	{
		if (IsLogOn)
			LogSocketMsg(SendMsg, Command_Len, FALSE);
	}	
	else
		return FALSE;
#pragma endregion

#pragma region Receive Message
	int TimeOut = 0;
	int RecvMsgLen = 0;
	char RecvMsg[BUF_SIZE];

	do
	{
		TimeOut++;
		Sleep(10);
	} while (TimeOut <= 200 && !ESMV_CS_TCPIP_Client_Check_New_Msg(ClientIndex_Plc));

	if (TimeOut > 200)
		return FALSE;
	else
	{
		RecvMsgLen = ESMV_CS_TCPIP_Client_Receive_Msg(ClientIndex_Plc, RecvMsg);
		if (IsLogOn)
			LogSocketMsg(RecvMsg, RecvMsgLen, TRUE);
	}

	//確認回覆結果是否正確
	if (RecvMsgLen != 11)
		return FALSE;

	int TmpValue = 0;
	int Mask = 0b00000001;
	for (int i = 0; i < 10; i++)
	{
		TmpValue = 0;
		Mask = 0b00000001;
		for (int j = 0; j < 8; j++)
		{
			if (RecvMsg[i] & Mask)
				TmpValue += pow(2, j);
			Mask = Mask << 1;
		}

		if (i == 0 && TmpValue != 0xD0)
			return FALSE;
		if (i == 1 && TmpValue != 0x00)
			return FALSE;
		if (i == 2 && TmpValue != 0x00)
			return FALSE;
		if (i == 3 && TmpValue != 0xFF)
			return FALSE;
		if (i == 4 && TmpValue != 0xFF)
			return FALSE;
		if (i == 5 && TmpValue != 0x03)
			return FALSE;
		if (i == 6 && TmpValue != 0x00)
			return FALSE;
		if (i == 7 && TmpValue != 0x02)
			return FALSE;
		if (i == 8 && TmpValue != 0x00)
			return FALSE;
		if (i == 9 && TmpValue != 0x00)
			return FALSE;
		if (i == 10 && TmpValue != 0x00)
			return FALSE;
	}

	return TRUE;
#pragma endregion
};