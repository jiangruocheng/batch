#include	"load.h"

/****************************************************************************************
 *	全局变量申请区 
 ****************************************************************************************/
IBPShm		g_ShmRun = {0};
RunTime		g_RunTime = {0};

/****************************************************************************************
 * 	必要内部商户申明	
 ****************************************************************************************/
void	vTblDisconnect(TABLE t);

/****************************************************************************************
 * 	获取查询结构
 ****************************************************************************************/
void*	pGetRunTime()
{
	return &g_RunTime;
}

/****************************************************************************************
 * 	获取连接创建共享内存的KEY值
 ****************************************************************************************/
IBPShm*	pGetShmRun()
{
	return &g_ShmRun;
}

/****************************************************************************************
 * 	初始化查询结构
 ****************************************************************************************/
void*	pInitRunTime(TABLE t)
{
	long		lRet = 0;
	BSysIdx		stSys;

	memset(&g_RunTime, 0, sizeof(g_RunTime));

	//	无论是否系统表，都必须从系统表查找起
/*
	if(t == SYS_TABLE_IDX)
	{	
		((IBPShm *)pGetShmRun())->m_lLSize[SYS_TABLE_IDX] = sizeof(BSysIdx);
		return &g_RunTime;
	}
*/
	memset(&stSys, 0, sizeof(stSys));
	stSys.m_table = lSetEXLong(t);

	g_RunTime.bSearch = TYPE_SYSTEM;
	g_RunTime.lFind = IDX_SELECT;
	g_RunTime.tblName = SYS_TABLE_IDX;
	g_RunTime.lSize = sizeof(BSysIdx);
	g_RunTime.pstVoid = &stSys;
	((IBPShm *)pGetShmRun())->m_lLSize[SYS_TABLE_IDX] = sizeof(BSysIdx);
	lRet = lSelectSpeed(&g_RunTime, &stSys);
	if(lRet != RC_SUCC)
		return NULL;

	lGetEXLong((long *)&stSys.m_yKey);
	lGetEXLong((long *)&stSys.m_ShmId);
	lGetEXLong((long *)&stSys.m_SemId);
	lGetEXLong((long *)&stSys.m_lLSize);
	lGetEXLong((long *)&stSys.m_lType);

	((IBPShm *)pGetShmRun())->m_lLSize[t] = stSys.m_lLSize;
	((IBPShm *)pGetShmRun())->m_bInit[t] = false;
	((IBPShm *)pGetShmRun())->m_bAttch = 0;
	((IBPShm *)pGetShmRun())->m_yKey = stSys.m_yKey;
	((IBPShm *)pGetShmRun())->m_shmID = stSys.m_ShmId;
	((IBPShm *)pGetShmRun())->m_semID = stSys.m_SemId;
	((IBPShm *)pGetShmRun())->m_pszAddr[SYS_TABLE_IDX] = NULL;

	//	获取客户表ID成功后，改用客户表方式查询
	memset(&g_RunTime, 0, sizeof(g_RunTime));
//	g_RunTime.bSearch = stSys.m_lType;
	g_RunTime.bSearch = TYPE_CLIENT;

	return &g_RunTime;
}

/****************************************************************************************
	获取更新/删除记录数
 ****************************************************************************************/
long	lGetEffect()
{
	return	((IBPShm *)pGetShmRun())->m_lEffect;
}

/****************************************************************************************
 * 	让进程保持与内存的连接
 ****************************************************************************************/
void	vHoldConnect()
{
//	建议不要轻易使用， 有些系统只能attch固定次数， 查询设置该参数就不能关闭共享内存连接
//	导致再次连接出现 12:Cannot allocate memory 
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_CONNECT;
}

/****************************************************************************************
 * 	端开与共享内存的连接
 ****************************************************************************************/
void	vDisConnect(TABLE t)
{
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_DISCONNECT;
	vTblDisconnect(t);
}

/****************************************************************************************
 *	是否保持连接
 ****************************************************************************************/
