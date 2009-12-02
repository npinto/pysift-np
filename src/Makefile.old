# file:        Makefile
# author:      Andrea Vedaldi
# description: Build SIFT++

NAME               := siftpp
VER                := 0.7.4
DIST                = $(NAME)-$(VER)
BINDIST             = $(DIST)-$(ARCH)

# --------------------------------------------------------------------
#                                                       Error messages
# --------------------------------------------------------------------

err_no_arch  =
err_no_arch +=$(shell echo "** Unknown host architecture '$(UNAME)'. This identifier"   1>&2)
err_no_arch +=$(shell echo "** was obtained by running 'uname -sm'. Edit the Makefile " 1>&2)
err_no_arch +=$(shell echo "** to add the appropriate configuration."                   1>&2)
err_no_arch +=Configuration failed

# --------------------------------------------------------------------
#                                                                Flags
# --------------------------------------------------------------------
# Options:
# LOWE_STRICT - Be more similar to Lowe's version
# USEFASTMATH - Use faster approximated computations

CXXFLAGS           += -I. -Wall -g -O9 -DNDEBUG -fPIC
#CXXFLAGS           += -Wno-variadic-macros
CXXFLAGS           += -DVL_LOWE_STRICT 
CXXFLAGS           += -DVL_USEFASTMATH

# Determine on the flight the system we are running on
Darwin_PPC_ARCH    := mac
Darwin_i386_ARCH   := mci
Linux_i386_ARCH    := glx
Linux_i686_ARCH    := glx
Linux_x86_64_ARCH  := g64
Linux_unknown_ARCH := glx
CYGWIN_NT-5.1_i686_ARCH := glx

UNAME             := $(shell uname -sm)
ARCH              := $($(shell echo "$(UNAME)" | tr \  _)_ARCH)


ifeq ($(ARCH),)
die:=$(error $(err_no_arch))
endif

mac_CXXFLAGS       := -DVL_MAC
mci_CXXFLAGS       := -DVL_MAC
glx_CXXFLAGS       :=
g64_CXXFLAGS       :=

CXXFLAGS           += $($(ARCH)_CXXFLAGS)

# --------------------------------------------------------------------
#
# --------------------------------------------------------------------

.PHONY: all
all: sift siftmodule.so

siftmodule.so:
	g++ -Wall -g -O9 -DNDEBUG -DVL_LOWE_STRICT  -DVL_USEFASTMATH -fPIC -I. -I/usr/include/python2.5 -c siftmodule.cpp -o siftmodule.o 
	g++ -shared siftmodule.o sift.o -o siftmodule.so -lpython2.5 -Wl,-no-undefined -Wl,-rpath,`pwd`

.PHONY: info
info:
	@echo ARCH=$(ARCH)

sift : sift.o sift-driver.o
	g++ $(CXXFLAGS) $^ -o $@

sift-static : sift.o sift-driver.o
	g++ $(CXXFLAGS) $^ -o $@ -static

benchmark : benchmark.o sift.o 
	g++ $(CXXFLAGS) $^ -o $@

sift-driver.o sift.o benchmark.o : sift.hpp sift.ipp sift-conv.tpp

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.so
	find . -name '*~' -exec rm -f \{\} \;
	find . -name '.DS_Store' -exec rm -f \{\} \;

.PHONY: distclean
distclean: clean
	rm -rf results dox
	rm -f sift sift-static benchmark
	rm -f .gdb_history
	rm -rf $(NAME)-*

.PHONY: dox
dox:
	(test -e dox || mkdir dox)
	doxygen doxygen.conf

.PHONY: test
test: sift
	(test -e results || mkdir results)
	./sift --verbose --output results/img3.key --save-gss data/img3.pgm

.PHONY: autorights
autorights:
	autorights . \
	  --verbose \
	  --recursive \
	  --template notice.txt \
	  --years 2006 \
	  --authors "Andrea Vedaldi (UCLA VisionLab)" \
	  --program "SIFT++"

.PHONY: dist
dist: distclean
	echo Version $(VER) - Archived on `date` > TIMESTAMP
	tar chvz --exclude "$(NAME)/test"        \
	         --exclude "$(NAME)/$(NAME)-*"   \
	         -f $(DIST).tar.gz ../$(NAME)

.PHONY: bindist
bindist: sift 
	test -e $(BINDIST) || mkdir $(BINDIST)
	cp sift $(BINDIST)
	cd $(BINDIST) ; strip -S sift
	tar cvzf $(BINDIST).tar.gz $(BINDIST)

.PHONY: bindist-static
bindist-static: sift-static
	test -e $(BINDIST)-static || mkdir $(BINDIST)-static
	cp sift-static $(BINDIST)-static
	cd $(BINDIST)-static ; strip -S sift-static
	tar cvzf $(BINDIST)-static.tar.gz $(BINDIST)-static
