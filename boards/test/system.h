#ifndef __SYSTEM_INC__
#define __SYSTEM_INC__

void devices_event_post(int dev, int which, int event);
void systick_event_post(void);

#endif /* __SYSTEM_INC__ */
