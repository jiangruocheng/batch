#include	"Ibp.h"
#include	"DbsApi.h"
#include	"pos_stlm.h"
#include	"load.h"
#include	"table.h"

/****************************************************************************************
	ȫ�ֱ���
 ****************************************************************************************/
TblDef	g_stTblDef[SHM_TABLE_NUM] = {0};

/****************************************************************************************
	��ȡ�ͻ�����
 ****************************************************************************************/
TblCreat*	pGetTblCreate(TABLE t)
{
	return &g_stCreate[t];
}

/****************************************************************************************
	�ͻ����巽��
 ****************************************************************************************/
long	lTableMethod(TABLE t, Tree **pstRoot, char *pszSettleDate, char *pszSettleNum, Benum lEnum)
{
	TABLE	i = 0;

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		if(t != pGetTblCreate(i)->table)
			continue;

		if(TBL_SIZE == lEnum)
		{
			if(!pGetTblCreate(i)->pfGetSize)
			{
				vSetErrMsg("δ�����(%d)�Ĵ����߼�(%d)", t, lEnum);
				return RC_FAIL;	
			}
			return pGetTblCreate(i)->pfGetSize(t, pszSettleDate, pszSettleNum);
		}
		else if(TBL_DATA == lEnum)
		{
			if(!pGetTblCreate(i)->pfLoadData)
			{
				vSetErrMsg("δ�����(%d)�Ĵ����߼�(%d)", t, lEnum);
				return RC_FAIL;	
			}
			return pGetTblCreate(i)->pfLoadData(t, pstRoot, pszSettleDate, pszSettleNum);
		}
		else
		{
			vSetErrMsg("δ�����(%d)�Ĵ����߼�(%d)", t, lEnum);
			return RC_FAIL;	
		}
	}

	vSetErrMsg("δ�����(%d)ת�ط���", t);
	return RC_FAIL;
}

/****************************************************************************************
	��ȡ������
 ****************************************************************************************/
char*	sGetTableName(TABLE t)
{
	TABLE	i = 0;
	static	char	szTable[100];

	memset(szTable, 0, sizeof(szTable));

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		if(t != pGetTblCreate(i)->table)
			continue;

		snprintf(szTable, sizeof(szTable), "%s", pGetTblCreate(i)->szTableName);
		return szTable;
	}

	return szTable;
}

/****************************************************************************************
	��ȡ�����
 ****************************************************************************************/
TABLE	tGetTableNumb(char *pszTable)
{
	TABLE	i = 0;

	if(!pszTable || !strlen(pszTable))
		return RC_FAIL;

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		if(!strcmp(pGetTblCreate(i)->szTableName, pszTable))
			return pGetTblCreate(i)->table;
	}

	return RC_FAIL;
}

/****************************************************************************************
	��ȡ�ڴ���Ӧ�����ݿ�
 ****************************************************************************************/
long	lGetConnect(TABLE t)
{
	long	lType = 0;
	TABLE	i = 0;

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		if(t != pGetTblCreate(i)->table)
			continue;

		return pGetTblCreate(i)->lDbsConnect;
	}

	return lType;
}

/****************************************************************************************
	��ȡװ�ر����
 ****************************************************************************************/
long	lGetLoadList(TABLE **ppt, long *plCount)
{
	long	lRec = 0;
	TABLE	i = 0, *pTable = NULL;

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		//	δ����Ĳ�װ��
		if(!pGetTblCreate(i)->pfGetSize || !pGetTblCreate(i)->pfLoadData || 
			0 == pGetTblCreate(i)->lDbsConnect)
			continue;

		if(0 == lRec)
			pTable = (TABLE *)malloc(sizeof(TABLE));
		else
			pTable = (TABLE *)realloc(pTable, sizeof(TABLE) * (lRec + 1));
		if(NULL == pTable)
		{
			vSetErrMsg("�����С(%d)�ڴ�ʧ��!", sizeof(TABLE) * (lRec + 1));
			return RC_FAIL;
		}
		pTable[lRec ++] = pGetTblCreate(i)->table;
	}

	*ppt 		= pTable;
	(*plCount)	= lRec;

	return RC_SUCC;
}

