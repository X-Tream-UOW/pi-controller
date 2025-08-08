#ifndef ACQUISITION_API_H
#define ACQUISITION_API_H

void set_duration_ms(int duration_ms);
void start_acquisition(void);
void set_custom_filename(const char* user_input);
void stop_acquisition(void);

#endif

