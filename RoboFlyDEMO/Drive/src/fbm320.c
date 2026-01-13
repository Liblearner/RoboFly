/*******************************************************************************************
										    �� ��
    ����Ŀ�����������ѧϰʹ�ã�����������ֲ�޸ģ������뱣����������Ϣ����ֲ�����г�������
	
���ɹ�����BUG��������������κ����Ρ��������ã�

* ����汾��V1.01
* �������ڣ�2018-8-18
* �������ߣ���ŭ��С��
* ��Ȩ���У��������������Ϣ�������޹�˾
*******************************************************************************************/
#include "stm32f10x.h"
#include "fbm320.h"
#include "iic.h"
#include "stdio.h"
#include "delay.h"
#include "structconfig.h"
#include "math.h"
#include "led.h"
#include "filter.h"

FBMTYPE FBM;
uint8_t ALTIUDE_OK = 0,ALT_Updated = 0;
float RPFilter;

/*****************************************************************************
* ��  ����uint8_t FBM320_WriteByte(uint8_t addr,uint8_t reg,uint8_t data)
* ��  �ܣ�дһ���ֽ����ݵ� FBM320 �Ĵ���
* ��  ����reg�� �Ĵ�����ַ
*         data: Ҫд�������
* ����ֵ��0�ɹ� 1ʧ��
* ��  ע��FBM320������ֲֻ���I2C�����޸ĳ��Լ��ļ���
*****************************************************************************/
uint8_t FBM320_WriteByte(uint8_t reg,uint8_t data)
{
	if(IIC_WriteByteToSlave(FBMAddr,reg,data))
	   return 1;
	else
	   return 0;
}

/*****************************************************************************
* ��  ����uint8_t FBM320_ReadByte(uint8_t reg,uint8_t *buf)
* ��  �ܣ���ָ��FBM320�Ĵ�����ȡһ���ֽ�����
* ��  ����reg�� �Ĵ�����ַ
*         buf:  ��ȡ���ݴ�ŵĵ�ַ
* ����ֵ��1ʧ�� 0�ɹ�
* ��  ע��FBM320������ֲֻ���I2C�����޸ĳ��Լ��ļ���
*****************************************************************************/
uint8_t FBM320_ReadByte(uint8_t reg,uint8_t *buf)
{
	if(IIC_ReadByteFromSlave(FBMAddr,reg,buf))
	   return 1;
	else
	   return 0;
}

/*****************************************************************************
* ��  ����uint8_t FBM320_WriteMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
* ��  �ܣ���ָ���Ĵ���д��ָ����������
* ��  ����reg���Ĵ�����ַ
*         len��д�����ݳ��� 
*         buf: д�����ݴ�ŵĵ�ַ
* ����ֵ��0�ɹ� 1ʧ��
* ��  ע��FBM320������ֲֻ���I2C�����޸ĳ��Լ��ļ���
*****************************************************************************/
uint8_t FBM320_WriteMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
{
	if(IIC_WriteMultByteToSlave(FBMAddr,reg,len,buf))
	   return 1;
	else
	   return 0;

}

/*****************************************************************************
* ��  ����uint8_t FBM320_ReadMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
* ��  �ܣ���ָ���Ĵ�����ȡָ����������
* ��  ����reg���Ĵ�����ַ
*         len����ȡ���ݳ��� 
*         buf: ��ȡ���ݴ�ŵĵ�ַ
* ����ֵ��0�ɹ� 0ʧ��
* ��  ע��FBM320������ֲֻ���I2C�����޸ĳ��Լ��ļ���
*****************************************************************************/
uint8_t FBM320_ReadMultBytes(uint8_t reg,uint8_t len,uint8_t *buf)
{
	if(IIC_ReadMultByteFromSlave(FBMAddr,reg,len,buf))
	   return 1;
	else
	   return 0;
}

/*============================���ϴ�����ֲʱ��Ҫ�޸�=========================*/

/******************************************************************************
* ��  ����uint8_tFBM320_getDeviceID(void)
* ��  �ܣ���ȡ  FBM320 WHO_AM_I ��ʶ������ 0x42
* ��  ������
* ����ֵ�����ض�ȡ����
* ��  ע����
*******************************************************************************/
uint8_t FBM320_getDeviceID(void)
{
    uint8_t buf;
	FBM320_ReadByte(FBM_ID, &buf);
    return buf;
}

/******************************************************************************
* ��  ����uint8_tFBM320_testConnection(void)
* ��  �ܣ����FBM320 �Ƿ��Ѿ�����
* ��  ������
* ����ֵ��1������ 0δ����
* ��  ע����
*******************************************************************************/
uint8_t FBM320_testConnection(void) 
{
	if(FBM320_getDeviceID() == FBMID)  //0x42
		return 1;
	else 
		return 0;
}

