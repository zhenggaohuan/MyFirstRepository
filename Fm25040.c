/*
 * Fm25040.c
 *
 * Created: 2015-10-29 16:59:46
 *  Author: ZhengGaoHuan
 */

 
#include <avr/io.h>
#include "Fm25040.h"
#include "Delay.h"

#define ID_FMOC_WREN 0x06 
#define ID_FMOC_WRDI 0x04
#define ID_FMOC_WRSR 0x01
#define ID_FMOC_RDSR 0x05
#define ID_FMOC_WRITE 0x02
#define ID_FMOC_READ 0x03

#define ID_FMWP_ALL 0x0C
#define ID_FMWP_HALF 0x08
#define ID_FMWP_QUARTER 0x04
#define ID_FMWP_NONE 0x00

#define ID_FMAD_CODE_FLAG 0
#define ID_FMAD_CODE_VALUE 1 //密码占用2个字节
#define ID_FMAD_NET_FLAG 3
#define ID_FMAD_NET_VALUE 4
#define ID_FMAD_VSCL_FLAG 5
#define ID_FMAD_VSCL_VALUE 6
#define ID_FMAD_ISCL_FLAG 7
#define ID_FMAD_ISCL_VALUE 8
#define ID_FMAD_R_U_FLAG 9
#define ID_FMAD_R_U_VALUE 10 //电压倍率占用2个字节
#define ID_FMAD_R_I_FLAG 12
#define ID_FMAD_R_I_VALUE 13 //电流倍率占用2个字节
#define ID_FMAD_BAUD_FLAG 15
#define ID_FMAD_BAUD_VALUE 16
#define ID_FMAD_DISP_FLAG 17
#define ID_FMAD_DISP_VALUE 18
#define ID_FMAD_VOL_A 19
#define ID_FMAD_VOL_B 23
#define ID_FMAD_VOL_C 27
#define ID_FMAD_CUR_A 31
#define ID_FMAD_CUR_B 35
#define ID_FMAD_CUR_C 39


#define ID_FMMV_FLAG_CODE 0x10
#define ID_FMMV_FLAG_NET 0x20
#define ID_FMMV_FLAG_VSCL 0x30
#define ID_FMMV_FLAG_ISCL 0x40
#define ID_FMMV_FLAG_R_U 0x50
#define ID_FMMV_FLAG_R_I 0x60
#define ID_FMMV_FLAG_BAUD 0x70
#define ID_FMMV_FLAG_DISP 0x80


//fm25040初始化
void fnFm25040Init(void)
{
	DDRB |= (1<<DDRB1); //设置PB1为输出，与Fm25040的WP相连，Fm25040的硬件写保护
	PORTB |= (1<<PORTB1); //设置WP高电平，取消Fm25040的写保护
	DDRB &= ~(1<<DDRB2); //设置PB2为输入，与Fm25040的SO相连，读Fm25040的数据
	DDRB |= (1<<DDRB3); //设置PB3为输出，与Fm25040的CS相连，Fm25040的芯片使能端
	PORTB |= (1<<PORTB3);	//设置CS高电平，Fm25040不使能
	DDRG |= (1<<DDRG3); //设置PG3为输出，与Fm25040的SI相连，写Fm25040
	PORTG |= (1<<PORTG3); //设置SI高电平，在使用的时候才置为低电平
	DDRG |= (1<<DDRG4); //设置PG4为输出，与Fm25040的SCK相连，Fm25040的读写时钟
	PORTG &= ~(1<<PORTG4); //设置SCK为低电平，只有在有数据读写的时候才改变
	fnWREN();
	fnWRSR(ID_FMWP_NONE);
}


//从Fm25040中读密码
unsigned int fnReadCodeFromFm25040(void)
{
	unsigned char byFlag,byValue[2];
	unsigned int iValue = 0;
	fnREAD(ID_FMAD_CODE_FLAG, &byFlag, 1);
	if (byFlag == ID_FMMV_FLAG_CODE)
	{
		fnREAD(ID_FMAD_CODE_VALUE, byValue, 2);
		iValue |= byValue[1];
		iValue <<= 8;
		iValue |= byValue[0];
	}
	return iValue;
}
//往Fm25040中写密码
void fnWriteCodeToFm25040(unsigned int uCode)
{
	unsigned char byFlag,byValue[2];
	byFlag = ID_FMMV_FLAG_CODE;
	fnWREN();
	fnWRITE(ID_FMAD_CODE_FLAG, &byFlag, 1);
	byValue[0] = uCode;
	byValue[1] = uCode>>8;
	fnWREN();
	fnWRITE(ID_FMAD_CODE_VALUE, byValue, 2);
}