void	vTblInitParam(TABLE table)
{
	((IBPShm *)pGetShmRun())->m_lCurLine[table] = 0;
	((IBPShm *)pGetShmRun())->m_pvCurAddr[table] = NULL;
	if(SHM_CONNECT == ((IBPShm *)pGetShmRun())->m_bAttch)
		return ;
	else
		vDisConnect(table);
}

/****************************************************************************************
 * 	获取连接创建共享内存的KEY值
 ****************************************************************************************/
long	lGetShmId()
{
	if(g_ShmRun.m_shmID <= 0)	
	{
		vSetErrMsg("共享内存还未初始化，请先初始化");
		return RC_FAIL;
	}
	return g_ShmRun.m_shmID;
}

/****************************************************************************************
 * 	获取连接创建共享内存的KEY值
 ****************************************************************************************/
void*	pGetShmAddr(TABLE t)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[t])
	{
		vSetErrMsg("共享内存还未初始化，请先初始化");
		return NULL;
	}

	return g_ShmRun.m_pszAddr[t];
}

/****************************************************************************************
	指向表数据的地址
 ****************************************************************************************/
void*	pGetTblAddr(TABLE t, long nOffset)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[t])
	{
		vSetErrMsg("共享内存还未初始化，请先初始化");
		return NULL;
	}

	return g_ShmRun.m_pszAddr[t] + g_ShmRun.m_lOffset + nOffset;
}

/****************************************************************************************
 * 	获取连接创建共享内存的KEY值
 ****************************************************************************************/
key_t	yGetShmKey()
{
	return g_ShmRun.m_yKey;
}

/****************************************************************************************
 * 	获取连接创建共享内存的KEY值
 ****************************************************************************************/
key_t	yGetIPCPath(Benum em)
{
	char	szKeyPath[512];
	key_t   yKey;

	memset(szKeyPath, 0, sizeof(szKeyPath));
	snprintf(szKeyPath, sizeof(szKeyPath), "%s/bin", getenv("HOME"));
	
	yKey = ftok(szKeyPath, em);
	if(yKey <= RC_FAIL)
	{
		vSetErrMsg("获取内存Key值失败(%s):(%s)", szKeyPath, strerror(errno));
		return RC_FAIL;
	}

	return yKey;
}

/****************************************************************************************
 * 	功能  :设置操作过程中的错误描述
 ****************************************************************************************/
void	vSetErrMsg(char *s, ...)
{
	va_list ap;

	memset(g_ShmRun.m_szMsg, 0, sizeof(g_ShmRun.m_szMsg));
	va_start(ap, s);
	vsnprintf(g_ShmRun.m_szMsg, sizeof(g_ShmRun.m_szMsg), s, ap);
	va_end(ap);
}

/****************************************************************************************
 * 	功能  :操作过程中的错误描述
 ****************************************************************************************/
char*	sGetError()
{
	return g_ShmRun.m_szMsg;
}

/****************************************************************************************
 * 	功能  :创建共享内存
 * 	参数  :
 ****************************************************************************************/
