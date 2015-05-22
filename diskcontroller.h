#ifndef DISKCONTROLLER_H
#define DISKCONTROLLER_H

#define DISKCONTROLLER_DEFAULT_TIMESLICE 10

int DISKCONTROLLER_TIMESLICE = DISKCONTROLLER_DEFAULT_TIMESLICE;
int diskcontroller_counter_enabled = 1;
int diskcontroller_counter;

void disableDiskControllerCounter() {
	diskcontroller_counter_enabled = 0;
}
void enableDiskControllerCounter() {
	diskcontroller_counter_enabled = 1;
}

int isDiskControllerEnabled() {
	return diskcontroller_counter;
}

void setDiskControllerTimeSlice(int timeSlice) {
	DISKCONTROLLER_TIMESLICE = timeSlice;
}

int isDiskControllerCountZero() {
	return (diskcontroller_counter == 0);
}

void diskcontrollerTick() {
	if (diskcontroller_counter > 0) diskcontroller_counter--;
}

void resetDiskcontroller() {
	enableDiskControllerCounter();
	diskcontroller_counter = DISKCONTROLLER_TIMESLICE;
}

#endif