/******************************************************************************
* ��  ����void FBM320_Check()
* ��  �ܣ����IIC�����ϵ�FBM320�Ƿ����
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/
void FBM320_Check(void) 
{ 
	while(!FBM320_testConnection())
	{
		printf("\rFBM320 no connect...\r\n");
		RGB_LED_Blue();//���Ƴ���
	}
} 

/******************************************************************************
* ��  ����void FBM320_Init(void)
* ��  �ܣ���ʼ��FBM320����ȡFBM320У׼����
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/
void FBM320_Init(void)
{
	FBM320_Check(); //���FBM320��MCU�Ƿ�����ͨ��
	
	FBM320_WriteByte(FBM_RESET,FBMRESET);//��λFBM320
	Delay_ms(100);
	FBM320_GetCoeff(); //��ȡFBM320��У׼����
}

/************************************************************************************
* ��  ����uint8_t Init_Altitude(void)
* ��  �ܣ���ʼ�����ʱ�ĸ߶�����
* ��  ������
* ����ֵ��0 ��ʼ��δ��� 1 ��ʼ����� 
* ��  ע���˺�����ʼ������ʵ�����ʱ�Ĵ���ѹ(FBM.InitPress)��Ϊ��������Ը߶���׼��
************************************************************************************/
uint8_t Init_Altitude(void)
{
	static uint8_t cnt_p = 0;
	static int64_t PressNUM = 0;
	if(GET_FLAG(BAR_OFFSET)) //INIT_ALTIUDE_OK=1 ��ʼ����ʼ
	{
		if(cnt_p == 0)
		{
			cnt_p = 1;
			PressNUM = 0;
		}
		PressNUM += FBM.RP; //100����ѹ�����ۼ�
		if(cnt_p == 100)
		{
			FBM.InitPress = (float)PressNUM/cnt_p; //��ƽ��ֵ
			SENSER_FLAG_RESET(BAR_OFFSET);//У׼��ѹ�ƽ���
			cnt_p = 0; 
			return 1;
		}
		cnt_p++;
   }
	return 0;
}

/******************************************************************************
* ��  ����void FBM320_GetAltitude(void) 
* ��  �ܣ�FBM320����ѹֵת������Ը߶�
* ��  ������
* ����ֵ����
* ��  ע��ע�⣺�˺�������Ƶ�����200Hz,�����ȡ�ĸ߶�ֵ���ϴ󣡣���
*******************************************************************************/
void FBM320_GetAltitude(void) //200Hz
{
	uint8_t buf[3];
	static uint8_t timecnt = 0;
	float  AltitudeFilter;
	switch(timecnt)//�¶Ⱥ���ѹ����ת��ʱ�䲻һ��
	{
		case 0:  
				FBM320_WriteByte(FBM_CONFIG,TEMP_CONVERSION); //280us
				break;
		case 1: //5msת��һ�� (836us)
				FBM320_ReadMultBytes(DATA_MSB,3,buf); 
				FBM.ADTemp = (buf[0]<<16)|(buf[1]<<8)|buf[2];
				FBM320_WriteByte(FBM_CONFIG,PRES_CONVERSION+OSR8192); //������ѹת��
				break;
		case 5: //15msת��һ�� (834us)
				FBM320_ReadMultBytes(DATA_MSB,3,buf);  
				FBM.ADPress = (buf[0]<<16)|(buf[1]<<8)|buf[2];
				FBM320_WriteByte(FBM_CONFIG,TEMP_CONVERSION); //�����¶�ת��
				FBM320_Calculate( FBM.ADPress , FBM.ADTemp );	//��FBM320��ԭʼ����ת���������� ��ѹ��λ�������¶ȵ�λ���϶�
		  
		if(Init_Altitude())
		{
			BAR_Offset_LED();//��ѹ��У׼�ɹ�ָʾ��
			ALTIUDE_OK = 1;
		}
		   
		SortAver_Filter((float)FBM.RP,&FBM.RPFilter,12); //ȥ��ֵ��ֵ�˲�
//		if(RPFilter)
//		Aver_Filter(FBM.RP,&FBM.RPFilter,5); //���������˲�//�˲��е�����!!!!!
		//printf("FBM.RP:%0.2f  FBM.RPFilter:%0.2f\r\n",FBM.RP,FBM.RPFilter);
		if(FBM.RPFilter && ALTIUDE_OK) 
		{
			ALT_Updated = 1;
			FBM.Altitude = 44330.0f * (1 - powf((float)FBM.RPFilter/ FBM.InitPress, 0.190295f));
		}
	//FBM.Altitude =  ((pow((1015.7f / (FBM.RP/100)), 0.1902630958) - 1.0f) * (25 + 273.15f)) / 0.0065f;
			 
		SortAver_Filter1(FBM.Altitude ,&AltitudeFilter,12); //ȥ��ֵ��ֵ�˲�
		if(AltitudeFilter)
			Aver_Filter1(AltitudeFilter,&FBM.AltitudeFilter,8); //���������˲�
		
			 
	// FBM.Altitude = ((float)Abs_Altitude((int32_t)FBM.RPFilter))/1000;		//������Ը߶�(����ں�ƽ��)
				break;
				default:
				break;
		
	  }
	timecnt++; //ת��ʱ�����
	if(timecnt>5)
		timecnt = 0;
//		printf("Altitude:%0.2f \r\n",FBM.Altitude);
}

