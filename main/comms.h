#ifndef __COMMS_H
#define __COMMS_H

void comms_init(void);
int comms_connect(void);
int publish(char *str);

#endif