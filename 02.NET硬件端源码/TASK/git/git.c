#include "git.h"

Data_TypeDef Data_init;						  // �豸���ݽṹ��
Threshold_Value_TypeDef threshold_value_init; // �豸��ֵ���ýṹ��
Device_Satte_Typedef device_state_init;		  // �豸״̬
DHT11_Data_TypeDef DHT11_Data;

// ��ȡ���ݲ���
mySta Read_Data(Data_TypeDef *Device_Data)
{
	
	Read_DHT11(&DHT11_Data); // ��ȡ��ʪ������
	Device_Data->temperatuer = DHT11_Data.temp_int + DHT11_Data.temp_deci * 0.01;
	Device_Data->humiditr = DHT11_Data.humi_int + DHT11_Data.humi_deci * 0.01;

	
	
	return MY_SUCCESSFUL;
}
// ��ʼ��
mySta Reset_Threshole_Value(Threshold_Value_TypeDef *Value, Device_Satte_Typedef *device_state)
{

	//Value->Distance_value = 100;
  // д
  //W_Test();
	// ��
	R_Test();
	return MY_SUCCESSFUL;
}
// ����OLED��ʾ��������
mySta Update_oled_massage()
{
#if OLED // �Ƿ��
	char str[50];
	
	if (Data_init.Device_State == 1)
	{
		sprintf(str, "ģʽ: �ֶ�    ");
		OLED_ShowCH(0, 0, (unsigned char *)str);
	}
	else if (Data_init.Device_State == 0)
	{
		sprintf(str, "ģʽ: �Զ�    ");
		OLED_ShowCH(0, 0, (unsigned char *)str);
	}
	sprintf(str, "����ǿ��: %d   ", Data_init.light);
	OLED_ShowCH(0, 2, (unsigned char *)str);
//	if(LEVEL1 == 0){
//		sprintf(str, "�Ƿ�����: �� ");
//	}else{
//		sprintf(str, "�Ƿ�����: �� ");
//	}
	sprintf(str, "T: %.1f H: %.1f  ", Data_init.temperatuer,Data_init.humiditr);
	OLED_ShowCH(0, 4, (unsigned char *)str);
	if(Data_init.light < 30 || Data_init.rain > 80){
		sprintf(str, "����: %d �ѹر�",device_state_init.check_device);
	}
		sprintf(str, "����: %d       ",device_state_init.check_device);
	OLED_ShowCH(0, 6, (unsigned char *)str);

#endif

	return MY_SUCCESSFUL;
}

// �����豸״̬
mySta Update_device_massage()
{

	// ����Ƿ�����
	if(LEVEL1 == 0)
	{
		BEEP =~ BEEP;
	}else{
		BEEP = 0;
	}

	return MY_SUCCESSFUL;
}

