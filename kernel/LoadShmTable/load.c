#include	"Ibp.h"
#include	"DbsApi.h"
#include	"pos_stlm.h"
#include	"load.h"
#include	"table.h"

/****************************************************************************************
	全局变量
 ****************************************************************************************/
TblDef	g_stTblDef[SHM_TABLE_NUM] = {0};

/****************************************************************************************
	获取客户表定义
 ****************************************************************************************/
TblCreat*	pGetTblCreate(TABLE t)
{
	return &g_stCreate[t];
}

/****************************************************************************************
	客户表定义方法
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
				vSetErrMsg("未定义表(%d)的处理逻辑(%d)", t, lEnum);
				return RC_FAIL;	
			}
			return pGetTblCreate(i)->pfGetSize(t, pszSettleDate, pszSettleNum);
		}
		else if(TBL_DATA == lEnum)
		{
			if(!pGetTblCreate(i)->pfLoadData)
			{
				vSetErrMsg("未定义表(%d)的处理逻辑(%d)", t, lEnum);
				return RC_FAIL;	
			}
			return pGetTblCreate(i)->pfLoadData(t, pstRoot, pszSettleDate, pszSettleNum);
		}
		else
		{
			vSetErrMsg("未定义表(%d)的处理逻辑(%d)", t, lEnum);
			return RC_FAIL;	
		}
	}

	vSetErrMsg("未定义表(%d)转载方法", t);
	return RC_FAIL;
}

/****************************************************************************************
	获取表名称
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
	获取表序号
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
	获取内存表对应的数据库
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
	获取装载表序号
 ****************************************************************************************/
long	lGetLoadList(TABLE **ppt, long *plCount)
{
	long	lRec = 0;
	TABLE	i = 0, *pTable = NULL;

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		//	未定义的不装载
		if(!pGetTblCreate(i)->pfGetSize || !pGetTblCreate(i)->pfLoadData || 
			0 == pGetTblCreate(i)->lDbsConnect)
			continue;

		if(0 == lRec)
			pTable = (TABLE *)malloc(sizeof(TABLE));
		else
			pTable = (TABLE *)realloc(pTable, sizeof(TABLE) * (lRec + 1));
		if(NULL == pTable)
		{
			vSetErrMsg("分配大小(%d)内存失败!", sizeof(TABLE) * (lRec + 1));
			return RC_FAIL;
		}
		pTable[lRec ++] = pGetTblCreate(i)->table;
	}

	*ppt 		= pTable;
	(*plCount)	= lRec;

	return RC_SUCC;
}

/****************************************************************************************
	获取表索引的个数
 ****************************************************************************************/
long	lTblIsNExist(TABLE t)
{
	TABLE	i = 0;

	for(i = 0; i < SHM_TABLE_NUM; i ++)
	{
		if(t == pGetTblCreate(i)->table)
			return RC_SUCC;	
	}

	vSetErrMsg("表序号(%d)不存在", t);
	return RC_FAIL;
}

/********************************************************************************
	获取当前时间,时间格式YYYY/MM/DD hh:mm:ss							  
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
	对整形0的转换
 ****************************************************************************************/
long	lSetEXLong(long n)
{
	if(n <= 0)
		n += LONG_ZERO;
	return n;
}

/****************************************************************************************
	对整形0的转换
 ****************************************************************************************/
long	lGetEXLong(long *pn)
{
	if(*pn <= LONG_ZERO)
		*pn -= LONG_ZERO;
	return *pn;
}

/****************************************************************************************
	获取表指针节点
 ****************************************************************************************/
TblDef*	pGetTblDef(TABLE t)
{
	return &g_stTblDef[t];
}

/****************************************************************************************
	获取表的索引位置
 ****************************************************************************************/
long	lGetTblPos(TABLE t)
{
	return pGetTblDef(t)->lOffSet;
}

/****************************************************************************************
	AVL快速查找root节点
 ****************************************************************************************/
long	lGetShmPos(TABLE t)
{
	return pGetTblDef(t)->lShmPos;
}