/****************************************************************************************
	��ȡ�������ĸ���
 ****************************************************************************************/
long	lTblIsNExist(TABLE t)
{
	TABLE	i = 0;

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		if(t == pGetTblCreate(i)->table)
			return RC_SUCC;	
	}

	vSetErrMsg("�����(%d)������", t);
	return RC_FAIL;
}

/********************************************************************************
	��ȡ��ǰʱ��,ʱ���ʽYYYY/MM/DD hh:mm:ss							  
 ********************************************************************************/
char*   sGetUpdTime()
{
	time_t  current;
	struct  tm *tm = NULL;
	static  char	szTime[20];

	tzset();
	time(&current);
	tm = localtime(&current);

	memset(szTime, 0, sizeof(szTime));
	snprintf(szTime, sizeof(szTime), "%4d/%02d/%02d %02d:%02d:%02d", tm->tm_year + 1900,
		tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return szTime;
}

/****************************************************************************************
	������0��ת��
 ****************************************************************************************/
long	lSetEXLong(long n)
{
	if(n <= 0)
		n += LONG_ZERO;
	return n;
}

/****************************************************************************************
	������0��ת��
 ****************************************************************************************/
long	lGetEXLong(long *pn)
{
	if(*pn <= LONG_ZERO)
		*pn -= LONG_ZERO;
	return *pn;
}

/****************************************************************************************
	��ȡ��ָ��ڵ�
 ****************************************************************************************/
TblDef*	pGetTblDef(TABLE t)
{
	return &g_stTblDef[t];
}

/****************************************************************************************
	��ȡ�������λ��
 ****************************************************************************************/
long	lGetTblPos(TABLE t)
{
	return pGetTblDef(t)->lOffSet;
}

/****************************************************************************************
	AVL���ٲ���root�ڵ�
 ****************************************************************************************/
long	lGetShmPos(TABLE t)
{
	return pGetTblDef(t)->lShmPos;
}

/****************************************************************************************
	��ȡ����Ч��������
 ****************************************************************************************/
long	lGetTblValid(TABLE t)
{
	return pGetTblDef(t)->lValid;
}

/****************************************************************************************
	��ȡ����Ч��������
 ****************************************************************************************/
long	lGetTblCount(TABLE t)
{
	return pGetTblDef(t)->lCount;
}

/****************************************************************************************
	��ȡ��������ʼ����
 ****************************************************************************************/
TblKey* pGetTblIdx(TABLE t)
{
	return  pGetTblDef(t)->m_stKeyIdx;
}

/****************************************************************************************
	��ʼ��д������
 ****************************************************************************************/
long	lInsertData(TABLE t, void *psvData, long lSize, Benum em)
{
	void	*p = NULL;

	if(RC_SUCC != lTblIsNExist(t))
		return RC_FAIL;

	if(false == em)
	{
		((IBPShm *)pGetShmRun())->m_lOffset = 0;
		pGetTblDef(t)->lValid = 0;
		pGetTblDef(t)->lTreeNode = 0;
		return RC_SUCC;
	}

	p = (char *)pGetTblAddr(t, lGetTblPos(t));
	if(!p) return RC_FAIL;

	memcpy(p, psvData, lSize);
	((IBPShm *)pGetShmRun())->m_lOffset += lSize;
	pGetTblDef(t)->lValid ++;

	return RC_SUCC;
}

/****************************************************************************************
	��ȡ�������ĸ���
 ****************************************************************************************/
long	lGetIdxNum(TABLE t)
{
	return  pGetTblDef(t)->m_lIdxNum;
}

/****************************************************************************************
 *  ����  :���ñ�����
 *  ����  :
 ****************************************************************************************/
long	lSetTableIdx(TABLE table, long lKeyOffSet, long lKeyLen, long lKeyAttr, long lKeyType)
{
	long	i = 0;
	TblKey  *pstIdx = (TblKey *)pGetTblIdx(table);

	for(i = 0; i < lGetIdxNum(table); i++)
	{
		//  ��������ˣ���ֱ�ӷ��سɹ�
		if((pstIdx[i].m_lKeyOffSet == lKeyOffSet) && pstIdx[i].m_lKeyLen == lKeyLen)
			return RC_SUCC;
	}

	if(lGetIdxNum(table) >= MAX_KEY_IDX)
	{
		vSetErrMsg("��(%d)�����������ó���, ���(%d)", table, SHM_TABLE_NUM);
		return RC_FAIL;
	}

	pstIdx[lGetIdxNum(table)].m_lKeyOffSet = lKeyOffSet;
	pstIdx[lGetIdxNum(table)].m_lKeyLen = lKeyLen;
	pstIdx[lGetIdxNum(table)].m_lKeyAttr = lKeyAttr;
	pstIdx[lGetIdxNum(table)].m_lIsPk = lKeyType;
	pGetTblDef(table)->m_lIdxNum ++;

	return RC_SUCC;
}

/****************************************************************************************
	���ýṹ������
 ****************************************************************************************/
void	vSetTblDef(TABLE t, char *pszAddr)
{
	memcpy(&g_stTblDef[t], (char *)pszAddr, sizeof(TblDef));
}

/****************************************************************************************
	���ñ�ṹ����  
 ****************************************************************************************/
long	lInitTblSize(TABLE table, long lCount, long lTblSize, long *plSize)
{
	if(RC_SUCC != lTblIsNExist(table))
		return RC_FAIL;

	pGetTblDef(table)->lLineSize = lTblSize;		//  �д�С
	lCount += EXT_TABLE_GAP;						//  ��ֹ�ڵ����ڴ���ʱ�������µ����ݡ�
	pGetTblDef(table)->lShmPos = (*plSize);			//  ����Root�ڵ�λ��
	(*plSize) += lCount * sizeof(SHTree);			//  �ڵ�������
	pGetTblDef(table)->lOffSet = *plSize;			//  ƫ����
	pGetTblDef(table)->lCount  = lCount;			//  ��¼��
	(*plSize) += lCount * lTblSize;

	return RC_SUCC;
}

/****************************************************************************************
	��ȡϵͳ����������С	
 ****************************************************************************************/
long	lGetSysTblSize()
{
	long	lSize = 0, lRecord = 0;

	lSize = sizeof(TblDef);
	lRecord = SHM_TABLE_NUM + EXT_TABLE_GAP;
	if(RC_SUCC != lInitTblSize(SYS_TABLE_IDX, lRecord, sizeof(BSysIdx), &lSize))
		return RC_FAIL;

	fprintf(stdout, "��ʼ����ϵͳ������!\n");
	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_lId, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_lIdʧ��!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_table, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_tableʧ��!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_yKey, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_yKeyʧ��!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_lPid, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_lPidʧ��!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_bAttch, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_bAttchʧ��!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_lRecord, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_lRecordʧ��!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_lMaxRows, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_lMaxRowsʧ��!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_lLSize, FIELD_LONG))
	{
		vSetErrMsg("����ϵͳ������(SYS_TABLE_IDX)�ֶ�m_lLSizeʧ��!");
		return RC_FAIL;
	}

	fprintf(stdout, "����ϵͳ��������ɣ�completed successfully!!!\n");
	return lSize;
}

