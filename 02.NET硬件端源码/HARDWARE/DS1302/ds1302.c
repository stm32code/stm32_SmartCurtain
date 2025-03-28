#include "ds1302.h"
#include "delay.h"
#include <string.h>
#include "usart.h"
/*
DS1302�ӿ�:
	VCC -> 5.0/3.3
	GND -> GND
	GPIOB_12 ->DS1302_RST
	GPIOB_13 ->DS1302_DAT
	GPIOB_14 ->DS1302_CLK
*/
// DS1302��ַ����
#define DS1302_SEC_ADDR 0x80	 // �����ݵ�ַ
#define DS1302_MIN_ADDR 0x82	 // �����ݵ�ַ
#define DS1302_HOUR_ADDR 0x84	 // ʱ���ݵ�ַ
#define DS1302_DAY_ADDR 0x86	 // �����ݵ�ַ
#define DS1302_MONTH_ADDR 0x88	 // �����ݵ�ַ
#define DS1302_WEEK_ADDR 0x8a	 // �������ݵ�ַ
#define DS1302_YEAR_ADDR 0x8c	 // �����ݵ�ַ
#define DS1302_CONTROL_ADDR 0x8e // �������ݵ�ַ
#define DS1302_CHARGER_ADDR 0x90 // ��繦�ܵ�ַ
#define DS1302_CLKBURST_ADDR 0xbe

// ��ʼʱ�䶨��


u8 time_buf[8] = {0x20, 0x23, 0x12, 0x13, 0x13, 0x010, 0x00, 0x03}; // ��ʼʱ��
char *TIME_SET = "2023121622104706";
u8 time_set[8] = {0x20, 0x23, 0x12, 0x13, 0x14, 0x40, 0x00, 0x03};		   // ��ʼʱ��
char *ALARM1_SET = "1111";
char *ALARM2_SET = "2222";
u8 alarm1_set[2] = {0, 0};		   // ����1
u8 alarm2_set[2] = {0, 0};		   // ����2
u8 alarm3_set[2] = {0, 0};		   // ����3
u8 alarm4_set[2] = {0, 0};		   // ����4
u8 alarm5_set[2] = {0, 0};		   // ����5
u8 alarm6_set[2] = {0, 0};		   // ����6

u8 sec_buf = 0;	 // �뻺��
u8 sec_flag = 0; // ���־λ

void DS1302_Init() // CE,SCLK�˿ڳ�ʼ��
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(CE_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = CE_GPIO_PIN; // PC.11  CE
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // �������
	GPIO_Init(CE_GPIO_PORT, &GPIO_InitStructure);	 // ��ʼ��GPIOC.11
	GPIO_ResetBits(CE_GPIO_PORT, CE_GPIO_PIN);

	GPIO_InitStructure.GPIO_Pin = SCLK__GPIO_PIN; // PC.12  SCLK
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // �������
	GPIO_Init(SCLK__GPIO_PORT, &GPIO_InitStructure); // ��ʼ��GPIOC.12
	GPIO_ResetBits(SCLK__GPIO_PORT, SCLK__GPIO_PIN);
}
// ����˫��I/O�˿�Ϊ���̬
static void ds1032_DATAOUT_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DATA_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DATA__GPIO_PIN; // PC.10  DATA
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DATA__GPIO_PORT, &GPIO_InitStructure); // ��ʼ��GPIOC.10
	GPIO_ResetBits(DATA__GPIO_PORT, DATA__GPIO_PIN);
}
// ����˫��I/O�˿�Ϊ����̬
static void ds1032_DATAINPUT_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DATA_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = DATA__GPIO_PIN; // PC.10 DATA
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DATA__GPIO_PORT, &GPIO_InitStructure); // ��ʼ��GPIOC.10
}
// ��DS1302д��һ�ֽ�����
void DS1302_WriteByte(u8 addr, u8 data)
{
	u8 i;
	DS1302_RST = 0; // ��ֹ���ݴ��� ��������������Ҫ
	DS1302_CLK = 0; // ȷ��д����ǰSCLKΪ�͵�ƽ
	DS1302_RST = 1; // ����DS1302����
	ds1032_DATAOUT_init();
	addr = addr & 0xFE;		// ���λ���㣬�Ĵ���0λΪ0ʱд��Ϊ1ʱ��
	for (i = 0; i < 8; i++) // д��Ŀ���ַ��addr
	{
		if (addr & 0x01)
			DS1302_OUT = 1;
		else
			DS1302_OUT = 0;
		DS1302_CLK = 1; // ʱ��������д������
		DS1302_CLK = 0;
		addr = addr >> 1;
	}
	for (i = 0; i < 8; i++) // д�����ݣ�data
	{
		if (data & 0x01)
			DS1302_OUT = 1;
		else
			DS1302_OUT = 0;
		DS1302_CLK = 1; // ʱ��������д������
		DS1302_CLK = 0;
		data = data >> 1;
	}
	DS1302_CLK = 1; // ��ʱ�ӵ�ƽ���ڸߵ�ƽ״̬ ��������֪״̬
	DS1302_RST = 0; // ֹͣDS1302����
}

