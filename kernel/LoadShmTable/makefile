#|--------------------------------------------------------------------------|
#|						   TOPMake 2.0									  	|
#| Copyright (c) 2000-2003 Shanghai Huateng Software Systems Co., Ltd.		|
#|	All Rights Reserved														|
#|--------------------------------------------------------------------------|
#| FILE NAME	: sample.exec.mak											|
#| DESCRIPTIONS : Sample makefile for executable binary						|
#|--------------------------------------------------------------------------|

include $(HOME)/mak/platform.mak
include $(HOME)/mak/database.mak
include $(HOME)/mak/tuxedo.mak

PRGOBJS = 


PRGTARG = 
PRGLIBS = 
PRGDEFS =

# used for db2 database
 PRGDBNM =

debug all: debugexec
release: releaseexec

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f make.lib
	@make -f make.init
	@make -f make.view
	@make -f make.delete
	@make -f make.reload
	@rm -rf *.o
