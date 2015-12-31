#ifndef _BATMAIN_H
#define _BATMAIN_H

#include "Ibp.h"
#include "load.h"
#include "DbsApi.h"
#include "pos_stlm.h"

//char gsLogFile[LOG_NAME_LEN_MAX];

#define MAX_BAT_PROC_NUM  10000
#define MAX_BAT_NUM	   200

typedef struct __ASSIGNTASK
{
	char	sBatId[6];
	int	 (*pfAssignFun)(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
	char	sAssignDsp[100];
}T_BatAssignFun;

typedef struct {
	char sBatId[5];
	char sStlmDate[8];
	char sStlmNum[4];
	char sArgc[2];
	char sArgv[256];
	char sRspCode[2];
	char sRspDsp[256];
}t_Console_Cmd_Def;

typedef struct {
	int   nProcFlag;			 /* �ӽ����Ƿ����б�ʾ 0-δ���� 1-������*/
	pid_t iPid;
	int	 nArgc;
	char	sMatchStr[31];
	char	szBatStartFalg[2];
}t_Pid_List_Def;

typedef struct {
	char  szPorcName[16 + 1];
	char  szStlmNum[LEN_NUM + 1];
	char  szStlmDate[LEN_DATE + 1];
	char  sBatId[5 + 1];
	int   nProcTotalNum;		 /* ����������*/
	int   nCurrentTotalNum;	  /* ����������������*/
	int   nCurrentProcNum;	   /* ����������������*/
	int   nRunProcQt;			/* �����񲢷���*/
	t_Pid_List_Def  tPidList[MAX_BAT_PROC_NUM];
}t_Bat_List_Def;

extern int	nGen_1_AssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
extern int	nGen_10_AssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
extern int	nGen_20_AssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
extern int	nLoadShmAssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
extern int	nStlmTCAssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
extern int	nStlmTNAssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
extern int	nCalacCupAssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);

extern int	nAlgoBrhTxnAssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);
extern int	nBrhServiceAssignTask(char *pszStlmDate, char *pszStlmNum, char *pszBatId, int lChdCount);


