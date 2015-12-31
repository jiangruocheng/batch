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
	int   nProcFlag;			 /* 子进程是否运行标示 0-未运行 1-已运行*/
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
	int   nProcTotalNum;		 /* 子任务总数*/
	int   nCurrentTotalNum;	  /* 已运行子任务总数*/
	int   nCurrentProcNum;	   /* 正在运行子任务数*/
	int   nRunProcQt;			/* 子任务并发数*/
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
	{"A0001", nGen_1_AssignTask, "T+0生成收单对账流水子任务分配"},
	{"A0002", nLoadShmAssignTask, "加载共享内存子任务分配"},
	{"A0003", nGen_1_AssignTask, "T+0加载收单对账流水子任务分配"},
	{"D0001", nGen_20_AssignTask, "T+0清分初始化任务"}, 
	{"D0002", nGen_20_AssignTask, "T+0商户资金清分任务分配"}, 
	{"E0001", nGen_10_AssignTask, "T+0统计商户周期资金明细任务"},
	{"E0002", nGen_10_AssignTask, "T+0生成商户代发明细任务"},
	{"F0001", nGen_10_AssignTask, "T+0生成手续费减免明细任务"},
	{"F0002", nGen_10_AssignTask, "T+0生成清算流水数据任务"},
	{"F0003", nGen_1_AssignTask, "生成手续费减免明细任务"},
	{"F0004", nGen_10_AssignTask, "修复清算流水冗余字段任务"},
	{"F0007", nGen_1_AssignTask, "生成账户数据任务"},

//		T + C 任务部分
	{"AC003", nLoadShmAssignTask, "加载内存表数据任务"},
	{"EC001", nGen_10_AssignTask, "统计商户周期资金明细任务"},
	{"EC002", nGen_10_AssignTask, "生成一清商户代发入账数据"},
	{"EC003", nGen_10_AssignTask, "截留恒丰申请资金任务"},
	{"FC001", nGen_10_AssignTask, "生成手续费减免明细"},
	{"FC002", nGen_10_AssignTask, "生成清算流水数据任务"},
	{"FC003", nGen_1_AssignTask, "生成一清清算汇总数据"},
	{"FC004", nGen_10_AssignTask, "修复清算流水冗余字段任务"},
	{"FC007", nGen_1_AssignTask, "生成账户数据任务"},
	{"CC001", nStlmTCAssignTask, "对账子任务分配"},
	{"ZC001", nGen_1_AssignTask, "批量自调任务"},

//		T + N 部分 开始
	{"AN001", nGen_1_AssignTask, "卸载posp流水任务"},
	{"AN002", nGen_1_AssignTask, "加载posp流水任务"},
	{"AN003", nLoadShmAssignTask, "加载内存表数据任务"},
	{"AN004", nGen_1_AssignTask, "生成对账状态表任务"},
	{"AN00N", nGen_1_AssignTask, "生成对账状态表任务"},
	{"DN001", nGen_20_AssignTask, "清分初始化任务"}, 	
	{"DN002", nGen_20_AssignTask, "商户资金清分任务分配"},
	{"DN003", nCalacCupAssignTask, "计算渠道手续费任务分配"},
	{"DN004", nAlgoBrhTxnAssignTask, "机构服务费资金清分分配"},
	{"DN005", nBrhServiceAssignTask, "代理服务费分润任务分配"},		//	机构分润 根据服务配置模型 目前由外面传入 固定值
	{"DN006", nGen_1_AssignTask, "将差错对平自动清分任务分配"},		//	考虑到这种数据比较小，采用单进程处理
	{"DN007", nGen_1_AssignTask, "卸载恒丰申购结果文件任务分配"},
	{"EN001", nGen_10_AssignTask, "统计商户周期资金明细任务"},
	{"EN002", nGen_10_AssignTask, "生成商户代发明细任务"},
	{"EN003", nGen_10_AssignTask, "截留恒丰申请资金任务"},
	{"EN004", nAlgoBrhTxnAssignTask, "生成机构服务费明细任务"},
	{"FN001", nGen_10_AssignTask, "生成手续费减免明细任务"},
	{"FN002", nGen_10_AssignTask, "生成清算流水数据任务"},
	{"FN003", nGen_1_AssignTask, "生成清算汇总数据任务"},
	{"FN004", nGen_1_AssignTask, "生成渠道成功流水文件任务"},
	{"FN005", nGen_1_AssignTask, "生成机构接入对账文件任务"},
	{"FN006", nGen_1_AssignTask, "生成商户对账文件任务"},
	{"FN007", nGen_1_AssignTask, "生成账户数据任务"},
	{"FN009", nGen_1_AssignTask, "拆分清算批次任务"},
	{"GN001", nGen_20_AssignTask, "修复清算流水冗余字段任务分配"},
	{"ZN001", nGen_1_AssignTask, "批量自调任务"},
//		T  + N 部分 结束

//	渠道文件装载部分
	{"BN001", nGen_1_AssignTask, "加载银商流水文件子任务分配"},
	{"BN002", nGen_1_AssignTask, "加载讯联流水文件子任务分配"},
	{"BN003", nGen_1_AssignTask, "加载支付宝流水文件子任务分配"},
	{"BN004", nGen_1_AssignTask, "加载浦发流水文件子任务分配"},
	{"BN005", nGen_1_AssignTask, "加载翰鑫文件子任务分配"},
	{"BN006", nGen_1_AssignTask, "加载腾付通流水文件子任务分配"},
	{"BN007", nGen_1_AssignTask, "加载银视通流水文件子任务分配"},
	{"BN008", nGen_1_AssignTask, "加载社区001流水文件子任务分配"},
	{"BN009", nGen_1_AssignTask, "加载卡富通流水文件子任务分配"},
	{"BN00A", nGen_1_AssignTask, "加载微信支付流水文件子任务分配"},
	{"BN00B", nGen_1_AssignTask, "加载轩宸流水文件子任务分配"},
	{"BN00C", nGen_1_AssignTask, "加载汇付流水文件子任务分配"},
	{"BN00D", nGen_1_AssignTask, "加载北京银商流水文件子任务分配"},
	{"BN00E", nGen_1_AssignTask, "加载海口银行流水文件子任务分配"},
	{"BN00F", nGen_1_AssignTask, "加载翰鑫新平台文件子任务分配"},
	{"BN00G", nGen_1_AssignTask, "加载银盛支付文件子任务分配"},
	{"BN00H", nGen_1_AssignTask, "加载宏达文件子任务分配"},
	{"BN00I", nGen_1_AssignTask, "加载通联对账流水文件子任务分配"},
	{"BN00J", nGen_1_AssignTask, "加载王府井对账流水文件子任务分配"},
	{"BN00K", nGen_1_AssignTask, "加载王府井新平台对账文件子任务分配"},
	{"BN00L", nGen_1_AssignTask, "加载华势对账文件子任务分配"},
	{"BN00M", nGen_1_AssignTask, "加载联动优势对账文件子任务分配"},
	{"BN00N", nGen_1_AssignTask, "加载块钱对账文件子任务分配"},
	{"BN00P", nGen_1_AssignTask, "加载恒丰申购文件任务分配"},
	{"BN00Q", nGen_1_AssignTask, "加载卡友文件任务分配"},
	{"BN00R", nGen_1_AssignTask, "加载宏达上业虚拟通道文件任务分配"},
	{"CN001", nStlmTNAssignTask, "对账子任务分配"}
};

#endif
