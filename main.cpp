#include "IM6100.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static IM6100 cpu;
static uint16_t m[0x1000];

struct TimeSpec : timespec {
	TimeSpec() {}
	TimeSpec(double t) {
		tv_sec = floor(t);
		tv_nsec = long(1e9 * (t - tv_sec));
	}
	TimeSpec(time_t s, long n) {
		tv_sec = s;
		tv_nsec = n;
	}
	TimeSpec operator+(const TimeSpec &t) const {
		time_t s = tv_sec + t.tv_sec;
		long n = tv_nsec + t.tv_nsec;
		if (n < 0) {
			s++;
			n -= 1000000000;
		}
		return TimeSpec(s, n);
	}
	TimeSpec operator-(const TimeSpec &t) const {
		time_t s = tv_sec - t.tv_sec;
		long n = tv_nsec - t.tv_nsec;
		if (n < 0) {
			s--;
			n += 1000000000;
		}
		return TimeSpec(s, n);
	}
};

int main(int argc,char *argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: emu6100 [-c <clock [MHz]>] <binary file>\n");
		return 0;
	}
	int mhz = 0;
	for (int c; (c = getopt(argc, argv, "c:")) != -1;) {
		switch (c) {
			case 'c':
				sscanf(optarg, "%d", &mhz);
				break;
		}
	}
	FILE *fi = fopen(argv[optind], "rb");
	if (!fi) {
		fprintf(stderr, "Cannot open %s.\n", argv[optind]);
		return 1;
	}
	int ofs = 0, c0, c1;
	fseek(fi, 239, SEEK_SET);
	while ((c0 = getc(fi)) != EOF && (c1 = getc(fi)) != EOF) {
		if (c0 & 0200) break;
		if (c0 & 0100) ofs = (c0 << 6 | c1) & 0xfff;
		else if (ofs < 0x1000) m[ofs++] = c0 << 6 | c1;
	}
	fclose(fi);
//
	termios term;
	tcgetattr(0, &term);
	term.c_lflag &= ~(ECHO | ICANON); // non-echo, non-canonical
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &term);
	fcntl(0, F_SETFL, O_NONBLOCK);
//
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	(uint32_t &)m[07770] = (uint32_t)ts.tv_sec; // for srand
	cpu.SetMemoryPtr(m);
	cpu.Reset();
	do {
		if (mhz) {
			const int SLICE = 100;
			TimeSpec tstart, tend;
			clock_gettime(CLOCK_MONOTONIC, &tstart);
			cpu.Execute(1000000L * mhz / SLICE);
			clock_gettime(CLOCK_MONOTONIC, &tend);
			TimeSpec duration = tstart + TimeSpec(1. / SLICE) - tend;
			if (duration.tv_sec >= 0)
				nanosleep(&duration, NULL);
		}
		else cpu.Execute(1000000);
	} while (!cpu.Halted());
	return 0;
}