/****************************************************************************************
	��ʼ���ڴ������
 ****************************************************************************************/
long	lInitTableIdx(Tree **pstNode, TABLE table, long lShmId, long lSize)
{
	long		lRes = 0;
	TABLE		t = 0;
	BSysIdx		stSysIdx;
	Tree		*pstRoot = NULL;

	((IBPShm *)pGetShmRun())->m_lOffset = 0;
	pGetTblDef(SYS_TABLE_IDX)->lValid = 0;
	pGetTblDef(SYS_TABLE_IDX)->lTreeNode = 0;

	for(t = 0; t < SHM_TABLE_NUM; t ++)
	{
		memset(&stSysIdx, 0, sizeof(stSysIdx));
		stSysIdx.m_lId		= lSetEXLong(t);
		stSysIdx.m_table	= lSetEXLong(t);
		stSysIdx.m_yKey		= lSetEXLong(0);
		
		if(table == t)
		{
			stSysIdx.m_ShmId  = lSetEXLong(lShmId);
			stSysIdx.m_lLSize = lSetEXLong(lSize);
		}
		else
		{
			stSysIdx.m_ShmId  = lSetEXLong(0);
			stSysIdx.m_lLSize = lSetEXLong(0);
		}
		stSysIdx.m_lType	= lSetEXLong(0);
		stSysIdx.m_lPid		= lSetEXLong(0);
		stSysIdx.m_bAttch	= lSetEXLong(0);
		stSysIdx.m_lRecord	= lSetEXLong(0);
		stSysIdx.m_lMaxRows	= lSetEXLong(0);
		snprintf(stSysIdx.m_szTable, sizeof(stSysIdx.m_szTable), "%s", sGetTableName(t));
		snprintf(stSysIdx.m_szTime, sizeof(stSysIdx.m_szTime), "%s", sGetUpdTime());

		pstRoot = (Tree *)IBPInsertTree(pstRoot, (void *)&stSysIdx.m_table, ++(pGetTblDef(SYS_TABLE_IDX)->lTreeNode),
			BSysIdx, m_table, lGetTblPos(SYS_TABLE_IDX) + ((IBPShm *)pGetShmRun())->m_lOffset, &lRes);
		if(RC_FAIL == lRes)
		{
			HtLog(ERR_PARAM, "װ�ر�(%d)(%s)ʧ��, ԭ��:%s", stSysIdx.m_table, 
				stSysIdx.m_szTable, sGetError());
			vDestroyTree(pstRoot);
			return RC_FAIL;
		}

		memcpy((char *)pGetTblAddr(SYS_TABLE_IDX, lGetTblPos(SYS_TABLE_IDX)), &stSysIdx, sizeof(BSysIdx));
		((IBPShm *)pGetShmRun())->m_lOffset += sizeof(BSysIdx);
		pGetTblDef(SYS_TABLE_IDX)->lValid ++;
	}
	*pstNode = pstRoot;

	return RC_SUCC;
}