/******************************************************************************
* ��  ����void FBM320_GetCoeff(void)
* ��  �ܣ���ȡ�洢��FBM320Ƭ�ڵ�У׼����
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/
void FBM320_GetCoeff(void)
{
	uint8_t data[20],i = 0;
	uint16_t R[10];
	FBM320_ReadMultBytes(FBM_COEFF1,18,data);
	FBM320_ReadByte(FBM_COEFF3,&data[18]);
	FBM320_ReadByte(FBM_COEFF4,&data[19]);
	for(i=0;i<10;i++)
	{
		R[i] = (uint16_t)(data[2*i]<<8)|data[2*i+1];
	}
		/* Use R0~R9 calculate C0~C12 of FBM320-02	*/
	FBM.C0 = R[0] >> 4;
	FBM.C1 = ((R[1] & 0xFF00) >> 5) | (R[2] & 7);
	FBM.C2 = ((R[1] & 0xFF) << 1) | (R[4] & 1);
	FBM.C3 = R[2] >> 3;
	FBM.C4 = ((uint32_t)R[3] << 2) | (R[0] & 3);
	FBM.C5 = R[4] >> 1;
	FBM.C6 = R[5] >> 3;
	FBM.C7 = ((uint32_t)R[6] << 3) | (R[5] & 7);
	FBM.C8 = R[7] >> 3;
	FBM.C9 = R[8] >> 2;
	FBM.C10 = ((R[9] & 0xFF00) >> 6) | (R[8] & 3);
	FBM.C11 = R[9] & 0xFF;
	FBM.C12 = ((R[0] & 0x0C) << 1) | (R[7] & 7);
	
//	for(i=0;i<10;i++)
//	printf("R = %d\n",R[i]);
//	printf("C0=%d C1=%d C2=%d\n",FBM.C0,FBM.C1,FBM.C2);
//	printf("C3=%d C4=%d C5=%d\n",FBM.C3,FBM.C4,FBM.C5);
//	printf("C6=%d C7=%d C8=%d\n",FBM.C6,FBM.C7,FBM.C8);
//	printf("C9=%d C10=%d C11=%d\n",FBM.C9,FBM.C10,FBM.C11);
//	printf("C12=%d\n",FBM.C12);
}

/******************************************************************************
* ��  ����void FBM320_Calculate(int32_t UP, int32_t UT)		
* ��  �ܣ���FBM320��ȡ��ADֵת����������
* ��  ����UP ��ѹ��ADֵ UT�¶ȵ�ADֵ
* ����ֵ����
* ��  ע����ѹ��λ�������¶ȵ�λ���϶�
*******************************************************************************/
void FBM320_Calculate(int32_t UP, int32_t UT)										
{
    int32_t DT, DT2, X01, X02, X03, X11, X12, X13, X21, X22, X23, X24, X25, X26, X31, X32, CF, PP1, PP2, PP3, PP4;

    DT	=	((UT - 8388608) >> 4) + (FBM.C0 << 4);
    X01	=	(FBM.C1 + 4459) * DT >> 1;
    X02	=	((((FBM.C2 - 256) * DT) >> 14) * DT) >> 4;
    X03	=	(((((FBM.C3 * DT) >> 18) * DT) >> 18) * DT);
    FBM.RT	=	((2500 << 15) - X01 - X02 - X03) >> 15;

    DT2	=	(X01 + X02 + X03) >> 12;

    X11	=	((FBM.C5 - 4443) * DT2);
    X12	=	(((FBM.C6 * DT2) >> 16) * DT2) >> 2;
    X13	=	((X11 + X12) >> 10) + ((FBM.C4 + 120586) << 4);

    X21	=	((FBM.C8 + 7180) * DT2) >> 10;
    X22	=	(((FBM.C9 * DT2) >> 17) * DT2) >> 12;
    X23 = (X22 >= X21) ? (X22 - X21) : (X21 - X22);

    X24	=	(X23 >> 11) * (FBM.C7 + 166426);
    X25	=	((X23 & 0x7FF) * (FBM.C7 + 166426)) >> 11;
    X26 = (X21 >= X22) ? (((0 - X24 - X25) >> 11) + FBM.C7 + 166426) : (((X24 + X25) >> 11) + FBM.C7 + 166426);

    PP1	=	((UP - 8388608) - X13) >> 3;
    PP2	=	(X26 >> 11) * PP1;
    PP3	=	((X26 & 0x7FF) * PP1) >> 11;
    PP4	=	(PP2 + PP3) >> 10;

    CF	=	(2097152 + FBM.C12 * DT2) >> 3;
    X31	=	(((CF * FBM.C10) >> 17) * PP4) >> 2;
    X32	=	(((((CF * FBM.C11) >> 15) * PP4) >> 18) * PP4);
    FBM.RP	=	((X31 + X32) >> 15) + PP4 + 99880; //99880��ѹ����

}

