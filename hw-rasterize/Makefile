.PHONEY: build, run, clean
SRCS = $(wildcard test/*.txt)
all:	$(SRCS:mp1%.txt=%.run)
done: compindexing compreq1 compreq2 compcull compdecals compdepth compfsaa2 compfsaa3 compfsaa4 comphyp comphyptex compindexing comppoint comprgba compsrgb comptexture compclipplane compfrustum compline compbillboard
run%:
	python3 program.py test/mp1$*.txt
comp%: run%
	-compare -fuzz 2% mp1$*.png test/mp1$*.png result/ae$*.png
	composite mp1$*.png test/mp1$*.png -compose difference result/rawdiff$*.png
	convert result/rawdiff$*.png -level 0%,8% result/diff$*.png
	convert +append test/mp1$*.png mp1$*.png result/ae$*.png result/rawdiff$*.png result/diff$*.png result/look_at_this_$*.png
	rm result/ae$*.png result/rawdiff$*.png result/diff$*.png

build:

run:
	python3 program.py $(file)

clean:
	rm *.png
cleanresult: clean
	rm result/*.png