// ��DS1302����һ�ֽ�����
u8 DS1302_ReadByte(u8 addr)
{
	u8 i, temp;
	DS1302_RST = 0; // ��������Ҫ
	DS1302_CLK = 0; // �Ƚ�SCLK�õ͵�ƽ,ȷ��д����ǰSCLK������
	DS1302_RST = 1; // ����DS1302����
	ds1032_DATAOUT_init();
	// д��Ŀ���ַ��addr
	addr = addr | 0x01; // ���λ�øߣ��Ĵ���0λΪ0ʱд��Ϊ1ʱ��
	for (i = 0; i < 8; i++)
	{
		if (addr & 0x01)
			DS1302_OUT = 1;
		else
			DS1302_OUT = 0;
		DS1302_CLK = 1; // д����
		DS1302_CLK = 0;
		addr = addr >> 1;
	}
	// ��DS1302�������ݣ�temp
	ds1032_DATAINPUT_init();
	for (i = 0; i < 8; i++)
	{
		temp = temp >> 1;
		if (DS1302_IN)
			temp |= 0x80;
		else
			temp &= 0x7F;
		DS1302_CLK = 1;
		DS1302_CLK = 0;
	}
	DS1302_CLK = 1; // ��ʱ�ӵ�ƽ������֪״̬
	DS1302_RST = 0; // ֹͣDS1302����
	return temp;
}

