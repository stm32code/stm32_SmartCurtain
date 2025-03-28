#include "git.h"

Data_TypeDef Data_init;						  // 设备数据结构体
Threshold_Value_TypeDef threshold_value_init; // 设备阈值设置结构体
Device_Satte_Typedef device_state_init;		  // 设备状态
DHT11_Data_TypeDef DHT11_Data;

// 获取数据参数
mySta Read_Data(Data_TypeDef *Device_Data)
{
	
	Read_DHT11(&DHT11_Data); // 获取温湿度数据
	Device_Data->temperatuer = DHT11_Data.temp_int + DHT11_Data.temp_deci * 0.01;
	Device_Data->humiditr = DHT11_Data.humi_int + DHT11_Data.humi_deci * 0.01;

	
	
	return MY_SUCCESSFUL;
}
// 初始化
mySta Reset_Threshole_Value(Threshold_Value_TypeDef *Value, Device_Satte_Typedef *device_state)
{

	//Value->Distance_value = 100;
  // 写
  //W_Test();
	// 读
	R_Test();
	return MY_SUCCESSFUL;
}
// 更新OLED显示屏中内容
mySta Update_oled_massage()
{
#if OLED // 是否打开
	char str[50];
	
	if (Data_init.Device_State == 1)
	{
		sprintf(str, "模式: 手动    ");
		OLED_ShowCH(0, 0, (unsigned char *)str);
	}
	else if (Data_init.Device_State == 0)
	{
		sprintf(str, "模式: 自动    ");
		OLED_ShowCH(0, 0, (unsigned char *)str);
	}
	sprintf(str, "光照强度: %d   ", Data_init.light);
	OLED_ShowCH(0, 2, (unsigned char *)str);
//	if(LEVEL1 == 0){
//		sprintf(str, "是否有人: 有 ");
//	}else{
//		sprintf(str, "是否有人: 无 ");
//	}
	sprintf(str, "T: %.1f H: %.1f  ", Data_init.temperatuer,Data_init.humiditr);
	OLED_ShowCH(0, 4, (unsigned char *)str);
	if(Data_init.light < 30 || Data_init.rain > 80){
		sprintf(str, "窗帘: %d 已关闭",device_state_init.check_device);
	}
		sprintf(str, "窗帘: %d       ",device_state_init.check_device);
	OLED_ShowCH(0, 6, (unsigned char *)str);

#endif

	return MY_SUCCESSFUL;
}

// 更新设备状态
mySta Update_device_massage()
{

	// 监测是否有人
	if(LEVEL1 == 0)
	{
		BEEP =~ BEEP;
	}else{
		BEEP = 0;
	}

	return MY_SUCCESSFUL;
}

// 定时器
void Automation_Close(void)
{
	Data_init.rain = rain_value(); // 获取雨滴数据
	Data_init.light = Light_value(); // 获取光感强度
	// 自动模式
	if (Data_init.Device_State == 0)
	{
		if (Data_init.light < 35 || Data_init.rain > 80)
		{
			// 关
			if (device_state_init.check_device != 0)
			{	
				device_state_init.check_device = 0;
				Motor_Ctrl_Angle_F(180, 3);
			}
		}
		else
		{
			// 开
			if (device_state_init.check_device != 180)
			{
				device_state_init.check_device = 180;
				Motor_Ctrl_Angle_Z(180, 3);
			}
		}
		
	}else{
		// 赋值旋转度数
		if(device_state_init.check_device_copy != device_state_init.check_device){
			// 大 正转
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
			SendMqtt(1); // 发送数据到APP
			break;
		case 2:
			SendData(); // 发送数据到云平台
			break;
		}
		Data_init.App = 0;
	}

}
// 检测按键是否按下
static U8 num_on = 0;
static U8 key_old = 0;
void Check_Key_ON_OFF()
{
	U8 key;
	key = KEY_Scan(1);
	// 与上一次的键值比较 如果不相等，表明有键值的变化，开始计时
	if (key != 0 && num_on == 0)
	{
		key_old = key;
		num_on = 1;
	}
	if (key != 0 && num_on >= 1 && num_on <= Key_Scan_Time) // 25*10ms
	{
		num_on++; // 时间记录器
	}
	if (key == 0 && num_on > 0 && num_on < Key_Scan_Time) // 短按
	{
		switch (key_old)
		{
		case KEY1_PRES:
			printf("Key1_Short\n");
			// 模式选择
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
	else if (key == 0 && num_on >= Key_Scan_Time) // 长按
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
// 解析json数据
mySta massage_parse_json(char *message)
{

	cJSON *cjson_test = NULL; // 检测json格式
	cJSON *cjson_data = NULL; // 数据
	//const char *massage;
	// 定义数据类型
	u8 cjson_cmd; // 指令,方向

	/* 解析整段JSO数据 */
	cjson_test = cJSON_Parse(message);
	if (cjson_test == NULL)
	{
		// 解析失败
		printf("parse fail.\n");
		return MY_FAIL;
	}

	/* 依次根据名称提取JSON数据（键值对） */
	cjson_cmd = cJSON_GetObjectItem(cjson_test, "cmd")->valueint;
	/* 解析嵌套json数据 */
	cjson_data = cJSON_GetObjectItem(cjson_test, "data");

	switch (cjson_cmd)
	{
	case 0x01: // 消息包

		Data_init.Device_State = cJSON_GetObjectItem(cjson_data, "flage")->valueint;
		if (Connect_Net && Data_init.App == 0) {
        Data_init.App = 1;
    }
		W_Test();
		break;
	case 0x02: // 消息包
		if (Data_init.Device_State == 1)
		{
			device_state_init.check_device = cJSON_GetObjectItem(cjson_data, "window")->valueint;
			if (Connect_Net && Data_init.App == 0) {
					Data_init.App = 1;
			}
			W_Test();
		}
	
		break;
	case 0x03: // 数据包
		 
		break;
	case 0x04: // 数据包


		break;
	default:
		break;
	}

	/* 清空JSON对象(整条链表)的所有数据 */
	cJSON_Delete(cjson_test);

	return MY_SUCCESSFUL;
}
