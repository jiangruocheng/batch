#include	"load.h"

#define	ZCHAR(x)	(x == 0x00)?(x = ' '):x

typedef struct __TBLPRINT
{
	char	szDsc[200];
	void	(*pfTable)(char *pszTemp);
}TblPrint;

/****************************************************************************************
	��ȡ������
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
	��ȡʱ�䣬��ȷ����
 ****************************************************************************************/
long	lGetTiskTime()
{
	long	lTime;
	struct timeval	t;
	struct tm	*ptm;

	gettimeofday(&t, NULL);
	ptm = localtime(&t.tv_sec);

	lTime = ptm->tm_min * 60 * 1000 + ptm->tm_sec * 1000 + t.tv_usec / 1000;	//΢�루����1000���Ǻ���)
	return lTime;
}

/****************************************************************************************
	��ȡ��ѯʱ���ַ���
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
	���ӹ����ڴ沢��ʼ����Ҫ����
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
		fprintf(stderr, "�򿪱�TBL_REGION_CODE�α�ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_REGION_CODE��������:\n", stReg.region_code);
	fprintf(stdout, "---------------------------------------------------------------------\n");
	fprintf(stdout, "line\tid\t��������\t��������\t\n");
	fprintf(stdout, "---------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stReg, 0, sizeof(stReg));	
		lRet = lTableFetch(TBL_REGION_CODE, &stReg, sizeof(stReg));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_REGION_CODE����!(%s), ԭ��:%s", stReg.region_code, sGetError());
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
	�鿴�ڴ������Ϣ��������Ϣ	
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
		fprintf(stderr, "�򿪱�TBL_BRH_INFO��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_BRH_INFO��������:\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"----------------------------------------\n");
	fprintf(stdout, "line\t   ���\t	 �����ȼ�\t\t������\t\t\t\t\t��������\t\t\t\t   "
		" �Ƿ�������\t���󷽰�   ���������ʾ\t ���������ļ�\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"----------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stBrh, 0, sizeof(stBrh));	
		lRet = lTableFetch(TBL_BRH_INFO, &stBrh, sizeof(stBrh));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_BRH_INFO����, ԭ��:%s", sGetError());
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
	�鿴MCC��Ϣ
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
		fprintf(stderr, "�򿪱�TBL_BRH_PROFIT_INFO��ʧ��, ԭ��:%s", sGetError());
		return ;
	}
	fprintf(stdout, "��TBL_BRH_PROFIT_INFO��������:\n");
	fprintf(stdout, "-------------------------------------------------------------------\n");
	fprintf(stdout, "line\tģ�ͱ��\t������ģ��\tֱ����ģ��\t������ģ��\n");
	fprintf(stdout, "-------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stPfitIf, 0, sizeof(stPfitIf));	
		lRet = lTableFetch(TBL_BRH_PROFIT_INFO, &stPfitIf, sizeof(stPfitIf));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_BRH_PROFIT_INFO����, ԭ��:%s", sGetError());
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
	�鿴MCC��Ϣ
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
		fprintf(stderr, "�򿪱�TBL_MCHT_MCC_INF�α�ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_MCHT_MCC_INF��������:\n");
	fprintf(stdout, "------------------------------\n");
	fprintf(stdout, "line\t���\tMCC��\tMCC���\t\n");
	fprintf(stdout, "------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stMcc, 0, sizeof(stMcc));	
		lRet = lTableFetch(TBL_MCHT_MCC_INF, &stMcc, sizeof(stMcc));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_MCHT_MCC_INF����, ԭ��:%s", sGetError());
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
	4.2.3.	����������� TBL_BRH_PROFIT_DEF
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
		fprintf(stderr, "�򿪱�TBL_BRH_PROFIT_DEF��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_BRH_PROFIT_DEF��������:\n");
	fprintf(stdout, "----------------------------------------------------------------------------------\n");
	fprintf(stdout, "line\tģ�ͱ��\tģ����������\t\t״̬\t�Ƿ�ͳһ����\t�������\n");
	fprintf(stdout, "----------------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stDbPfitDef, 0, sizeof(stDbPfitDef));	
		lRet = lTableFetch(TBL_BRH_PROFIT_DEF, &stDbPfitDef, sizeof(stDbPfitDef));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_BRH_PROFIT_DEF����, ԭ��:%s", sGetError());
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
	�����������ñ�	TBL_BRH_PROFIT_ALGO
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
		fprintf(stderr, "�򿪱�TBL_BRH_PROFIT_ALGO��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_BRH_PROFIT_ALGO��������:\n");
	fprintf(stdout, "-------------------------------------------------------------\n");
	fprintf(stdout, "line\tģ�ͱ��	�������� �̻��� ������ ��׼��   �������\n");
	fprintf(stdout, "-------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stBrhAlgo, 0, sizeof(stBrhAlgo));	
		lRet = lTableFetch(TBL_BRH_PROFIT_ALGO, &stBrhAlgo, sizeof(stBrhAlgo));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_BRH_PROFIT_ALGO����, ԭ��:%s", sGetError());
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
	�鿴�̻��˺���Ϣ
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
		fprintf(stderr, "�򿪱�TBL_ACCT_INFO��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_ACCT_INFO��������:\n");
	fprintf(stdout, "-------------------------------------------------------------------"
		"-------------------------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"-------------------------------------\n");
	fprintf(stdout, "line\t�˺���� Ӧ������  Ӧ�ñ�ʾ��   ���� �Ƿ�Ĭ��\t�˻��˺�\t"
		"\t\t�˻�����\t   �˻������к�\t	�˻�����������  �˻�����֧�к�\t   �˻�����֧������"
		"\t\t\t\t����֧�е�ַ\t֧�е����� ����֧���� �˻����к�\n");
	fprintf(stdout, "-------------------------------------------------------------------"
		"-------------------------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"-------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stAct, 0, sizeof(stAct));	
		lRet = lTableFetch(TBL_ACCT_INFO, &stAct, sizeof(stAct));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_ACCT_INFO����, ԭ��:%s", sGetError());
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
	���㸶���˺Ų�ѯ
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
		fprintf(stderr, "�򿪱�TBL_OPR_INFO�α�ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_OPR_INFO��������:\n");
	fprintf(stdout, "----------------------------------------------------"
		"-----------------------------\n");
	fprintf(stdout, "line\t����Ա���\t\t��¼��\t\t\t\t����Ա����\n");
	fprintf(stdout, "----------------------------------------------------"
		"-----------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stOpr, 0, sizeof(stOpr));	
		lRet = lTableFetch(TBL_OPR_INFO, &stOpr, sizeof(stOpr));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_OPR_INFO����, ԭ��:%s", sGetError());
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
	�鿴�ַ�������Ϣ
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
		fprintf(stderr, "�򿪱�TBL_SPDB_BRH��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_SPDB_BRH��������:\n");
	fprintf(stdout, "-----------------------------------------------------------------\n");
	fprintf(stdout, "line\t���\t\t\t\t\t������\t��������\n");
	fprintf(stdout, "-----------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stSpdb, 0, sizeof(stSpdb));	
		lRet = lTableFetch(TBL_SPDB_BRH, &stSpdb, sizeof(stSpdb));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_SPDB_BRH����, ԭ��:%s", sGetError());
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
	�鿴�̻�������Ϣ
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
		fprintf(stderr, "�򿪱�TBL_MCHT_BASE��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_MCHT_BASE��������:\n");
	fprintf(stdout, "------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------\n");
	fprintf(stdout, "line\t���\t\t�̻���\t\t\t\t�̻�����\t   ״̬ �̻�����   �ϼ��̻���"
		" �������\t\t\t\t�̻���ַ\tMCC��\t����������\t ��չԱ\t\t�������	  T+0����"
		" ����ģ�� ������ʾ\t�������̻�\n");
	fprintf(stdout, "------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();	
	//	�������м�¼
	while(1)
	{
		memset(&stBase, 0, sizeof(stBase));	
		lRet = lTableFetch(TBL_MCHT_BASE, &stBase, sizeof(stBase));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_MCHT_BASE����, ԭ��:%s", sGetError());
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
	�̻��������ñ�	TBL_MCHT_ALGO
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
		fprintf(stderr, "�򿪱�TBL_MCHT_INFO��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_MCHT_ALGO��������:\n");
	fprintf(stdout, "------------------------------------------------------------"
		"----------------\n");
	fprintf(stdout, "line\tģ�ͱ��  �������� �������� �Ƿ�һһ��Ӧ ״̬ ��������"
		" �̻��� ������\n");
	fprintf(stdout, "------------------------------------------------------------"
		"----------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stMtAlog, 0, sizeof(stMtAlog));	
		lRet = lTableFetch(TBL_MCHT_ALGO, &stMtAlog, sizeof(stMtAlog));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_MCHT_ALGO����, ԭ��:%s", sGetError());
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
	�鿴������ģ��������Ϣ	TBL_PUB_DISC_ALGO
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
		fprintf(stderr, "�򿪱�TBL_PUB_DISC_ALGO��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_PUB_DISC_ALGO��������:\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	fprintf(stdout, "line\t���\t   ����ģ��  ����\t���׷�\t	 �ⶥ��\t�ֶε���\t"
		"�ֶθ���	����ģʽ\t	����  ������\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stAlgo, 0, sizeof(stAlgo));	
		lRet = lTableFetch(TBL_PUB_DISC_ALGO, &stAlgo, sizeof(stAlgo));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_PUB_DISC_ALGO����, ԭ��:%s", sGetError());
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
	�鿴�̻��û���Ϣ
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
		fprintf(stderr, "�򿪱�TBL_MCHT_USER��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_MCHT_USER��������:\n");
	fprintf(stdout, "------------------------------------------------------------"
		"------------------------------------------------------------------------"
		"-------------------------------------\n");
	fprintf(stdout, "line\t�û����\t�̻���\t\t\t\t\t\t�û�����\t\t\t�û�Ӣ����\t�û��ֻ���\t"
		"�û���������\t�û�֤������\n");
	fprintf(stdout, "------------------------------------------------------------"
		"------------------------------------------------------------------------"
		"-------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stUser, 0, sizeof(stUser));	
		lRet = lTableFetch(TBL_MCHT_USER, &stUser, sizeof(stUser));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_MCHT_USER����, ԭ��:%s", sGetError());
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
	�鿴�̻�·�ɱ�
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
		fprintf(stderr, "�򿪱�TBL_MCHT_CUPS_INF��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_MCHT_CUPS_INF��������:\n");
	fprintf(stdout, "---------------------------------------------------------------------------------------"
		"--------------------------------------------\n");
	fprintf(stdout, "line\t��� �������\t\t��������\t\t�����̻���  �����ն˺�\t\t����\t\t�ⶥֵ\t\t����ʱ���\n");
	fprintf(stdout, "---------------------------------------------------------------------------------------"
		"--------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stCups, 0, sizeof(stCups));	
		lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stCups, sizeof(stCups));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_MCHT_CUPS_INF����, ԭ��:%s", sGetError());
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
	�鿴�ַ��̵���Ϣ
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
		fprintf(stderr, "�򿪱�TBL_N_MCHT_MATCH��ʧ��, ԭ��:%s\n", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_N_MCHT_MATCH��������:\n");
	fprintf(stdout, "--------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------\n");
	fprintf(stdout, "line\t��� ������� ����״̬ ͬ��״̬\t�����̻���\t   �����̻���"
		"\t�����̻���\t\t\tͬ��ʧ��ԭ��\tԤ��0\tԤ��1\t   ����\t  �ⶥֵ\t\tͬ���̻�����"
		"\t\t\t	   ͬ��ʱ�� �̻���  MCC ͬ����������\n");
	fprintf(stdout, "--------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------------"
		"-------------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stMatch, 0, sizeof(stMatch));	
		lRet = lTableFetch(TBL_N_MCHT_MATCH, &stMatch, sizeof(stMatch));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_N_MCHT_MATCH����, ԭ��:%s", sGetError());
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
	����-����-�������Ϣ
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
		fprintf(stderr, "�򿪱�TBL_SERVICE_CONFIG��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_SERVICE_CONFIG��������:\n");
	fprintf(stdout, "----------------------------------------------------------"
		"----------------------------------------------------------------------"
		"--------------------------------------------------------\n");
	fprintf(stdout, "line\t	 id\t �������\t\t\t��������\t\t	  ����汾 "
		"����ģ�� �����Żݼ� �������� �ƻ�����ʱ�� ��ֹʱ�� ����״̬ ����ģ��\t�������\t�Ƿ����\n");
	fprintf(stdout, "----------------------------------------------------------"
		"----------------------------------------------------------------------"
		"--------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stCfg, 0, sizeof(stCfg));	
		lRet = lTableFetch(TBL_SERVICE_CONFIG, &stCfg, sizeof(stCfg));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_SERVICE_CONFIG����, ԭ��:%s", sGetError());
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
	��ӡ������Ϣ
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
		fprintf(stderr, "�򿪱�TBL_CUPS_AMT��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_CUPS_AMT��������:\n");
	fprintf(stdout, "-------------------------------------------------------------------------\n");
	fprintf(stdout, "line\t�������\t��������\t���׽��\t�������\n");
	fprintf(stdout, "-------------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stCupAmt, 0, sizeof(stCupAmt));	
		lRet = lTableFetch(TBL_CUPS_AMT, &stCupAmt, sizeof(stCupAmt));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_SERVICE_CONFIG����, ԭ��:%s", sGetError());
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
	//	0����¼ stderr��stdout���ȼ��ߣ����´���
	fflush(stdout);
}

/****************************************************************************************
	�鿴������ģ��������Ϣ	TBL_DISC_ALGO
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
		fprintf(stderr, "�򿪱�TBL_DISC_ALGO��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_DISC_ALGO��������:\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	fprintf(stdout, "line\t���\t   ����ģ��  ����\t���׷�\t	 �ⶥ��\t�ֶε���\t"
		"�ֶθ���	����ģʽ\t	����  ������\n");
	fprintf(stdout, "-----------------------------------------------------------"
		"--------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stDisc, 0, sizeof(stDisc));	
		lRet = lTableFetch(TBL_DISC_ALGO, &stDisc, sizeof(stDisc));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_DISC_ALGO����, ԭ��:%s", sGetError());
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
	�鿴������ģ��������Ϣ	TBL_SERVICE_REGISTER
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
		fprintf(stderr, "�򿪱�TBL_SERVICE_REGISTE��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_SERVICE_REGISTE��������:\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-------\n");
	fprintf(stdout, "line\t������\t�����������\t   ���������\t	��ͨʱ��\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"-------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stRegist, 0, sizeof(stRegist));	
		lRet = lTableFetch(TBL_SERVICE_REGISTER, &stRegist, sizeof(stRegist));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_SERVICE_REGISTE����, ԭ��:%s", sGetError());
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
	��ӡ������Ϣ
 ****************************************************************************************/