long	lInitCreateShm(TABLE t, long lSize, BOOL bCreate)
{
	if(lSize <= 0)
	{
		vSetErrMsg("错误, 创建空间大小(%d)<0", lSize);
		return RC_FAIL;
	}

	memset(&g_ShmRun, 0, sizeof(g_ShmRun));

	//	必须重建共享内存，每次的数量都是没法估计的
	if(bCreate)
	{
		((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath(IPC_SHM);
		if(((IBPShm *)pGetShmRun())->m_yKey <= RC_FAIL)
			return RC_FAIL;

		((IBPShm *)pGetShmRun())->m_shmID = shmget(((IBPShm *)pGetShmRun())->m_yKey, lSize, 
			IPC_CREAT|IPC_EXCL|0660);
	}
	else
		((IBPShm *)pGetShmRun())->m_shmID = shmget(IPC_PRIVATE, lSize, IPC_CREAT|IPC_EXCL|0660);
	if(RC_FAIL >= ((IBPShm *)pGetShmRun())->m_shmID)
	{
		vSetErrMsg("创建共享内存失败:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr[t] = (void *)shmat(((IBPShm *)pGetShmRun())->m_shmID, NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr[t] || 
		(void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr[t])
	{	   
		vSetErrMsg("连接共享内存失败:(%s)", strerror(errno));
		return RC_FAIL;
	}

	memset(((IBPShm *)pGetShmRun())->m_pszAddr[t], 0, lSize);
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr[t]);
	((IBPShm *)pGetShmRun())->m_pszAddr[t] = NULL;
	((IBPShm *)pGetShmRun())->m_bInit[t] = false;

	return RC_SUCC;
}

/****************************************************************************************
	初始化内存指针
 ****************************************************************************************/
long	lInitTblShm(long lType)
{
	long	bAttch = ((IBPShm *)pGetShmRun())->m_bAttch;

	//	如果是客户表。ID在系统表中登记，无需根据Kye值获取
	if(TYPE_CLIENT == lType)	return RC_SUCC;

	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = bAttch;

	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath(IPC_SHM);
	if(((IBPShm *)pGetShmRun())->m_yKey == RC_FAIL)
	{
		vSetErrMsg("连接内存的Key已无效");
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_shmID = shmget(((IBPShm *)pGetShmRun())->m_yKey, 0, IPC_CREAT|0660);
	if(RC_FAIL == ((IBPShm *)pGetShmRun())->m_shmID)
	{
		vSetErrMsg("连接共享内存失败, 该内存块可能已消失:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
	断开与共享内存的链接 
 ****************************************************************************************/
void	vTblDisconnect(TABLE t)
{
	if(((IBPShm *)pGetShmRun())->m_pszAddr[t])
		shmdt(((IBPShm *)pGetShmRun())->m_pszAddr[t]);
	((IBPShm *)pGetShmRun())->m_pszAddr[t] = NULL;
	((IBPShm *)pGetShmRun())->m_bInit[t] = false;
}

/****************************************************************************************
 * 	链接共享内存	
 ****************************************************************************************/
long	lAttachShm(TABLE t)
{
	void	*p = NULL;

	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}	   

	p = (void* )shmat(lGetShmId(), NULL, 0);
	if(NULL == (char *)p || (void *)(-1) == (void *)p)
	{	   
		vSetErrMsg("链接共享内存失败:(%d)(%d)(%s)!", lGetShmId(), errno, strerror(errno));
		return RC_FAIL;
	}
	((IBPShm *)pGetShmRun())->m_pszAddr[t] = p;
	((IBPShm *)pGetShmRun())->m_bInit[t] = true;

	return RC_SUCC;
}

/****************************************************************************************
 * 	删除共享内存	
 ****************************************************************************************/
void	vDeleteTable(long lShmId, TABLE t)
{
	int		iRet = 0;

	//	先断开在删除
	vTblDisconnect(t);

	((IBPShm *)pGetShmRun())->m_shmID = lShmId;

	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return ;
	}	   

	iRet = shmctl(((IBPShm *)pGetShmRun())->m_shmID, IPC_RMID, NULL);
	if(iRet)
	{	   
		vSetErrMsg("删除共享内存失败:(%s)!", strerror(errno));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr[t] = NULL;
}

/****************************************************************************************
	指向对查询出来的结果的首地址	
 ****************************************************************************************/
char*	pGetSelectAddr(TABLE t)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[t])
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return NULL;
	}
	return (char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[t];
}

/****************************************************************************************
 *	 连接共享内存并初始化必要索引
 ****************************************************************************************/
long	lInitMemTable(TABLE t, long lType)
{
	long	lRet = 0;

	lRet = lInitTblShm(lType);
	if(lRet)	return RC_FAIL;

	//	有些机器 只能attch一次
	lRet = lAttachShm(t);
	if(lRet) return RC_FAIL;

	//  初始化头结点
	vSetTblDef(t, (char *)pGetShmAddr(t));
	
	//  退出信号	vTblDisconnect();
	//  sigal(

	return RC_SUCC;
}

/****************************************************************************************
 * 	检查字符串是否都是0x00	
 ****************************************************************************************/
long	lIsNRealZore(char *s, long l)
{
	long	i = 0, lFlag = 0;

	for(i = 0; i < l; i ++)
	{
		if(0x00 != s[i])
		{
			lFlag = 1;
			break;
		}
	}
	
	if(lFlag)	return RC_FAIL;
	else		return RC_SUCC;
}

/****************************************************************************************
	对唯一索引进行验证
 ****************************************************************************************/
long	lTblIdxCompare(RunTime *pstRun, void *p, SHTree *pstTree)
{
	long	nCompare = 0, i = 0;
	TblKey  *pstIdx = pGetTblIdx(pstRun->tblName);

	nCompare = lGetIdxNum(pstRun->tblName);
	if(0 == nCompare)	   //  对于未设置主索引，无论采用什么方式查询直接返回不匹配
		return RC_FAIL;

	for(i = 0; i < nCompare; i ++)  //  下面比较的都是唯一索引
	{
		if(IDX_SELECT != pstIdx[i].m_lIsPk || pstTree->m_lIdxPos != pstIdx[i].m_lKeyOffSet)
			continue;

		if(!memcmp((char *)pstRun->pstVoid, (char *)p + pstTree->m_lIdxPos, pstTree->m_lIdxLen))
			return RC_SUCC;
	}

	return RC_FAIL;
}

/****************************************************************************************
 * 	表按照唯一索引查询
 ****************************************************************************************/
long	lTblIdxMatch(RunTime *pstRun, void *p, long lFindType)
{
	long	lRet = 0, nCompare = 0, i = 0;
	char	szTemp[50];
	TblKey  *pstIdx = pGetTblIdx(pstRun->tblName);
	
	memset(szTemp, 0, sizeof(szTemp));
	nCompare = lGetIdxNum(pstRun->tblName);
	//	 对于未设置主索引，无论采用什么方式查询直接返回不匹配
	if(0 == nCompare && IDX_SELECT == lFindType)
		return RC_FAIL;

	for(i = 0; i < nCompare; i ++)	//	下面比较的都是唯一索引
	{
		//	如果按照索引来查询，其他查询条件将被剔除
		//	如果不是按照查询索引来查，那么所有查询条件将会去匹配
		if(IDX_SELECT == lFindType)
		{
			if(pstRun->lFind != pstIdx[i].m_lIsPk)
				continue;
		}

		switch(pstIdx[i].m_lKeyAttr)
		{
		case FIELD_LONG:	//	先采用一致处理
			//	无论是否查询索引还是主索引，CHAR类型未赋值，表示不查询
			//	0不处理，设置为0，用LONG_ZER
			if(!memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, szTemp, pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, 
				(char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)					//	任意一条主索引查找直接返回
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else	//  无论使主索引不匹配还是查询索引不匹配，退出该记录不匹配, 无需继续查找
				return RC_FAIL;	
			break;	//	匹配该查询索引了，break出来， 还要匹配其他的查询索引
		case FIELD_CHAR:
			//	无论是否查询索引还是主索引，CHAR类型未赋值，表示不查询
			if(RC_SUCC == lIsNRealZore((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, 
				pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, 
				(char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	任意一条主索引查找直接返回
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else	//	无论使主索引不匹配还是查询索引不匹配，推出该记录不匹配?无需继续查找
				return RC_FAIL;
			break;
		default:
			vSetErrMsg("错误的字段类型(%d)", pstIdx[i].m_lKeyAttr);
			return RC_FAIL;
		}
	}
	if(IDX_SELECT == lFindType)	//	主索引匹配有for循环完毕未找到，返回失败
		return RC_FAIL;

	//	查询索引不能在for循环里面返回，如果for里面没有可匹配的字段，本条默认查询匹配
	return RC_SUCC;
}
/****************************************************************************************
	定义游标起始地址
 ****************************************************************************************/
long	lTableDeclare(RunTime *pstRun)
{
	long	lRet = 0;

	if(!pstRun)	return RC_FAIL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if((0 == pstRun->lSize && pstRun->pstVoid) || (0 != pstRun->lSize && !pstRun->pstVoid))
	{
		vSetErrMsg("设置查询条件错误(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return RC_FAIL;
	}

	if((char *)g_ShmRun.stRunTime[pstRun->tblName].pstVoid)
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("定义游标出错, 游标重复定义");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(pstRun->tblName, lGetTblPos(pstRun->tblName));
	
	if(pstRun->pstVoid)
	{
		g_ShmRun.stRunTime[pstRun->tblName].pstVoid = (char *)malloc(pstRun->lSize);
		memset((char *)g_ShmRun.stRunTime[pstRun->tblName].pstVoid, 0, pstRun->lSize);
		memcpy((char *)g_ShmRun.stRunTime[pstRun->tblName].pstVoid, pstRun->pstVoid, pstRun->lSize);
	}
	else
		pstRun->lFind = CHK_SELECT;	

	g_ShmRun.stRunTime[pstRun->tblName].lFind = pstRun->lFind;
	g_ShmRun.stRunTime[pstRun->tblName].tblName = pstRun->tblName;
	g_ShmRun.stRunTime[pstRun->tblName].lSize = pstRun->lSize;

	return RC_SUCC;
}

/****************************************************************************************
	关闭游标指针
 ****************************************************************************************/
void	vTableClose(TABLE table, long lType)
{
	vTblInitParam(table);
	if((char *)g_ShmRun.stRunTime[table].pstVoid)
	{
		free((char *)g_ShmRun.stRunTime[table].pstVoid);
		g_ShmRun.stRunTime[table].pstVoid = NULL;
	}
	memset((char *)&g_ShmRun.stRunTime[table], 0, sizeof(RunTime));

	if(SHM_CONNECT == lType)	//	保持连接
		return ;
	vTblDisconnect(table);
}

/****************************************************************************************
	获取下一下记录	
 ****************************************************************************************/
long	lTableFetch(TABLE table, void *pszVoid, long lSize)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[table])
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return RC_FAIL;
	}

	if(!(char *)(((IBPShm *)pGetShmRun())->m_pvCurAddr[table]))
	{
		vSetErrMsg("无效的游标或者游标未定义(%d)", table);
		return RC_FAIL;
	}

	if(((IBPShm *)pGetShmRun())->m_lLSize[table] != lSize)
	{
		vSetErrMsg("内存表(%s)版本存在问题，(%d)(%d)请升/降重试", sGetTableName(table),
			((IBPShm *)pGetShmRun())->m_lLSize[table], lSize);
		return RC_FAIL;
	}

	while(1)
	{
		if(lGetTblValid(table) < g_ShmRun.m_lCurLine[table])
		{
			vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
			return RC_NOTFOUND;
		}

		if((char *)g_ShmRun.stRunTime[table].pstVoid)
		{
			//	游标查询肯定按照查询索引来寻找
			if(RC_SUCC != lTblIdxMatch(&g_ShmRun.stRunTime[table], 
				((IBPShm *)pGetShmRun())->m_pvCurAddr[table], CHK_SELECT))
			{
				g_ShmRun.m_lCurLine[table] ++;
				((IBPShm *)pGetShmRun())->m_pvCurAddr[table] += lSize;
				continue;	
			}
		}

		memcpy((char *)pszVoid, (char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[table], lSize);

		g_ShmRun.m_lCurLine[table] ++;
		((IBPShm *)pGetShmRun())->m_pvCurAddr[table] += lSize;
		break;
	}

	return RC_SUCC;
}

/****************************************************************************************
	按照索引获取极值
 ****************************************************************************************/
long	lGetExtremeIdx(RunTime *pstRun, void *pstVoid, long lMask)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("设置查询条件或规则错误(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("表(%d)未设置索引，查询方式不被支持", pstRun->tblName);
		return RC_FAIL;
	}

	if(FIELD_MAX == lMask)
		pstTree = (SHTree *)pGetMaxSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)));
	else if(FIELD_MIN == lMask)
		pstTree = (SHTree *)pGetMinSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)));
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr(pstRun->tblName) + pstTree->m_lData, pstRun->lSize);
	vTblInitParam(pstRun->tblName);
	return RC_SUCC;
}

