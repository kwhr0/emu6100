PROGRAM = queen

.SUFFIXES: .b .bin

all: emu6100 $(PROGRAM).bin exec

emu6100:
	c++ -std=c++11 -O3 *.cpp -o emu6100

.b.bin:
	8bc -k $<

exec:
	./emu6100 $(PROGRAM).bin

clean:
	rm -f *.{bin,lst,pal} emu6100