/****************************************************************************************
	�жϸñ��Ƿ�Ϊϵͳ��
 ****************************************************************************************/
long	lIsNSysTable(TABLE t, BOOL *bType)
{
	long		lRet = 0;
	BSysIdx		stSys, stChk;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_TABLE_IDX)))
		return RC_FAIL;

	memset(&stSys, 0, sizeof(stSys));
	stSys.m_table = lSetEXLong(t);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = SYS_TABLE_IDX;
	pstRun->lSize = sizeof(BSysIdx);
	pstRun->pstVoid = &stSys;

	lRet = lSelectSpeed(pstRun, &stChk);
	if(lRet != RC_SUCC)
		return RC_FAIL;

	lGetEXLong(&stChk.m_lType);

	if(TYPE_SYSTEM == stChk.m_lType)
		*bType = TRUE;
	else 
		*bType = FALSE;

	return RC_SUCC;
}

/****************************************************************************************
	ע���shmid
 ****************************************************************************************/
long	lRegisterTable(TABLE t, long lShmId, long lType, long lSize, TblDef *pstDef)
{
	long		lRet = 0, lSearch = 0;
	BSysIdx		stSysIdx, stIdx;
	RunTime		*pstRun = NULL;

	if(lSize <= 0 || t < 0)
	{
		vSetErrMsg("ע���(%d)ʧ��, �ڴ�����ID(%d)�д�С(%d)�쳣!", t, 
			lShmId, lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitCreateSem(1, SEM_INIT, false))
		return RC_FAIL;

	memset(&stIdx, 0, sizeof(stIdx));
	memset(&stSysIdx, 0, sizeof(stSysIdx));

	stSysIdx.m_lId		= lSetEXLong(t);
	stSysIdx.m_lType	= lSetEXLong(lType);
	stSysIdx.m_table	= lSetEXLong(t);
	stSysIdx.m_yKey		= lSetEXLong(0);
	stSysIdx.m_ShmId	= lSetEXLong(lShmId);
	stSysIdx.m_SemId	= lSetEXLong(((IBPShm *)pGetShmRun())->m_semID);
	stSysIdx.m_lPid		= lSetEXLong(0);
	stSysIdx.m_bAttch	= lSetEXLong(0);
	stSysIdx.m_lRecord	= lSetEXLong(pstDef->lValid);
	stSysIdx.m_lMaxRows	= lSetEXLong(pstDef->lCount);
	stSysIdx.m_lLSize	= lSetEXLong(lSize);
	snprintf(stSysIdx.m_szTable, sizeof(stSysIdx.m_szTable), "%s", sGetTableName(t));
	snprintf(stSysIdx.m_szTime, sizeof(stSysIdx.m_szTime), "%s", sGetUpdTime());

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_TABLE_IDX)))
		return RC_FAIL;

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = SYS_TABLE_IDX;
	if(RC_SUCC != lTableDeclare(pstRun))
		return RC_FAIL;

	while(1)
	{
		memset(&stIdx, 0, sizeof(stIdx));
		lRet = lTableFetch(SYS_TABLE_IDX, &stIdx, sizeof(BSysIdx));
		if(RC_FAIL == lRet)
		{
			vTableClose(SYS_TABLE_IDX, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		lGetEXLong((long *)&stIdx.m_table);
		if(stIdx.m_table == t)
		{
			pGetTblDef(SYS_TABLE_IDX)->lValid --;		//	�ҵ���¼���޳�������
			((IBPShm *)pGetShmRun())->m_pvCurAddr[SYS_TABLE_IDX] -= sizeof(BSysIdx);
			break;
		}
	}
	pGetTblDef(SYS_TABLE_IDX)->lValid ++;
	memcpy((void *)((IBPShm *)pGetShmRun())->m_pvCurAddr[SYS_TABLE_IDX], &stSysIdx, sizeof(BSysIdx));
	memcpy((void *)pGetShmAddr(SYS_TABLE_IDX), (void *)pGetTblDef(SYS_TABLE_IDX), sizeof(TblDef));
	vTableClose(SYS_TABLE_IDX, SHM_DISCONNECT);

	fprintf(stdout, "ע���(%s)�ɹ���completed successfully!!!\n", sGetTableName(t));

	return RC_SUCC;
}

/****************************************************************************************
	����ϵͳ��
 ****************************************************************************************/
long	lBuildSysParam()
{
	void	*pvData = NULL;
	Tree	*pstRoot = NULL;

	memset(&g_stTblDef[SYS_TABLE_IDX], 0, sizeof(TblDef));

	//	����ָ��Key������
	if(RC_SUCC != lInitCreateShm(SYS_TABLE_IDX, lGetSysTblSize(), TRUE))
		return RC_FAIL;

	if(RC_SUCC != lAttachShm(SYS_TABLE_IDX))
		return RC_FAIL;

	if(!(pvData = (void *)pGetShmAddr(SYS_TABLE_IDX)))
	{
		HtLog(ERR_PARAM, "��ȡϵͳ�������ַʧ��, err:(%s)", sGetError());
		return RC_FAIL;
	}

	if(RC_SUCC != lInitTableIdx(&pstRoot, SYS_TABLE_IDX, lGetShmId(), pGetTblDef(SYS_TABLE_IDX)->lLineSize))
	{
		HtLog(ERR_PARAM, "��ʼ���ڴ���ʧ��, err:(%s)", sGetError());
		vDestroyTree(pstRoot);
		vTblDisconnect(SYS_TABLE_IDX);
		return RC_FAIL;
	}

	//  ����ͷ���
	memcpy(pvData, (void *)pGetTblDef(SYS_TABLE_IDX), sizeof(TblDef));

	//	�����������
 	((IBPShm *)pGetShmRun())->m_lOffset = 0;
	vInorderTree((char *)pGetTblAddr(SYS_TABLE_IDX, lGetShmPos(SYS_TABLE_IDX)), pstRoot);
	memcpy((void *)pGetTblAddr(SYS_TABLE_IDX, lGetShmPos(SYS_TABLE_IDX)), 
		(void *)pInitSHTree(pstRoot), sizeof(SHTree));
	vDestroyTree(pstRoot);
	vTblDisconnect(SYS_TABLE_IDX);

	if(RC_SUCC != lRegisterTable(SYS_TABLE_IDX, lGetShmId(), TYPE_SYSTEM, 
		pGetTblDef(SYS_TABLE_IDX)->lLineSize, (TblDef *)pGetTblDef(SYS_TABLE_IDX)))
	{
		HtLog(ERR_PARAM, "ע���(%s)ʧ��!", sGetTableName(SYS_TABLE_IDX));
		return RC_FAIL;
	}
	return RC_SUCC;
}

/****************************************************************************************
	����ϵͳ������
 ****************************************************************************************/
long	lUpdateSysIdx(BSysIdx *pstSys)
{
	BSysIdx		stSIdx;
	RunTime		*pstRun = NULL;

	memset(&stSIdx, 0, sizeof(stSIdx));
	stSIdx.m_table = lSetEXLong(pstSys->m_table);

	pstSys->m_ShmId = lSetEXLong(0);
	pstSys->m_SemId = lSetEXLong(0);
	pstSys->m_lRecord = lSetEXLong(0);
	pstSys->m_lMaxRows = lSetEXLong(0);
	pstSys->m_lLSize = lSetEXLong(0);

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_TABLE_IDX)))
  		return RC_FAIL;

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = SYS_TABLE_IDX;
	pstRun->lSize = sizeof(BSysIdx);
	pstRun->pstVoid = &stSIdx;

	if(RC_SUCC != lUpdateSpeed(pstRun, pstSys, sizeof(BSysIdx)))
	{
		fprintf(stderr, "���±�(%s), ���¼�¼(%d)ʧ��, err:(%s)!\n", 
			pstSys->m_szTable, lGetEffect(), sGetError());
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
	ɾ�����б�
 ****************************************************************************************/
long	lDropTables(Benum em, TABLE t, BOOL bDebug)
{
	TABLE		i = 0, lTable = 0; 
	BSysIdx		stSys, *pstSys = NULL;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_TABLE_IDX)))
		return RC_FAIL;

	memset(&stSys, 0, sizeof(stSys));
	if(TABLE_ONLY == em)
		stSys.m_table = lSetEXLong(t);
	else		//	TABLE_ALL
		;

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = SYS_TABLE_IDX;
	pstRun->lSize = sizeof(BSysIdx);
	pstRun->pstVoid = &stSys;

	if(RC_SUCC != lRunSelect(pstRun, (void *)&pstSys, (long *)&lTable))
		return RC_FAIL;

	for(i = 0; i < lTable; i ++)
	{
		lGetEXLong((long *)&pstSys[i].m_ShmId);
		lGetEXLong((long *)&pstSys[i].m_table);
		((IBPShm *)pGetShmRun())->m_shmID = pstSys[i].m_ShmId;

		if(pstSys[i].m_ShmId <= 0)
			continue;

		if(RC_SUCC != lInitMemTable(pstSys[i].m_table, TYPE_CLIENT))
		{
			fprintf(stderr, "ɾ��(%d/%d)��(%d)(%s)ʧ��, err:%s\n", i, lTable, 
				pstSys[i].m_table, pstSys[i].m_szTable, sGetError());
			continue;
		}

		//	��ɾ���ͻ������ɾ������
		vDeleteTable(pstSys[i].m_ShmId, pstSys[i].m_table);
		vDeleteTableSem(pstSys[i].m_SemId, pstSys[i].m_table);
		if(bDebug)
			fprintf(stdout, "��ʼɾ����(%s), shmID(%d), semId(%d) completed successfully!\n", 
				pstSys[i].m_szTable, pstSys[i].m_ShmId, pstSys[i].m_SemId);

		if(TABLE_ONLY == em)	//	�����Ƿ���³ɹ�������������
			lUpdateSysIdx(&pstSys[i]);
	}
	IBPFree(pstSys);

	return RC_SUCC;
}

