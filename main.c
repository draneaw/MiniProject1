/*
 * main.c
 *
 *  Created on: Sep 12, 2012
 *      Author: alex
 */
#include "BoneHeader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include "i2c-dev.h"

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */


/****************************************************************
 * signal_handler copied from gpio-int-test.c
 ****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
int keepRunning = 1;

void signal_handler(int sig)
{
        printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
        keepRunning = 0;
}

int main(int argc, char **argv)
{
	int blinkSpeed;
	char * codeName;
	int ainInput, adcInput, rc, len;
	struct pollfd fdset[2];
	char *buf[MAX_BUF];
	int input_fd,output_fd, inputButton_fd, startOverButton_fd, timeout;

	signal(SIGINT,signal_handler);

	if (argc < 3) {
		printf("Usage: BlinkSpeed in Sec <blinkSpeed> , Morse Code Name <codeName>\n\n");
		printf("Waits for a change in the GPIO pin voltage level or input on stdin\n");
		exit(-1);
	}
	blinkSpeed = atoi(argv[1]);
	codeName = (char*) malloc (strlen(argv[2]));
	codeName = argv[2];

	//Output Light - Output bitstream response
	export_gpio(7);
	set_gpio_direction(7,"out");
	set_gpio_edge(7, "falling");
	output_fd = gpio_fd_open(7);
	//Waiting for input Light
	export_gpio(49);
	set_gpio_direction(49,"out");
	set_gpio_edge(49, "falling");
	input_fd = gpio_fd_open(49);
	//Alert Light Blink at alert speed ISSUE with PWM.
	//export_gpio(115);
	set_mux_value("gpmc_a2", 6);

	//Input Button
	export_gpio(117);
	set_gpio_direction(117,"in");
	set_gpio_edge(117, "falling");
	inputButton_fd = gpio_fd_open(117);

	//Start over Button.
	export_gpio(48);
	set_gpio_direction(48,"in");
	set_gpio_edge(48, "falling");
	startOverButton_fd = gpio_fd_open(48);
	//Ain1 Temp Control input
	// read_ain("ain1");
	//set_gpio_value(7, 1);
	timeout = POLL_TIMEOUT;
	while(keepRunning){

	    memset((void*)fdset, 0, sizeof(fdset));

	    fdset[0].fd = STDIN_FILENO;
	    fdset[0].events = POLLIN;

	    fdset[1].fd = inputButton_fd;
	    fdset[1].events = POLLPRI;

	    rc = poll(fdset, 2, timeout);

		set_gpio_value(49, 1);
		//Get Analog to Compare
		ainInput = read_ain("ain1");
		//Get I2C to Compare
		adcInput = i2cRead(3, 0x48);
		//Comparing values to activate the alarm.
		printf("\nain input: %d\n", ainInput);
		printf("I2C input: %d", adcInput);
		if((ainInput/4094)<(adcInput/38)){
			//set_pwm("ehrpwm.1:0",10, 25);
			printf("Set PWM");
		}
		else{
			//unset_pwm("ehrpwm.1:0");
			printf("Unset PWM");
		}
		morseCodeLight(codeName);
		if (fdset[1].revents & POLLPRI) {
			 lseek(fdset[1].fd, 0, SEEK_SET);  // Read from the star$
		     len = read(fdset[1].fd, buf, MAX_BUF);
		   //  printf("\npoll() GPIO %d interrupt occurred, value=%c", $gpio, buf[0], len);
		}

		if (fdset[0].revents & POLLIN) {
		     (void)read(fdset[0].fd, buf, 1);
		     //printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) $);
		}
	}
	return 0;
}

int i2cRead(int bus, int devAddress){
	// The following code is based on the myi2cget.c provided in the
	// git repository.

	char filename[20];
	int size, file, output;

	size = 1;
	sprintf(filename, "/dev/i2c-%d",bus);
	file = open(filename, O_RDWR);

	if (file<0) {
		if (errno == ENOENT) {
			fprintf(stderr, "Error: Could not open file "
				"/dev/i2c-%d: %s\n", bus, strerror(ENOENT));
		} else {
			fprintf(stderr, "Error: Could not open file "
				"`%s': %s\n", filename, strerror(errno));
			if (errno == EACCES)
				fprintf(stderr, "Run as root?\n");
		}
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, devAddress) < 0) {
		fprintf(stderr,
			"Error: Could not set address to 0x%02x: %s\n",
			devAddress, strerror(errno));
		return -errno;
	}

	output = i2c_smbus_read_byte_data(file, devAddress);
	close(file);

	if (output < 0) {
		fprintf(stderr, "Error: Read failed, res=%d\n", output);
		exit(2);
	}

	return output;
}

int morseCodeLight(char * word){
	int length;
	length = strlen(word);
	int i;
	for(i=0;i<length; i++){
		switch(word[i]){
		case 'a':
			set_gpio_value(7, 1);
			usleep(250000);
			set_gpio_value(7, 0);
			usleep(250000);
			set_gpio_value(7, 1);
			usleep(750000);
			break;
		case 'b':
			set_gpio_value(7, 1);
			usleep(750000);
			set_gpio_value(7, 0);
			usleep(250000);
			set_gpio_value(7, 1);
			usleep(250000);
			set_gpio_value(7, 0);
			usleep(250000);
			set_gpio_value(7, 1);
			usleep(250000);
			set_gpio_value(7, 0);
			usleep(250000);
			set_gpio_value(7, 1);
			usleep(250000);
			set_gpio_value(7, 0);
			usleep(250000);
			break;
		case 'c':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'd':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'e':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'f':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'g':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'h':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'i':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'j':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'k':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'l':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'm':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'n':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'o':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'p':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'q':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'r':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 's':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 't':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'u':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'v':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'w':
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'x':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'y':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case 'z':
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(750000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					set_gpio_value(7, 1);
					usleep(250000);
					set_gpio_value(7, 0);
					usleep(250000);
					break;
		case ' ':
					set_gpio_value(7, 0);
					sleep(1);
					break;
		}
		set_gpio_value(7,0);
		usleep(75000);

	}
	return 1;
}