/******************************************************************************
* ��  ����int32_t Abs_Altitude(int32_t Press)	
* ��  �ܣ���ȡ���Ը߶�
* ��  ����Press ��ѹֵ
* ����ֵ����
* ��  ע��Calculate absolute altitude, unit: mm
*******************************************************************************/
int32_t Abs_Altitude(int32_t Press)																				
{
	int8_t P0;			
	int16_t hs1, dP0;			
	int32_t h0, hs0, HP1, HP2;			
					
	if(Press >= 103000)
	{	
		P0	=	103;
		h0	=	-138507;
		hs0	=	-21007;
		hs1	=	311;
	}	
	else if(Press >= 98000)
	{	
		P0	=	98;
		h0	=	280531;
		hs0	=	-21869;
		hs1	=	338;
	}	
	else if(Press >= 93000)
	{	
		P0	=	93;
		h0	=	717253;
		hs0	=	-22813;
		hs1	=	370;
	}	
				
	else if(Press >= 88000)
	{	
		P0	=	88;
		h0	=	1173421;
		hs0	=	-23854;
		hs1	=	407;
	}	
	else if(Press >= 83000)
	{	
		P0	=	83;
		h0	=	1651084;
		hs0	=	-25007;
		hs1	=	450;
	}	
	else if(Press >= 78000)
	{	
		P0	=	78;
		h0	=	2152645;
		hs0	=	-26292;
		hs1	=	501;
	}	
	else if(Press >= 73000)
	{	
		P0	=	73;
		h0	=	2680954;
		hs0	=	-27735;
		hs1	=	560;
	}	
	else if(Press >= 68000)
	{	
		P0	=	68;
		h0	=	3239426;
		hs0	=	-29366;
		hs1	=	632;
	}	
	else if(Press >= 63000)
	{	
		P0	=	63;
		h0	=	3832204;
		hs0	=	-31229;
		hs1	=	719;
	}	
	else if(Press >= 58000)
	{	
		P0	=	58;
		h0	=	4464387;
		hs0	=	-33377;
		hs1	=	826;
	}	
	else if(Press >= 53000)
	{	
		P0	=	53;
		h0	=	5142359;
		hs0	=	-35885;
		hs1	=	960;
	}		
	else if(Press >= 48000)
	{	
		P0	=	48;
		h0	=	5874268;
		hs0	=	-38855;
		hs1	=	1131;
	}	
	else if(Press >= 43000)
	{	
		P0	=	43;
		h0	=	6670762;
		hs0	=	-42434;
		hs1	=	1354;
	}	
	else if(Press >= 38000)
	{	
		P0	=	38;
		h0	=	7546157;
		hs0	=	-46841;
		hs1	=	1654;
	}	
	else if(Press >= 33000)
	{	
		P0	=	33;
		h0	=	8520395;
		hs0	=	-52412;
		hs1	=	2072;
	}	
	else
	{	
		P0	=	28;
		h0	=	9622536;
		hs0	=	-59704;
		hs1	=	2682;
	}
					
	dP0	=	Press - P0 * 1000;
	HP1	=	(hs0 * dP0) >> 2;
	HP2	=	(((hs1 * dP0) >> 10)* dP0) >> 4;			

	return	((h0 << 6) + HP1 + HP2) >> 6;	//Return absolute altitude ���ؾ��ȸ߶�
}


