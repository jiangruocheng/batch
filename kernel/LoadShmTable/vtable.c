#include	"Ibp.h"
#include	"load.h"

#define	ZCHAR(x)	(x == 0x00)?(x = ' '):x

typedef struct __TBLPRINT
{
	char	szDsc[200];
	void	(*pfTable)(TABLE t, char *pszTemp);
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
	����ϵͳ������ڵ�
 ****************************************************************************************/
void	vPrintSysIdx()
{
	BSysIdx	stSys;
	long	lRet = 0, nRecord = 0, lTime = 0;
	RunTime	*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_TABLE_IDX)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(SYS_TABLE_IDX),
			 sGetError());
		return ;
	}

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = SYS_TABLE_IDX;
	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "�򿪱�SYS_TABLE_IDX�α�ʧ��, ԭ��:%s\n", sGetError());
		return ;
	}

	fprintf(stdout, "------------------------------------------------------------------------"
		"------------------------------------------------------------------------------------"
		"------------------\n");
	fprintf(stdout, "line\t ���\t  ������	�����\t\t������\t\t��Keyֵ  ������ID	 �ź�ID	"
		"����PID\t	 ����	 ��Ч��¼   ���֧��..  truck�д�С\t����ʱ��\n");
	fprintf(stdout, "------------------------------------------------------------------------"
		"------------------------------------------------------------------------------------"
		"------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stSys, 0, sizeof(stSys));	
		lRet = lTableFetch(SYS_TABLE_IDX, &stSys, sizeof(stSys));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��SYS_TABLE_IDX����!, ԭ��:%s", sGetError());
			vTableClose(SYS_TABLE_IDX, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong((long *)&stSys.m_lId);
		lGetEXLong((long *)&stSys.m_table);
		lGetEXLong((long *)&stSys.m_lType);
		lGetEXLong((long *)&stSys.m_yKey);
		lGetEXLong((long *)&stSys.m_ShmId);
		lGetEXLong((long *)&stSys.m_SemId);
		lGetEXLong((long *)&stSys.m_lPid);
		lGetEXLong((long *)&stSys.m_bAttch);
		lGetEXLong((long *)&stSys.m_lRecord);
		lGetEXLong((long *)&stSys.m_lMaxRows);
		lGetEXLong((long *)&stSys.m_lLSize);
		fprintf(stdout, "[%d]:\t%8d|%8d|%8d|%-25s|%10d|%10d|%10d|%10d|%10d|%10d|%12d|%12d|%20s|\n", 
			nRecord, stSys.m_lId, stSys.m_lType, stSys.m_table, stSys.m_szTable, stSys.m_yKey, 
			stSys.m_ShmId, stSys.m_SemId, stSys.m_lPid, stSys.m_bAttch, stSys.m_lRecord, 
			stSys.m_lMaxRows, stSys.m_lLSize, stSys.m_szTime);
	}
	lTime -= lGetTiskTime();	
	vTableClose(SYS_TABLE_IDX, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------------------------------------------------------"
		"-------%s---------------------------------------------------------------------------"
		"-------------\n", sGetCostTime(-1 * lTime));

	return ;
}

/****************************************************************************************
	��ӡ���ֶ�
 ****************************************************************************************/
