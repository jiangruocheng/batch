#|--------------------------------------------------------------------------|
#|						   TOPMake 2.0									  	|
#| Copyright (c) 2000-2003 Shanghai Huateng Software Systems Co., Ltd.		|
#|	All Rights Reserved													 	|
#|--------------------------------------------------------------------------|
#| FILE NAME	: sample.dynamiclib.mak									 	|
#| DESCRIPTIONS : Sample makefile for dynamic library						|
#|--------------------------------------------------------------------------|

include $(BATHOME)/mak/platform.mak
include $(BATHOME)/mak/database.mak
include $(BATHOME)/mak/tuxedo.mak

PRGOBJS = \
	date.o \
	file.o \
	log.o \
	string.o \
	BatCom.o 
PRGTARG = BatCom
PRGDEFS = 

# used for db2 database
PRGDBNM = $(DBLINK)

debug all: debugstatic
release: releasestatic

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f $(BATHOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" DEFS="$(PRGDEFS)" DBNM="$(PRGDBNM)"
