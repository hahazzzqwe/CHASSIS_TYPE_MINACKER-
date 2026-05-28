/**
 * @file app.c
 * @author Wu TongXing
 * @brief 主应用逻辑
 * @version 0.1
 * @date 2023-11-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "cmsis_os2.h"
#include "led.h"
#include "lwmem_porting.h"
#include "global.h"
#include "adc.h"
#include "pwm_servo.h"
#include "serial_servo.h"
#include "FlashSave_porting.h"

#define PWM_SERVO_DEV_MAX  2200
#define PWM_SERVO_DEV_MIN  800

/* 蜂鸣器	的频率 */
#define BUZZER_FREQ				 1300

//底盘类型（默认为 小阿克曼底盘）
uint32_t Chassis_run_type = CHASSIS_TYPE_MINACKER;
uint8_t  save_flash = 0;
uint32_t buzzer_count = 0;

/* 硬件初始化声明 */
void buzzers_init(void);    //蜂鸣器
void buttons_init(void);	//按键
void motors_init(void);     //电机
void pwm_servos_init(void); //舵机
void chassis_init(void);    //小车参数初始化


//按键回调函数
void button_event_callback(ButtonObjectTypeDef *button,  ButtonEventIDEnum event)
{
    if(button == buttons[0]) { /* 按键1的事件  */
        if(event == BUTTON_EVENT_CLICK) {	//若为单击
			
			//底盘类型
			if( CHASSIS_TYPE_JETACKER == Chassis_run_type )
			{
				Chassis_run_type = CHASSIS_TYPE_MINACKER;
				buzzer_count = 2;
			}else{
				Chassis_run_type = CHASSIS_TYPE_JETACKER;
				buzzer_count = 1;
			}
			//重新初始化底盘
			set_chassis_type(Chassis_run_type);
			
			//进入中断级代码临界区
			uint32_t ret = taskENTER_CRITICAL_FROM_ISR(); 
			//写入Flash
			hw_flash_write(SAVE_ADD,&Chassis_run_type,1);
			//退出中断级代码临界区
			taskEXIT_CRITICAL_FROM_ISR(ret);
			//鸣响
			buzzer_didi(buzzers[0], 1800, 100, 200, buzzer_count);	//蜂鸣器响，提示底盘类型
        }
    }
	if(button == buttons[1]) { /* 按键2的事件  */
        if(event == BUTTON_EVENT_LONGPRESS) {	//若为长按
			buzzer_didi(buzzers[0], 1800, 100, 200, Chassis_run_type);	//蜂鸣器响，提示底盘类型
        }
		if(event == BUTTON_EVENT_CLICK) {	//若为长按
			
        }
    }
}


void send_type(ChassisTypeEnum chassis_type);

//大阿克曼车控制函数
void jetacker_control(void);

//小阿克曼车控制函数
void minacker_control(void);


/* 用户入口函数 */
void app_task_entry(void *argument)
{
    /* 声明外部句柄 */
    //蜂鸣器句柄
    extern osTimerId_t buzzer_timerHandle;  
	//电量监控句柄
    extern osTimerId_t battery_check_timerHandle;
	//按键控制句柄
    extern osTimerId_t button_timerHandle;

	/* Flash读取类型 */
	hw_flash_Read(SAVE_ADD,&Chassis_run_type,1);
	if( CHASSIS_TYPE_MINACKER != Chassis_run_type && 
				CHASSIS_TYPE_JETACKER != Chassis_run_type )
	{
		Chassis_run_type = CHASSIS_TYPE_MINACKER;  //设置为小阿克曼
		hw_flash_write(SAVE_ADD,&Chassis_run_type,1);
	}
	
    /* 硬件初始化 */
    motors_init();      //电机初始化
	pwm_servos_init();  //云台舵机初始化
    buzzers_init();     //蜂鸣器初始化
    buttons_init();		//按键初始化
	
	
	
	//注册按键回调函数，处理按键值
    button_register_callback(buttons[0], button_event_callback);
    button_register_callback(buttons[1], button_event_callback);

    //开启蜂鸣器定时器，让其在中断中运作，后面调用接口函数即可
    //参数1：定时器句柄 ， 参数2：定时器的工作间隔 ms
    osTimerStart(buzzer_timerHandle, BUZZER_TASK_PERIOD);   
	//开启按键定时器
	osTimerStart(button_timerHandle, BUTTON_TASK_PERIOD);
	//开启电量监控定时器，实时监控电量
    osTimerStart(battery_check_timerHandle, BATTERY_TASK_PERIOD);
	//开启ADC通道转换
	HAL_ADC_Start(&hadc1);
	
	

    //初始化底盘电机运动参数
    chassis_init();     

    //选择底盘类型
    set_chassis_type(Chassis_run_type);
	
	osDelay(2000);
	
	switch(Chassis_run_type)
	{
		case CHASSIS_TYPE_JETACKER:
			buzzer_count = 1;
			break;
		case CHASSIS_TYPE_MINACKER:
			buzzer_count = 2;
			break;
	}
	buzzer_didi(buzzers[0], 1800, 100, 200, buzzer_count);	//蜂鸣器响，提示底盘类型
	
	chassis->stop(chassis); //停止
	
	
	//底盘控制函数
	switch(Chassis_run_type)
	{
		case CHASSIS_TYPE_JETACKER: //大阿克曼
			printf("Jetacker\n");
			jetacker_control();
			break;
		case CHASSIS_TYPE_MINACKER: //小阿克曼
			printf("Minacker\n");
			minacker_control();
			break;
		default:
			break;
	}

	// 循环  : RTOS任务中的循环，必须要有osDelay或者其他系统阻塞函数，否则会导致系统异常
    for(;;) {
		
		osDelay(5000);
    
	}
}

/* 
*  大阿克曼底盘控制函数
*/
void jetacker_control(void)
{
    //定义电机运动速度
    //建议范围为 [50 , 450]
    static float speed = 300.0f;    
	
	//以x轴 speed 的线速度 运动（即向前运动）
	chassis->set_velocity(chassis, speed, 0, 0);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	chassis->set_velocity(chassis, -speed, 0, 0); //向后运动
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	//以x轴 speed 的线速度，绕自身左边的半径500mm圆做转圈运动
	chassis->set_velocity_radius(chassis, speed, 500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	//以x轴 speed 的线速度，绕自身左边的半径500mm圆做转圈运动
	chassis->set_velocity_radius(chassis, -speed, 500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	chassis->set_velocity_radius(chassis, speed, -500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	chassis->set_velocity_radius(chassis, -speed, -500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
}


/* 
*  小阿克曼底盘控制函数
*/
void minacker_control(void)
{
    //定义电机运动速度
    //建议范围为 [50 , 300]
    static float speed = 150.0f;
	
	//以x轴 speed 的线速度 运动（即向前运动）
	chassis->set_velocity(chassis, speed, 0, 0);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	chassis->set_velocity(chassis, -speed, 0, 0); //向后运动
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	//以x轴 speed 的线速度，绕自身左边的半径500mm圆做转圈运动
	chassis->set_velocity_radius(chassis, speed, 500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	//以x轴 speed 的线速度，绕自身左边的半径500mm圆做转圈运动
	chassis->set_velocity_radius(chassis, -speed, 500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	chassis->set_velocity_radius(chassis, speed, -500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
	
	chassis->set_velocity_radius(chassis, -speed, -500, true);
	osDelay(2000); //延时2s
	
	chassis->stop(chassis); //停止
	osDelay(1000); //延时1s
}

