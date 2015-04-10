#ifndef TIMER_H
#define TIMER_H

#define DEFAULT_TIMESLICE 10

int TIMESLICE = DEFAULT_TIMESLICE;
int timer_enabled = 1;
int time_counter;

void disableTimer() {
	timer_enabled = 0;
}

void enableTimer() {
	timer_enabled = 1;
}

int isTimerEnabled() {
	return timer_enabled;
}

void setTimeSlice(int timeSlice) {
	TIMESLICE = timeSlice;
}

int isTimerCountZero() {
	return (time_counter == 0);
}

void timerTick() {
	if (time_counter > 0) time_counter--;
}

void resetTimer() {
	time_counter = TIMESLICE;
}

#endif
