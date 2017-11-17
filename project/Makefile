# Makefile by Scott Leland Crossen
# @Copyright 2017 Scott Leland Crossen

# ==== Definitions ====

BYUNETID = slxn42
VERSION = 1
BINDIR = bin
SOURCEDIR = source
RELDIR = rel
TESTDIR = test
COMP = gcc
BINOBJS = ${BINDIR}/proxy.o ${BINDIR}/csapp.o ${BINDIR}/cache.o
MAIN = ./proxy
COMPFLAGS = -g -Wall
LINKFLAGS = -lrt -lpthread
HANDINDIR = /users/faculty/snell/CS324/handin/Fall2017/ProxyLab1

# ==== Recipes ====

.PHONY: default
.DEFAULT: default
default: all

.PHONY: all
all: $(MAIN)

.PHONY: clean
clean:
	rm -rf $(BINDIR) $(RELDIR) $(MAIN)

.PHONY: handin
handin:
	(make clean; cd ..; tar cvf proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*"; scp proxylab-handin.tar ${BYUNETID}@schizo.cs.byu.edu:$(HANDINDIR)/$(BYUNETID)-$(VERSION)-proxylab-handin.tar)

# ==== Targets ====

.PRECIOUS: ${BINDIR}/proxy.o
${BINDIR}/proxy.o: ${SOURCEDIR}/proxy.c ${SOURCEDIR}/csapp.h
	mkdir -p $(@D)
	$(COMP) -c $< -o $@ $(COMPFLAGS)

.PRECIOUS: ${BINDIR}/%.o
${BINDIR}/%.o: ${SOURCEDIR}/%.c ${SOURCEDIR}/%.h
	mkdir -p $(@D)
	$(COMP) -c $< -o $@ $(COMPFLAGS)

.PRECIOUS: ${RELDIR}/%
${RELDIR}/%: $(BINOBJS)
	mkdir -p $(@D)
	$(COMP) $^ -o $@ $(COMPFLAGS) $(LINKFLAGS)

.PRECIOUS: proxy
proxy: $(BINOBJS)
	$(COMP) $^ -o $@ $(COMPFLAGS) $(LINKFLAGS)