void	vPrintHeadInfo()
{
	long	lRet = 0, i = 0, j = 0;
	TblKey	*pstKey = NULL;

	lRet = lInitMemTable();
	if(lRet != RC_SUCC)
	{
		fprintf(stderr, "��ʼ����������ʧ��, ԭ��:%s\n", sGetError());
		exit(-1);
	}

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		fprintf(stdout, "��(%d)��Ч��¼����(%d)(%d), ƫ����(%d), ��������(%d)\n", i, ((TblDef *)pGetTblDef(i))->lValid, 
			lGetTblCount(i), ((TblDef *)pGetTblDef(i))->lOffSet, ((TblDef *)pGetTblDef(i))->m_lIdxNum);
		for(j = 0; j < ((TblDef *)pGetTblDef(i))->m_lIdxNum; j ++)
		{
			pstKey = (TblKey *)pGetTblIdx(i);
			if(IDX_SELECT == pstKey[j].m_lIsPk)
				fprintf(stdout, "������: ��%d����������ʼλ��(%d)��������(%d)��������(%d)\n", j, pstKey[j].m_lKeyOffSet, 
					pstKey[j].m_lKeyLen, pstKey[j].m_lKeyAttr);
			else if(CHK_SELECT == pstKey[j].m_lIsPk)
				fprintf(stdout, "������: ��%d����������ʼλ��(%d)��������(%d)��������(%d)\n", j, pstKey[j].m_lKeyOffSet, 
					pstKey[j].m_lKeyLen, pstKey[j].m_lKeyAttr);
			else
			{
				fprintf(stderr, "�������������(%d)!!\n", pstKey[j].m_lIsPk);
				return ;
			}
		}
	}

	vDisConnect();
}

