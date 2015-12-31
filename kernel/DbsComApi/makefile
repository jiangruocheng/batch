#******************************************************************************#
#*       操作系统  : CentOS                                                   *#
#*       数据库系统: ORACLE9i                                                 *#
#******************************************************************************#

PROCINCLUDE=include=$(ORACLE_HOME)/precomp/public include=$(SWHOME)/include include=$(HOME)/include include=$(HOME)/incl_c
APPINCLUDE= -I../incl -I$(ORACLE_HOME)/precomp/public -I$(HOME)/include -I$(HOME)/incl_c
SYS_INCLUDE='sys_include=(/usr/include)'
ORALIBDIR= -L$(ORACLE_HOME)/lib
APPLIBDIR= -L$(BATHOME)/lib 
DBSDIR= $(BATHOME)/dbsvr
BINDIR= $(DBSDIR)/bin
LIBDIR= $(BATHOME)/lib
PROC=proc
CC=cc
ORALIB= -lpthread -lclntsh -lc `cat $(ORACLE_HOME)/lib/sysliblist`
CFLAGS= -fPIC -g -DLINUX -D__DEBUG__ -DNOWHAT=1 -D_THREAD_SAFE

#############################################
##           USER DEFINE PART			   ##
#############################################
OBJFILE = DbsCommon.o DbsApi.o DbsTrim.o
TARGET= libDbsApi.a

all: $(TARGET)  clean
$(TARGET): $(OBJFILE)
	@ar -ruc $@ $(OBJFILE)
	@mv $@ $(LIBDIR)

.SUFFIXES: .pc .c .o

.pc.o:
	$(PROC) $(PROCINCLUDE) dbms=v7 unsafe_null=yes lines=yes parse=none iname=$*.pc
	$(CC) -g $(CFLAGS) -w $(APPINCLUDE) -c $*.c
	@rm -f $*.c
	@rm -f $*.lis

.c.o:
	$(CC)  $(CFLAGS) -w $(APPINCLUDE) -c $*.c

fall:
	@touch *c
	@make all
clean:
	rm -f *.o *.so *.c *~
