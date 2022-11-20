FIRST_NAME=Johnson
LAST_NAME=Umeike
KUID=3094849
PROJECT=Project01
TAR_BASENAME=$(PROJECT)_$(FIRST_NAME)_$(LAST_NAME)_$(KUID)

all: quash

quash: quash.cpp
	g++ -g $^ -o $@

clean:
	rm -rf *.o quash $(TAR_BASENAME) $(TAR_BASENAME).tar.gz

tar: clean
#	create temp dir
	mkdir $(TAR_BASENAME)
#	copy the necessary files into the temp dir
	cp quash.cpp Jobs.h Report.txt Makefile $(TAR_BASENAME)
#	create the submission tar.gz
	tar cvzf $(TAR_BASENAME).tar.gz $(TAR_BASENAME)
#	remove the temp dir
	rm -rf $(TAR_BASENAME)

.PHONY: clean tar