/****************************************************************************************
	�����ͻ���
 ****************************************************************************************/
long	lCreateTable(char *pszSettleDate, char *pszSettleNum, TABLE table)
{
	void	*pvData = NULL;
	Tree	*pstRoot = NULL;
	long	lShmId = 0, lTblSize = 0;
	BOOL	bInit = false;

	memset(&g_stTblDef[table], 0, sizeof(TblDef));

	//	����ָ��Key������
	lTblSize = lTableMethod(table, NULL, pszSettleDate, pszSettleNum, TBL_SIZE);
	if(lTblSize <= RC_FAIL)
		return RC_FAIL;

	if(RC_SUCC != lInitCreateShm(table, lTblSize, false))
		return RC_FAIL;

	if(RC_SUCC != lAttachShm(table))
		return RC_FAIL;

	if(!(pvData = (void *)pGetShmAddr(table)))
	{
		HtLog(ERR_PARAM, "��ȡϵͳ�������ַʧ��, err:(%s)", sGetError());
		return RC_FAIL;
	}

	if(RC_SUCC != lTableMethod(table, &pstRoot, pszSettleDate, pszSettleNum, TBL_DATA))
	{
		HtLog(ERR_PARAM, "װ�ر�(%d)����ʧ��, err:(%s)", table, sGetError());
		vDeleteTable(lGetShmId(), table);
		return RC_FAIL;
	}

	//  ����ͷ���
	memcpy(pvData, (void *)pGetTblDef(table), sizeof(TblDef));

	//	�����������
	((IBPShm *)pGetShmRun())->m_lOffset = 0;
   	vInorderTree((char *)pGetTblAddr(table, lGetShmPos(table)), pstRoot);
   	memcpy((void *)pGetTblAddr(table, lGetShmPos(table)), (void *)pInitSHTree(pstRoot), sizeof(SHTree));
	vDestroyTree(pstRoot);
	vTblDisconnect(table);

	//	����
	bInit	= ((IBPShm *)pGetShmRun())->m_bInit[table];
	lShmId	= ((IBPShm *)pGetShmRun())->m_shmID;
		
	//	ע��֮ǰ������ɾ��ԭ����
	if(RC_SUCC != lDropTables(TABLE_ONLY, table, false))
		return RC_FAIL;

	//	��ԭ
	((IBPShm *)pGetShmRun())->m_shmID			= lShmId;
	((IBPShm *)pGetShmRun())->m_bInit[table]	= bInit;
	((IBPShm *)pGetShmRun())->m_pszAddr[table]	= pvData;

	if(RC_SUCC != lRegisterTable(table, lGetShmId(), TYPE_CLIENT, pGetTblDef(table)->lLineSize, 
		(TblDef *)pGetTblDef(table)))
	{
		vSetErrMsg("ע���(%s)ʧ��!", sGetTableName(table));
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/****************************************************************************************
	code end
 ****************************************************************************************/
