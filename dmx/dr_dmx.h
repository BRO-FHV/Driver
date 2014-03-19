/*
 * Driver: dr_dmx.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 19.03.2014
 * Description: 
 * TODO
 */

#ifndef DR_DMX_H_
#define DR_DMX_H_

/* Channels */
#define DMX_CHANNEL_1 		1
#define DMX_CHANNEL_2 		2
#define DMX_CHANNEL_3 		3
#define DMX_CHANNEL_4 		4
#define DMX_CHANNEL_5 		5
#define DMX_CHANNEL_6 		6
#define DMX_CHANNEL_7 		7
#define DMX_CHANNEL_8 		8
#define DMX_CHANNEL_9 		9
#define DMX_CHANNEL_10 		10
#define DMX_CHANNEL_11 		11
#define DMX_CHANNEL_12 		12

/*  Rotation for accepts percentage values */
#define DMX_ROTATION(x) 							((int)(255.0/100*x))

/*  Inclination for Channel */
#define DMX_INCLINATION(x) 							((int)(255.0/100*x))

/*  Fine tuning rotation for Channel 3  */
#define DMX_MINOR_ROTATION(x) 						((int)(255.0/100*x))

/*  Fine tuning inclination for Channel 4  */
#define DMX_MINOR_INCLINATION(x)					((int)(255.0/100*x))

/*  speed of movments for Channel 5 */
#define DMX_MOVEMENT_SPEED(x) 						((int)(255.0/100*x))

/* Colors for Channel 6*/
typedef enum dmx_colors {
	DMX_COLORS_WHITE = 2,
	DMX_COLORS_WHITE_1 = 0,
	DMX_COLORS_WHITE_2 = 1,
	DMX_COLORS_WHITE_3 = 2,
	DMX_COLORS_WHITE_4 = 3,
	DMX_COLORS_WHITE_5 = 4,

	DMX_COLORS_YELLOW = 7,
	DMX_COLORS_YELLOW_1 = 5,
	DMX_COLORS_YELLOW_2 = 6,
	DMX_COLORS_YELLOW_3 = 7,
	DMX_COLORS_YELLOW_4 = 8,
	DMX_COLORS_YELLOW_5 = 9,

	DMX_COLORS_PINK = 12,
	DMX_COLORS_PINK_1 = 10,
	DMX_COLORS_PINK_2 = 11,
	DMX_COLORS_PINK_3 = 12,
	DMX_COLORS_PINK_4 = 13,
	DMX_COLORS_PINK_5 = 14,

	DMX_COLORS_GREEN = 17,
	DMX_COLORS_GREEN_1 = 15,
	DMX_COLORS_GREEN_2 = 16,
	DMX_COLORS_GREEN_3 = 17,
	DMX_COLORS_GREEN_4 = 18,
	DMX_COLORS_GREEN_5 = 19,

	DMX_COLORS_PEACHBLOW = 22,
	DMX_COLORS_PEACHBLOW_1 = 20,
	DMX_COLORS_PEACHBLOW_2 = 21,
	DMX_COLORS_PEACHBLOW_3 = 22,
	DMX_COLORS_PEACHBLOW_4 = 23,
	DMX_COLORS_PEACHBLOW_5 = 24,

	DMX_COLORS_LIGHT_BLUE = 27,
	DMX_COLORS_LIGHT_BLUE_1 = 25,
	DMX_COLORS_LIGHT_BLUE_2 = 26,
	DMX_COLORS_LIGHT_BLUE_3 = 27,
	DMX_COLORS_LIGHT_BLUE_4 = 28,
	DMX_COLORS_LIGHT_BLUE_5 = 29,

	DMX_COLORS_YELLOW_GREEN = 32,
	DMX_COLORS_YELLOW_GREEN_1 = 30,
	DMX_COLORS_YELLOW_GREEN_2 = 31,
	DMX_COLORS_YELLOW_GREEN_3 = 32,
	DMX_COLORS_YELLOW_GREEN_4 = 33,
	DMX_COLORS_YELLOW_GREEN_5 = 34,

	DMX_COLORS_RED = 37,
	DMX_COLORS_RED_1 = 35,
	DMX_COLORS_RED_2 = 36,
	DMX_COLORS_RED_3 = 37,
	DMX_COLORS_RED_4 = 38,
	DMX_COLORS_RED_5 = 39,

	DMX_COLORS_DARK_BLUE = 42,
	DMX_COLORS_DARK_BLUE_1 = 40,
	DMX_COLORS_DARK_BLUE_2 = 41,
	DMX_COLORS_DARK_BLUE_3 = 42,
	DMX_COLORS_DARK_BLUE_4 = 43,
	DMX_COLORS_DARK_BLUE_5 = 44
} DMX_COLORS;

