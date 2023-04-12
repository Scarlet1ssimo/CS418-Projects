.PHONEY: build, run, clean, cleanresult, todo, see%, run%, comp%, all, done,gdb
SRCS = $(wildcard test/*.txt)
DONES=$(wildcard result/*.png)
all:	$(SRCS:mpray%.txt=run%)
done:	$(DONES:result/look_at_this_%.png=comp%)

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

program: raytracer.cpp geometry.h
	clang++ -O3 -I. raytracer.cpp -o program -std=c++17 -lpng
programdbg: raytracer.cpp geometry.h
	g++ -g -I. raytracer.cpp -o programdbg -std=c++17 -lpng
gdb: programdbg
	gdb programdbg -nx
clean:
	rm *.png
cleanresult: clean
	rm result/*.png