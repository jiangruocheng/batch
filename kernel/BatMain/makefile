#|------------------------------------------------------------------------------|
#|						TOPMake 2.0	 					  						|
#| Copyright (c) 2000-2003 Shanghai Huateng Software Systems Co., Ltd.			|
#|	All Rights Reserved							  								|
#|------------------------------------------------------------------------------|
#| FILE NAME	: sample.dynamiclib.mak							  				|
#| DESCRIPTIONS : Sample makefile for dynamic library				  			|
#|------------------------------------------------------------------------------|

include $(BATHOME)/mak/platform.mak
include $(BATHOME)/mak/database.mak
include $(BATHOME)/mak/tuxedo.mak

PRGOBJS = AlgoBrhTxnAssign.o CommonAssign.o BatMain.o 

PRGTARG = BatMain
PRGLIBS = -lShmTbl -lDbsApi -lBatCom $(NETLIBS) $(UXLIBS) $(DBLIBS)
PRGDEFS =

# used for db2 database
PRGDBNM = $(DBLINK)

debug all: debugexec
release: releaseexec

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f $(BATHOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" DEFS="$(PRGDEFS)" DBNM="$(PRGDBNM)"
