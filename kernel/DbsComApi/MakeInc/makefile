#******************************************************************************#
#* ˵��::�������ļ���Ӧ����������:                                            *#
#*       ����ϵͳ  : AIX5                                                     *#
#*       ���ݿ�ϵͳ: ORACLE9i                                                 *#
#*       ������    : gcc                                                      *#
#*----------------------------------------------------------------------------*#
#* ���ߣ�Li JunXu    ��д����:20030112   �޸���:           �޸�����:          *#
#******************************************************************************#

#############################################
##             PUBLIC PART                 ##
#############################################
PROCINCLUDE=include=$(ORACLE_HOME)/precomp/public include=$(SWHOME)/include include=$(HOME)/include
APPINCLUDE= -I../incl -I$(ORACLE_HOME)/precomp/public -I$(HOME)/include
SYS_INCLUDE='sys_include=(/usr/include)'
ORALIBDIR= -L$(ORACLE_HOME)/lib
APPLIBDIR= -L$(BATHOME)/lib 
DBSDIR= $(BATHOME)/dbsvr
BINDIR= $(DBSDIR)/bin
LIBDIR= $(BATHOME)/lib
USERID=scott/tiger
PROC=proc 
CC=cc 
ORALIB= -lpthread -lclntsh -lc `cat $(ORACLE_HOME)/lib/sysliblist`
CFLAGS= -fPIC -g -DLINUX -D__DEBUG__ -DNOWHAT=1 -D_THREAD_SAFE

#############################################
##           USER DEFINE PART			   ##
#############################################
TUSERLIB =
TARGET= ../MakeDbInc

#############################################
#############################################

#############################################
##           PUBLIC PART				   ##
#############################################
all: $(TARGET) 
$(TARGET): gendbch.o
	$(CC) -g $(ORALIBDIR) $(APPLIBDIR) $(CFLAGS) $(APPINCLUDE) $< -o $@ $(ORALIB) $(TUSERLIB) -L$(HOME)/lib -lBatCom

	@echo "$@ is OK!"
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