//从Fm25040中读输入网络
ty_net fnReadNetFromFm25040(void)
{
	unsigned char byFlag,byValue = 0;
	fnREAD(ID_FMAD_NET_FLAG, &byFlag, 1);
	if (byFlag == ID_FMMV_FLAG_NET)
	{
		fnREAD(ID_FMAD_NET_VALUE, &byValue, 1);
	}
	return (ty_net)byValue;
}
//往Fm25040中写输入网络
void fnWriteNetToFm25040(ty_net eNet)
{
	unsigned char byFlag, byValue;
	byFlag = ID_FMMV_FLAG_NET;
	fnWREN();
	fnWRITE(ID_FMAD_NET_FLAG, &byFlag, 1);
	byValue = eNet;
	fnWREN();
	fnWRITE(ID_FMAD_NET_VALUE, &byValue, 1);
}

//从Fm25040中读电流范围
ty_iscl fnReadISclFromFm25040(void)
{
	unsigned char byFlag,byValue = 0;
	fnREAD(ID_FMAD_ISCL_FLAG, &byFlag, 1);
	if (byFlag == ID_FMMV_FLAG_ISCL)
	{
		fnREAD(ID_FMAD_ISCL_VALUE, &byValue, 1);
	}
	return (ty_iscl)byValue;
}

//往Fm25040中写电流范围
void fnWriteISclToFm25040(ty_iscl eIScl)
{
	unsigned char byFlag,byValue;
	byFlag = ID_FMMV_FLAG_ISCL;
	fnWREN();
	fnWRITE(ID_FMAD_ISCL_FLAG, &byFlag, 1);
	byValue = eIScl;
	fnWREN();
	fnWRITE(ID_FMAD_ISCL_VALUE, &byValue, 1);
}

//从Fm25040中读电压范围
ty_uscl fnReadVSclFromFm25040(void)
{
	unsigned char byFlag,byValue=0;
	fnREAD(ID_FMAD_VSCL_FLAG, &byFlag, 1);
	if (byFlag == ID_FMMV_FLAG_VSCL)
	{
		fnREAD(ID_FMAD_VSCL_VALUE, &byValue, 1);
	}
	return (ty_uscl)byValue;
}

//往Fm25040中写电压范围
void fnWriteVSclToFm25040(ty_uscl eUScl)
{
	unsigned char byFlag, byValue;
	byFlag = ID_FMMV_FLAG_VSCL;
	fnWREN();
	fnWRITE(ID_FMAD_VSCL_FLAG, &byFlag, 1);
	byValue = eUScl;
	fnWREN();
	fnWRITE(ID_FMAD_VSCL_VALUE, &byValue, 1);
}

//从Fm25040中读电压倍率
unsigned int fnReadRUFromFm25040(void)
{
	unsigned char byFlag, byValue[2];
	unsigned int iValue = 0;
	fnREAD(ID_FMAD_R_U_FLAG, &byFlag, 1);
	if (byFlag == ID_FMMV_FLAG_R_U)
	{
		fnREAD(ID_FMAD_R_I_VALUE, byValue, 2);
		iValue |= byValue[1];
		iValue << 8;
		iValue |= byValue[0];
	}
	return iValue;
}

//往Fm25040中写电压倍率
void fnWriteRUToFm25040(unsigned int uRU)
{
	unsigned char byFlag, byValue[2];
	byFlag = ID_FMMV_FLAG_R_U;
	fnWREN();
	fnWRITE(ID_FMAD_R_U_FLAG, &byFlag, 1);
	byValue[0] = uRU;
	byValue[1] = uRU >> 8;
	fnWREN();
	fnWRITE(ID_FMAD_R_I_VALUE, byValue, 2);
}

//从Fm25040中读电流倍率
unsigned int fnReadRIFromFm25040(void)
{
	unsigned char byFlag, byValue[2];
	unsigned int iValue = 0;
	fnREAD(ID_FMAD_R_I_FLAG, &byFlag, 1);
	if (byFlag == ID_FMMV_FLAG_R_I)
	{
		fnREAD(ID_FMAD_R_I_VALUE, byValue, 2);
		iValue |= byValue[1];
		iValue << 8;
		iValue |= byValue[0]; 
	}
	return iValue;
}
//往Fm25040中写电流倍率
void fnWriteRIToFm25040(unsigned int uRI)
{
	unsigned char byFlag, byValue[2];
	byFlag = ID_FMMV_FLAG_R_I;
	fnWREN();
	fnWRITE(ID_FMAD_R_I_FLAG, &byFlag, 1);
	byValue[0] = uRI;
	byValue[1] = uRI>>8;
	fnWREN();
	fnWRITE(ID_FMAD_R_I_VALUE, byValue, 2);
}