/****************************************************************************************
	�鿴�̻�������Ϣ
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
		fprintf(stderr, "�򿪱�TBL_MCHT_BASE��ʧ��, ԭ��:%s", sGetError());
		return RC_FAIL;
	}
	
	while(1)
	{
		memset(&stBase, 0, sizeof(stBase));	
		lRet = lTableFetch(TBL_MCHT_BASE, &stBase, sizeof(stBase));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_MCHT_BASE����, ԭ��:%s", sGetError());
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

	fprintf(stdout, "------------------------------����testģʽ---------------------------------\n");
	fprintf(stdout, "���Ա��̻�������Ϣ��:TBL_MCHT_BASE\n");
	fprintf(stdout, "�������ݣ�ȡ�ڴ��ĵ�һ�����ݺ����һ�����ݲ�ѯ����ʱ����бȽ�.\n");
	memset(szFMchtNo, 0, sizeof(szFMchtNo));
	memset(szEMchtNo, 0, sizeof(szEMchtNo));
	lRet = lGetMchtNo(szFMchtNo, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	fprintf(stdout, "���Ա��¼������:%d\n", lGetTblValid(TBL_MCHT_BASE));
	fprintf(stdout, "׼�����Բ�ѯ���е�һ���̻�(%s)�����һ���̻�(%s)\n", szFMchtNo, szEMchtNo);

	memset(&stMcht, 0, sizeof(stMcht));
	dTime = dGetUSecTime();
	lRet = lSelectBase(&stMcht, szFMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dTime -= dGetUSecTime();
	fprintf(stdout, "��һ���̻�(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szFMchtNo, -1 * dTime);

	memset(&stMcht, 0, sizeof(stMcht));
	dEnd = dGetUSecTime();
	lRet = lSelectBase(&stMcht, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dEnd -= dGetUSecTime();
	fprintf(stdout, "�����̻�(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szEMchtNo, -1 * dEnd);
	fprintf(stdout, "��ѯ��һ���������ʱ���:%.3fms\n", (-1 * dEnd) - (-1 * dTime));

	fprintf(stdout, "\n����AVL���ٲ�ѯ��ʽ��ѯ��һ�������һ������.\n");
	dTime = dGetUSecTime();
	lRet = lGetMchtBase(&stMcht, szFMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dTime -= dGetUSecTime();
	fprintf(stdout, "��һ���̻�(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szFMchtNo, -1 * dTime);

	memset(&stMcht, 0, sizeof(stMcht));
	dEnd = dGetUSecTime();
	lRet = lGetMchtBase(&stMcht, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dEnd -= dGetUSecTime();
	fprintf(stdout, "�����̻�(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szEMchtNo, -1 * dEnd);
	fprintf(stdout, "��ѯ��һ���������ʱ���:%.3fms\n", (-1 * dEnd) - (-1 * dTime));

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
	{"������������:", &vTableView_REGION_CODE},
	{"������������:", &vTableView_BRH_INFO},
	{"������������������ģ�ͱ��:", &vTableView_BRH_PROFIT_INFO},
	{"������MCC���:", &vTableView_MCHT_MCC_INF},
	{"����������������ģ�ͱ��:", &vTableView_BRH_PROFIT_DEF},
	{"����������������ñ�ģ�ͱ��:", &vTableView_BRH_PROFIT_ALGO},
	{"�������˺������������̻����:", &vTableView_ACCT_INFO},
	{"���������ԱID:", &vTableView_OPR_INFO},
	{"������������:", &vTableView_SPDB_BRH},
	{"�������̻����:", &vTableView_MCHT_BASE},
	{"������������ñ��:", &vTableView_MCHT_ALGO},
	{"���������ģ�����ñ��:", &vTableView_PUB_DISC_ALGO},
	{"�������̻����:", &vTableView_MCHT_USER},
	{"�������������:", &vTableView_MCHT_CUPS_INF},
	{"�������������:", &vTableView_N_MCHT_MATCH},
	{"��������������:", &vTableView_SERVICE_CONFIG},
	{"�����������ʱ��:", &vTableView_DISC_ALGO},
	{"������������ע����:", &vTableView_SERVICE_REGISTER},
	{"�������������:", &vTableView_CUPS_AMT},
};

/****************************************************************************************
	������
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

			if(!strlen(pszPut))	//	��1ģʽ
				n = -1;			//	���µȴ�����
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
			fprintf(stderr, "�������:%s\n", sGetError());
			return RC_FAIL;
		}
	
		g_TblePrint[table].pfTable(argv[2]);
		return RC_SUCC;
	}
	while(1)
	{
		vPrintTable();
		fprintf(stdout, "�������ѯ�����:");	
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
			fprintf(stdout, "��Ч�ı����(%s)\n", szTemp);
			continue;
		}

		table = atol(szTemp);
		if(RC_SUCC != lTblIsNExist(table))
		{
			fprintf(stderr, "�������:%s\n", sGetError());
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
