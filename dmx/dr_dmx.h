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
typedef enum dmx_transitions {
	 DMX_TRANSITION_WHITE_YELLOW = 50,
	 DMX_TRANSITION_YELLOW_PINK = 60,
	 DMX_TRANSITION_PINK_GREEN = 69,
	 DMX_TRANSITION_GREEN_PEACHBLOW = 79,
	 DMX_TRANSITION_PEACHBLOW_LIGHT_BLUE = 88,
	 DMX_TRANSITION_LIGHT_BLUE_YELLOW_GREEN = 97,
	 DMX_TRANSITION_YELLOW_GREEN_RED = 106,
	 DMX_TRANSITION_RED_DARK_BLUE = 115,
	 DMX_TRANSITION_DARK_BLUE_WHITE = 123
} DMX_TRANSITIONS;

/*  Rainboweffect for Channel 6*/
typedef enum dmx_rainbow {
	DMX_RAINBOW_CLOCKWISE = 159,
	DMX_RAINBOW_COUNTER_CLOCKWISE = 223
} DMX_RAINBOW;

typedef enum dmx_shutter {
	DMX_SHUTTER_CLOSED = 1,
	DMX_SHUTTER_CLOSED_0 = 0,
	DMX_SHUTTER_CLOSED_1 = 1,
	DMX_SHUTTER_CLOSED_2 = 2,
	DMX_SHUTTER_CLOSED_3 = 3,

	DMX_SHUTTER_OPEN = 5,
	DMX_SHUTTER_OPEN_1 = 4,
	DMX_SHUTTER_OPEN_2 = 5,
	DMX_SHUTTER_OPEN_3 = 6,
	DMX_SHUTTER_OPEN_4 = 7,

	DMX_SHUTTER_FLASH = 125,
	DMX_SHUTTER_FLASH_1 = 8,
	DMX_SHUTTER_FLASH_2 = 20,
	DMX_SHUTTER_FLASH_3 = 50,
	DMX_SHUTTER_FLASH_4 = 100,
	DMX_SHUTTER_FLASH_5 = 150,
	DMX_SHUTTER_FLASH_6 = 200,
	DMX_SHUTTER_FLASH_7 = 215
} DMX_SHUTTER;

#endif /* DR_DMX_H_ */

/*
 7           6       0...255     Shutter
 0...3       Blackout
 4...7       Open
 8...215     Strobe effect, increasing speed
 216...255   Open
 8           7       0...255     Dimmer (0 to 100%)
 9           8       0...255     Gobo wheel
 0...7       Open
 8...15      Gobo 2
 16...23     Gobo 3
 24...31     Gobo 4
 32...39     Gobo 5
 40...47     Gobo 6
 48...55     Gobo 7
 56...63     Gobo 8
 64...71     Gobo 8 shake, increasing speed
 72...79     Gobo 7 shake, increasing speed
 80...87     Gobo 6 shake, increasing speed
 88...95     Gobo 5 shake, increasing speed
 96...103    Gobo 4 shake, increasing speed
 104...111   Gobo 3 shake, increasing speed
 112...119   Gobo 2 shake, increasing speed
 120...127   Open
 128...191   Rainbow effect clockwise, increasing speed
 192...255   Rainbow effect counter-clockwise, increasing speed
 10          9       0...255     Gobo rotation
 0...63      Fixed position from 0° to 360°
 64...147    Rotation clockwise, increasing speed
 148...231   Rotation counter-clockwise, increasing speed
 232...255   Yo-yo effect (bouncing gobo) with alternating rotation direction, increasing length of rotation intervals
 11          10      0...255     Special functions
 0...7       Not in use
 8...15      Blackout during pan or tilt movement
 16...23     No blackout during pan or tilt movement
 24...31     Blackout during colour wheel movement
 32...39     No blackout during colour wheel movement
 40...47     Blackout during gobo wheel movement
 48...55     No blackout during gobo wheel movement
 56...87     Not in use
 88...95     Blackout during movement
 96...103    Pan and tilt reset
 104...111   Not in use
 112...119   Colour wheel reset
 120...127   Gobo wheel reset
 128...135   Gobo rotation reset
 136...151   Not in use
 152...159   All channel reset
 160...255   Not in use
 12          11      0...255     Built-in programmes
 0...7       Not in use
 8...23      Programme 1
 24...39     Programme 2
 40...55     Programme 3
 56...71     Programme 4
 72...87     Programme 5
 88...103    Programme 6
 104...119   Programme 7
 120...135   Programme 8
 136...151   Sound-control 1
 152...167   Sound-control 2
 168...183   Sound-control 3
 184...199   Sound-control 4
 200...215   Sound-control 5
 216...231   Sound-control 6
 232...247   Sound-control 7
 248...255   Sound-control 8
 */