// ��DS1302д��ʱ������,����ʱ��У׼�޸�
void DS1302_WriteTime()
{

	u32 outlen = 0;
	u8 buf_hex[18];

	StringToHex(TIME_SET, buf_hex, &outlen);
	memcpy(time_set, buf_hex, outlen);


	DS1302_WriteByte(DS1302_CONTROL_ADDR, 0x00); // �ر�д����
	DS1302_WriteByte(DS1302_SEC_ADDR, 0x80);	 // ��ͣʱ��
	// DS1302_WriteByte(DS1302_CHARGER_ADDR,0xa9);     //������
	DS1302_WriteByte(DS1302_YEAR_ADDR, time_set[1]);  // ��
	DS1302_WriteByte(DS1302_MONTH_ADDR, time_set[2]); // ��
	DS1302_WriteByte(DS1302_DAY_ADDR, time_set[3]);	  // ��
	DS1302_WriteByte(DS1302_HOUR_ADDR, time_set[4]);  // ʱ
	DS1302_WriteByte(DS1302_MIN_ADDR, time_set[5]);	  // ��
	DS1302_WriteByte(DS1302_SEC_ADDR, time_set[6]);	  // ��
	DS1302_WriteByte(DS1302_WEEK_ADDR, time_set[7]);  // ��
	DS1302_WriteByte(DS1302_CHARGER_ADDR, 0xA5);	  // �򿪳�繦�� ѡ��2K�����緽ʽ
	DS1302_WriteByte(DS1302_CONTROL_ADDR, 0x80);	  // ��д����
}
// ��ȡ��������
void DS1302_AlarmTime(u8 cmd)
{

	u32 outlen1,outlen2 = 0;
	u8 buf1_hex[4],buf2_hex[4];
	switch(cmd){
		case 1:
				StringToHex(ALARM1_SET, buf1_hex, &outlen1);
				memcpy(alarm1_set, buf1_hex, outlen1);
				StringToHex(ALARM2_SET, buf2_hex, &outlen2);
				memcpy(alarm2_set, buf2_hex, outlen2);
			break;
		case 2:
				StringToHex(ALARM1_SET, buf1_hex, &outlen1);
				memcpy(alarm3_set, buf1_hex, outlen1);
				StringToHex(ALARM2_SET, buf2_hex, &outlen2);
				memcpy(alarm4_set, buf2_hex, outlen2);
			break;
		case 3:
				StringToHex(ALARM1_SET, buf1_hex, &outlen1);
				memcpy(alarm5_set, buf1_hex, outlen1);
				StringToHex(ALARM2_SET, buf2_hex, &outlen2);
				memcpy(alarm6_set, buf2_hex, outlen2);
			break;
	}



}
// ��DS1302����ʱ������
void DS1302_ReadTime(void)
{
	time_buf[1] = DS1302_ReadByte(DS1302_YEAR_ADDR);		 // ��
	time_buf[2] = DS1302_ReadByte(DS1302_MONTH_ADDR);		 // ��
	time_buf[3] = DS1302_ReadByte(DS1302_DAY_ADDR);			 // ��
	time_buf[4] = DS1302_ReadByte(DS1302_HOUR_ADDR);		 // ʱ
	time_buf[5] = DS1302_ReadByte(DS1302_MIN_ADDR);			 // ��
	time_buf[6] = (DS1302_ReadByte(DS1302_SEC_ADDR)) & 0x7f; // �룬������ĵ�7λ�����ⳬ��59
	time_buf[7] = DS1302_ReadByte(DS1302_WEEK_ADDR);		 // ��
}

// ������
void DS1302_GetTime(DS1302_Time_t *time)
{
	DS1302_ReadTime();																									  // ��ȡʱ��
	time->year = (time_buf[0] >> 4) * 1000 + (time_buf[0] & 0x0F) * 100 + (time_buf[1] >> 4) * 10 + (time_buf[1] & 0x0F); // �������
	time->month = (time_buf[2] >> 4) * 10 + (time_buf[2] & 0x0F);														  // �����·�
	time->day = (time_buf[3] >> 4) * 10 + (time_buf[3] & 0x0F);															  // ��������
	time->hour = (time_buf[4] >> 4) * 10 + (time_buf[4] & 0x0F);														  // ����Сʱ
	time->minute = (time_buf[5] >> 4) * 10 + (time_buf[5] & 0x0F);														  // �������
	time->second = (time_buf[6] >> 4) * 10 + (time_buf[6] & 0x0F);														  // ��������
	time->week = (time_buf[7] & 0x0F);																					  // ��������
																														  // printf("ʱ��:%d-%d-%d %d:%d:%d %d \n",TimeData.year,TimeData.month,TimeData.day,TimeData.hour,TimeData.minute,TimeData.second,TimeData.week);
}
// ʮ�������ַ���ת����ʮ����������
u32 StringToHex(char *str, u8 *out, u32 *outlen)
{
	char *p = str;
	char high = 0, low = 0;
	int tmplen = strlen(p), cnt = 0;
	tmplen = strlen(p);
	while (cnt < (tmplen / 2))
	{
		high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
		low = (*(++p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p)-48 - 7 : *(p)-48;
		out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
		p++;
		cnt++;
	}
	if (tmplen % 2 != 0)
		out[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;

	if (outlen != NULL)
		*outlen = tmplen / 2 + tmplen % 2;
	return tmplen / 2 + tmplen % 2;
}