/* Transitions for Channel 6*/
#define DMX_TRANSITION_WHITE_YELLOW(x) 				((int)(45+(9.0/100*x)),
#define DMX_TRANSITION_YELLOW_PINK(x) 				((int)(55+(9.0/100*x)),
#define DMX_TRANSITION_PINK_GREEN(x) 				((int)(65+(9.0/100*x)),
#define DMX_TRANSITION_GREEN_PEACHBLOW(x) 			((int)(74+(9.0/100*x)),
#define DMX_TRANSITION_PEACHBLOW_LIGHT_BLUE(x) 		((int)(84+(9.0/100*x)),
#define DMX_TRANSITION_LIGHT_BLUE_YELLOW_GREEN(x) 	((int)(93+(9.0/100*x)),
#define DMX_TRANSITION_YELLOW_GREEN_RED(x) 			((int)(102+(9.0/100*x)),
#define DMX_TRANSITION_RED_DARK_BLUE(x) 			((int)(111+(9.0/100*x)),
#define DMX_TRANSITION_DARK_BLUE_WHITE(x) 			((int)(120+(9.0/100*x))

/*  Rainboweffect for Channel 6*/
#define DMX_RAINBOW_CLOCKWISE(x) 					((int)(128+(63.0/100*x))
#define	DMX_RAINBOW_COUNTER_CLOCKWISE(x) 			((int)(192+(63.0/100*x))

/*  Shutter for Channel 7*/
#define DMX_SHUTTER_CLOSED 							((int)((3.0/100*x))
#define DMX_SHUTTER_OPEN							((int)(4+(3.0/100*x))
#define DMX_SHUTTER_FLASH(x) 						((int)(8+(207.0/100*x))

/*  Dimmer for Channel 8  */
#define DMX_DIMMER(x) ((int)(255.0/100*x))

/* Gobo cycle for Channel 9 */
#define DMX_GOBO_OPEN								((int)((3.0/100*x))

#define DMX_GOBO_2(x)								((int)(8+(7.0/100*x))
#define DMX_GOBO_3(x)								((int)(16+(7.0/100*x))
#define DMX_GOBO_4(x)								((int)(24+(7.0/100*x))
#define DMX_GOBO_5(x) 								((int)(32+(7.0/100*x))
#define DMX_GOBO_6(x) 								((int)(40+(7.0/100*x))
#define DMX_GOBO_7(x) 								((int)(48+(7.0/100*x))
#define DMX_GOBO_8(x) 								((int)(56+(7.0/100*x))

#define DMX_GOBO_8_SHAKE(x)							((int)(64+(7.0/100*x))
#define DMX_GOBO_7_SHAKE(x)							((int)(72+(7.0/100*x))
#define DMX_GOBO_6_SHAKE(x)							((int)(80+(7.0/100*x))
#define DMX_GOBO_5_SHAKE(x)							((int)(88+(7.0/100*x))
#define DMX_GOBO_4_SHAKE(x)							((int)(96+(7.0/100*x))
#define DMX_GOBO_3_SHAKE(x)							((int)(104+(7.0/100*x))
#define DMX_GOBO_2_SHAKE(x)							((int)(112+(7.0/100*x))

#define DMX_GOBO_RAINBOW_CLOCKWISE(x)				((int)(128+(7.0/100*x))
#define DMX_GOBO_RAINBOW_COUNTER_CLOCKWISE(x)		((int)(192+(7.0/100*x))

/* Gobo rotation for Channel 10 */
#define	DMX_GOBO_ROTATION_POSITION					((int)(63.0/100*x)
#define	DMX_GOBO_ROTATION_CLOCKWISE					((int)(64+(83.0/100*x))
#define	DMX_GOBO_ROTATION_COUNTER_CLOCKWISE			((int)(148+(83.0/100*x))
#define	DMX_GOBO_ROTATION_JOJO						((int)(232+(23.0/100*x))

/* special functions Channel 11*/
#define DMX_RESET_COLOR_CYLCE						((int)(96+(7.0/100*x))
#define DMX_RESET_POSITION							((int)(112+(7.0/100*x))
#define DMX_RESET_GOBO								((int)(120+(7.0/100*x))
#define DMX_RESET_GOBO_MOVEMENT						((int)(128+(7.0/100*x))
#define DMX_RESET_CHANNELS							((int)(152+(7.0/100*x))

/* Programms for Channel 12 */
#define	DMX_PROGRAMM_1								((int)(8+(15.0/100*x))
#define	DMX_PROGRAMM_2								((int)(24+(15.0/100*x))
#define	DMX_PROGRAMM_3								((int)(40+(15.0/100*x))
#define	DMX_PROGRAMM_4								((int)(56+(15.0/100*x))
#define	DMX_PROGRAMM_5								((int)(72+(15.0/100*x))
#define	DMX_PROGRAMM_6								((int)(88+(15.0/100*x))

#endif /* DR_DMX_H_ */
