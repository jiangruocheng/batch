#include	"load.h"

#define	ZCHAR(x)	(x == 0x00)?(x = ' '):x

typedef struct __TBLPRINT
{
	char	szDsc[200];
	void	(*pfTable)(char *pszTemp);
}TblPrint;

/****************************************************************************************
	获取到毫秒
 ****************************************************************************************/
double	dGetUSecTime()
{
	double	dTime;
	struct timeval	t;
	struct tm	*ptm;

	gettimeofday(&t, NULL);
	ptm = localtime(&t.tv_sec);

	dTime = ptm->tm_min * 60 * 1000 + ptm->tm_sec * 1000 + t.tv_usec / 1000 + t.tv_usec % 1000 / 1000.0;
	return dTime;
}


/****************************************************************************************
	获取时间，精确毫秒
 ****************************************************************************************/
long	lGetTiskTime()
{
	long	lTime;
	struct timeval	t;
	struct tm	*ptm;

	gettimeofday(&t, NULL);
	ptm = localtime(&t.tv_sec);

	lTime = ptm->tm_min * 60 * 1000 + ptm->tm_sec * 1000 + t.tv_usec / 1000;	//微秒（除以1000就是毫秒)
	return lTime;
}

/****************************************************************************************
	获取查询时间字符串
 ****************************************************************************************/
char*	sGetCostTime(long lTime)
{
	static	char	szTime[30];

	memset(szTime, 0, sizeof(szTime));

	if(lTime < 0)
		return szTime;
	if(lTime < 1000)
		snprintf(szTime, sizeof(szTime), "cost:%dms", lTime);
	else
		snprintf(szTime, sizeof(szTime), "cost:%ds%dms", lTime / 1000, lTime % 1000);
	return szTime;
}

/****************************************************************************************
	连接共享内存并初始化必要索引
 ****************************************************************************************/
void	vTableView_REGION_CODE(char *pszTemp)
{
	dbReginCode		stReg;
	long			lRet = 0, nRecord = 0, lTime = 0;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_REGION_CODE;
	if(pszTemp)
	{
		memset(&stReg, 0, sizeof(stReg));
		strcpy(stReg.region_code, pszTemp);
		sTrimAll(stReg.region_code);

		pstRun->lSize = sizeof(stReg);
		pstRun->pstVoid = &stReg;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_REGION_CODE游标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_REGION_CODE数据如下:\n", stReg.region_code);
	fprintf(stdout, "---------------------------------------------------------------------\n");
	fprintf(stdout, "line\tid\t地区编码\t地区名称\t\n");
	fprintf(stdout, "---------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stReg, 0, sizeof(stReg));	
		lRet = lTableFetch(TBL_REGION_CODE, &stReg, sizeof(stReg));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_REGION_CODE错误!(%s), 原因:%s", stReg.region_code, sGetError());
			vTableClose(TBL_REGION_CODE, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stReg.id);
		fprintf(stdout, "[%d]:\t%6d|\t%11s|%40s|\n", nRecord, stReg.id, stReg.region_code, stReg.region_name);
	}
	lTime -= lGetTiskTime();	
	vTableClose(TBL_REGION_CODE, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------------%s-------------------------------\n", sGetCostTime(-1 * lTime));

	return ;
}

/****************************************************************************************
	查看内存机构信息表数据信息	
 ****************************************************************************************/
void	vTableView_BRH_INFO(char *pszTemp)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbBrhInfo		stBrh;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_BRH_INFO;
	if(pszTemp)
	{
		memset(&stBrh, 0, sizeof(stBrh));
		strcpy(stBrh.brh_code, pszTemp);
		sTrimAll(stBrh.brh_code);

		pstRun->lSize = sizeof(stBrh);
		pstRun->pstVoid = &stBrh;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_BRH_INFO标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_BRH_INFO数据如下:\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"----------------------------------------\n");
	fprintf(stdout, "line\t   序号\t	 机构等级\t\t机构号\t\t\t\t\t机构名称\t\t\t\t   "
		" 是否参与分润\t分润方案   机构接入标示\t 机构对账文件\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"----------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stBrh, 0, sizeof(stBrh));	
		lRet = lTableFetch(TBL_BRH_INFO, &stBrh, sizeof(stBrh));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_BRH_INFO错误, 原因:%s", sGetError());
			vTableClose(TBL_BRH_INFO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stBrh.brh_id);
		lGetEXLong(&stBrh.brh_level);
		fprintf(stdout, "[%d]:\t%10d|%10d|%20s|%-70s|%14s|\t%-9s|\t\t%c|\t%5c|\n", nRecord, 
			stBrh.brh_id, stBrh.brh_level, stBrh.brh_code, stBrh.brh_name, 
			stBrh.brh_stlm_flag, stBrh.brh_disc_id, ZCHAR(stBrh.brh_tag[0]), 
			ZCHAR(stBrh.brh_file_flag[0]));
	}
	lTime -= lGetTiskTime();	
	vTableClose(TBL_BRH_INFO, SHM_DISCONNECT);
	fprintf(stdout, "----------------------------------------------------------"
		"----------------------------------%s------------------------------------"
		"---------------------------------------------\n", sGetCostTime(-1 * lTime));

	return ;
}

/****************************************************************************************
	查看MCC信息
 ****************************************************************************************/
