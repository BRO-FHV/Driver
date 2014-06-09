/*
 * Driver: dr_broadcast.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 14.05.2014
 * Description:
 * Sending all 100 ms a broadcast udp message with content "message"
 *
 * It uses port 2000.
 *
 * Uses TIMER4!
 */

#ifndef DR_BROADCAST_H_
#define DR_BROADCAST_H_

void BroadcastStart();

#endif /* DR_BROADCAST_H_ */