void	vPrintFiled()
{
	BFieldDef	stField;
	long		lRet = 0, nRecord = 0, lTime = 0;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_FILED_DEF)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(SYS_FILED_DEF),
			 sGetError());
		return ;
	}

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = SYS_FILED_DEF;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "�򿪱�SYS_FILED_DEF�α�ʧ��, ԭ��:%s\n", sGetError());
		return ;
	}

	fprintf(stdout, "��SYS_FILED_DEF��������:\n");
	fprintf(stdout, "---------------------------------------------------------------------"
		"-------------------------------------------------\n");
	fprintf(stdout, "line\ttable\tOwner\t\tTableName\t\tSeq\t\tFieldName	KeyAttr  "
		"KeyOffSet\tKeyLen\tmark\n");
	fprintf(stdout, "---------------------------------------------------------------------"
		"-------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stField, 0, sizeof(stField));	
		lRet = lTableFetch(SYS_FILED_DEF, &stField, sizeof(stField));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��SYS_FILED_DEF����, ԭ��:%s", sGetError());
			vTableClose(SYS_FILED_DEF, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong((long *)&stField.m_table);
		lGetEXLong(&stField.m_lKeyAttr);
		lGetEXLong(&stField.m_lKeyOffSet);
		lGetEXLong(&stField.m_lKeyLen);
		lGetEXLong(&stField.m_lIsPk);
		lGetEXLong(&stField.m_lSeq);
		fprintf(stdout, "[%d]:\t%4d|%12s|%-25s|%8d|%20s|%8d|%8d|%8d|%8d|\n", nRecord, stField.m_table, 
			stField.m_szOwner, stField.m_szTable, stField.m_lSeq, stField.m_szField, stField.m_lKeyAttr, 
			stField.m_lKeyOffSet, stField.m_lKeyLen, stField.m_lIsPk);
	}
	lTime -= lGetTiskTime();	
	vTableClose(SYS_FILED_DEF, SHM_DISCONNECT);
	fprintf(stdout, "------------------------------------------------------%s-----------------"
		"---------------------------------------\n", sGetCostTime(-1 * lTime));

	return ;
}

/****************************************************************************************
	���ӹ����ڴ沢��ʼ����Ҫ����
 ****************************************************************************************/
