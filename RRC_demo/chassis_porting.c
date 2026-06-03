#include "global.h"
#include "ackermann_chassis.h"
#include "encoder_motor.h"
#include "chassis.h"
#include "motors_param.h"

AckermannChassisTypeDef   minacker;

static void minacker_set_motors(void* self, float rps_lh, float rps_lt,int position)
{
	static int position_last = 0;
	if( position_last != position )
	{
		pwm_servo_set_position(pwm_servos[0] , position , 100);
		position_last = position;
	}
    encoder_motor_set_speed(motors[1], -rps_lh);
    encoder_motor_set_speed(motors[0], rps_lt);
}

ChassisTypeDef *chassis = (ChassisTypeDef*)&minacker;

void chassis_init(void)
{
    minacker.base.chassis_type = CHASSIS_TYPE_MINACKER;
    minacker.correction_factor = MINACKER_CORRECITION_FACTOR;
    minacker.wheel_diameter = MINACKER_WHEEL_DIAMETER;
    minacker.shaft_length = MINACKER_SHAFT_LENGTH;
    minacker.wheelbase = MINACKER_WHEELBASE;
    minacker.set_motors = minacker_set_motors;
    ackermann_chassis_object_init(&minacker);
}

void set_chassis_type(uint8_t chassis_type)
{
    (void)chassis_type;
    chassis = (ChassisTypeDef*)&minacker;
    set_motor_type(motors[0], MOTOR_TYPE_JGB520);
    set_motor_type(motors[1], MOTOR_TYPE_JGB520);
    set_motor_type(motors[2], MOTOR_TYPE_JGB520);
    set_motor_type(motors[3], MOTOR_TYPE_JGB520);
}