/****************************************************************************************
	在AVL树上搜索唯一索引记录
 ****************************************************************************************/
long	lQuerySpeed(RunTime *pstRun, void *pstVoid)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("设置查询条件或规则错误(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != pstRun->lSize)
	{
		vSetErrMsg("内存表(%s)版本存在问题，(%d)(%d)请升/降重试!", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("表(%d)(%s)未设置快速查询节点或数据未装载", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_NOTFOUND;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)), 
		0, pstRun->pstVoid);
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr(pstRun->tblName) + pstTree->m_lData, pstRun->lSize);

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  避免出现问题，再次匹配一下, 只匹配主索引
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
 			vTblInitParam(pstRun->tblName);
			vSetErrMsg("严重错误，索引与表中数据不匹配");
			return RC_FAIL;
		}
	}

	vTblInitParam(pstRun->tblName);

	return RC_SUCC;
}

/****************************************************************************************
 * 快速查询，在内存表中匹配记录立刻返回，要求输入的查询条件必须使主索引	
 ****************************************************************************************/
long	lSelectSpeed(RunTime *pstRun, void *pstVoid)
{
	long	lRet = 0, lCount = 0;
	void	*p = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("设置查询条件或规则错误(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != pstRun->lSize)
	{
		vSetErrMsg("内存表(%s)版本存在问题，(%d)(%d)请升/降重试!", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("表(%d)索引未设置，查询方式不被支持", pstRun->tblName);
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(pstRun->tblName, lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("无效的游标或者游标未定义");
		vTblInitParam(pstRun->tblName);
		return RC_FAIL;
	}

	while(1)
	{
		if(lGetTblValid(pstRun->tblName) < g_ShmRun.m_lCurLine[pstRun->tblName])
			break;

		if(RC_SUCC == lTblIdxMatch(pstRun, ((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName], IDX_SELECT))
		{
			lCount ++;
			p = ((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName];
			break;
		}
		g_ShmRun.m_lCurLine[pstRun->tblName] ++;
		((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName] += pstRun->lSize;
	}
	if(0 == lCount)
	{
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		vTblInitParam(pstRun->tblName);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)p, pstRun->lSize);
	vTblInitParam(pstRun->tblName);

	return RC_SUCC;
}

/****************************************************************************************
 * 	查询单条记录
 ****************************************************************************************/
long	lSelectTable(RunTime *pstRun, void *pstVoid)
{
	long	lRet = 0, lCount = 0;
	void	*p = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid)
	{
		vSetErrMsg("设置查询条件错误(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(pstRun->tblName, 
		lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("无效的游标或者游标未定义");
		vTblInitParam(pstRun->tblName);
		return RC_FAIL;
	}

	while(1)
	{
		if(lGetTblValid(pstRun->tblName) < g_ShmRun.m_lCurLine[pstRun->tblName])
			break;

		if(RC_SUCC == lTblIdxMatch(pstRun, ((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName], CHK_SELECT))
		{
			lCount ++;
			if(1 < lCount)
			{
				vSetErrMsg("存在至少(%d)多行记录被选择!", lCount);
				vTblInitParam(pstRun->tblName);
				return RC_FAIL;
			}
			p = ((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName];
		}
		g_ShmRun.m_lCurLine[pstRun->tblName] ++;
		((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName] += pstRun->lSize;
	}
	if(0 == lCount)
	{
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		vTblInitParam(pstRun->tblName);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)p, pstRun->lSize);
	vTblInitParam(pstRun->tblName);

	return RC_SUCC;
}

/****************************************************************************************
	更新记录	注:无进程维护树的结构, 因此不能更新AVL节点
 ****************************************************************************************/
long	lUpdateQuick(RunTime *pstRun, void *pstVoid, long lSize)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("设置查询条件或规则错误(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != lSize)
	{   
		vSetErrMsg("内存表(%s)版本存在问题，(%d)(%d)请升/降重试", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], lSize);
		return RC_FAIL;
	}   

	if(((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("表(%d)(%s)已正在处理，无法更新处理", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_FAIL;
	}   

	//	P 操作
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_P))
		return RC_FAIL;

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化");
		return RC_FAIL;
	}   

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("表(%d)(%s)未设置快速查询节点或数据未装载", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_NOTFOUND;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)),
		0, pstRun->pstVoid);
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  避免出现问题，再次匹配一下, 只匹配主索引
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
			vTblInitParam(pstRun->tblName);
			vSetErrMsg("严重错误，索引与表中数据不匹配!");
			return RC_FAIL;
		}
	}

	memcpy((char *)pGetShmAddr(pstRun->tblName) + pstTree->m_lData, (char *)pstVoid, pstRun->lSize);

	vTblInitParam(pstRun->tblName);

	//	V 操作
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_V))
		return RC_FAIL;

	return RC_SUCC;
}