//从Fm25040中读波特率
ty_baud fnReadBaudFromFm25040(void)
{
	unsigned char byFlag, byValue=0;
	fnREAD(ID_FMAD_BAUD_FLAG, &byFlag, 1);
	if(byFlag==ID_FMMV_FLAG_BAUD)
	{
		fnREAD(ID_FMAD_BAUD_VALUE,&byValue,1);
	}
	return (ty_baud)byValue;	
}

//往Fm25040中写波特率
void fnWriteBaudToFm25040(ty_baud eBaud)
{
	unsigned char byFlag,byValue;
	byFlag = ID_FMMV_FLAG_BAUD;
	fnWREN();
	fnWRITE(ID_FMAD_BAUD_FLAG, &byFlag, 1);
	byValue = eBaud;
	fnWREN();
	fnWRITE(ID_FMAD_BAUD_VALUE, &byValue, 1);
}

//从Fm25040中读显示亮度
unsigned char fnReadDispFromFm25040(void)
{
	unsigned char byFlag, byValue = 0;
	fnREAD(ID_FMAD_DISP_FLAG, &byFlag, 1);
	fnDelayUs();
	if (byFlag == ID_FMMV_FLAG_DISP)
	{
		fnDelayUs();
		fnREAD(ID_FMAD_DISP_VALUE, &byValue, 1);
	}
	return byValue;
}

//往Fm25040中写显示亮度
void fnWriteDispToFm25040(unsigned char byDisp)
{
	unsigned char byFlag, byValue;
	byFlag = ID_FMMV_FLAG_DISP;
	fnWREN();
	fnDelayUs();
	fnWRITE(ID_FMAD_DISP_FLAG, &byFlag, 1);
	byValue = byDisp;
	fnDelayUs();
	fnWREN();
	fnDelayUs();
	fnWRITE(ID_FMAD_DISP_VALUE, &byValue, 1);
}


//从Fm25040中读全部系统参数
ty_osparameter fnReadOSParameteFromFm25040(void)
{
	ty_osparameter sOSParameter;
	sOSParameter.uCode = fnReadCodeFromFm25040();
	sOSParameter.eNet = fnReadNetFromFm25040();
	sOSParameter.eIScl = fnReadISclFromFm25040();
	sOSParameter.eUScl = fnReadVSclFromFm25040();
	sOSParameter.uRU = fnReadRUFromFm25040();
	sOSParameter.uRI = fnReadRIFromFm25040();
	sOSParameter.eBaud = fnReadBaudFromFm25040();
	sOSParameter.byDisp = fnReadDispFromFm25040();
	return sOSParameter;
}

//往Fm25040中写全部系统参数
void fnWriteOSParameterToFm25040(ty_osparameter sOSParameter)
{
	fnWriteCodeToFm25040(sOSParameter.uCode);
	fnWriteNetToFm25040(sOSParameter.eNet);
	fnWriteISclToFm25040(sOSParameter.eIScl);
	fnWriteVSclToFm25040(sOSParameter.eUScl);
	fnWriteRUToFm25040(sOSParameter.uRU);
	fnWriteRIToFm25040(sOSParameter.uRI);
	fnWriteBaudToFm25040(sOSParameter.eBaud);
	fnWriteDispToFm25040(sOSParameter.byDisp);
}

//往FM25040中写A相电流校正
void fnWriteCurAToFm25040(unsigned long int ulCurA)
{
	unsigned char byValue[4];
	byValue[0] = ulCurA;
	byValue[1] = (ulCurA>>8);
	byValue[2] = (ulCurA>>16);
	byValue[3] = (ulCurA>>24);
	fnWREN();
	fnWRITE(ID_FMAD_CUR_A, byValue, 4);
}

//往FM25040中写B相电流校正
void fnWriteCurBToFm25040(unsigned long int ulCurB)
{
		unsigned char byValue[4];
		byValue[0] = ulCurB;
		byValue[1] = (ulCurB>>8);
		byValue[2] = (ulCurB>>16);
		byValue[3] = (ulCurB>>24);
		fnWREN();
		fnWRITE(ID_FMAD_CUR_B, byValue, 4);
}

