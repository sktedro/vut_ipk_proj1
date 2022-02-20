LOGIN=xskalo01
SOURCE=hinfosvc
TESTSCRIPT=test.sh
PORT=12321

all:
	gcc -o $(SOURCE) $(SOURCE).c

run: all
	./$(SOURCE) $(PORT)

test: all
	./$(TESTSCRIPT) $(PORT)

pack: clean
	zip $(LOGIN).zip $(SOURCE).c Makefile README.md

pack_all: clean
	zip $(SOURCE).zip ./*

clean:
	rm -f $(SOURCE) *.zip
