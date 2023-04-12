.PHONEY: build, run, clean
SRCS = $(wildcard test/*.txt)
all:	$(SRCS:mpray%.txt=%.run)

run%:
	python3 program.py test/mpray$*.txt
comp%: run%
	-compare -fuzz 2% mpray$*.png test/mpray$*.png result/ae$*.png
	composite mpray$*.png test/mpray$*.png -compose difference result/rawdiff$*.png
	convert result/rawdiff$*.png -level 0%,8% result/diff$*.png
	convert +append test/mpray$*.png mpray$*.png result/ae$*.png result/rawdiff$*.png result/diff$*.png result/look_at_this_$*.png
	rm result/ae$*.png result/rawdiff$*.png result/diff$*.png

build:

run:
	python3 program.py $(file)

clean:
	rm *.png
cleanresult: clean
	rm result/*.png