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
#define DMX_WHITE			2
#define DMX_WHITE_1			0
#define DMX_WHITE_2			1
#define DMX_WHITE_3			2
#define DMX_WHITE_4			3
#define DMX_WHITE_5			4

#define DMX_YELLOW			7
#define DMX_YELLOW_1		5
#define DMX_YELLOW_2		6
#define DMX_YELLOW_3		7
#define DMX_YELLOW_4		8
#define DMX_YELLOW_5		9

#define DMX_PINK			12
#define DMX_PINK_1			10
#define DMX_PINK_2			11
#define DMX_PINK_3			12
#define DMX_PINK_4			13
#define DMX_PINK_5			14

#define DMX_GREEN			17
#define DMX_GREEN_1			15
#define DMX_GREEN_2			16
#define DMX_GREEN_3			17
#define DMX_GREEN_4			18
#define DMX_GREEN_5			19

#define DMX_PEACHBLOW		22
#define DMX_PEACHBLOW_1		20
#define DMX_PEACHBLOW_2		21
#define DMX_PEACHBLOW_3		22
#define DMX_PEACHBLOW_4		23
#define DMX_PEACHBLOW_5		24

#define DMX_LIGHT_BLUE		27
#define DMX_LIGHT_BLUE_1	25
#define DMX_LIGHT_BLUE_2	26
#define DMX_LIGHT_BLUE_3	27
#define DMX_LIGHT_BLUE_4	28
#define DMX_LIGHT_BLUE_5	29

#define DMX_YELLOW_GREEN	32
#define DMX_YELLOW_GREEN_1	30
#define DMX_YELLOW_GREEN_2	31
#define DMX_YELLOW_GREEN_3	32
#define DMX_YELLOW_GREEN_4	33
#define DMX_YELLOW_GREEN_5	34

#define DMX_RED				37
#define DMX_RED_1			35
#define DMX_RED_2			36
#define DMX_RED_3			37
#define DMX_RED_4			38
#define DMX_RED_5			39

#define DMX_DARK_BLUE		42
#define DMX_DARK_BLUE_1		40
#define DMX_DARK_BLUE_2		41
#define DMX_DARK_BLUE_3		42
#define DMX_DARK_BLUE_4		43
#define DMX_DARK_BLUE_5		44


/* Color Transitions */
#define DMX_TRANSITION_WHITE_YELLOW
#define DMX_
#define DMX_
#define DMX_
#define DMX_
#define DMX_
#define DMX_



#endif /* DR_DMX_H_ */


/*
Channel     Index   Value       Function
1           0       0...255     Rotation (pan) (0° up to the maximum value of Pan range: 180°, 270° or 540°)
2           1       0...255     Inclination (tilt) (0° up to the maximum value of Tilt range: 90°, 180° or 270°)
3           2       0...255     Fine tuning of rotation (pan)
4           3       0...255     Fine tuning of inclination (tilt)
5           4       0...255     Speed of pan and tilt movement
6           5       0...255     Color wheel
                                0...4       White
                                5...9       Yellow
                                10...14     Pink
                                15...19     Green
                                20...24     Peachblow
                                25...29     Light blue
                                30...34     Yellow green
                                35...39     Red
                                40...44     Dark blue
                                45...54     Gradual transition from white to yellow
                                55...64     Gradual transition from yellow to pink
                                65...73     Gradual transition from pink to green
                                74...83     Gradual transition from green to peachblow
                                84...92     Gradual transition from peachblow to light blue
                                93...101    Gradual transition from light blue to yellow green
                                102...110   Gradual transition from yellow green to red
                                111...119   Gradual transition from red to dark blue
                                120...127   Gradual transition from dark blue to white
                                128...191   Rainbow effect clockwise, increasing speed
                                192...255   Rainbow effect counter-clockwise, increasing speed
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
