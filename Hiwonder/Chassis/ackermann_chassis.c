#include "ackermann_chassis.h"
#include "math.h"
#include <stdio.h>


#define PI 3.141592654f

static inline float linear_speed_to_rps(AckermannChassisTypeDef *self,  float speed)
{
    return speed / (PI * self->wheel_diameter);
}

/*vx mm/s
  angule_rate rad/s
*/
float ackermann_velocity_difference(AckermannChassisTypeDef *self,int size,float v1){
	int angle = 0;
	if (size > 500){angle = (size-500)/(1000/240);}
	else if(size == 500){float v2=v1;return v2;}
	else {angle = (500-size)/(1000/240);}
	
	double t = tan((angle*PI)/180);
	float v_t = ((AckermannChassisTypeDef*)self)->shaft_length / t + (((AckermannChassisTypeDef*)self)->wheel_diameter/2);
	float v2 = v1*((v_t+(((AckermannChassisTypeDef*)self)->wheel_diameter/2))/(v_t-(((AckermannChassisTypeDef*)self)->wheel_diameter/2)));
	
	return v2;
}

void minacker_chassis_move(AckermannChassisTypeDef *self, float vx, float r )
{
	float vr = 0 , vl = 0;
	float angle = 0;
	if(r != 0)
	{
		angle = atan(self->shaft_length/r);
		vl = vx/r * (r - self->wheelbase/2);
		vr = vx/r * (r + self->wheelbase/2);
	}else{ //r == 0 直走
		angle = 0;
		vr = vx;
		vl = vx;
	}
	vr = linear_speed_to_rps(self , vr); //求右轮线速度
	vl = linear_speed_to_rps(self , vl); //求左轮线速度
	if(angle > PI/4.5f)
	{
		angle = PI/4.5f;
	}else if(angle < -PI/4.5f)
	{
		angle = -PI/4.5f;
	}
	angle = -2000/PI * angle + 1500; //求转向角
	self->set_motors(self , vl , vr , angle);
}

static void minacker_stop(void *self)
{
    ((AckermannChassisTypeDef*)self)->set_motors(self, 0, 0,1500);
}

static void minacker_set_velocity(void *self, float vx, float vy, float r)
{
    minacker_chassis_move(self, vx, r);
}

static void minacker_set_velocity_radius(void* self, float linear, float r,bool swerve)
{
		minacker_chassis_move(self, linear, r);
}

void ackermann_chassis_object_init(AckermannChassisTypeDef *self){
	self->base.stop = minacker_stop;
	self->base.set_velocity = minacker_set_velocity;
	self->base.set_velocity_radius = minacker_set_velocity_radius;
}

