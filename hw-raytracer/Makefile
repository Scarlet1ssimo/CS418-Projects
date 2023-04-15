.PHONEY: build, run, clean, cleanresult, todo, see%, run%, comp%, all, done, gdb, programdbg
SRCS = $(wildcard test/*.txt)
DONES=$(wildcard result/*.png)
CFLAGS = -O3 -I. -std=c++17
all:	$(SRCS:mpray%.txt=run%)
done:	comp_aa comp_color comp_eye comp_inside comp_normal comp_shadow-plane comp_shine3 comp_suns comp_trans3 comp_behind comp_dof comp_fisheye comp_ior comp_overlap comp_rough comp_shadow-suns comp_sphere comp_trif comp_bounces comp_expose1 comp_forward comp_many comp_panorama comp_shadow-basic comp_shadow-triangle comp_tex comp_trit comp_bulb comp_expose2 comp_gi comp_neglight comp_plane comp_shadow-bulb comp_shine1 comp_sun comp_trans1 comp_up

run%: program
	time ./program test/mpray$*.txt
comp%: run%
	-compare -fuzz 1% mpray$*.png test/mpray$*.png result/ae$*.png
	composite mpray$*.png test/mpray$*.png -compose difference result/rawdiff$*.png
	convert result/rawdiff$*.png -level 0%,8% result/diff$*.png
	convert +append test/mpray$*.png mpray$*.png result/ae$*.png result/rawdiff$*.png result/diff$*.png result/look_at_this_$*.png
	rm result/ae$*.png result/rawdiff$*.png result/diff$*.png
	code result/look_at_this_$*.png
see%:
	code test/mpray$*.txt
todo:
	./todo.sh
build: program

run: program
	./program $(file)
CSRCS = $(wildcard *.cpp)
COBJS = $(CSRCS:.cpp=.o)
%.o: %.cpp
	clang++  -c $< -o $@ $(CFLAGS)
program: $(COBJS)
	clang++ $^ -o $@ $(CFLAGS) -lpng
programdbg: $(CSRCS)
	g++ -g -I. $^ -o $@ -std=c++17 -lpng
gdb: programdbg
	gdb programdbg -nx
clean:
	rm *.png programdbg gmon.out analysis.txt test_gprof $(COBJS)
cleanresult: clean
	rm result/*.png
test_gprof: raytracer.cpp geometry.h
	g++ -Wall -pg raytracer.cpp -o test_gprof -std=c++17 -lpng -O3
gmon.out: test_gprof
	./test_gprof test/mpray_redchair.txt
analysis.txt: gmon.out
	gprof test_gprof gmon.out > analysis.txt