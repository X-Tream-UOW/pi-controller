#ifndef GPIO_HANDLER_H
#define GPIO_HANDLER_H

int init_gpio(void);
void cleanup_gpio(void);
void wait_for_ready(void);
void send_ack_pulse(void);
void set_acq(int value);

#endif
