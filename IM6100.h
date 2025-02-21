// IM6100
// Copyright 2025 © Yasuo Kuwahara
// MIT License

#include <cstdint>

#define IM6100_TRACE			0

#if IM6100_TRACE
#define IM6100_TRACE_LOG(adr, data, type) \
	if (tracep->index < ACSMAX) tracep->acs[tracep->index++] = { adr, (u16)data, type }
#else
#define IM6100_TRACE_LOG(adr, data, type)
#endif

class IM6100 {
	using u8 = uint8_t;
	using u16 = uint16_t;
public:
	IM6100();
	void Reset();
	void SetMemoryPtr(u16 *p) { m = p; }
	int Execute(int n);
	bool Halted() const { return halted; }
private:
	u16 imm() {
		u16 o = m[pc++];
#if IM6100_TRACE
		if (tracep->opn < 1) tracep->op[tracep->opn++] = o;
#endif
		return o;
	}
	u16 ld(u16 adr) {
		u16 data = m[adr] & 07777;
		IM6100_TRACE_LOG(adr, data, acsLoad);
		return data;
	}
	void st(u16 adr, u16 data) {
		m[adr] = data & 07777;
		IM6100_TRACE_LOG(adr, data, acsStore);
	}
	u16 ea(u16 op);
	void iot(u16 op);
	void opr(u16 op);
	u16 *m;
	u16 ac, mq, pc;
	u8 l, ie;
	int cycle;
	bool halted;
#if IM6100_TRACE
	static constexpr int TRACEMAX = 10000;
	static constexpr int ACSMAX = 3;
	enum {
		acsLoad = 1, acsStore
	};
	struct Acs {
		u16 adr, data;
		u8 type;
	};
	struct TraceBuffer {
		u16 pc, ac;
		u16 op[1];
		u8 l, index, opn;
		Acs acs[ACSMAX];
	};
	TraceBuffer tracebuf[TRACEMAX];
	TraceBuffer *tracep;
public:
	void StopTrace();
#endif
};
