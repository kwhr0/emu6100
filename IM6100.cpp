// IM6100
// Copyright 2025 © Yasuo Kuwahara
// MIT License

#include "IM6100.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

#define C(n)		(cycle += (n))

IM6100::IM6100() {
#if IM6100_TRACE
	memset(tracebuf, 0, sizeof(tracebuf));
	tracep = tracebuf;
#endif
}

void IM6100::Reset() {
	ac = mq = 0;
	pc = 0200;
	l = ie = 0;
	halted = false;
}

IM6100::u16 IM6100::ea(u16 op) {
	u16 r, a = op & 0177;
	if (op & 0200) a |= (pc - 1) & 07600;
	if (op & 0400) {
		r = ld(a);
		if (a >= 010 && a <= 017) { st(a, ++r); C(16); }
		else C(15);
	}
	else { r = a; C(10); }
	return r;
}

void IM6100::iot(u16 op) {
	static int key;
	switch (op & 0770) {
		case 030: // keyboard
			switch (op & 7) {
				case 1: key = getchar(); if (key != EOF) pc++; break; // ksf
				case 2: ac = 0; break; // kcc
				case 6: ac = key & 07777; break; // krb
			}
			break;
		case 040: // teleprinter
			switch (op & 7) {
				case 1: pc++; break; // tsf
				case 4: case 6: putchar(ac & 0xff); break; // tpc/tls
			}
			break;
	}
	C(17);
}

void IM6100::opr(u16 op) {
	if (!(op & 0400)) { // group1
		// 1st
		if (op & 0200) ac = 0; // cla
		if (op & 0100) l = 0; // cll
		// 2nd
		if (op & 040) ac = ~ac & 07777; // cma
		if (op & 020) l = !l; // cml
		// 3rd
		if (op & 1) { u16 t = ++ac; ac = t & 07777; l ^= t >> 12 & 1; } // iac
		// 4th
		u16 t = ac;
		if (op & 010) {
			if (op & 2) { ac = (t << 11 & 04000) | (l << 10 & 02000) | (t >> 2 & 01777); l = t >> 1 & 1; } // rtr
			else { ac = (t >> 1 & 03777) | (l << 11 & 04000); l = t & 1; } // rar
		}
		else if (op & 4) {
			if (op & 2) { ac = (t << 2 & 07774) | (l << 1 & 2) | (t >> 11 & 1); l = t >> 10 & 1; } // rtl
			else { ac = (t << 1 & 07776) | (l & 1); l = t >> 11 & 1; } // ral
		}
		else if (op & 2) ac = (t << 6 & 07700) | (t >> 6 & 077); // bsw
		C(op & 016 ? 15 : 10);
	}
	else if (!(op & 1)) { // group2
		// 1st
		pc += ((op & 0100 && ac & 04000) | (op & 040 && !ac) | (op & 020 && l)) ^ (op & 010) >> 3; // spa sna szl / sma sza snl
		// 2nd
		if (op & 0200) ac = 0; // cla
		// 3rd
		//if (op & 4) ac |= switch_register; // osr
		if (op & 2) halted = true;
		C(op & 4 ? 15 : 10);
	}
	else { // group3
		// 1st
		if (op & 0200) ac = 0; // cla
		// 2nd
		if ((op & 0120) == 0120) std::swap(ac, mq); // mqa and mlq
		else if (op & 0100) ac |= mq; // mqa
		if (op & 020) mq = ac; ac = 0; // mql
		C(10);
	}
}

int IM6100::Execute(int n) {
	n >>= 1;
	cycle = 0;
	do {
#if IM6100_TRACE
		tracep->pc = pc;
		tracep->index = tracep->opn = 0;
#endif
		u16 t, d, op = imm();
		switch (op >> 9 & 7) {
			case 0: ac &= ld(ea(op)); break; // and
			case 1: t = ac + ld(ea(op)); ac = t & 07777; l ^= t >> 12 & 1; break; // tad
			case 2: t = ea(op); d = ld(t) + 1 & 07777; st(t, d); pc += !d; break; // isz
			case 3: st(ea(op), ac); ac = 0; break; // dca
			case 4: t = ea(op); st(t, pc); pc = t + 1; break; // jms
			case 5: pc = ea(op); break; // jmp
			case 6: iot(op); break;
			case 7: opr(op); break;
		}
#if IM6100_TRACE
		tracep->ac = ac;
		tracep->l = l;
#if IM6100_TRACE > 1
		if (++tracep >= tracebuf + TRACEMAX - 1) StopTrace();
#else
		if (++tracep >= tracebuf + TRACEMAX) tracep = tracebuf;
#endif
#endif
	} while (!halted && cycle < n);
	return halted ? 0 : (cycle - n) << 1;
}

#if IM6100_TRACE
#include <string>
void IM6100::StopTrace() {
	TraceBuffer *endp = tracep;
	int i = 0, j;
	FILE *fo;
	if (!(fo = fopen((std::string(getenv("HOME")) + "/Desktop/trace.txt").c_str(), "w"))) exit(1);
	do {
		if (++tracep >= tracebuf + TRACEMAX) tracep = tracebuf;
		fprintf(fo, "%4d %04o ", i++, tracep->pc);
		for (j = 0; j < 1; j++) fprintf(fo, j < tracep->opn ? "%04o " : "     ", tracep->op[j]);
		fprintf(fo, "%04o %d ", tracep->ac, tracep->l);
		for (Acs *p = tracep->acs; p < tracep->acs + tracep->index; p++) {
			switch (p->type) {
				case acsLoad:
					fprintf(fo, "L %04o %04o ", p->adr, p->data);
					break;
				case acsStore:
					fprintf(fo, "S %04o %04o ", p->adr, p->data);
					break;
			}
		}
		fprintf(fo, "\n");
	} while (tracep != endp);
	fclose(fo);
	fprintf(stderr, "trace dumped.\n");
	exit(1);
}
#endif	// IM6100_TRACE