//往FM25040中写C相电流校正
void fnWriteCurCToFm25040(unsigned long int ulCurC)
{
	unsigned char byValue[4];
	byValue[0] = ulCurC;
	byValue[1] = (ulCurC>>8);
	byValue[2] = (ulCurC>>16);
	byValue[3] = (ulCurC>>24);
	fnWREN();
	fnWRITE(ID_FMAD_CUR_C, byValue, 4);	
}

//往FM25040中写A相电压校正
void fnWriteVolAToFm25040(unsigned long int ulVolA)
{
		unsigned char byValue[4];
		byValue[0] = ulVolA;
		byValue[1] = (ulVolA>>8);
		byValue[2] = (ulVolA>>16);
		byValue[3] = (ulVolA>>24);
		fnWREN();
		fnWRITE(ID_FMAD_VOL_A, byValue, 4);
}

//往FM25040中写B相电压校正
void fnWriteVolBToFm25040(unsigned long int ulVolB)
{
		unsigned char byValue[4];
		byValue[0] = ulVolB;
		byValue[1] = (ulVolB>>8);
		byValue[2] = (ulVolB>>16);
		byValue[3] = (ulVolB>>24);
		fnWREN();
		fnWRITE(ID_FMAD_VOL_B, byValue, 4);
}

//往FM25040中写C相电压校正
void fnWriteVolCToFm25040(unsigned long int ulVolC)
{
		unsigned char byValue[4];
		byValue[0] = ulVolC;
		byValue[1] = (ulVolC>>8);
		byValue[2] = (ulVolC>>16);
		byValue[3] = (ulVolC>>24);
		fnWREN();
		fnWRITE(ID_FMAD_VOL_C, byValue, 4);
}

//从FM25040中读A相电流校正
unsigned long int fnReadCurAFromFm25040(void)
{
	unsigned char byValue[4];
	unsigned long int ulCurA = 0;
	fnREAD(ID_FMAD_CUR_A, byValue, 4);
	for (int i=0; 1<4; i++)
	{
		ulCurA <<= 8;
		ulCurA |= byValue[i];
	}
	
	return ulCurA;
}

//从FM25040中读B相电流校正
unsigned long int fnReadCurBFromFm25040(void)
{
	unsigned char byValue[4];
	unsigned long int ulCurB = 0;
	fnREAD(ID_FMAD_CUR_B, byValue, 4);
	for (int i=0; 1<4; i++)
	{
		ulCurB <<= 8;
		ulCurB |= byValue[i];
	}
	
	return ulCurB;	
}

//从FM25040中读C相电流校正
unsigned long int fnReadCurCFromFm25040(void)
{
	unsigned char byValue[4];
	unsigned long int ulCurC = 0;
	fnREAD(ID_FMAD_CUR_C, byValue, 4);
	for (int i=0; 1<4; i++)
	{
		ulCurC <<= 8;
		ulCurC |= byValue[i];
	}
	
	return ulCurC;	
}

//从FM25040中读A相电压校正
unsigned long int fnReadVolAFromFm25040(void)
{
		unsigned char byValue[4];
		unsigned long int ulVolA = 0;
		fnREAD(ID_FMAD_VOL_A, byValue, 4);
		for (int i=0; 1<4; i++)
		{
			ulVolA <<= 8;
			ulVolA |= byValue[i];
		}
		
		return ulVolA;
}

//从FM25040中读B相电压校正
unsigned long int fnReadVolBFromFm25040(void)
{
		unsigned char byValue[4];
		unsigned long int ulVolB = 0;
		fnREAD(ID_FMAD_VOL_B, byValue, 4);
		for (int i=0; 1<4; i++)
		{
			ulVolB <<= 8;
			ulVolB |= byValue[i];
		}
		
		return ulVolB;	
}

//从FM25040中读C相电压校正
unsigned long int fnReadVolCFromFm25040(void)
{
		unsigned char byValue[4];
		unsigned long int ulVolC = 0;
		fnREAD(ID_FMAD_VOL_C, byValue, 4);
		for (int i=0; 1<4; i++)
		{
			ulVolC <<= 8;
			ulVolC |= byValue[i];
		}
		
		return ulVolC;	
}


/*
*以下函数是Fm25040文件的基础函数，不对外开放，只在文件内部使用
*/

//输入单个字节到fm25040
void fnMOSIByteToFm25040(unsigned char byData)
{
	for(int i=0; i<8; i++)
	{
		PORTG = (byData&0x80) ? (PORTG | (1<<PORTG3)) : (PORTG & ~(1<<PORTG3));  //取byData最高位
		byData <<= 1;
		fnDelayUs();
		PORTG |= (1<<PORTG4);  //SCK上升沿采集电平
		fnDelayUs();
		PORTG &= ~(1<<PORTG4);  //SCK下降沿，为下次采集电平做准备	
		fnDelayUs();
	}
}