void	vTableView_BRH_PROFIT_INFO(char *pszTemp)
{
	long	lRet = 1, nRecord = 0, lTime = 0;
	RunTime		*pstRun = (RunTime *)pInitRunTime();
	dbPfitInfo	stPfitIf;

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_INFO;
	if(pszTemp)
	{
		memset(&stPfitIf, 0, sizeof(stPfitIf));
		strcpy(stPfitIf.model_id, pszTemp);
		sTrimAll(stPfitIf.model_id);

		pstRun->lSize = sizeof(stPfitIf);
		pstRun->pstVoid = &stPfitIf;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_BRH_PROFIT_INFO标失败, 原因:%s", sGetError());
		return ;
	}
	fprintf(stdout, "表TBL_BRH_PROFIT_INFO数据如下:\n");
	fprintf(stdout, "-------------------------------------------------------------------\n");
	fprintf(stdout, "line\t模型编号\t间联类模型\t直联类模型\t其它类模型\n");
	fprintf(stdout, "-------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stPfitIf, 0, sizeof(stPfitIf));	
		lRet = lTableFetch(TBL_BRH_PROFIT_INFO, &stPfitIf, sizeof(stPfitIf));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_BRH_PROFIT_INFO错误, 原因:%s", sGetError());
			vTableClose(TBL_BRH_PROFIT_INFO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		fprintf(stdout, "[%d]:\t%10s|\t%10s|\t%10s|\t%10s|\n", nRecord, stPfitIf.model_id, 
			stPfitIf.treaty_id, stPfitIf.standard_id, stPfitIf.other_id);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_BRH_PROFIT_INFO, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------------%s------------------------------\n", 
		sGetCostTime(-1 * lTime));
	return ;
}

/****************************************************************************************
	查看MCC信息
 ****************************************************************************************/
void	vTableView_MCHT_MCC_INF(char *pszTemp)
{
	long			lRet = 1, nRecord = 0, lTime = 0;
	dbMchtMccInf	stMcc;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_MCC_INF;
	if(pszTemp)
	{
		memset(&stMcc, 0, sizeof(stMcc));
		strcpy(stMcc.mchnt_tp, pszTemp);
		sTrimAll(stMcc.mchnt_tp);

		pstRun->lSize = sizeof(stMcc);
		pstRun->pstVoid = &stMcc;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_MCHT_MCC_INF游标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_MCHT_MCC_INF数据如下:\n");
	fprintf(stdout, "------------------------------\n");
	fprintf(stdout, "line\t序号\tMCC码\tMCC组别\t\n");
	fprintf(stdout, "------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stMcc, 0, sizeof(stMcc));	
		lRet = lTableFetch(TBL_MCHT_MCC_INF, &stMcc, sizeof(stMcc));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_MCHT_MCC_INF错误, 原因:%s", sGetError());
			vTableClose(TBL_MCHT_MCC_INF, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stMcc.id);
		fprintf(stdout, "[%d]:\t%5d|\t%5s|\t%5s|\n", nRecord, stMcc.id, stMcc.mchnt_tp, 
			stMcc.mchnt_tp_grp);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_MCHT_MCC_INF, SHM_DISCONNECT);
	fprintf(stdout, "------------%s----------\n", sGetCostTime(-1 * lTime));
}

/****************************************************************************************
	4.2.3.	机构分润定义表 TBL_BRH_PROFIT_DEF
 ****************************************************************************************/
void	vTableView_BRH_PROFIT_DEF(char *pszModel)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbPfitDef		stDbPfitDef;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_DEF;
	if(pszModel)
	{
		memset(&stDbPfitDef, 0, sizeof(stDbPfitDef));
		strcpy(stDbPfitDef.model_id, pszModel);
		sTrimAll(stDbPfitDef.model_id);

		pstRun->lSize = sizeof(stDbPfitDef);
		pstRun->pstVoid = &stDbPfitDef;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_BRH_PROFIT_DEF标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_BRH_PROFIT_DEF数据如下:\n");
	fprintf(stdout, "----------------------------------------------------------------------------------\n");
	fprintf(stdout, "line\t模型编号\t模型所属机构\t\t状态\t是否统一分润\t分润比例\n");
	fprintf(stdout, "----------------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stDbPfitDef, 0, sizeof(stDbPfitDef));	
		lRet = lTableFetch(TBL_BRH_PROFIT_DEF, &stDbPfitDef, sizeof(stDbPfitDef));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_BRH_PROFIT_DEF错误, 原因:%s", sGetError());
			vTableClose(TBL_BRH_PROFIT_DEF, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		fprintf(stdout, "[%d]:\t%-9s|\t%18s|\t%4s|\t%12s|\t%9s|\n", nRecord, stDbPfitDef.model_id, 
			stDbPfitDef.model_brh, stDbPfitDef.model_flag, stDbPfitDef.unite_ratio, stDbPfitDef.profit_ratio);
	}
	lTime -= lGetTiskTime();	
	vTableClose(TBL_BRH_PROFIT_DEF, SHM_DISCONNECT);
	fprintf(stdout, "---------------------------------%s-----------------------------------------\n", 
		sGetCostTime(-1 * lTime));

}

/****************************************************************************************
	机构分润配置表	TBL_BRH_PROFIT_ALGO
 ****************************************************************************************/
void	vTableView_BRH_PROFIT_ALGO(char *pszModel)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbBrhAlgo		stBrhAlgo;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_ALGO;
	if(pszModel)
	{
		memset(&stBrhAlgo, 0, sizeof(stBrhAlgo));
		strcpy(stBrhAlgo.model_id, pszModel);
		sTrimAll(stBrhAlgo.model_id);

		pstRun->lSize = sizeof(stBrhAlgo);
		pstRun->pstVoid = &stBrhAlgo;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_BRH_PROFIT_ALGO标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_BRH_PROFIT_ALGO数据如下:\n");
	fprintf(stdout, "-------------------------------------------------------------\n");
	fprintf(stdout, "line\t模型编号	交易种类 商户组 卡类型 基准费   分润比例\n");
	fprintf(stdout, "-------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stBrhAlgo, 0, sizeof(stBrhAlgo));	
		lRet = lTableFetch(TBL_BRH_PROFIT_ALGO, &stBrhAlgo, sizeof(stBrhAlgo));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_BRH_PROFIT_ALGO错误, 原因:%s", sGetError());
			vTableClose(TBL_BRH_PROFIT_ALGO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stBrhAlgo.card_type);
		fprintf(stdout, "[%d]:\t%9s|%9s|%5s|%6d|%9s|%9s|\n", nRecord, stBrhAlgo.model_id, 
			stBrhAlgo.trans_type, stBrhAlgo.mcht_grp, stBrhAlgo.card_type, stBrhAlgo.base_fee, 
			stBrhAlgo.profit_ratio);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_BRH_PROFIT_ALGO, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------%s------------------------------\n", 
		sGetCostTime(-1 * lTime));

}

/****************************************************************************************
	查看商户账号信息
 ****************************************************************************************/
void	vTableView_ACCT_INFO(char *pszTemp)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbAcctInfo		stAct;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_ACCT_INFO;
	if(pszTemp)
	{
		memset(&stAct, 0, sizeof(stAct));
		strcpy(stAct.app_id, pszTemp);
		sTrimAll(stAct.app_id);

		pstRun->lSize = sizeof(stAct);
		pstRun->pstVoid = &stAct;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_ACCT_INFO标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_ACCT_INFO数据如下:\n");
	fprintf(stdout, "-------------------------------------------------------------------"
		"-------------------------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"-------------------------------------\n");
	fprintf(stdout, "line\t账号序号 应用类型  应用标示号   类型 是否默认\t账户账号\t"
		"\t\t账户名称\t   账户开户行号\t	账户开户行名称  账户开户支行号\t   账户开户支行名称"
		"\t\t\t\t开户支行地址\t支行地区号 网银支付号 账户联行号\n");
	fprintf(stdout, "-------------------------------------------------------------------"
		"-------------------------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"-------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stAct, 0, sizeof(stAct));	
		lRet = lTableFetch(TBL_ACCT_INFO, &stAct, sizeof(stAct));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_ACCT_INFO错误, 原因:%s", sGetError());
			vTableClose(TBL_ACCT_INFO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stAct.acct_id);
		lGetEXLong(&stAct.app_type);
		lGetEXLong(&stAct.acct_default);
		fprintf(stdout, "[%d]:\t%10d|%4d|%15s|%4s|%4d|%25s|%30s|%12s|%20s|%12s|%48s|%21s|%8s|%12s|%12s|\n", 
			nRecord, stAct.acct_id, stAct.app_type, stAct.app_id, stAct.acct_type, stAct.acct_default, 
			stAct.acct_no, stAct.acct_nm, stAct.acct_bank_no, stAct.acct_bank_nm, stAct.acct_zbank_no, 
			stAct.acct_zbank_nm, stAct.acct_zbank_addr, stAct.acct_zbank_code, stAct.acct_net_no, 
			stAct.acct_union_no);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_ACCT_INFO, SHM_DISCONNECT);
	fprintf(stdout, "---------------------------------------------------------------------"
		"----------------------------------------------------%s--------------------------"
		"-----------------------------------------------------------------------------"
		"----------------------------\n", sGetCostTime(-1 * lTime));
}

/****************************************************************************************
	清算付款账号查询
 ****************************************************************************************/
void	vTableView_OPR_INFO(char *pszId)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbOprInf	stOpr;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_OPR_INFO;
	if(pszId && strlen(pszId))
	{
		memset(&stOpr, 0, sizeof(stOpr));
		stOpr.opr_id = lSetEXLong(atol(pszId));

		pstRun->lSize = sizeof(stOpr);
		pstRun->pstVoid = &stOpr;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_OPR_INFO游标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_OPR_INFO数据如下:\n");
	fprintf(stdout, "----------------------------------------------------"
		"-----------------------------\n");
	fprintf(stdout, "line\t操作员编号\t\t登录名\t\t\t\t操作员姓名\n");
	fprintf(stdout, "----------------------------------------------------"
		"-----------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stOpr, 0, sizeof(stOpr));	
		lRet = lTableFetch(TBL_OPR_INFO, &stOpr, sizeof(stOpr));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_OPR_INFO错误, 原因:%s", sGetError());
			vTableClose(TBL_OPR_INFO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stOpr.opr_id);
		fprintf(stdout, "[%d]:\t%10d|%30s|%30s|\n", nRecord, stOpr.opr_id, 
			stOpr.opr_code, stOpr.opr_name);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_OPR_INFO, SHM_DISCONNECT);
	fprintf(stdout, "-------------------------------------%s--------------"
		"----------------------\n", sGetCostTime(-1 * lTime));
	return ;
}

/****************************************************************************************
	查看浦发机构信息
 ****************************************************************************************/
void	vTableView_SPDB_BRH(char *pszRegNo)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbSpdbBrh		stSpdb;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_SPDB_BRH;
	if(pszRegNo)
	{
		memset(&stSpdb, 0, sizeof(stSpdb));
		strcpy(stSpdb.brh_area_no, pszRegNo);
		sTrimAll(stSpdb.brh_area_no);

		pstRun->lSize = sizeof(stSpdb);
		pstRun->pstVoid = &stSpdb;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_SPDB_BRH标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_SPDB_BRH数据如下:\n");
	fprintf(stdout, "-----------------------------------------------------------------\n");
	fprintf(stdout, "line\t序号\t\t\t\t\t地区名\t地区编码\n");
	fprintf(stdout, "-----------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stSpdb, 0, sizeof(stSpdb));	
		lRet = lTableFetch(TBL_SPDB_BRH, &stSpdb, sizeof(stSpdb));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_SPDB_BRH错误, 原因:%s", sGetError());
			vTableClose(TBL_SPDB_BRH, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stSpdb.id);
		fprintf(stdout, "[%d]:\t%4d|%40s|%10s|\n", nRecord, stSpdb.id, stSpdb.region_name, stSpdb.brh_area_no);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_SPDB_BRH, SHM_DISCONNECT);
	fprintf(stdout, "-------------------------%s-------------------------"
		"------\n", sGetCostTime(-1 * lTime));
}

/****************************************************************************************
	查看商户基本信息
 ****************************************************************************************/
void	vTableView_MCHT_BASE(char *pszTemp)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbMchtBase		stBase;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_BASE;
	if(pszTemp)
	{
		memset(&stBase, 0, sizeof(stBase));
		strcpy(stBase.mcht_no, pszTemp);
		sTrimAll(stBase.mcht_no);

		pstRun->lSize = sizeof(stBase);
		pstRun->pstVoid = &stBase;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_MCHT_BASE标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_MCHT_BASE数据如下:\n");
	fprintf(stdout, "------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------\n");
	fprintf(stdout, "line\t序号\t\t商户号\t\t\t\t商户名称\t   状态 商户种类   上级商户号"
		" 区域代码\t\t\t\t商户地址\tMCC码\t所属机构号\t 拓展员\t\t清算机构	  T+0费率"
		" 费率模型 操作标示\t渠道大商户\n");
	fprintf(stdout, "------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();	
	//	遍历所有记录
	while(1)
	{
		memset(&stBase, 0, sizeof(stBase));	
		lRet = lTableFetch(TBL_MCHT_BASE, &stBase, sizeof(stBase));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_MCHT_BASE错误, 原因:%s", sGetError());
			vTableClose(TBL_MCHT_BASE, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stBase.id);
		lGetEXLong(&stBase.mcht_resv1);
		lGetEXLong(&stBase.mcht_expand);
		fprintf(stdout, "[%d]:\t%9d|%s|%40s|%4s|%6s|%15s|%6s|%40s|%5s|%18s|%10d|%18s|%8s|%8s|%8s|%8d|\n", 
			nRecord, stBase.id, stBase.mcht_no, stBase.mcht_nm, stBase.mcht_status, stBase.mcht_kind, 
			stBase.mcht_up_no, stBase.mcht_area_no, stBase.mcht_addr, stBase.mcht_mcc, stBase.mcht_brh, 
			stBase.mcht_expand, stBase.mcht_stlm_brh, stBase.mcht_disc_id0, stBase.mcht_disc_id, 
			stBase.mcht_opr_flag, stBase.mcht_resv1);
	}
	lTime -= lGetTiskTime();	
	vTableClose(TBL_MCHT_BASE, SHM_DISCONNECT);
	fprintf(stdout, "------------------------------------------------------------------------"
		"------------------------------------------%s-------------------------------------------"
		"-----------------------------------------------------------------------------\n", 
		sGetCostTime(-1 * lTime));
}

/****************************************************************************************
	商户费率配置表	TBL_MCHT_ALGO
 ****************************************************************************************/
void	vTableView_MCHT_ALGO(char *pszModel)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbMchtAlgo		stMtAlog;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_ALGO;
	if(pszModel)
	{
		memset(&stMtAlog, 0, sizeof(stMtAlog));
		strcpy(stMtAlog.model_id, pszModel);
		sTrimAll(stMtAlog.model_id);

		pstRun->lSize = sizeof(stMtAlog);
		pstRun->pstVoid = &stMtAlog;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_MCHT_INFO标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_MCHT_ALGO数据如下:\n");
	fprintf(stdout, "------------------------------------------------------------"
		"----------------\n");
	fprintf(stdout, "line\t模型编号  费率类型 周期类型 是否一一对应 状态 交易种类"
		" 商户组 卡类型\n");
	fprintf(stdout, "------------------------------------------------------------"
		"----------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stMtAlog, 0, sizeof(stMtAlog));	
		lRet = lTableFetch(TBL_MCHT_ALGO, &stMtAlog, sizeof(stMtAlog));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_MCHT_ALGO错误, 原因:%s", sGetError());
			vTableClose(TBL_MCHT_ALGO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stMtAlog.card_type);
		fprintf(stdout, "[%d]:\t%8s|%8s|%8s|%12s|%4s|%8s|%6s|%6d|\n", nRecord, 
			stMtAlog.model_id, stMtAlog.model_type, stMtAlog.cycle_type, stMtAlog.stlm_flag, 
			stMtAlog.model_flag, stMtAlog.trans_type, stMtAlog.mcht_grp, stMtAlog.card_type);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_MCHT_ALGO, SHM_DISCONNECT);
	fprintf(stdout, "--------------------------------%s------------------------------------\n", 
		sGetCostTime(-1 * lTime));
}

/****************************************************************************************
	查看手续费模型配置信息	TBL_PUB_DISC_ALGO
 ****************************************************************************************/
void	vTableView_PUB_DISC_ALGO(char *pszDiscId)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbDiscAlgo		stAlgo;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_PUB_DISC_ALGO;
	if(pszDiscId)
	{
		memset(&stAlgo, 0, sizeof(stAlgo));
		strcpy(stAlgo.disc_id, pszDiscId);
		sTrimAll(stAlgo.disc_id);

		pstRun->lSize = sizeof(stAlgo);
		pstRun->pstVoid = &stAlgo;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_PUB_DISC_ALGO标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_PUB_DISC_ALGO数据如下:\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	fprintf(stdout, "line\t序号\t   费用模型  索引\t保底费\t	 封顶费\t分段底线\t"
		"分段高线	费率模式\t	费率  卡类型\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stAlgo, 0, sizeof(stAlgo));	
		lRet = lTableFetch(TBL_PUB_DISC_ALGO, &stAlgo, sizeof(stAlgo));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_PUB_DISC_ALGO错误, 原因:%s", sGetError());
			vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stAlgo.id);
		lGetEXLong(&stAlgo.index_num);
		lGetEXLong(&stAlgo.flag);
		lGetEXLong(&stAlgo.card_type);
		fprintf(stdout, "[%d]:\t%10d|%8s|%4d|%12.2f|%13.2f|%15.2f|%15.2f|%8d|%12.2f|%6d|\n", 
			nRecord, stAlgo.id, stAlgo.disc_id, stAlgo.index_num, stAlgo.min_fee, stAlgo.max_fee, 
			stAlgo.floor_amount, stAlgo.upper_amount, stAlgo.flag, stAlgo.fee_value, stAlgo.card_type);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
	fprintf(stdout, "------------------------------------------------------"
		"-%s---------------------------------------------------------\n", 
		sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	查看商户用户信息
 ****************************************************************************************/
void	vTableView_MCHT_USER(char *pszMchtNo)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbMchtUser		stUser;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_USER;
	if(pszMchtNo)
	{
		memset(&stUser, 0, sizeof(stUser));
		strcpy(stUser.mcht_no, pszMchtNo);
		sTrimAll(stUser.mcht_no);

		pstRun->lSize = sizeof(stUser);
		pstRun->pstVoid = &stUser;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_MCHT_USER标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_MCHT_USER数据如下:\n");
	fprintf(stdout, "------------------------------------------------------------"
		"------------------------------------------------------------------------"
		"-------------------------------------\n");
	fprintf(stdout, "line\t用户编号\t商户号\t\t\t\t\t\t用户名称\t\t\t用户英文名\t用户手机号\t"
		"用户电子邮箱\t用户证件号码\n");
	fprintf(stdout, "------------------------------------------------------------"
		"------------------------------------------------------------------------"
		"-------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stUser, 0, sizeof(stUser));	
		lRet = lTableFetch(TBL_MCHT_USER, &stUser, sizeof(stUser));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_MCHT_USER错误, 原因:%s", sGetError());
			vTableClose(TBL_MCHT_USER, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stUser.user_id);
		fprintf(stdout, "[%d]:\t%10d|%15s|%50s|%30s|%11s|%20s|%18s|\n", nRecord, stUser.user_id, stUser.mcht_no, 
			stUser.user_name, stUser.user_engname, stUser.user_phone, stUser.user_email, stUser.user_card_no);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_MCHT_USER, SHM_DISCONNECT);
	fprintf(stdout, "---------------------------------------------------"
		"-------------------%s--------------------------------------------"
		"-----------------------------------------------\n", sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	查看商户路由表
 ****************************************************************************************/
void	vTableView_MCHT_CUPS_INF(char *pszCupsNo)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbMchtCupsInf	stCups;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	if(pszCupsNo)
	{
		memset(&stCups, 0, sizeof(stCups));
		strcpy(stCups.cups_no, pszCupsNo);
		sTrimAll(stCups.cups_no);

		pstRun->lSize = sizeof(stCups);
		pstRun->pstVoid = &stCups;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_MCHT_CUPS_INF标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_MCHT_CUPS_INF数据如下:\n");
	fprintf(stdout, "---------------------------------------------------------------------------------------"
		"--------------------------------------------\n");
	fprintf(stdout, "line\t序号 渠道编号\t\t渠道名称\t\t渠道商户号  渠道终端号\t\t费率\t\t封顶值\t\t渠道时间戳\n");
	fprintf(stdout, "---------------------------------------------------------------------------------------"
		"--------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stCups, 0, sizeof(stCups));	
		lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stCups, sizeof(stCups));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_MCHT_CUPS_INF错误, 原因:%s", sGetError());
			vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stCups.id);
		fprintf(stdout, "[%d]:\t%6d|%4s|%30s|%15s|%10s|%15.5f|%15.5f|%20s|\n", nRecord, 
			stCups.id, stCups.cups_no, stCups.cups_nm, stCups.mcht_cups_no, 
			stCups.term_cups_no, stCups.fee_rate, stCups.fee_val, stCups.cup_stamp);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------------------------------------------%s--------"
		"---------------------------------------------------------\n", sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	查看浦发商导信息
 ****************************************************************************************/
void	vTableView_N_MCHT_MATCH(char *pszCupsNo)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbNMchtMatch	stMatch;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_N_MCHT_MATCH;
	if(pszCupsNo)
	{
		memset(&stMatch, 0, sizeof(stMatch));
		strcpy(stMatch.cups_no, pszCupsNo);
		sTrimAll(stMatch.cups_no);

		pstRun->lSize = sizeof(stMatch);
		pstRun->pstVoid = &stMatch;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_N_MCHT_MATCH标失败, 原因:%s\n", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_N_MCHT_MATCH数据如下:\n");
	fprintf(stdout, "--------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------\n");
	fprintf(stdout, "line\t序号 渠道编号 导出状态 同步状态\t盒子商户号\t   归类商户号"
		"\t渠道商户号\t\t\t同步失败原因\t预留0\t预留1\t   费率\t  封顶值\t\t同步商户名称"
		"\t\t\t	   同步时间 商户组  MCC 同步费率名称\n");
	fprintf(stdout, "--------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stMatch, 0, sizeof(stMatch));	
		lRet = lTableFetch(TBL_N_MCHT_MATCH, &stMatch, sizeof(stMatch));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_N_MCHT_MATCH错误, 原因:%s", sGetError());
			vTableClose(TBL_N_MCHT_MATCH, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stMatch.id);
		lGetEXLong(&stMatch.resv_0);
		fprintf(stdout, "[%d]:\t%6d|%4s|%8s|%10s|%18s|%15s|%15s|%30s|%6d|%8s|%8.2f|%8.2f|%50s|%8s"
			"|%6s|%4s|%10s|\n", nRecord, stMatch.id, stMatch.cups_no, stMatch.mcht_stat, 
			stMatch.export_stat, stMatch.local_mcht_no, stMatch.cups_mcht_sort_no, stMatch.cups_mcht_no, 
			stMatch.reason, stMatch.resv_0, stMatch.resv_1, stMatch.fee_rate, stMatch.fee_val, 
			stMatch.mcht_nm, stMatch.export_date, stMatch.mcht_grp, stMatch.mcht_mcc, stMatch.mcht_disc_id);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_N_MCHT_MATCH, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------------------------------------------------------"
		"----------------------------------------------%s--------------------------------------"
		"----------------------------------------------------------------------------\n", 
		sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	服务-配置-表相关信息
 ****************************************************************************************/
void	vTableView_SERVICE_CONFIG(char *pszCode)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbSvceCfg		stCfg;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_SERVICE_CONFIG;
	if(pszCode)
	{
		memset(&stCfg, 0, sizeof(stCfg));
		strcpy(stCfg.service_code, pszCode);
		sTrimAll(stCfg.service_code);

		pstRun->lSize = sizeof(stCfg);
		pstRun->pstVoid = &stCfg;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_SERVICE_CONFIG标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_SERVICE_CONFIG数据如下:\n");
	fprintf(stdout, "----------------------------------------------------------"
		"----------------------------------------------------------------------"
		"--------------------------------------------------------\n");
	fprintf(stdout, "line\t	 id\t 服务代码\t\t\t服务名称\t\t	  服务版本 "
		"费率模型 试用优惠价 试用天数 计划开放时间 终止时间 任务状态 分润模型\t分润机构\t是否分润\n");
	fprintf(stdout, "----------------------------------------------------------"
		"----------------------------------------------------------------------"
		"--------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stCfg, 0, sizeof(stCfg));	
		lRet = lTableFetch(TBL_SERVICE_CONFIG, &stCfg, sizeof(stCfg));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_SERVICE_CONFIG错误, 原因:%s", sGetError());
			vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stCfg.id);
		lGetEXLong(&stCfg.prob_days);
		fprintf(stdout, "[%d]:\t%8d|%8s|%50s|%10s|%8s|%8s|%8d|%12s|%10s|%8s|%8s|%18s|%8s|\n", 
			nRecord, stCfg.id, stCfg.service_code, stCfg.service_name, stCfg.service_version, 
			stCfg.fee_rate, stCfg.prob_concess_rate, stCfg.prob_days, stCfg.plan_starttime, 
			stCfg.plan_endtime, stCfg.task_status, stCfg.profit_rate, stCfg.profit_brno, 
			stCfg.profile_flag);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------------------------------------------"
		"---------%s-------------------------------------------------------------"
		"------------------------------------------------\n", sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	打印索引信息
 ****************************************************************************************/
void	vTableView_CUPS_AMT(char *pszCupsNo)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	EXTCupAmt	stCupAmt;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_CUPS_AMT;
	if(pszCupsNo)
	{
		memset(&stCupAmt, 0, sizeof(stCupAmt));
		strcpy(stCupAmt.m_szCupsNo, pszCupsNo);
		sTrimAll(stCupAmt.m_szCupsNo);

		pstRun->lSize = sizeof(stCupAmt);
		pstRun->pstVoid = &stCupAmt;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_CUPS_AMT标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_CUPS_AMT数据如下:\n");
	fprintf(stdout, "-------------------------------------------------------------------------\n");
	fprintf(stdout, "line\t渠道编号\t对账日期\t交易金额\t渠道金额\n");
	fprintf(stdout, "-------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stCupAmt, 0, sizeof(stCupAmt));	
		lRet = lTableFetch(TBL_CUPS_AMT, &stCupAmt, sizeof(stCupAmt));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_SERVICE_CONFIG错误, 原因:%s", sGetError());
			vTableClose(TBL_CUPS_AMT, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		fprintf(stdout, "[%d]:\t%s|\t%s|\t%.2f|\t%.2f|\n", nRecord, stCupAmt.m_szCupsNo, 
			stCupAmt.m_szStlmDate, stCupAmt.m_dTxAmt, stCupAmt.m_dFeeAmt);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_CUPS_AMT, SHM_DISCONNECT);
	fprintf(stdout, "----------------------------%s-------------------------------------\n",
		sGetCostTime(-1 * lTime));
	//	0条记录 stderr比stdout优先级高，导致错乱
	fflush(stdout);
}

/****************************************************************************************
	查看手续费模型配置信息	TBL_DISC_ALGO
 ****************************************************************************************/
void	vTableView_DISC_ALGO(char *pszDiscId)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbDisc			stDisc;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_DISC_ALGO;
	if(pszDiscId)
	{
		memset(&stDisc, 0, sizeof(stDisc));
		strcpy(stDisc.disc_id, pszDiscId);
		sTrimAll(stDisc.disc_id);

		pstRun->lSize = sizeof(stDisc);
		pstRun->pstVoid = &stDisc;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_DISC_ALGO标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_DISC_ALGO数据如下:\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	fprintf(stdout, "line\t序号\t   费用模型  索引\t保底费\t	 封顶费\t分段底线\t"
		"分段高线	费率模式\t	费率  卡类型\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stDisc, 0, sizeof(stDisc));	
		lRet = lTableFetch(TBL_DISC_ALGO, &stDisc, sizeof(stDisc));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_DISC_ALGO错误, 原因:%s", sGetError());
			vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stDisc.id);
		lGetEXLong(&stDisc.index_num);
		lGetEXLong(&stDisc.flag);
		lGetEXLong(&stDisc.card_type);

		fprintf(stdout, "[%d]:\t%10d|%8s|%4d|%12.2f|%13.2f|%15.2f|%15.2f|%8d|%12.2f|%6d|\n",
			nRecord, stDisc.id, stDisc.disc_id, stDisc.index_num, stDisc.min_fee, stDisc.max_fee,
			stDisc.floor_amount, stDisc.upper_amount, stDisc.flag, stDisc.fee_value, stDisc.card_type);  
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
	fprintf(stdout, "------------------------------------------------------"
		"-%s----------------------------------------------------------\n", 
		sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	查看手续费模型配置信息	TBL_SERVICE_REGISTER
 ****************************************************************************************/
void	vTableView_SERVICE_REGISTER(char *pszKey)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbResigter		stRegist;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_SERVICE_REGISTER;
	if(pszKey)
	{
		memset(&stRegist, 0, sizeof(stRegist));
		strcpy(stRegist.service_object_id, pszKey);
		sTrimAll(stRegist.service_object_id);

		pstRun->lSize = sizeof(stRegist);
		pstRun->pstVoid = &stRegist;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_SERVICE_REGISTE标失败, 原因:%s", sGetError());
		return ;
	}

	fprintf(stdout, "表TBL_SERVICE_REGISTE数据如下:\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-------\n");
	fprintf(stdout, "line\t服务编号\t服务对象类型\t   服务对象编号\t	开通时间\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-------\n");
	lTime = lGetTiskTime();
	//	遍历所有记录
	while(1)
	{
		memset(&stRegist, 0, sizeof(stRegist));	
		lRet = lTableFetch(TBL_SERVICE_REGISTER, &stRegist, sizeof(stRegist));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_SERVICE_REGISTE错误, 原因:%s", sGetError());
			vTableClose(TBL_SERVICE_REGISTER, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stRegist.service_id);
		fprintf(stdout, "[%d]:\t%10d|%16s|%20s|%14s|\n", nRecord, stRegist.service_id, stRegist.service_object_type, 
			stRegist.service_object_id, stRegist.reg_datetime);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_SERVICE_REGISTER, SHM_DISCONNECT);
	fprintf(stdout, "------------------------------%s------------"
		"----------------------\n", sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	打印索引信息
 ****************************************************************************************/
void	vPrintHeadInfo()
{
	long	lRet = 0, i = 0, j = 0;
	TblKey	*pstKey = NULL;

	lRet = lInitMemTable();
	if(lRet != RC_SUCC)
	{
		fprintf(stderr, "初始化数据连接失败, 原因:%s\n", sGetError());
		exit(-1);
	}

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		fprintf(stdout, "表(%d)有效记录个数(%d)(%d), 偏移量(%d), 索引个数(%d)\n", i, ((TblDef *)pGetTblDef(i))->lValid, 
			lGetTblCount(i), ((TblDef *)pGetTblDef(i))->lOffSet, ((TblDef *)pGetTblDef(i))->m_lIdxNum);
		for(j = 0; j < ((TblDef *)pGetTblDef(i))->m_lIdxNum; j ++)
		{
			pstKey = (TblKey *)pGetTblIdx(i);
			if(IDX_SELECT == pstKey[j].m_lIsPk)
				fprintf(stdout, "主索引: 第%d个索引，起始位置(%d)索引长度(%d)索引属性(%d)\n", j, pstKey[j].m_lKeyOffSet, 
					pstKey[j].m_lKeyLen, pstKey[j].m_lKeyAttr);
			else if(CHK_SELECT == pstKey[j].m_lIsPk)
				fprintf(stdout, "从索引: 第%d个索引，起始位置(%d)索引长度(%d)索引属性(%d)\n", j, pstKey[j].m_lKeyOffSet, 
					pstKey[j].m_lKeyLen, pstKey[j].m_lKeyAttr);
			else
			{
				fprintf(stderr, "错误的索引类型(%d)!!\n", pstKey[j].m_lIsPk);
				return ;
			}
		}
	}

	vDisConnect();
}

/****************************************************************************************
	查看商户基本信息
 ****************************************************************************************/
long	lGetMchtNo(char *pszFOut, char *pszEOut)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbMchtBase		stBase;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_BASE;
	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "打开表TBL_MCHT_BASE标失败, 原因:%s", sGetError());
		return RC_FAIL;
	}
	
	while(1)
	{
		memset(&stBase, 0, sizeof(stBase));	
		lRet = lTableFetch(TBL_MCHT_BASE, &stBase, sizeof(stBase));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "查询表TBL_MCHT_BASE错误, 原因:%s", sGetError());
			vTableClose(TBL_MCHT_BASE, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		if(1 == nRecord)
			strcpy(pszFOut, stBase.mcht_no);
		else
			strcpy(pszEOut, stBase.mcht_no);

		lGetEXLong(&stBase.id);
		lGetEXLong(&stBase.mcht_resv1);
		lGetEXLong(&stBase.mcht_expand);
	}
	vTableClose(TBL_MCHT_BASE, SHM_DISCONNECT);
	return RC_SUCC;
}

/****************************************************************************************
	table 
 ****************************************************************************************/
long	lSelectBase(dbMchtBase *pstMcht, char *pszMchtNo)
{
	long		lRet = 0;
	dbMchtBase  stDbMcht;
	RunTime	 *pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszMchtNo);
	memset(&stDbMcht, 0, sizeof(stDbMcht));
	strcpy(stDbMcht.mcht_no, pszMchtNo);
	sTrimAll(stDbMcht.mcht_no);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_BASE;
	pstRun->lSize = sizeof(stDbMcht);
	pstRun->pstVoid = &stDbMcht;
	lRet = lSelectSpeed(pstRun, pstMcht);
//	pstRun->pstVoid = stDbMcht.mcht_no;
//	lRet = lQuerySpeed(pstRun, pstMcht);
	if(lRet !=  RC_SUCC)
		return lRet;
	
	lGetEXLong(&pstMcht->id);
	lGetEXLong(&pstMcht->mcht_resv1);
	lGetEXLong(&pstMcht->mcht_expand);

	return RC_SUCC;
}

/****************************************************************************************
	table 
 ****************************************************************************************/
void	vPrintModeTest()
{
	long	lRet;
	double	dTime = 0, dEnd = 0, dRes = 0;
	char	szFMchtNo[50], szEMchtNo[50];
	dbMchtBase	stMcht;

	fprintf(stdout, "------------------------------进入test模式---------------------------------\n");
	fprintf(stdout, "测试表商户基本信息表:TBL_MCHT_BASE\n");
	fprintf(stdout, "测试内容，取内存表的第一条数据和最后一条数据查询花费时间进行比较.\n");
	memset(szFMchtNo, 0, sizeof(szFMchtNo));
	memset(szEMchtNo, 0, sizeof(szEMchtNo));
	lRet = lGetMchtNo(szFMchtNo, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "查询错误, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	fprintf(stdout, "测试表记录总条数:%d\n", lGetTblValid(TBL_MCHT_BASE));
	fprintf(stdout, "准备测试查询表中第一个商户(%s)和最后一个商户(%s)\n", szFMchtNo, szEMchtNo);

	memset(&stMcht, 0, sizeof(stMcht));
	dTime = dGetUSecTime();
	lRet = lSelectBase(&stMcht, szFMchtNo);
	if(lRet)
	{
		fprintf(stdout, "查询错误, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dTime -= dGetUSecTime();
	fprintf(stdout, "第一个商户(%s), 查询消耗花费时间:%.3fms\n", szFMchtNo, -1 * dTime);

	memset(&stMcht, 0, sizeof(stMcht));
	dEnd = dGetUSecTime();
	lRet = lSelectBase(&stMcht, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "查询错误, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dEnd -= dGetUSecTime();
	fprintf(stdout, "最后个商户(%s), 查询消耗花费时间:%.3fms\n", szEMchtNo, -1 * dEnd);
	fprintf(stdout, "查询第一条和最后条时间差:%.3fms\n", (-1 * dEnd) - (-1 * dTime));

	fprintf(stdout, "\n采用AVL快速查询方式查询第一条和最后一条数据.\n");
	dTime = dGetUSecTime();
	lRet = lGetMchtBase(&stMcht, szFMchtNo);
	if(lRet)
	{
		fprintf(stdout, "查询错误, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dTime -= dGetUSecTime();
	fprintf(stdout, "第一个商户(%s), 查询消耗花费时间:%.3fms\n", szFMchtNo, -1 * dTime);

	memset(&stMcht, 0, sizeof(stMcht));
	dEnd = dGetUSecTime();
	lRet = lGetMchtBase(&stMcht, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "查询错误, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dEnd -= dGetUSecTime();
	fprintf(stdout, "最后个商户(%s), 查询消耗花费时间:%.3fms\n", szEMchtNo, -1 * dEnd);
	fprintf(stdout, "查询第一条和最后条时间差:%.3fms\n", (-1 * dEnd) - (-1 * dTime));

	return ;
}
/****************************************************************************************
	table 
 ****************************************************************************************/
void	vPrintTable()
{
//	fprintf(stderr, "Param err:\n\tUsage: vtable table\n");
	fprintf(stderr, "\ntable value must be the follow one:\n");
	fprintf(stderr, "\t0\t--TBL_REGION_CODE\n");  
	fprintf(stderr, "\t1\t--TBL_BRH_INFO\n");	 
	fprintf(stderr, "\t2\t--TBL_BRH_PROFIT_INFO\n"); 
	fprintf(stderr, "\t3\t--TBL_MCHT_MCC_INF\n");	
	fprintf(stderr, "\t4\t--TBL_BRH_PROFIT_DEF\n");	
	fprintf(stderr, "\t5\t--TBL_BRH_PROFIT_ALGO\n");	
	fprintf(stderr, "\t6\t--TBL_ACCT_INFO\n");	
	fprintf(stderr, "\t7\t--TBL_OPR_INFO\n");	 
	fprintf(stderr, "\t8\t--TBL_SPDB_BRH\n");	
	fprintf(stderr, "\t9\t--TBL_MCHT_BASE\n"); 
	fprintf(stderr, "\t10\t--TBL_MCHT_ALGO\n");
	fprintf(stderr, "\t11\t--TBL_PUB_DISC_ALGO\n");
	fprintf(stderr, "\t12\t--TBL_MCHT_USER\n");
	fprintf(stderr, "\t13\t--TBL_MCHT_CUPS_INF\n");
	fprintf(stderr, "\t14\t--TBL_N_MCHT_MATCH\n");
	fprintf(stderr, "\t15\t--TBL_SERVICE_CONFIG\n");
	fprintf(stderr, "\t16\t--TBL_DISC_ALGO\n");
	fprintf(stderr, "\t17\t--TBL_SERVICE_REGISTER\n");
	fprintf(stderr, "\t18\t--TBL_CUPS_AMT\n");
}

TblPrint	g_TblePrint[SHM_TABLE_NUM] = {
	{"请输入地区编号:", &vTableView_REGION_CODE},
	{"请输入机构编号:", &vTableView_BRH_INFO},
	{"请输入机构分润主体表模型编号:", &vTableView_BRH_PROFIT_INFO},
	{"请输入MCC编号:", &vTableView_MCHT_MCC_INF},
	{"请输入机构分润定义表模型编号:", &vTableView_BRH_PROFIT_DEF},
	{"请输入机构分润配置表模型编号:", &vTableView_BRH_PROFIT_ALGO},
	{"请输入账号所属机构或商户编号:", &vTableView_ACCT_INFO},
	{"请输入操作员ID:", &vTableView_OPR_INFO},
	{"请输入地区编号:", &vTableView_SPDB_BRH},
	{"请输入商户编号:", &vTableView_MCHT_BASE},
	{"请输入费率配置编号:", &vTableView_MCHT_ALGO},
	{"请输入费用模型配置编号:", &vTableView_PUB_DISC_ALGO},
	{"请输入商户编号:", &vTableView_MCHT_USER},
	{"请输入渠道编号:", &vTableView_MCHT_CUPS_INF},
	{"请输入渠道编号:", &vTableView_N_MCHT_MATCH},
	{"请输入服务代码编号:", &vTableView_SERVICE_CONFIG},
	{"请输入服务费率编号:", &vTableView_DISC_ALGO},
	{"请输入服务费率注册编号:", &vTableView_SERVICE_REGISTER},
	{"请输入渠道编号:", &vTableView_CUPS_AMT},
};

/****************************************************************************************
	输入检测
 ****************************************************************************************/
void	vGetInput(char *pszPut, long l, long m)
{
	long	n = 0, len;

	for(n = 0; n < l; n ++)
	{
		pszPut[n] = getchar();	
		if(10 == pszPut[n])
		{
			pszPut[n] = 0x00;
			if(1 == m) return ;

			if(!strlen(pszPut))	//	非1模式
				n = -1;			//	重新等待输入
			else
				return ;
		}
	}
}

/****************************************************************************************
	main
 ****************************************************************************************/
int		main(int argc, char** argv)
{
	long	lRet = 0;
	TABLE	table;
	char	szTemp[500], chTable;

	vSetLogName("vtable.log");
	if(2 == argc && !strcmp(argv[1], "test"))
	{
		vPrintModeTest();
		return RC_SUCC;
	}
	else if(3 == argc)
	{
		table = atol(argv[1]);
		if(RC_SUCC != lTblIsNExist(table))
		{
			fprintf(stderr, "输入错误:%s\n", sGetError());
			return RC_FAIL;
		}
	
		g_TblePrint[table].pfTable(argv[2]);
		return RC_SUCC;
	}
	while(1)
	{
		vPrintTable();
		fprintf(stdout, "请输入查询表序号:");	
		memset(szTemp, 0, sizeof(szTemp));
		vGetInput(szTemp, sizeof(szTemp) - 1, 0);
		if(!strcmp(szTemp, "q") || !strcmp(szTemp, "Q") || !strcmp(szTemp, "exit"))
			exit(0);
		else if(!strcmp(szTemp, "P") || !strcmp(szTemp, "p"))
		{
			vPrintHeadInfo();
			continue;
		}

		if('0' > szTemp[0] || szTemp[0] > '9')
		{
			fprintf(stdout, "无效的表序号(%s)\n", szTemp);
			continue;
		}

		table = atol(szTemp);
		if(RC_SUCC != lTblIsNExist(table))
		{
			fprintf(stderr, "输入错误:%s\n", sGetError());
			continue;
		}

		memset(szTemp, 0, sizeof(szTemp));
		fprintf(stdout, "%s", g_TblePrint[table].szDsc);
		fflush(stdin);
		vGetInput(szTemp, sizeof(szTemp) - 1, 1);
		if(!strcmp(szTemp, "q") || !strcmp(szTemp, "Q") || !strcmp(szTemp, "exit"))
			exit(0);

		if(!memcmp(szTemp, "NULL", 4) || !memcmp(szTemp, "null", 4))
			g_TblePrint[table].pfTable(NULL);
		else
			g_TblePrint[table].pfTable(szTemp);
	}
	return RC_SUCC;
}