void	vTableView_REGION_CODE(TABLE t, char *pszTemp)
{
	dbReginCode	stReg;
	long		lRet = 0, nRecord = 0, lTime = 0;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
		fprintf(stderr, "�򿪱�TBL_REGION_CODE�α�ʧ��, ԭ��:%s\n", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_REGION_CODE��������:\n");
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
			fprintf(stderr, "��ѯ��TBL_REGION_CODE����, ԭ��:%s", sGetError());
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
void	vTableView_BRH_INFO(TABLE t, char *pszTemp)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbBrhInfo	stBrh;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_BRH_PROFIT_INFO(TABLE t, char *pszTemp)
{
	long	lRet = 1, nRecord = 0, lTime = 0;
	dbPfitInfo	stPfitIf;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_MCHT_MCC_INF(TABLE t, char *pszTemp)
{
	long			lRet = 1, nRecord = 0, lTime = 0;
	dbMchtMccInf	stMcc;
	RunTime			*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
		fprintf(stderr, "�򿪱�TBL_MCHT_MCC_INF�α�ʧ��, ԭ��:%s\n", sGetError());
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
void	vTableView_BRH_PROFIT_DEF(TABLE t, char *pszModel)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbPfitDef	stDbPfitDef;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_BRH_PROFIT_ALGO(TABLE t, char *pszModel)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbBrhAlgo	stBrhAlgo;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_ACCT_INFO(TABLE t, char *pszTemp)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbAcctInfo	stAct;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_OPR_INFO(TABLE t, char *pszId)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbOprInf	stOpr;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
		fprintf(stderr, "�򿪱�TBL_OPR_INFO�α�ʧ��, ԭ��:%s\n", sGetError());
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
void	vTableView_SPDB_BRH(TABLE t, char *pszRegNo)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbSpdbBrh	stSpdb;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_MCHT_BASE(TABLE t, char *pszTemp)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbMchtBase	stBase;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
		fprintf(stdout, "[%d]:\t%9d|%15s|%40s|%4s|%6s|%15s|%6s|%40s|%5s|%18s|%10d|%18s|%8s|%8s|%8s|%8d|\n", 
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
void	vTableView_MCHT_ALGO(TABLE t, char *pszModel)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbMchtAlgo	stMtAlog;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_PUB_DISC_ALGO(TABLE t, char *pszDiscId)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbDiscAlgo	stAlgo;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
void	vTableView_MCHT_USER(TABLE t, char *pszMchtNo)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbMchtUser	stUser;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
		"-----------------------------------------------\n");
	fprintf(stdout, "line\t�û����\t�̻���\t\t\t\t\t\t�û�����\t\t\t�û�Ӣ����\t�û��ֻ���\t"
		"�û���������\t\t�û�֤������\n");
	fprintf(stdout, "------------------------------------------------------------"
		"------------------------------------------------------------------------"
		"-----------------------------------------------\n");
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
		fprintf(stdout, "[%d]:\t%10d|%15s|%50s|%30s|%11s|%30s|%18s|\n", nRecord, stUser.user_id, stUser.mcht_no, 
			stUser.user_name, stUser.user_engname, stUser.user_phone, stUser.user_email, stUser.user_card_no);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_MCHT_USER, SHM_DISCONNECT);
	fprintf(stdout, "-------------------------------------------------------------"
		"-------------------%s--------------------------------------------"
		"-----------------------------------------------\n", sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	�鿴�̻�·�ɱ�
 ****************************************************************************************/
void	vTableView_MCHT_CUPS_INF(TABLE t, char *pszCupsNo)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbMchtCupsInf	stCups;
	RunTime			*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
		fprintf(stdout, "[%d]:\t%6d|%4s|%30s|%15s|%12s|%15.5f|%15.5f|%20s|\n", nRecord, 
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
void	vTableView_N_MCHT_MATCH(TABLE t, char *pszCupsNo)
{
	long			lRet = 0, nRecord = 0, lTime = 0;
	dbNMchtMatch	stMatch;
	RunTime			*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
	fprintf(stdout, "line\t��� ������� ����״̬ ͬ��״̬\t   �����̻���\t   �����̻���"
		"\t	  �����̻���\tͬ��ʧ��ԭ��\tԤ��0\t	  Ԥ��1\t\t����   �ⶥֵ\t\tͬ���̻�����"
		"\t\t\t\tͬ��ʱ�� �̻���  MCC ͬ����������\n");
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
		fprintf(stdout, "[%d]:\t%6d|%4s|%8s|%10s|%18s|%15s|%15s|%20s|%6d|%20s|%8.2f|%8.2f|%50s|%8s"
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
void	vTableView_SERVICE_CONFIG(TABLE t, char *pszCode)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbSvceCfg	stCfg;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

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
		"����ģ�� �����Żݼ� �������� �ƻ�����ʱ�� ��ֹʱ�� ����״̬ ����ģ��\t"
		"�������\t�Ƿ����\n");
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
void	vTableView_PRO_SPECIAL_MCHT(TABLE t, char *pszCode)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbSpeMcht	stSpec;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_PRO_SPECIAL_MCHT;
	if(pszCode)
	{
		memset(&stSpec, 0, sizeof(stSpec));
		strcpy(stSpec.service_code, pszCode);
		sTrimAll(stSpec.service_code);

		pstRun->lSize = sizeof(stSpec);
		pstRun->pstVoid = &stSpec;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "�򿪱�TBL_PRO_SPECIAL_MCHT��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_PRO_SPECIAL_MCHT������:\n");
	fprintf(stdout, "--------------------------------------------------------------------\n");
	fprintf(stdout, "line\t���\t\t�̻���\t   ������  ��� ����ģ�� �̶�ģ��\n");
	fprintf(stdout, "--------------------------------------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stSpec, 0, sizeof(stSpec));	
		lRet = lTableFetch(TBL_PRO_SPECIAL_MCHT, &stSpec, sizeof(stSpec));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_PRO_SPECIAL_MCHT����, ԭ��:%s", sGetError());
			vTableClose(TBL_PRO_SPECIAL_MCHT, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stSpec.id);
		fprintf(stdout, "[%d]:%8d|%20s|%10s|%4s|%8s|%8s|\n", nRecord, stSpec.id, stSpec.mcht_no, 
			stSpec.service_code, stSpec.flag, stSpec.pro_rate, stSpec.pro_fix);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_PRO_SPECIAL_MCHT, SHM_DISCONNECT);
	fprintf(stdout, "----------------------------%s--------------------------------\n", 
		sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	�鿴������ģ��������Ϣ	TBL_DISC_ALGO
 ****************************************************************************************/
void	vTableView_DISC_ALGO(TABLE t, char *pszDiscId)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbDisc		stDisc;
	RunTime 	*pstRun = (RunTime *)pInitRunTime(t);

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
	�鿴������ģ��������Ϣ	TBL_PRO_CONFIG
 ****************************************************************************************/
void	vTableView_PRO_CONFIG(TABLE t, char *pszService)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbProCfg	stPro;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_PRO_CONFIG;
	if(pszService)
	{
		memset(&stPro, 0, sizeof(stPro));
		strcpy(stPro.service_code, pszService);
		sTrimAll(stPro.service_code);

		pstRun->lSize = sizeof(stPro);
		pstRun->pstVoid = &stPro;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "�򿪱�TBL_PRO_CONFIG��ʧ��, ԭ��:%s", sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_PRO_CONFIG������:\n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"---------------------------\n");
	fprintf(stdout, "line\tPK\t	 ��������  ������ �������� ������� �������� ��� "
		"����ģ�� �̶�ģ�� \n");
	fprintf(stdout, "-----------------------------------------------------------------"
		"---------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stPro, 0, sizeof(stPro));	
		lRet = lTableFetch(TBL_PRO_CONFIG, &stPro, sizeof(stPro));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��TBL_PRO_CONFIG����, ԭ��:%s", sGetError());
			vTableClose(TBL_PRO_CONFIG, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		lGetEXLong(&stPro.pay_day);
		lGetEXLong(&stPro.tx_seq);
		fprintf(stdout, "[%d]:\t%12s|%8s|%10s|%8d|%8d|%8s|%4s|%8s|%8s|\n", nRecord, stPro.id, 
			stPro.tx_date, stPro.service_code, stPro.pay_day, stPro.tx_seq, stPro.stlm_date, 
			stPro.flag, stPro.pro_rate, stPro.pro_fix);
	}
	lTime -= lGetTiskTime();
	vTableClose(TBL_PRO_CONFIG, SHM_DISCONNECT);
	fprintf(stdout, "-----------------------------------------%s------------"
		"--------------------------------\n", sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	��ӡ�����̻�����
 ****************************************************************************************/
void	vTableView_MCHT_CARD(TABLE t, char *pszMcht)
{
	long		lRet = 0, nRecord = 0, lTime = 0;
	dbMCard		stCard;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(t),
			 sGetError());
		return ;
	}

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = t;
	if(pszMcht)
	{
		memset(&stCard, 0, sizeof(stCard));
		strcpy(stCard.mcht_no, pszMcht);
		sTrimAll(stCard.mcht_no);

		pstRun->lSize = sizeof(stCard);
		pstRun->pstVoid = &stCard;	
	}

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{
		fprintf(stderr, "�򿪱�(%s)��ʧ��, ԭ��:%s", sGetTableName(t), sGetError());
		return ;
	}

	fprintf(stdout, "��TBL_MCHT_CARD������:\n");
	fprintf(stdout, "------------------------------------\n");
	fprintf(stdout, "line\t�̻���\t\t   ����ģ��\n");
	fprintf(stdout, "------------------------------------\n");
	lTime = lGetTiskTime();
	//	�������м�¼
	while(1)
	{
		memset(&stCard, 0, sizeof(stCard));	
		lRet = lTableFetch(TBL_MCHT_CARD, &stCard, sizeof(stCard));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ѯ��(%s)����, ԭ��:%s", sGetTableName(t), sGetError());
			vTableClose(t, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		nRecord ++;
		fprintf(stdout, "[%d]:\t%-18s|%8s|\n", nRecord, stCard.mcht_no, stCard.disc_id);
	}
	lTime -= lGetTiskTime();
	vTableClose(t, SHM_DISCONNECT);
	fprintf(stdout, "---------------%s-------------\n", sGetCostTime(-1 * lTime));
	fflush(stdout);
}

/****************************************************************************************
	��ӡ������Ϣ
 ****************************************************************************************/
void	vPrintHeadInfo()
{
	long		lRet = 0, j = 0, lTableNum = 0;
	TblKey		*pstKey = NULL;
	BSysIdx		stIdx;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_TABLE_IDX)))
	{
		fprintf(stderr, "��ʼ����(%s)ʧ��, err:(%s)", sGetTableName(SYS_TABLE_IDX),
			sGetError());
		return ;
	}

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = SYS_TABLE_IDX;
	if(RC_SUCC != lTableDeclare(pstRun))
		return ;

	while(1)
	{
		memset(&stIdx, 0, sizeof(stIdx));
		lRet = lTableFetch(SYS_TABLE_IDX, &stIdx, sizeof(BSysIdx));
		if(RC_FAIL == lRet)
		{
			fprintf(stderr, "��ȡ��(%s)����ʧ��, err:%s\n", sGetTableName(SYS_TABLE_IDX), 
				sGetError());
			vTableClose(SYS_TABLE_IDX, SHM_DISCONNECT);
			return ;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		lGetEXLong((long *)&stIdx.m_ShmId);
		lGetEXLong((long *)&stIdx.m_table);

		//	Ϊ������������
		if(SYS_TABLE_IDX != stIdx.m_table)
		{
			((IBPShm *)pGetShmRun())->m_shmID = stIdx.m_ShmId;
			if(RC_SUCC != lInitMemTable(stIdx.m_table, TYPE_CLIENT))
			{
				fprintf(stderr, "\n��ʼ���ͻ���(%d)(%s)ʧ��, err:(%s)\n", stIdx.m_table, 
					stIdx.m_szTable, sGetError());
				continue;
			}
			vTblDisconnect(stIdx.m_table);			
		}

		fprintf(stdout, "\n��(%d)(%s)��Ч��¼��(%d)�ڵ�(%d)���֧����(%d), ƫ����(%d), ƥ����������(%d)\n", 
			stIdx.m_table, sGetTableName(stIdx.m_table), ((TblDef *)pGetTblDef(stIdx.m_table))->lValid, 
			((TblDef *)pGetTblDef(stIdx.m_table))->lTreeNode, lGetTblCount(stIdx.m_table),
			((TblDef *)pGetTblDef(stIdx.m_table))->lOffSet, ((TblDef *)pGetTblDef(stIdx.m_table))->m_lIdxNum);

		for(j = 0; j < ((TblDef *)pGetTblDef(stIdx.m_table))->m_lIdxNum; j ++)
		{
			pstKey = (TblKey *)pGetTblIdx(stIdx.m_table);
			if(IDX_SELECT == pstKey[j].m_lIsPk)
				fprintf(stdout, "������: ��%2d����������ʼλ��(%d)��������(%d)��������(%d)\n", j + 1, 
					pstKey[j].m_lKeyOffSet, pstKey[j].m_lKeyLen, pstKey[j].m_lKeyAttr);
			else if(CHK_SELECT == pstKey[j].m_lIsPk)
				fprintf(stdout, "������: ��%2d����������ʼλ��(%d)��������(%d)��������(%d)\n", j + 1, 
					pstKey[j].m_lKeyOffSet, pstKey[j].m_lKeyLen, pstKey[j].m_lKeyAttr);
			else
			{
				fprintf(stderr, "�������������(%d)!!\n", pstKey[j].m_lIsPk);
				return ;
			}
		}
	}

	vTableClose(SYS_TABLE_IDX, SHM_DISCONNECT);

	return ;
}

/****************************************************************************************
	�鿴�̻�������Ϣ
 ****************************************************************************************/
long	lGetMchtNo(char *pszFOut, char *pszEOut)
{
	dbMchtBase	stBase;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_BASE)))
	{
		fprintf(stderr, "��ʼ����(%s)�ڴ��ʧ��, err:(%s)\n", sGetTableName(TBL_MCHT_BASE),
			sGetError());
		return RC_FAIL;
	}

	memset(&stBase, 0, sizeof(stBase));
	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_BASE;
	pstRun->pstVoid = &stBase;
	pstRun->lSize = sizeof(stBase);

	if(RC_SUCC != lGetExtremeIdx(pstRun, &stBase, FIELD_MAX))
	{
		fprintf(stderr, "��ȡ�̻�TBL_MCHT_BASE���ֵʧ��, ԭ��:%s\n", sGetError());
		return RC_FAIL;
	}
	strcpy(pszEOut, stBase.mcht_no);

	memset(&stBase, 0, sizeof(stBase));
	if(RC_SUCC != lGetExtremeIdx(pstRun, &stBase, FIELD_MIN))
	{
		fprintf(stderr, "��ȡ��TBL_MCHT_BASE��Сֵʧ��, ԭ��:%s\n", sGetError());
		return RC_FAIL;
	}
	strcpy(pszFOut, stBase.mcht_no);

	return RC_SUCC;
}

/****************************************************************************************
	table 
 ****************************************************************************************/
long	lQueryMchtBase(dbMchtBase *pstBase, char *pszMchtNo)
{
	long		lRet = 0;
	dbMchtBase  stBase;
	RunTime	 *pstRun = (RunTime *)pInitRunTime(TBL_MCHT_BASE);

	memset(&stBase, 0, sizeof(stBase));
	strcpy(stBase.mcht_no, pszMchtNo);
	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_BASE;
	pstRun->lSize = sizeof(stBase);
	pstRun->pstVoid = stBase.mcht_no;

	lRet = lQuerySpeed(pstRun, &stBase);
	if(lRet !=  RC_SUCC)
		return RC_FAIL;

	lGetEXLong(&stBase.id);
	lGetEXLong(&stBase.mcht_resv1);
	lGetEXLong(&stBase.mcht_expand);
	memcpy(pstBase, &stBase, sizeof(stBase));
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
	lRet = lQueryMchtBase(&stMcht, szFMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dTime -= dGetUSecTime();
	fprintf(stdout, "��һ���̻�(%s)-->(%s)(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szFMchtNo, 
		stMcht.mcht_no, stMcht.mcht_nm, -1 * dTime);

	memset(&stMcht, 0, sizeof(stMcht));
	dEnd = dGetUSecTime();
	lRet = lQueryMchtBase(&stMcht, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dEnd -= dGetUSecTime();
	fprintf(stdout, "��һ���̻�(%s)-->(%s)(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szEMchtNo, 
		stMcht.mcht_no, stMcht.mcht_nm, -1 * dEnd);
	fprintf(stdout, "��ѯ��һ���������ʱ���:%.3fms\n", (-1 * dEnd) - (-1 * dTime));

	fprintf(stdout, "\n����AVL���ٲ�ѯ��ʽ��ѯ��һ�������һ������.\n");
	memset(&stMcht, 0, sizeof(stMcht));
	dTime = dGetUSecTime();
	lRet = lGetMchtBase(&stMcht, szFMchtNo);
	if(lRet)
	{
		fprintf(stderr, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dTime -= dGetUSecTime();
	fprintf(stdout, "��һ���̻�(%s)-->(%s)(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szFMchtNo, 
		stMcht.mcht_no, stMcht.mcht_nm, -1 * dTime);

	memset(&stMcht, 0, sizeof(stMcht));
	dEnd = dGetUSecTime();
	lRet = lGetMchtBase(&stMcht, szEMchtNo);
	if(lRet)
	{
		fprintf(stdout, "��ѯ����, err:(%d)(%s)\n", lRet, sGetError());
		return ;
	}
	dEnd -= dGetUSecTime();
	fprintf(stdout, "��һ���̻�(%s)-->(%s)(%s), ��ѯ���Ļ���ʱ��:%.3fms\n", szEMchtNo, 
		stMcht.mcht_no, stMcht.mcht_nm, -1 * dEnd);
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
	fprintf(stderr, "\t17\t--TBL_PRO_CONFIG\n");
	fprintf(stderr, "\t18\t--TBL_PRO_SPECIAL_MCHT\n");
	fprintf(stderr, "\t19\t--TBL_MCHT_CARD\n");
}

TblPrint	g_TblePrint[] = {
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
	{"�����������:", &vTableView_PRO_CONFIG},
	{"�����������:", &vTableView_PRO_SPECIAL_MCHT},
	{"�������̻����:", &vTableView_MCHT_CARD},
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
	�ڴ�SQL����
 ****************************************************************************************/
void	vEditSQLMode()
{
	long	lOfs = 0, b = 0;
	char	szTemp[2048], szSQL[1024], *p = NULL;

	fprintf(stdout, "SQLģʽ��ʼ��...\n");
	if(RC_SUCC != lCreateTable("", "", SYS_FILED_DEF))
  	{
		fprintf(stderr, "�����ڴ����ݳ���, err:%s\n", sGetError());
		return ;
	}
	system("clear");

	while(1)
	{
		fprintf(stdout, "------------------------------------------------------����SQL"
			"-----------------------------------------------------\n");
		memset(szTemp, 0, sizeof(szTemp));
		vGetInput(szTemp, sizeof(szTemp) - 1, 0);
		if(!strcmp(szTemp, "q") || !strcmp(szTemp, "Q") || !strcmp(szTemp, "exit"))
			exit(0);
		else if(!strcasecmp(szTemp, "clear"))
		{
			system("clear");
			continue;
		}

		lOfs = b = 0;
		strcat(szTemp, ";");
		while((p = strstr(szTemp + lOfs, ";")))
		{
			memset(szSQL, 0, sizeof(szSQL));
			strcpy(szSQL, sGetValueByIdx(szTemp, ";", ++ b));
			lOfs += strlen(szSQL) + strlen(";");
			sTrimLeft(szSQL);
			sTrimRight(szSQL);
			if(!strlen(szSQL))
				continue;
			vAnalySQL(szSQL);
		}
		fprintf(stdout, "\n\n\n");
	}

	return ;
}

/****************************************************************************************
	�����
 ****************************************************************************************/
void	vOperateTables()
{
	long	lRet = 0, lChoose = 0;
	BOOL	bPrint = true, bRet = false;
	TABLE	table;
	char	szTemp[500], szTable[20], *p = NULL;
	char	szSettleDate[9], szSettleNum[5];

	memset(szSettleNum, 0, sizeof(szSettleNum));
	memset(szSettleDate, 0, sizeof(szSettleDate));
	system("clear");
	while(1)
	{
		lChoose = 0;
		if(bPrint)		vPrintSysIdx();
		fprintf(stdout, "������ָ��:");	
		memset(szTemp, 0, sizeof(szTemp));
		vGetInput(szTemp, sizeof(szTemp) - 1, 0);
		if(!strcmp(szTemp, "b") || !strcmp(szTemp, "B") || !strcmp(szTemp, "back"))
			break;
		else if(!strcmp(szTemp, "q") || !strcmp(szTemp, "Q") || !strcmp(szTemp, "exit"))
			exit(0);
		else if(!strcmp(szTemp, "clear"))
		{
			system("clear");
			continue;
		}
		else if(!strcmp(szTemp, "help") || strstr(szTemp, "-?"))
		{
			bPrint = false;
			fprintf(stdout, "\n\t--[load|drop] table\tװ��|ɾ����\n\n");
			continue;
		}
		else if((p = strstr(szTemp, "load")))
			lChoose = 1;	
		else if((p = strstr(szTemp, "drop")))
			lChoose = 2;	
		else
			continue;

		sTrimLeft(szTemp);
		memset(szTable, 0, sizeof(szTable));
		strcpy(szTable, szTemp + 4);
		sTrimAll(szTable);
		if('0' > szTable[0] || szTable[0] > '9')
		{
			fprintf(stdout, "\n\t--[load|drop] table\tװ��|ɾ����\n\n");
			continue;
		}
		else if('0' > szTable[0] || szTable[0] > '9')
		{
			fprintf(stdout, "��Ч�ı����(%s)\n", szTable);
			continue;
		}

		if(1 != lChoose	&& 2 != lChoose)
		{
			bPrint = false;
			fprintf(stdout, "%s", szTemp);
			fprintf(stdout, "\n\t--[load|drop] table\tװ��|ɾ����\n\n");
			continue;
		}
		
		table = atol(szTable);
		fprintf(stderr, "������(%d)(%s)\n", table, sGetTableName(table));

		if(RC_SUCC != lTblIsNExist(table))
		{
			bPrint = true;
			fprintf(stderr, "�������:%s\n", sGetError());
			continue;
		}

		if(RC_SUCC != lIsNSysTable(table, &bRet))
		{
			fprintf(stderr, "��ѯ��(%d)(%s)�ݳ���, err:%s\n", table, sGetTableName(table), 
				sGetError());
			return ;
		}

		if(bRet)
		{
			bPrint = true;
   			fprintf(stderr, "����, �޷�����ϵͳ��!\n");
			continue;
		}

		if(1 == lChoose)
		{
			fprintf(stdout, "�������������(YYYYMMDD):");
			vGetInput(szSettleDate, sizeof(szSettleDate) - 1, 1);
			sTrimAll(szSettleDate);
			if(!strlen(szSettleDate))
			{
				strncpy(szSettleDate, sGetDate(), 8);
				fprintf(stdout, "%s\n", szSettleDate);
			}
			else
				getchar();

			fprintf(stdout, "�������������(TN00/TC00/TX00/T088):");
			vGetInput(szSettleNum, sizeof(szSettleNum) - 1, 1);
			sTrimAll(szSettleNum);
			if(!strlen(szSettleNum))
			{
				strcpy(szSettleNum, "TN00");
				fprintf(stdout, "%s\n", szSettleNum);
			}
			else
				getchar();

			if(RC_SUCC != lDbsConnect(lGetConnect(table)))
			{
				fprintf(stderr, "�������ݿ�ʧ��, err:(%d)(%s)(%s)", lGetConnect(table),
					sGetError(), sDbsError());
				return ;
			}

			if(RC_SUCC != lCreateTable(szSettleDate, szSettleNum, table))
			{
				fprintf(stderr, "�����ڴ����ݳ���, err:%s\n", sGetError());
				vDbsDisconnect();
				return ;
			}

			vDbsDisconnect();
		}
		else
		{
			if(RC_SUCC != lDropTables(TABLE_ONLY, table, true))
			{
				fprintf(stderr, "�����ڴ����ݳ���, err:%s\n", sGetError());
				return ;
			}
		}
		bPrint = true;
	}

	return ;
}

/****************************************************************************************
	main
 ****************************************************************************************/
int		main(int argc, char** argv)
{
	long	lRet = 0, lChoose = 0;
	TABLE	table;
	char	szTemp[500], chTable;

	sprintf(gsLogFile, "vtable.log");
	vSetLogName("vtable.log");
	system("stty erase ^h");

	if(2 == argc && !strcmp(argv[1], "test"))
	{
		vPrintModeTest();
		return RC_SUCC;
	}
	else if(2 == argc && (!strcmp(argv[1], "SQL") || !strcmp(argv[1], "sql")))
	{
		vEditSQLMode();
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
	
		g_TblePrint[table].pfTable(table, argv[2]);
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
		else if(!strcmp(szTemp, "table"))
		{
			vOperateTables();
			continue;
		}
		else if(!strcmp(szTemp, "P") || !strcmp(szTemp, "p"))
		{
			vPrintHeadInfo();
			continue;
		}
		else if(!strcmp(szTemp, "y") || !strcmp(szTemp, "Y"))
		{
			vPrintSysIdx();
			continue;
		}
		else if(!strcmp(szTemp, "f") || !strcmp(szTemp, "f"))
		{
			vPrintFiled();
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

		if(table >= sizeof(g_TblePrint)/sizeof(TblPrint))
		{
			fprintf(stderr, "��Ȩ�޷���ϵͳ��!\n");
			continue;
		}

		memset(szTemp, 0, sizeof(szTemp));
		fprintf(stdout, "%s", g_TblePrint[table].szDsc);
		fflush(stdin);
		vGetInput(szTemp, sizeof(szTemp) - 1, 1);
		if(!strcmp(szTemp, "q") || !strcmp(szTemp, "Q") || !strcmp(szTemp, "exit"))
			exit(0);

		if(!memcmp(szTemp, "NULL", 4) || !memcmp(szTemp, "null", 4))
			g_TblePrint[table].pfTable(table, NULL);
		else
			g_TblePrint[table].pfTable(table, szTemp);
	}
	return RC_SUCC;
}
