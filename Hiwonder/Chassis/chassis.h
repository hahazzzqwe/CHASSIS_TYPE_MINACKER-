#ifndef __CHASSIS_H_
#define __CHASSIS_H_
#include <stdbool.h>

typedef enum {
	CHASSIS_TYPE_START = 0x00,

	CHASSIS_TYPE_MINACKER = 0x06,

	CHASSIS_TYPE_NONE = 0x07
}ChassisTypeEnum;

typedef struct {
	ChassisTypeEnum chassis_type;
	void (*set_velocity)(void *self, float vx, float vy, float angular_rate);
	void (*set_velocity_radius)(void *self, float linear, float r, bool insitu);
	void (*stop)(void *self);
} ChassisTypeDef;




#define MINACKER_WHEEL_DIAMETER 60.0 /* mm */
#define MINACKER_CORRECITION_FACTOR 1.0 /* mm */
#define MINACKER_SHAFT_LENGTH 170.0 /* mm */
#define MINACKER_WHEELBASE  180.0 /* mm */

#endif