// ��ʱ��
void Automation_Close(void)
{
	Data_init.rain = rain_value(); // ��ȡ�������
	Data_init.light = Light_value(); // ��ȡ���ǿ��
	// �Զ�ģʽ
	if (Data_init.Device_State == 0)
	{
		if (Data_init.light < 35 || Data_init.rain > 80)
		{
			// ��
			if (device_state_init.check_device != 0)
			{	
				device_state_init.check_device = 0;
				Motor_Ctrl_Angle_F(180, 3);
			}
		}
		else
		{
			// ��
			if (device_state_init.check_device != 180)
			{
				device_state_init.check_device = 180;
				Motor_Ctrl_Angle_Z(180, 3);
			}
		}
		
	}else{
		// ��ֵ��ת����
		if(device_state_init.check_device_copy != device_state_init.check_device){
			// �� ��ת
			if(device_state_init.check_device > device_state_init.check_device_copy){
				Motor_Ctrl_Angle_Z(device_state_init.check_device-device_state_init.check_device_copy, 3);
			}else{
				Motor_Ctrl_Angle_F(device_state_init.check_device_copy-device_state_init.check_device , 3);
			}
			device_state_init.check_device_copy = device_state_init.check_device ;  
		}
	
	}
	if (Data_init.App)
	{
		switch (Data_init.App)
		{
		case 1:
			SendMqtt(1); // �������ݵ�APP
			break;
		case 2:
			SendData(); // �������ݵ���ƽ̨
			break;
		}
		Data_init.App = 0;
	}

}
// ��ⰴ���Ƿ���
static U8 num_on = 0;
static U8 key_old = 0;
void Check_Key_ON_OFF()
{
	U8 key;
	key = KEY_Scan(1);
	// ����һ�εļ�ֵ�Ƚ� �������ȣ������м�ֵ�ı仯����ʼ��ʱ
	if (key != 0 && num_on == 0)
	{
		key_old = key;
		num_on = 1;
	}
	if (key != 0 && num_on >= 1 && num_on <= Key_Scan_Time) // 25*10ms
	{
		num_on++; // ʱ���¼��
	}
	if (key == 0 && num_on > 0 && num_on < Key_Scan_Time) // �̰�
	{
		switch (key_old)
		{
		case KEY1_PRES:
			printf("Key1_Short\n");
			// ģʽѡ��
			if (Data_init.Device_State == 0)
			{
				Data_init.Device_State = 1;
			}
			else
			{
				Data_init.Device_State = 0;
			}
			W_Test();
			break;
		case KEY2_PRES:
			printf("Key2_Short\n");
			if (Data_init.Device_State == 1)
			{
				if (device_state_init.check_device == 180)
				{
					Motor_Ctrl_Angle_F(180, 3);
					device_state_init.check_device = 0;
				}
				else
				{
					Motor_Ctrl_Angle_Z(180, 3);
					device_state_init.check_device = 180;
				}
			}
			
			break;
		case KEY3_PRES:
			printf("Key3_Short\n");
		
			break;

		default:
			break;
		}
		num_on = 0;
	}
	else if (key == 0 && num_on >= Key_Scan_Time) // ����
	{
		switch (key_old)
		{
		case KEY1_PRES:
			printf("Key1_Long\n");

			Data_init.App = 1;

			break;
		case KEY2_PRES:
			printf("Key2_Long\n");

			break;
		case KEY3_PRES:
			printf("Key3_Long\n");

			break;
		default:
			break;
		}
		num_on = 0;
	}
}
// ����json����
mySta massage_parse_json(char *message)
{

	cJSON *cjson_test = NULL; // ���json��ʽ
	cJSON *cjson_data = NULL; // ����
	//const char *massage;
	// ������������
	u8 cjson_cmd; // ָ��,����

	/* ��������JSO���� */
	cjson_test = cJSON_Parse(message);
	if (cjson_test == NULL)
	{
		// ����ʧ��
		printf("parse fail.\n");
		return MY_FAIL;
	}

	/* ���θ���������ȡJSON���ݣ���ֵ�ԣ� */
	cjson_cmd = cJSON_GetObjectItem(cjson_test, "cmd")->valueint;
	/* ����Ƕ��json���� */
	cjson_data = cJSON_GetObjectItem(cjson_test, "data");

	switch (cjson_cmd)
	{
	case 0x01: // ��Ϣ��

		Data_init.Device_State = cJSON_GetObjectItem(cjson_data, "flage")->valueint;
		if (Connect_Net && Data_init.App == 0) {
        Data_init.App = 1;
    }
		W_Test();
		break;
	case 0x02: // ��Ϣ��
		if (Data_init.Device_State == 1)
		{
			device_state_init.check_device = cJSON_GetObjectItem(cjson_data, "window")->valueint;
			if (Connect_Net && Data_init.App == 0) {
					Data_init.App = 1;
			}
			W_Test();
		}
	
		break;
	case 0x03: // ���ݰ�
		 
		break;
	case 0x04: // ���ݰ�


		break;
	default:
		break;
	}

	/* ���JSON����(��������)���������� */
	cJSON_Delete(cjson_test);

	return MY_SUCCESSFUL;
}