/****************************************************************************************
	更新记录
 ****************************************************************************************/
long	lUpdateSpeed(RunTime *pstRun, void *pstVoid, long lSize)
{
	long	lRet = 0;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{   
		vSetErrMsg("设置查询条件或规则错误(%d)", pstRun->lFind);
		return RC_FAIL;
	}   

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != lSize)
	{   
		vSetErrMsg("内存表(%s)版本存在问题，(%d)(%d)请升/降重试!", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], lSize);
		return RC_FAIL;
	}   

	if(((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("表(%d)(%s)已正在处理，无法更新处理!", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_FAIL;
	}   

	//	P 操作
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_P))
		return RC_FAIL;

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}   

	if(0 == lGetIdxNum(pstRun->tblName))
	{   
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("表(%d)索引未设置，查询方式不被支持", pstRun->tblName);
		return RC_FAIL;
	}   

	((IBPShm *)pGetShmRun())->m_lEffect  = 0;
	((IBPShm *)pGetShmRun())->m_lCurLine[pstRun->tblName] = 1;
	((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName] = 
		(void *)pGetTblAddr(pstRun->tblName, lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{   
		vSetErrMsg("无效的游标或者游标未定义");
		vTblInitParam(pstRun->tblName);
		return RC_FAIL;
	}   

	while(1)
	{   
		if(lGetTblValid(pstRun->tblName) < ((IBPShm *)pGetShmRun())->m_lCurLine[pstRun->tblName])
			break;

		if(RC_SUCC == lTblIdxMatch(pstRun, ((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName],
			pstRun->lFind))
		{
			((IBPShm *)pGetShmRun())->m_lEffect ++;
			memcpy(((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName], (char *)pstVoid, lSize);
			if(IDX_SELECT == pstRun->lFind)
				break;
		}

		((IBPShm *)pGetShmRun())->m_lCurLine[pstRun->tblName] ++;
		((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName] += lSize;
	}

	vTblInitParam(pstRun->tblName);

	//	V 操作
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_V))
		return RC_FAIL;

	return RC_SUCC;
}

/****************************************************************************************
 *	按规则查询全表，返回满足条件所有记录
 ****************************************************************************************/
long	lRunSelect(RunTime *pstRun, void **ppszOut, long *plOut)
{
	long	lRet = 0, lCount = 0;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid)
	{
		vSetErrMsg("设置查询条件错误(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(pstRun->tblName, lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("无效的游标或者游标未定义");
		vTblInitParam(pstRun->tblName);
		return RC_FAIL;
	}

	while(1)
	{
		if(lGetTblValid(pstRun->tblName) < g_ShmRun.m_lCurLine[pstRun->tblName])
			break;

		if(RC_SUCC == lTblIdxMatch(pstRun, ((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName], CHK_SELECT))
		{
			if(0 == lCount)
				*ppszOut = (char *)malloc(pstRun->lSize);
			else
				*ppszOut = (char *)realloc(*ppszOut, pstRun->lSize * (lCount + 1));
			if(NULL == *ppszOut)
			{
				vSetErrMsg("分配大小(%d)内存失败!", pstRun->lSize * (lCount + 1));
				vTblInitParam(pstRun->tblName);
				return RC_FAIL;
			}
			memcpy((char *)*ppszOut + lCount * pstRun->lSize, (char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName], 
				pstRun->lSize);
			lCount ++;
		}
		g_ShmRun.m_lCurLine[pstRun->tblName] ++;
		((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName] += pstRun->lSize;
	}
	*plOut = lCount;
	vTblInitParam(pstRun->tblName);
	if(0 == lCount)
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);

	return RC_SUCC;
}

/****************************************************************************************
	code end 
 ****************************************************************************************/
