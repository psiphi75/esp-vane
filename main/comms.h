#ifndef __COMMS_H
#define __COMMS_H

void comms_init(void);
int comms_connect(void);
void publish(char *str);

#endif