T_BatAssignFun  gBatAssignFun[] = {
	{"A0001", nGen_1_AssignTask, "T+0�����յ�������ˮ���������"},
	{"A0002", nLoadShmAssignTask, "���ع����ڴ����������"},
	{"A0003", nGen_1_AssignTask, "T+0�����յ�������ˮ���������"},
	{"D0001", nGen_20_AssignTask, "T+0��ֳ�ʼ������"}, 
	{"D0002", nGen_20_AssignTask, "T+0�̻��ʽ�����������"}, 
	{"E0001", nGen_10_AssignTask, "T+0ͳ���̻������ʽ���ϸ����"},
	{"E0002", nGen_10_AssignTask, "T+0�����̻�������ϸ����"},
	{"F0001", nGen_10_AssignTask, "T+0���������Ѽ�����ϸ����"},
	{"F0002", nGen_10_AssignTask, "T+0����������ˮ��������"},
	{"F0003", nGen_1_AssignTask, "���������Ѽ�����ϸ����"},
	{"F0004", nGen_10_AssignTask, "�޸�������ˮ�����ֶ�����"},
	{"F0007", nGen_1_AssignTask, "�����˻���������"},

//		T + C ���񲿷�
	{"AC003", nLoadShmAssignTask, "�����ڴ����������"},
	{"EC001", nGen_10_AssignTask, "ͳ���̻������ʽ���ϸ����"},
	{"EC002", nGen_10_AssignTask, "����һ���̻�������������"},
	{"EC003", nGen_10_AssignTask, "������������ʽ�����"},
	{"FC001", nGen_10_AssignTask, "���������Ѽ�����ϸ"},
	{"FC002", nGen_10_AssignTask, "����������ˮ��������"},
	{"FC003", nGen_1_AssignTask, "����һ�������������"},
	{"FC004", nGen_10_AssignTask, "�޸�������ˮ�����ֶ�����"},
	{"FC007", nGen_1_AssignTask, "�����˻���������"},
	{"CC001", nStlmTCAssignTask, "�������������"},
	{"ZC001", nGen_1_AssignTask, "�����Ե�����"},

//		T + N ���� ��ʼ
	{"AN001", nGen_1_AssignTask, "ж��posp��ˮ����"},
	{"AN002", nGen_1_AssignTask, "����posp��ˮ����"},
	{"AN003", nLoadShmAssignTask, "�����ڴ����������"},
	{"AN004", nGen_1_AssignTask, "���ɶ���״̬������"},
	{"AN00N", nGen_1_AssignTask, "���ɶ���״̬������"},
	{"DN001", nGen_20_AssignTask, "��ֳ�ʼ������"}, 	
	{"DN002", nGen_20_AssignTask, "�̻��ʽ�����������"},
	{"DN003", nCalacCupAssignTask, "���������������������"},
	{"DN004", nAlgoBrhTxnAssignTask, "����������ʽ���ַ���"},
	{"DN005", nBrhServiceAssignTask, "�������ѷ����������"},		//	�������� ���ݷ�������ģ�� Ŀǰ�����洫�� �̶�ֵ
	{"DN006", nGen_1_AssignTask, "������ƽ�Զ�����������"},		//	���ǵ��������ݱȽ�С�����õ����̴���
	{"DN007", nGen_1_AssignTask, "ж�غ���깺����ļ��������"},
	{"EN001", nGen_10_AssignTask, "ͳ���̻������ʽ���ϸ����"},
	{"EN002", nGen_10_AssignTask, "�����̻�������ϸ����"},
	{"EN003", nGen_10_AssignTask, "������������ʽ�����"},
	{"EN004", nAlgoBrhTxnAssignTask, "���ɻ����������ϸ����"},
	{"FN001", nGen_10_AssignTask, "���������Ѽ�����ϸ����"},
	{"FN002", nGen_10_AssignTask, "����������ˮ��������"},
	{"FN003", nGen_1_AssignTask, "�������������������"},
	{"FN004", nGen_1_AssignTask, "���������ɹ���ˮ�ļ�����"},
	{"FN005", nGen_1_AssignTask, "���ɻ�����������ļ�����"},
	{"FN006", nGen_1_AssignTask, "�����̻������ļ�����"},
	{"FN007", nGen_1_AssignTask, "�����˻���������"},
	{"FN009", nGen_1_AssignTask, "���������������"},
	{"GN001", nGen_20_AssignTask, "�޸�������ˮ�����ֶ��������"},
	{"ZN001", nGen_1_AssignTask, "�����Ե�����"},
//		T  + N ���� ����

//	�����ļ�װ�ز���
	{"BN001", nGen_1_AssignTask, "����������ˮ�ļ����������"},
	{"BN002", nGen_1_AssignTask, "����Ѷ����ˮ�ļ����������"},
	{"BN003", nGen_1_AssignTask, "����֧������ˮ�ļ����������"},
	{"BN004", nGen_1_AssignTask, "�����ַ���ˮ�ļ����������"},
	{"BN005", nGen_1_AssignTask, "���غ����ļ����������"},
	{"BN006", nGen_1_AssignTask, "�����ڸ�ͨ��ˮ�ļ����������"},
	{"BN007", nGen_1_AssignTask, "��������ͨ��ˮ�ļ����������"},
	{"BN008", nGen_1_AssignTask, "��������001��ˮ�ļ����������"},
	{"BN009", nGen_1_AssignTask, "���ؿ���ͨ��ˮ�ļ����������"},
	{"BN00A", nGen_1_AssignTask, "����΢��֧����ˮ�ļ����������"},
	{"BN00B", nGen_1_AssignTask, "���������ˮ�ļ����������"},
	{"BN00C", nGen_1_AssignTask, "���ػ㸶��ˮ�ļ����������"},
	{"BN00D", nGen_1_AssignTask, "���ر���������ˮ�ļ����������"},
	{"BN00E", nGen_1_AssignTask, "���غ���������ˮ�ļ����������"},
	{"BN00F", nGen_1_AssignTask, "���غ�����ƽ̨�ļ����������"},
	{"BN00G", nGen_1_AssignTask, "������ʢ֧���ļ����������"},
	{"BN00H", nGen_1_AssignTask, "���غ���ļ����������"},
	{"BN00I", nGen_1_AssignTask, "����ͨ��������ˮ�ļ����������"},
	{"BN00J", nGen_1_AssignTask, "����������������ˮ�ļ����������"},
	{"BN00K", nGen_1_AssignTask, "������������ƽ̨�����ļ����������"},
	{"BN00L", nGen_1_AssignTask, "���ػ��ƶ����ļ����������"},
	{"BN00M", nGen_1_AssignTask, "�����������ƶ����ļ����������"},
	{"BN00N", nGen_1_AssignTask, "���ؿ�Ǯ�����ļ����������"},
	{"BN00P", nGen_1_AssignTask, "���غ���깺�ļ��������"},
	{"BN00Q", nGen_1_AssignTask, "���ؿ����ļ��������"},
	{"BN00R", nGen_1_AssignTask, "���غ����ҵ����ͨ���ļ��������"},
	{"CN001", nStlmTNAssignTask, "�������������"}
};

#endif
