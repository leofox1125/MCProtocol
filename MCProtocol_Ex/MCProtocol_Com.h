#pragma once

#include "pch.h"   
#include "ESMV_CS_TCPIP_Client_Lib.h"



enum Device_Header
{
	B = 0,
	W
};

BOOL Init_PLC_Com(int ClientIndex, BOOL CreateLog);

BOOL ReadBitDevice(Device_Header Device_Type, char* Start_Index, int BitLength, BOOL* Result);
BOOL ReadWordDevice(Device_Header Device_Type, char* Start_Index, int CharLength, char* Result);

BOOL WriteBitDevice(Device_Header Device_Type, char* Start_Index, int BitLength, BOOL* Data);
BOOL WriteWordDevice(Device_Header Device_Type, char* Start_Index, int CharLength, char* Data);
