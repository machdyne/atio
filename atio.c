/*
 * ATIO
 * Copyright (c) 2023 Lone Dynamics Corporation <info@lonedynamics.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#ifdef SIM
#include <ncurses.h>
#else
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#endif

#define BUFLEN 32
#define PBUFLEN (size_t)16

void atio_parse(char *buf, size_t len);
void atoi_command(char *bufcmd, int num, int val, bool is_assign, bool is_query);

int main(void) {

#ifdef SIM
   initscr();
   noecho();
#else
	stdio_init_all();
	while (!stdio_usb_connected()) {
		sleep_ms(100);
	}
#endif

	printf("# atio initializing ...\r\n");

#ifndef SIM
	for (int i = 0; i <= 29; i++) gpio_init(i);
#endif

	printf("READY\r\n");

	char buf[BUFLEN];
	int bptr = 0;
	int c;

	bzero(buf, BUFLEN);

	while (1) {

#ifdef SIM
		c = getch();
#else
		c = getchar();
#endif

		if (c > 0) {
			if (c == 0x0a || c == 0x0d) {
				putchar(0x0a);
				putchar(0x0d);
				fflush(stdout);
				atio_parse(buf, BUFLEN);
				bptr = 0;
				bzero(buf, BUFLEN);
				continue;
			}
			if (c == 0x08 || c == 0x7f) {
				if (bptr) bptr--;
				buf[bptr] = '\0';
				putchar(0x08);
				putchar(' ');
				putchar(0x08);
				fflush(stdout);
				continue;
			}
			if (bptr >= BUFLEN - 1) {
				printf("# buffer overflow\r\n");
				bptr = 0;
				bzero(buf, BUFLEN);
				printf("ERROR\r\n");
				continue;
			}
			putchar(c);
			fflush(stdout);
			buf[bptr++] = c;
		}

	}

	return 0;

}

enum MODE { MODE_PREFIX, MODE_CMD, MODE_NUM, MODE_VAL };

void atio_parse(char *buf, size_t len) {

	// printf("# got: '%s'\r\n", buf);

	int mode = MODE_PREFIX;
	bool is_assign = false;
	bool is_query = false;
	bool is_error = false;

	char bufcmd[PBUFLEN];
	char bufnum[PBUFLEN];
	char bufval[PBUFLEN];

	bzero(bufcmd, PBUFLEN);
	bzero(bufnum, PBUFLEN);
	bzero(bufval, PBUFLEN);

	int pos = 0;
	int p = 0;

	while (1) {

		// printf("# pos %i len %i char %c\r\n", pos, len, buf[pos]);

		if (buf[pos] == 0x00 || pos > len - 1) break;

		if (mode == MODE_PREFIX && buf[pos] == '+') {
			mode = MODE_CMD;
			p = 0;
			pos++;
			continue;
		}

		if (mode == MODE_CMD && isdigit(buf[pos])) {
			mode = MODE_NUM; 
			p = 0;
			continue;
		}

		if (mode == MODE_NUM && buf[pos] == '=') {
			mode = MODE_VAL; 
			is_assign = true;
			p = 0;
			pos++;
			continue;
		}

		if (buf[pos] == '?') {
			is_query = true;
			break;
		}

		if (mode == MODE_CMD) bufcmd[p++] = buf[pos];
		if (mode == MODE_NUM) bufnum[p++] = buf[pos];
		if (mode == MODE_VAL) bufval[p++] = buf[pos];

		if (p > PBUFLEN - 1) {
			printf("# buffer overflow\r\n");
			printf("ERROR\r\n");
			p = 0;
			is_error = true;
			break;
		}

		pos++;

	}

	if (is_error) return;

	int num = atoi(bufnum);
	int val = atoi(bufval);

	// printf("cmd: %s\r\n", bufcmd);
	// printf("num: %i\r\n", num);
	// printf("val: %i\r\n", val);

	atoi_command(bufcmd, num, val, is_assign, is_query);

}

#ifdef SIM
void watchdog_reboot(int a , int b, int c) { return; }
void gpio_set_dir(int a, int b) { printf("# gpio_set_dir %i %i\r\n", a, b); return; }
void gpio_pull_up(int a) { printf("# gpio_pull_up %i\r\n", a); return; }
void gpio_pull_down(int a) { printf("# gpio_pull_down %i\r\n", a); return; }
void gpio_put(int a, int b) { printf("# gpio_put %i %i\r\n", a, b); return; }
int gpio_get(int a) { printf("# gpio_get %i\r\n", a); return 0; }
int gpio_disable_pulls(int a) { printf("# gpio_disable_pulls %i\r\n", a); return 0; }
#endif

void atoi_command(char *bufcmd, int num, int val, bool is_assign, bool is_query)
{

	if (!strncmp(bufcmd, "", PBUFLEN)) {
		printf("OK\r\n");
	}

	// reset RP2040
	else if (!strncmp(bufcmd, "RST", PBUFLEN)) {
		watchdog_reboot(0, 0, 0);
	}

	// set gpio direction
	else if (!strncmp(bufcmd, "GPIOD", PBUFLEN)) {
		gpio_set_dir(num, val);
		printf("OK\r\n");
	}

	// enable pull-up
	else if (!strncmp(bufcmd, "GPIOPU", PBUFLEN)) {
		gpio_pull_up(num);
		printf("OK\r\n");
	}

	// enable pull-down
	else if (!strncmp(bufcmd, "GPIOPD", PBUFLEN)) {
		gpio_pull_down(num);
		printf("OK\r\n");
	}

	// disable pull-up and pull-down
	else if (!strncmp(bufcmd, "GPIOPO", PBUFLEN)) {
		gpio_disable_pulls(num);
		printf("OK\r\n");
	}

	// set gpio value
	else if (!strncmp(bufcmd, "GPIO", PBUFLEN) && is_query == false) {
		gpio_put(num, val);
		printf("OK\r\n");
	}

	// get gpio value
	else if (!strncmp(bufcmd, "GPIO", PBUFLEN) && is_query == true) {
		int val = gpio_get(num);
		printf("+%i\r\n", val);
	}

	else {
		printf("ERROR\r\n");
	}

}
