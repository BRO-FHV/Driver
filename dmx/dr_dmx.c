/*
 * Driver: dr_dmx.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 19.03.2014
 * Description: 
 * TODO
 */

#include <stdio.h>
#include "dr_dmx.h"


void foo(DMX_COLORS color, DMX_TRANSITIONS trans, DMX_RAINBOW rainbow, DMX_SHUTTER shutter){
	printf("color: %d", color);
	printf("trans: %d", trans);
	printf("rainbow: %d", rainbow);
	printf("shutter: %d", shutter);
}

void main(){
	foo(DMX_COLORS_GREEN_5, DMX_TRANSITION_DARK_BLUE_WHITE, DMX_RAINBOW_CLOCKWISE, DMX_SHUTTER_CLOSED_2);
}