//从fm25040接收单个字节
unsigned char fnMISOByteFromFm25040(void)
{
	unsigned char byData = 0;
	for(int i=0; i<8; i++)
	{
		PORTG |= (1<<PORTG4);  //SCK上升沿，采集数据
		byData<<=1;
		fnDelayUs();
		byData |= (PINB&0x04)?1:0;
		fnDelayUs();
		PORTG &= ~(1<<PORTG4);
		fnDelayUs();
	}
	return byData;
}

//fm25040写使能
void fnWREN(void)
{
	PORTB &= ~(1<<PORTB3);  //CS低电平
	fnDelayUs();	
	PORTG &= ~(1<<PORTG4);  //SCK低电平
	fnDelayUs();
	fnMOSIByteToFm25040(ID_FMOC_WREN);  //输入写使能操作码
	PORTG &= ~(1<<PORTG4); //还原SCK高电平
	PORTB |= (1<<PORTB3);  //还原CS高电平	
}

//fm25040写复位，禁止所有的写操作
void fnWRDI(void)
{
	PORTB &= ~(1<<PORTB3);  //CS低电平
	fnDelayUs();
	PORTG &= ~(1<<PORTG4);  //SCK低电平
	fnMOSIByteToFm25040(ID_FMOC_WRDI);  //输入写复位操作码
	PORTG &= ~(1<<PORTG4); //还原SCK高电平
	PORTB |= (1<<PORTB3);  //还原CS高电平
}

//fm25040读状态寄存器
unsigned char fnRDSR(void)
{
	unsigned char byData;
	PORTB &= ~(1<<PORT3);  //CS低电平
	fnDelayUs();
	PORTG &= ~(1<<PORTG4); //SCK低电平
	fnMOSIByteToFm25040(ID_FMOC_RDSR); //往fm2504里面写读状态寄存器操作码
	byData = fnMISOByteFromFm25040();	//从fm25040里面取得状态寄存器数据
	PORTG &= ~(1<<PORTG4); //SCK低电平
	PORTB |= (1<<PORTB3); //高电平
	return byData;
}

//fm25040写状态寄存器
void fnWRSR(unsigned char byData)
{
	PORTB &= ~(1<<PORTB3); //CS低电平
	PORTG &= ~(1<<PORTG4); //SCK低电平
	fnMOSIByteToFm25040(ID_FMOC_WRSR); //往fm25040写写状态寄存器操作码
	fnMOSIByteToFm25040(byData); //往fm25040写状态寄存器内容
	PORTG &= ~(1<<PORTG4); //SCK低电平
	PORTB |= (1<<PORTB3); //CS高电平
}

//fm25040读内存
void fnREAD(unsigned int iAddress, unsigned char * pbyData, int iCount)
{
	PORTB &= ~(1<<PORTB3); //CS低电平
	fnDelayUs();
	PORTG &= ~(1<<PORTG4); //SCK低电平
	fnDelayUs();
	fnMOSIByteToFm25040((unsigned char)(iAddress>>5)&0x08|ID_FMOC_READ); //往fm25040写读内存操作码，因为写内存的操作码中的第3位为内存地址的最高位 
	fnDelayUs();
	fnMOSIByteToFm25040((unsigned char)iAddress); //往fm25040写内存地址 
	for(int i=0; i<iCount; i++) //读fm25040内存中数据
	{
		*(pbyData+i) = fnMISOByteFromFm25040();
	}
	PORTG &= ~(1<<PORTG4); //SCK低电平
	PORTB |= (1<<PORTB3); //CS高电平 	
}

//fm25040写内存
void fnWRITE(unsigned int iAddress, unsigned char * pbyData, int iCount)
{
	PORTB &= ~(1<<PORTB3); //CS低电平
	fnDelayUs();
	PORTG &= ~(1<<PORTG4); //SCK低电平
	fnDelayUs();
	fnMOSIByteToFm25040((unsigned char)(iAddress>>5)&0x08|ID_FMOC_WRITE); //往fm25040写写内存操作码，因为写内存的操作码中的第3位为内存地址的最高位
	fnMOSIByteToFm25040((unsigned char)iAddress); //往fm25040写内存地址
	for(int i=0; i<iCount; i++) //往fm25040内存中写数据
	{
		fnMOSIByteToFm25040(*(pbyData+i));
	}
	PORTG &= ~(1<<PORTG4); //SCK低电平
	PORTB |= (1<<PORTB3); //CS高电平 
}


