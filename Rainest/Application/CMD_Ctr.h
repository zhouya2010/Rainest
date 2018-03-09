
#ifndef CMD_Ctr_H_
#define CMD_Ctr_H_

extern unsigned char set_adjust_flag;
extern unsigned char get_adjust_flag;
extern unsigned char get_weather_flag;
extern unsigned char set_raindelay_flag;
extern unsigned char set_sch_flag;
extern unsigned char net_connect_flag ;	//网络连接状态： 0-未连接或连接中断， 1-连接上
extern unsigned char user_band_flag;//用户是否绑定标志 0-未绑定， 1-设备关联成功
extern unsigned char conn_cloud_flag ;//是否连接云端成功标志 0-未连接， 1-连接云端成功
extern unsigned char send_devcodes_flag ;//是否上传设备序列码给云端成功，0-未上传  1-上传成功
extern unsigned char get_param_flag ;//是否获取参数 0-未取得，1-获取成功
extern unsigned char get_time_flag;	//是否获取到云端时间，0-未取得，1-获取成功
extern unsigned char sentwater_flag;//是否有用水量要上传到云端，1-需要上传，0-没有要上传的用水量
extern unsigned char get_domain_flag ;//轮询域名标志 1-获取成功 0-未获取到
extern unsigned char send_work_value_flag; //发送旋钮选择模式标志 0-需要更新 1-不更新
extern unsigned char send_spary_status; //发送当前喷淋路数状态标志 0-需要更新 1-不更新
extern unsigned char sent_sensor_flag;//发送传感器标志  0-不上传  1-上传
extern unsigned char set_del_zone_flag;
extern unsigned char get_schedule_flag;
extern unsigned char sent_zone_time_flag;
extern unsigned char sent_sensor_state;
extern unsigned char get_devstatus_flag;
extern unsigned char return_cid_flag;
extern unsigned char sent_master_state;
extern unsigned char sent_device_type_flag;
extern unsigned char sent_zone_num_flag;
extern unsigned char check_command_flag;  //16.8.17
extern unsigned char return_executed_flag; //16.8.19
//extern const char  mainserve[];

void cmd_ctr(void);//命令流程控制
//void set_mainserve(void);

#endif /* CMD_Ctr_H_ */