/****************************************************************************************
	获取表有效数据条数
 ****************************************************************************************/
long	lGetTblValid(TABLE t)
{
	return pGetTblDef(t)->lValid;
}

/****************************************************************************************
	获取表有效数据条数
 ****************************************************************************************/
long	lGetTblCount(TABLE t)
{
	return pGetTblDef(t)->lCount;
}

/****************************************************************************************
	获取表索引起始定义
 ****************************************************************************************/
TblKey* pGetTblIdx(TABLE t)
{
	return  pGetTblDef(t)->m_stKeyIdx;
}

/****************************************************************************************
	初始化写入数据
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
	获取表索引的个数
 ****************************************************************************************/
long	lGetIdxNum(TABLE t)
{
	return  pGetTblDef(t)->m_lIdxNum;
}

/****************************************************************************************
 *  功能  :设置表索引
 *  参数  :
 ****************************************************************************************/
long	lSetTableIdx(TABLE table, long lKeyOffSet, long lKeyLen, long lKeyAttr, long lKeyType)
{
	long	i = 0;
	TblKey  *pstIdx = (TblKey *)pGetTblIdx(table);

	for(i = 0; i < lGetIdxNum(table); i++)
	{
		//  如果设置了，就直接返回成功
		if((pstIdx[i].m_lKeyOffSet == lKeyOffSet) && pstIdx[i].m_lKeyLen == lKeyLen)
			return RC_SUCC;
	}

	if(lGetIdxNum(table) >= MAX_KEY_IDX)
	{
		vSetErrMsg("表(%d)索引个数设置超限, 最大(%d)", table, SHM_TABLE_NUM);
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
	设置结构体数据
 ****************************************************************************************/
void	vSetTblDef(TABLE t, char *pszAddr)
{
	memcpy(&g_stTblDef[t], (char *)pszAddr, sizeof(TblDef));
}

/****************************************************************************************
	设置表结构属性  
 ****************************************************************************************/
long	lInitTblSize(TABLE table, long lCount, long lTblSize, long *plSize)
{
	if(RC_SUCC != lTblIsNExist(table))
		return RC_FAIL;

	pGetTblDef(table)->lLineSize = lTblSize;		//  行大小
	lCount += EXT_TABLE_GAP;						//  防止在导入内存中时候又有新的数据。
	pGetTblDef(table)->lShmPos = (*plSize);			//  查找Root节点位置
	(*plSize) += lCount * sizeof(SHTree);			//  节点最大个数
	pGetTblDef(table)->lOffSet = *plSize;			//  偏移量
	pGetTblDef(table)->lCount  = lCount;			//  记录数
	(*plSize) += lCount * lTblSize;

	return RC_SUCC;
}

/****************************************************************************************
	获取系统启动所需表大小	
 ****************************************************************************************/
long	lGetSysTblSize()
{
	long	lSize = 0, lRecord = 0;

	lSize = sizeof(TblDef);
	lRecord = SHM_TABLE_NUM + EXT_TABLE_GAP;
	if(RC_SUCC != lInitTblSize(SYS_TABLE_IDX, lRecord, sizeof(BSysIdx), &lSize))
		return RC_FAIL;

	fprintf(stdout, "开始建立系统索引表!\n");
	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_lId, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_lId失败!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_table, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_table失败!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_yKey, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_yKey失败!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetUnIdx(SYS_TABLE_IDX, BSysIdx, m_lPid, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_lPid失败!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_bAttch, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_bAttch失败!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_lRecord, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_lRecord失败!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_lMaxRows, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_lMaxRows失败!");
		return RC_FAIL;
	}

	if(RC_SUCC != IBPSetIdx(SYS_TABLE_IDX, BSysIdx, m_lLSize, FIELD_LONG))
	{
		vSetErrMsg("设置系统索引表(SYS_TABLE_IDX)字段m_lLSize失败!");
		return RC_FAIL;
	}

	fprintf(stdout, "建立系统索引表完成，completed successfully!!!\n");
	return lSize;
}

/****************************************************************************************
	初始化内存表数据
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
			HtLog(ERR_PARAM, "装载表(%d)(%s)失败, 原因:%s", stSysIdx.m_table, 
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
	判断该表是否为系统表
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
	注册表shmid
 ****************************************************************************************/
long	lRegisterTable(TABLE t, long lShmId, long lType, long lSize, TblDef *pstDef)
{
	long		lRet = 0, lSearch = 0;
	BSysIdx		stSysIdx, stIdx;
	RunTime		*pstRun = NULL;

	if(lSize <= 0 || t < 0)
	{
		vSetErrMsg("注册表(%d)失败, 内存连接ID(%d)行大小(%d)异常!", t, 
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
			pGetTblDef(SYS_TABLE_IDX)->lValid --;		//	找到记录先剔除后增加
			((IBPShm *)pGetShmRun())->m_pvCurAddr[SYS_TABLE_IDX] -= sizeof(BSysIdx);
			break;
		}
	}
	pGetTblDef(SYS_TABLE_IDX)->lValid ++;
	memcpy((void *)((IBPShm *)pGetShmRun())->m_pvCurAddr[SYS_TABLE_IDX], &stSysIdx, sizeof(BSysIdx));
	memcpy((void *)pGetShmAddr(SYS_TABLE_IDX), (void *)pGetTblDef(SYS_TABLE_IDX), sizeof(TblDef));
	vTableClose(SYS_TABLE_IDX, SHM_DISCONNECT);

	fprintf(stdout, "注册表(%s)成功，completed successfully!!!\n", sGetTableName(t));

	return RC_SUCC;
}

/****************************************************************************************
	建立系统表
 ****************************************************************************************/
long	lBuildSysParam()
{
	void	*pvData = NULL;
	Tree	*pstRoot = NULL;

	memset(&g_stTblDef[SYS_TABLE_IDX], 0, sizeof(TblDef));

	//	采用指定Key创建表
	if(RC_SUCC != lInitCreateShm(SYS_TABLE_IDX, lGetSysTblSize(), TRUE))
		return RC_FAIL;

	if(RC_SUCC != lAttachShm(SYS_TABLE_IDX))
		return RC_FAIL;

	if(!(pvData = (void *)pGetShmAddr(SYS_TABLE_IDX)))
	{
		HtLog(ERR_PARAM, "获取系统索引表地址失败, err:(%s)", sGetError());
		return RC_FAIL;
	}

	if(RC_SUCC != lInitTableIdx(&pstRoot, SYS_TABLE_IDX, lGetShmId(), pGetTblDef(SYS_TABLE_IDX)->lLineSize))
	{
		HtLog(ERR_PARAM, "初始化内存表段失败, err:(%s)", sGetError());
		vDestroyTree(pstRoot);
		vTblDisconnect(SYS_TABLE_IDX);
		return RC_FAIL;
	}

	//  填充表头结点
	memcpy(pvData, (void *)pGetTblDef(SYS_TABLE_IDX), sizeof(TblDef));

	//	填充索引区域
 	((IBPShm *)pGetShmRun())->m_lOffset = 0;
	vInorderTree((char *)pGetTblAddr(SYS_TABLE_IDX, lGetShmPos(SYS_TABLE_IDX)), pstRoot);
	memcpy((void *)pGetTblAddr(SYS_TABLE_IDX, lGetShmPos(SYS_TABLE_IDX)), 
		(void *)pInitSHTree(pstRoot), sizeof(SHTree));
	vDestroyTree(pstRoot);
	vTblDisconnect(SYS_TABLE_IDX);

	if(RC_SUCC != lRegisterTable(SYS_TABLE_IDX, lGetShmId(), TYPE_SYSTEM, 
		pGetTblDef(SYS_TABLE_IDX)->lLineSize, (TblDef *)pGetTblDef(SYS_TABLE_IDX)))
	{
		HtLog(ERR_PARAM, "注册表(%s)失败!", sGetTableName(SYS_TABLE_IDX));
		return RC_FAIL;
	}
	return RC_SUCC;
}

/****************************************************************************************
	更新系统索引表
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
		fprintf(stderr, "更新表(%s), 更新记录(%d)失败, err:(%s)!\n", 
			pstSys->m_szTable, lGetEffect(), sGetError());
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
	删除所有表
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
			fprintf(stderr, "删除(%d/%d)表(%d)(%s)失败, err:%s\n", i, lTable, 
				pstSys[i].m_table, pstSys[i].m_szTable, sGetError());
			continue;
		}

		//	先删除客户表，最后删除本身
		vDeleteTable(pstSys[i].m_ShmId, pstSys[i].m_table);
		vDeleteTableSem(pstSys[i].m_SemId, pstSys[i].m_table);
		if(bDebug)
			fprintf(stdout, "开始删除表(%s), shmID(%d), semId(%d) completed successfully!\n", 
				pstSys[i].m_szTable, pstSys[i].m_ShmId, pstSys[i].m_SemId);

		if(TABLE_ONLY == em)	//	无论是否更新成功，不做错误处理
			lUpdateSysIdx(&pstSys[i]);
	}
	IBPFree(pstSys);

	return RC_SUCC;
}

/****************************************************************************************
	创建客户表
 ****************************************************************************************/
long	lCreateTable(char *pszSettleDate, char *pszSettleNum, TABLE table)
{
	void	*pvData = NULL;
	Tree	*pstRoot = NULL;
	long	lShmId = 0, lTblSize = 0;
	BOOL	bInit = false;

	memset(&g_stTblDef[table], 0, sizeof(TblDef));

	//	采用指定Key创建表
	lTblSize = lTableMethod(table, NULL, pszSettleDate, pszSettleNum, TBL_SIZE);
	if(lTblSize <= RC_FAIL)
		return RC_FAIL;

	if(RC_SUCC != lInitCreateShm(table, lTblSize, false))
		return RC_FAIL;

	if(RC_SUCC != lAttachShm(table))
		return RC_FAIL;

	if(!(pvData = (void *)pGetShmAddr(table)))
	{
		HtLog(ERR_PARAM, "获取系统索引表地址失败, err:(%s)", sGetError());
		return RC_FAIL;
	}

	if(RC_SUCC != lTableMethod(table, &pstRoot, pszSettleDate, pszSettleNum, TBL_DATA))
	{
		HtLog(ERR_PARAM, "装载表(%d)数据失败, err:(%s)", table, sGetError());
		vDeleteTable(lGetShmId(), table);
		return RC_FAIL;
	}

	//  填充表头结点
	memcpy(pvData, (void *)pGetTblDef(table), sizeof(TblDef));

	//	填充索引区域
	((IBPShm *)pGetShmRun())->m_lOffset = 0;
   	vInorderTree((char *)pGetTblAddr(table, lGetShmPos(table)), pstRoot);
   	memcpy((void *)pGetTblAddr(table, lGetShmPos(table)), (void *)pInitSHTree(pstRoot), sizeof(SHTree));
	vDestroyTree(pstRoot);
	vTblDisconnect(table);

	//	备份
	bInit	= ((IBPShm *)pGetShmRun())->m_bInit[table];
	lShmId	= ((IBPShm *)pGetShmRun())->m_shmID;
		
	//	注册之前，首先删除原本表
	if(RC_SUCC != lDropTables(TABLE_ONLY, table, false))
		return RC_FAIL;

	//	还原
	((IBPShm *)pGetShmRun())->m_shmID			= lShmId;
	((IBPShm *)pGetShmRun())->m_bInit[table]	= bInit;
	((IBPShm *)pGetShmRun())->m_pszAddr[table]	= pvData;

	if(RC_SUCC != lRegisterTable(table, lGetShmId(), TYPE_CLIENT, pGetTblDef(table)->lLineSize, 
		(TblDef *)pGetTblDef(table)))
	{
		vSetErrMsg("注册表(%s)失败!", sGetTableName(table));
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/****************************************************************************************
	code end
 ****************************************************************************************/
