#include	"load.h"

/****************************************************************************************
 *	全局变量申请区 
 ****************************************************************************************/
IBPShm		g_ShmRun = {0};
RunTime		g_RunTime = {0};

/****************************************************************************************
 * 	必要内部商户申明	
 ****************************************************************************************/
extern  long	lGetIdxNum(TABLE table);
extern  void	vSetTblDef(char *p);
extern  TblDef* pGetTblDef(TABLE t);
extern  TblKey* pGetTblIdx(TABLE t);
void	vDetachShm();

/****************************************************************************************
 * 	初始化查询结构
 ****************************************************************************************/
void*	pInitRunTime()
{
	memset(&g_RunTime, 0, sizeof(g_RunTime));
	return &g_RunTime;
}

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
void*	pGetShmRun()
{
	return &g_ShmRun;
}

/****************************************************************************************
 * 	让进程保持与内存的连接
 ****************************************************************************************/
void	vHoldConnect()
{
//	建议不要轻易使用， 有些系统只能attch一下， 查询设置该参数就不能关闭共享内存连接
//	导致再次连接出现 12:Cannot allocate memory 
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_CONNECT;
}

/****************************************************************************************
 * 	端开与共享内存的连接
 ****************************************************************************************/
void	vDisConnect()
{
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_DISCONNECT;
	vDetachShm();
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
		vDisConnect();
}

/****************************************************************************************
 * 	获取连接创建共享内存的KEY值
 ****************************************************************************************/
long	lGetShmId()
{
	if(g_ShmRun.m_shmID <= 0)	
	{
		vSetErrMsg("共享内存还未初始化，请先初始化!");
		return RC_FAIL;
	}
	return g_ShmRun.m_shmID;
}

/****************************************************************************************
 * 	获取连接创建共享内存的KEY值
 ****************************************************************************************/
void*	pGetShmAddr()
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("共享内存还未初始化，请先初始化!");
		return NULL;
	}
	return g_ShmRun.m_pszAddr;
}

/****************************************************************************************
	指向表数据的地址
 ****************************************************************************************/
void*	pGetTblAddr(long nOffset)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("共享内存还未初始化，请先初始化!");
		return NULL;
	}
	return g_ShmRun.m_pszAddr + g_ShmRun.m_lOffset + nOffset;
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
key_t	yGetIPCPath()
{
	char	szKeyPath[512];
	key_t   yKey;

	memset(szKeyPath, 0, sizeof(szKeyPath));
	snprintf(szKeyPath, sizeof(szKeyPath), "%s/etc", getenv("HOME"));
	
	yKey = ftok(szKeyPath, 0x03);
	if(yKey <= RC_FAIL)
	{
		vSetErrMsg("获取内存Key值失败(%s):(%s)!", szKeyPath, strerror(errno));
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
long	lInitCreateShm(long	lSize)
{
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath();
	if(((IBPShm *)pGetShmRun())->m_yKey <= RC_FAIL)
		return RC_FAIL;

	if(lSize <= 0)
	{
		vSetErrMsg("错误, 创建空间大小(%d)<0", lSize);
		return RC_FAIL;
	}

	//	必须重建共享内存，每次的数量都是没法估计的
	((IBPShm *)pGetShmRun())->m_shmID = shmget(((IBPShm *)pGetShmRun())->m_yKey, lSize, 
		IPC_CREAT|IPC_EXCL|0660);
	if(RC_FAIL >= ((IBPShm *)pGetShmRun())->m_shmID)
	{
		vSetErrMsg("创建共享内存失败:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr = (void *)shmat(((IBPShm *)pGetShmRun())->m_shmID, NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr || (void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr)
	{	   
		vSetErrMsg("连接共享内存失败:(%s)", strerror(errno));
		return RC_FAIL;
	}

	memset(((IBPShm *)pGetShmRun())->m_pszAddr, 0, lSize);
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr);
	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
	((IBPShm *)pGetShmRun())->m_bInit = 0;

	return RC_SUCC;
}

/****************************************************************************************
	初始化内存指针
 ****************************************************************************************/
long	lInitTblShm()
{
	long	bAttch = ((IBPShm *)pGetShmRun())->m_bAttch;

	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = bAttch;

	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath();
	if(((IBPShm *)pGetShmRun())->m_yKey == RC_FAIL)
	{
		vSetErrMsg("连接内存的Key已无效!");
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
void	vDetachShm()
{
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr);
	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
	((IBPShm *)pGetShmRun())->m_bInit = 0;
}

/****************************************************************************************
 * 	链接共享内存	
 ****************************************************************************************/
long	lAttachShm()
{
	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}	   

	((IBPShm *)pGetShmRun())->m_pszAddr = (void* )shmat(lGetShmId(), NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr || (void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr)
	{	   
		vSetErrMsg("链接共享内存失败:(%d)(%d)(%s)!", lGetShmId(), errno, strerror(errno));
		return RC_FAIL;
	}
	((IBPShm *)pGetShmRun())->m_bInit = 1;
	return RC_SUCC;
}

/****************************************************************************************
 * 	删除共享内存	
 ****************************************************************************************/
void	vDeleteShm()
{
	int		iRet = 0;

	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return ;
	}	   

	iRet = shmctl(((IBPShm *)pGetShmRun())->m_shmID, IPC_RMID, NULL);
	if(iRet)
	{	   
		vSetErrMsg("删除共享内存失败:(%s)!", strerror(errno));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
}

/****************************************************************************************
	指向对查询出来的结果的首地址	
 ****************************************************************************************/
char*	pGetSelectAddr(TABLE table)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return NULL;
	}
	return (char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[table];
}

/****************************************************************************************
 *	 连接共享内存并初始化必要索引
 ****************************************************************************************/
long	lInitMemTable()
{
	long	lRet = 0;

	//	与共享内存仍在保持连接中
	if(1 == ((IBPShm *)pGetShmRun())->m_bInit)
	{
		//  初始化头结点
		vSetTblDef((char *)pGetShmAddr());
		return RC_SUCC;	
	}

	lRet = lInitTblShm();
	if(lRet)
		return RC_FAIL;

	//	有些机器 只能attch一次
	lRet = lAttachShm();
	if(lRet) return RC_FAIL;

	//  初始化头结点
	vSetTblDef((char *)pGetShmAddr());
	
	//  退出信号	vDetachShm();
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
	if(0 == nCompare && IDX_SELECT == lFindType)	//	对于未设置主索引，无论采用什么方式查询直接返回不匹配
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

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, (char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	任意一条主索引查找直接返回
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else
				return RC_FAIL;								//	无论使主索引不匹配还是查询索引不匹配，推出该记录不匹配, 无需继续查找
			break;											//	改个查询索引匹配了，break出来， 还要匹配其他的查询索引
		case FIELD_CHAR:
			//	无论是否查询索引还是主索引，CHAR类型未赋值，表示不查询
			if(RC_SUCC == lIsNRealZore((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, (char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	任意一条主索引查找直接返回
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else
				return RC_FAIL;								//	无论使主索引不匹配还是查询索引不匹配，推出该记录不匹配� 无需继续查找
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

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	if((char *)g_ShmRun.stRunTime[pstRun->tblName].pstVoid)
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("定义游标出错, 游标重复定义!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
	
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
	vDetachShm();
}

/****************************************************************************************
	获取下一下记录	
 ****************************************************************************************/
long	lTableFetch(TABLE table, void *pszVoid, long lSize)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	if(!(char *)(((IBPShm *)pGetShmRun())->m_pvCurAddr[table]))
	{
		vSetErrMsg("无效的游标或者游标未定义(%d)", table);
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
			if(RC_SUCC != lTblIdxMatch(&g_ShmRun.stRunTime[table], ((IBPShm *)pGetShmRun())->m_pvCurAddr[table], CHK_SELECT))
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

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("表(%d)未设置主索引未设置，查询方式不被支持", pstRun->tblName);
		return RC_FAIL;
	}

	if(FIELD_MAX == lMask)
		pstTree = (SHTree *)pGetMaxSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)));
	else if(FIELD_MIN == lMask)
		pstTree = (SHTree *)pGetMinSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)));
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);
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

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("表(%d)未设置快速查询节点!", pstRun->tblName);
		return RC_FAIL;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)), 0, pstRun->pstVoid);
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);

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

	vTblInitParam(pstRun->tblName);
	return RC_SUCC;
}

/****************************************************************************************
	对不能attch2次的系统(已经attch一次了，并未释放)，添加特殊函数
 ****************************************************************************************/
long	lQuerySpeedAt(RunTime *pstRun, void *pstVoid)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("设置查询条件或规则错误(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(!g_ShmRun.m_bInit)
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	if(!((IBPShm *)pGetShmRun())->m_pszAddr)
	{
		vSetErrMsg("共享内存数据库指针错误，可能还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vSetErrMsg("表(%d)未设置快速查询节点!", pstRun->tblName);
		return RC_FAIL;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)), 0, pstRun->pstVoid);
	if(!pstTree)
	{
		vSetErrMsg("无匹配记录, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  避免出现问题，再次匹配一下, 只匹配主索引
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
			vSetErrMsg("严重错误，索引与表中数据不匹配!");
			return RC_FAIL;
		}
	}

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

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vDetachShm();
		vSetErrMsg("表(%d)主索引未设置，查询方式不被支持", pstRun->tblName);
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
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

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
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

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("共享内存数据库还未初始化, 请先初始化!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
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
 *	应用层
 ****************************************************************************************/
/****************************************************************************************
 *	根据商户号索引获取商户信息
 ****************************************************************************************/
long	lGetMchtBase(dbMchtBase *pstMcht, char *pszMchtNo)
{
	long		lRet = 0;
	dbMchtBase	stDbMcht;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszMchtNo);
	memset(&stDbMcht, 0, sizeof(stDbMcht));
	strcpy(stDbMcht.mcht_no, pszMchtNo);
	sTrimAll(stDbMcht.mcht_no);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_BASE;
	pstRun->lSize = sizeof(stDbMcht);
//	pstRun->pstVoid = &stDbMcht;
//	lRet = lSelectSpeed(pstRun, pstMcht);
	pstRun->pstVoid = stDbMcht.mcht_no;
	lRet = lQuerySpeed(pstRun, pstMcht);
	if(lRet !=  RC_SUCC)
		return lRet;

	lGetEXLong(&pstMcht->id);
	lGetEXLong(&pstMcht->mcht_resv1);
	lGetEXLong(&pstMcht->mcht_expand);

	return RC_SUCC;
}

/****************************************************************************************
 *	根据商户号用户信息
 ****************************************************************************************/
long	lGetMchtUser(dbMchtUser *pstUser, long lUserId)
{
	long		lRet = 0;
	dbMchtUser	stUser;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	memset(&stUser, 0, sizeof(stUser));
	stUser.user_id = lSetEXLong(lUserId);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_USER;

	pstRun->lSize = sizeof(stUser);
	pstRun->pstVoid = (char *)&stUser.user_id;
	lRet = lQuerySpeed(pstRun, pstUser);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstUser->user_id);

	return RC_SUCC;
}

/****************************************************************************************
 *	根据商户号索引清算机构
 ****************************************************************************************/
long	lIsMchtSettle(char *pszBrh, long *plRes)
{
	long		lRet = 0;
	dbBrhInfo	stDbBrh;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszBrh);
	memset(&stDbBrh, 0, sizeof(stDbBrh));
	strcpy(stDbBrh.brh_code, pszBrh);
	sTrimAll(stDbBrh.brh_code);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_INFO;
	pstRun->lSize = sizeof(stDbBrh);
//	pstRun->pstVoid = &stDbBrh;
//	lRet = lSelectSpeed(pstRun, &stDbBrh);
	pstRun->pstVoid = stDbBrh.brh_code;
	lRet = lQuerySpeed(pstRun, &stDbBrh);
	if(lRet !=  RC_SUCC)
		return lRet;

	if('1' == stDbBrh.brh_tag[0])
		*plRes = RC_SUCC;	
	else
		*plRes = RC_FAIL;	

	return RC_SUCC;
}

/****************************************************************************************
 *	根据机构号索引获取机构信息
 ****************************************************************************************/
long	lIsBrhSettle(char *pszBrhCode, long *plRes)
{
	long		lRet = 0;
	dbBrhInfo	stDbBrh;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszBrhCode);
	memset(&stDbBrh, 0, sizeof(stDbBrh));
	strcpy(stDbBrh.brh_code, pszBrhCode);
	sTrimAll(stDbBrh.brh_code);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_INFO;
	pstRun->lSize = sizeof(stDbBrh);
//	pstRun->pstVoid = &stDbBrh;
//	lRet = lSelectSpeed(pstRun, &stDbBrh);
	pstRun->pstVoid = stDbBrh.brh_code;
	lRet = lQuerySpeed(pstRun, &stDbBrh);
	if(lRet != RC_SUCC)
		return lRet;

	if(BRH_STLM_YES == stDbBrh.brh_stlm_flag[0])
		*plRes = TRUE;	
	else
		*plRes = FALSE;	

	return RC_SUCC;
}

/****************************************************************************************
 *	根据机构号索引获取机构信息
 ****************************************************************************************/
long	lGetBrhInfo(dbBrhInfo *pstBrhIf, char *pszBrhCode)
{
	long		lRet = 0;
	dbBrhInfo	stDbBrh;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszBrhCode);
	memset(&stDbBrh, 0, sizeof(stDbBrh));
	strcpy(stDbBrh.brh_code, pszBrhCode);
	sTrimAll(stDbBrh.brh_code);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_INFO;
	pstRun->lSize = sizeof(stDbBrh);
//	pstRun->pstVoid = &stDbBrh;
//	lRet = lSelectSpeed(pstRun, pstBrhIf);
	pstRun->pstVoid = stDbBrh.brh_code;
	lRet = lQuerySpeed(pstRun, pstBrhIf);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstBrhIf->brh_id);
	return RC_SUCC;
}

/****************************************************************************************
*	根据MCC码索引获取MCC信息
 ****************************************************************************************/
long	lGetMccInfo(dbMchtMccInf *pstMcc, char *pszMchtTp)
{
	long			lRet = 0;
	dbMchtMccInf	stDbMcc;
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszMchtTp);
	memset(&stDbMcc, 0, sizeof(stDbMcc));
	strcpy(stDbMcc.mchnt_tp, pszMchtTp);
	sTrimAll(stDbMcc.mchnt_tp);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_MCC_INF;
	pstRun->lSize = sizeof(stDbMcc);
//	pstRun->pstVoid = &stDbMcc;
//	lRet = lSelectSpeed(pstRun, pstMcc);
	pstRun->pstVoid = stDbMcc.mchnt_tp;
	lRet = lQuerySpeed(pstRun, pstMcc);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstMcc->id);
	return RC_SUCC;
}

/****************************************************************************************
 *	获取DISC_ID所有分段的费率	
 ****************************************************************************************/
long	lGetMchtAlgo(dbMchtAlgo *pstMAlgo, char *pszModeId)
{
	long		lRet = 0;
	dbMchtAlgo	stMAlgo;	
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszModeId);
	memset(&stMAlgo, 0, sizeof(stMAlgo));
	strcpy(stMAlgo.model_id, pszModeId);
	sTrimAll(stMAlgo.model_id);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_ALGO;
	pstRun->lSize = sizeof(stMAlgo);
//	pstRun->pstVoid = &stMAlgo;
//	lRet = lSelectSpeed(pstRun, pstMAlgo);
	pstRun->pstVoid = stMAlgo.model_id;
	lRet = lQuerySpeed(pstRun, pstMAlgo);
	if(RC_SUCC != lRet)
		return lRet;

	lGetEXLong(&pstMAlgo->card_type);
	return RC_SUCC;
}

/****************************************************************************************
 *	获取机构分润主体信息
 ****************************************************************************************/
long	lGetBrhPfitInf(dbPfitInfo *pstPfitInf, char *pszModeId)
{
	long			lRet = 0;
	dbPfitInfo		stPfitInf;	
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszModeId);
	memset(&stPfitInf, 0, sizeof(stPfitInf));
	strcpy(stPfitInf.model_id, pszModeId);
	sTrimAll(stPfitInf.model_id);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_INFO;
	pstRun->lSize = sizeof(stPfitInf);
//	pstRun->pstVoid = &stPfitInf;
//	return lSelectSpeed(pstRun, pstPfitInf);
	pstRun->pstVoid = stPfitInf.model_id;

	return lQuerySpeed(pstRun, pstPfitInf);
}

/****************************************************************************************
 *	机构费率模型定义表
 ****************************************************************************************/
long	lGetBrhPfitDef(dbPfitDef *pstPfitDef, char *pszModeId)
{
	long			lRet = 0;
	dbPfitDef		stPfitDef;
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszModeId);
	memset(&stPfitDef, 0, sizeof(stPfitDef));
	strcpy(stPfitDef.model_id, pszModeId);
	sTrimAll(stPfitDef.model_id);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_DEF;
	pstRun->lSize = sizeof(stPfitDef);
//	pstRun->pstVoid = &stPfitDef;
//	return lSelectSpeed(pstRun, pstPfitDef);
	pstRun->pstVoid = stPfitDef.model_id;
	return lQuerySpeed(pstRun, pstPfitDef);
}

/****************************************************************************************
 *	根据机构号索引获取机构信息
 ****************************************************************************************/
char*	sGetCupsName(char *pszCupsNo)
{
	long			lRet = 0;
	dbMchtCupsInf	stDbCupsIf;
	static	char	szCupsName[100];
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	strcpy(stDbCupsIf.cups_no, pszCupsNo);
	sTrimAll(stDbCupsIf.cups_no);

	//	获取渠道名称，随意取第一条就够了， 不用CHK_SELECT
	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stDbCupsIf);
	pstRun->pstVoid = &stDbCupsIf;

	memset(szCupsName, 0, sizeof(szCupsName));
	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return szCupsName;

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stDbCupsIf, sizeof(stDbCupsIf));
	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	if(RC_SUCC != lRet)
		return szCupsName;

	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	snprintf(szCupsName, sizeof(szCupsName), "%s", stDbCupsIf.cups_nm);
	return szCupsName;
}

/****************************************************************************************
 *	根据机构号索引获取机构信息
 ****************************************************************************************/
char*	sGetCupsStamp(char *pszCupsNo)
{
	long			lRet = 0;
	dbMchtCupsInf	stDbCupsIf;
	static	char	szStamp[64];
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	strcpy(stDbCupsIf.cups_no, pszCupsNo);
	sTrimAll(stDbCupsIf.cups_no);

	//	获取渠道名称，随意取第一条就够了， 不用CHK_SELECT
	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stDbCupsIf);
	pstRun->pstVoid = &stDbCupsIf;

	memset(szStamp, 0, sizeof(szStamp));
	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return szStamp;

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stDbCupsIf, sizeof(stDbCupsIf));
	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	if(RC_SUCC != lRet)
		return szStamp;

	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	snprintf(szStamp, sizeof(szStamp), "%s", stDbCupsIf.cup_stamp);
	return szStamp;
}

/****************************************************************************************
 *	获取账号信息
 ****************************************************************************************/
long	lGetAcctInfo(dbAcctInfo *pstAcctNo, char *pszAppId, long lAppType, long lAttrib)
{
	long			lRet = 0;
	dbAcctInfo		stActNo;
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszAppId);
	memset(&stActNo, 0, sizeof(stActNo));
	memcpy(stActNo.app_id, pszAppId, sizeof(stActNo.app_id) - 1);
	stActNo.app_type = lAppType;
	stActNo.acct_default = lAttrib;

#if		0
	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_ACCT_INFO;
	pstRun->lSize = sizeof(dbAcctInfo);
	pstRun->pstVoid = &stActNo;
	lRet = lSelectTable(pstRun, pstAcctNo);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstAcctNo->acct_id);
	lGetEXLong(&pstAcctNo->app_type);
	lGetEXLong(&pstAcctNo->acct_default);
#else

	pstRun->lFind = IDX_SELECT;
	pstRun->lReMatch = MATCH_NO;
	pstRun->tblName = TBL_ACCT_INFO;
	pstRun->lSize = sizeof(stActNo);
	pstRun->pstVoid = sGetComIdx(stActNo.app_id, sizeof(stActNo.app_id) - 1,
		stActNo.app_type, stActNo.acct_default);
	lRet = lQuerySpeed(pstRun, pstAcctNo);
	if(lRet !=  RC_SUCC)
		return lRet;
	//	在这里在再次核对一下
	if(strcmp(pstAcctNo->app_id, pszAppId))
	{
		vSetErrMsg("严重错误，索引与表中数据不匹配!");
		return RC_FAIL;
	}

	lGetEXLong(&pstAcctNo->acct_id);
	lGetEXLong(&pstAcctNo->app_type);
	lGetEXLong(&pstAcctNo->acct_default);
#endif

	return RC_SUCC;
}

/****************************************************************************************
 *	获取类似社区001没有终端号对账文件装载查询商户号
 ****************************************************************************************/
long	lGetDefRoute(dbMchtCupsInf *pstCupsInf, char *pszCupsNo, char *pszMcht)
{
	long			lRet = 0;
	dbMchtCupsInf	stCupsInf;
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszCupsNo);
	IBPStrIsNull(pszMcht);
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	memcpy(stCupsInf.cups_no, pszCupsNo, sizeof(stCupsInf.cups_no) - 1);
	memcpy(stCupsInf.mcht_cups_no, pszMcht, sizeof(stCupsInf.mcht_cups_no) - 1);
	sTrimAll(stCupsInf.cups_no);
	sTrimAll(stCupsInf.mcht_cups_no);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stCupsInf);
	pstRun->pstVoid = &stCupsInf;

	lRet = lSelectTable(pstRun, pstCupsInf);
	if(lRet !=  RC_SUCC)
		return lRet;

	lGetEXLong(&pstCupsInf->id);
	return RC_SUCC;
}

/****************************************************************************************
 *	获取交易路由表信息（用于渠道流水装载和计算费率)	
 ****************************************************************************************/
long	lGetMchtRoute(dbMchtCupsInf *pstCupsInf, char *pszCupsNo, char *pszMcht, char *pszTerm)
{
	long			lRet = 0;
	dbMchtCupsInf	stCupsInf;
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszCupsNo);
	IBPStrIsNull(pszMcht);
	IBPStrIsNull(pszTerm);
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	memcpy(stCupsInf.cups_no, pszCupsNo, sizeof(stCupsInf.cups_no) - 1);
	memcpy(stCupsInf.mcht_cups_no, pszMcht, sizeof(stCupsInf.mcht_cups_no) - 1);
	memcpy(stCupsInf.term_cups_no, pszTerm, sizeof(stCupsInf.term_cups_no) - 1);
	sTrimAll(stCupsInf.cups_no);
	sTrimAll(stCupsInf.mcht_cups_no);
	sTrimAll(stCupsInf.term_cups_no);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stCupsInf);
	pstRun->pstVoid = &stCupsInf;

	lRet = lSelectTable(pstRun, pstCupsInf);
	if(lRet !=  RC_SUCC)
		return lRet;

	lGetEXLong(&pstCupsInf->id);
	return RC_SUCC;
}

/****************************************************************************************
 *	获取浦发表信息（用于渠道流水装载和计算费率)	
 ****************************************************************************************/
long	lGetSpdbRoute(dbNMchtMatch *pstMatch, char *pszCupsNo, char *pszMcht)
{
	long			lRet = 0;
	dbNMchtMatch	stMatch;
	RunTime			*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszCupsNo);
	IBPStrIsNull(pszMcht);
	memset(&stMatch, 0, sizeof(stMatch));
	memcpy(stMatch.cups_no, pszCupsNo, sizeof(stMatch.cups_no) - 1);
	memcpy(stMatch.cups_mcht_no, pszMcht, sizeof(stMatch.cups_mcht_no) - 1);
	sTrimAll(stMatch.cups_no);
	sTrimAll(stMatch.cups_mcht_no);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_N_MCHT_MATCH;
	pstRun->lSize = sizeof(stMatch);
	pstRun->pstVoid = &stMatch;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return lRet;

	lRet = lTableFetch(TBL_N_MCHT_MATCH, pstMatch, sizeof(dbNMchtMatch));
	vTableClose(TBL_N_MCHT_MATCH, SHM_DISCONNECT);
	if(RC_SUCC != lRet)
		return lRet;

	lGetEXLong(&pstMatch->id);
	lGetEXLong(&pstMatch->resv_0);

	return RC_SUCC;
}

/****************************************************************************************
 *	获取DISC_ID所有分段的费率	
 ****************************************************************************************/
long	lActTypeCmp(long lAllType, long lActType)
{
/*
	lActType = (int)pow(2., lActType - 1);

	if(lActType != (lAllType & lActType))
		return RC_FAIL;
	return RC_SUCC;
*/
	register long	lMask = 1, i = 0;

	for(i = 1; i <= sizeof(int) * 8; i ++)
	{
		if(!(lAllType & lMask) && (lActType & lMask))
			return RC_FAIL;
		lMask = lMask << 1;
	}
	return RC_SUCC;
}

/****************************************************************************************
 *	获取账号类型SQL IN字符串
 ****************************************************************************************/
char*	sGetActInl(long lActType)
{
	static	char	szOut[60];
	long	lMask = 1, i = 0;

	memset(szOut, 0, sizeof(szOut));
	for(i = 1; i <= sizeof(int) * 8; i ++)
	{
		if((lActType & lMask) > 0)
			sprintf(szOut + strlen(szOut), "%d,", i);
		lMask = lMask << 1;
	}
	szOut[strlen(szOut) - 1] = 0x00;

	return szOut;
}

/****************************************************************************************
 *	获取机构分润配置表列表
 ****************************************************************************************/
long	lGetPfitAlgo(char *pszModel, dbBrhAlgo **ppstBAlgo, long *plOut)
{
	long		lRet = 0, i = 0;
	dbBrhAlgo	stBAlgo;	
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	memset(&stBAlgo, 0, sizeof(stBAlgo));
	strcpy(stBAlgo.model_id, pszModel);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_ALGO;
	pstRun->lSize = sizeof(stBAlgo);
	pstRun->pstVoid = &stBAlgo;

	lRet = lRunSelect(pstRun, (void *)ppstBAlgo, plOut);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	返回值ppstAlgo 指定了void类型，条数位移采用以下方式
	for(i = 0; i < *plOut; i ++)
		lGetEXLong(&((dbBrhAlgo *)*ppstBAlgo + i)->card_type);

	return RC_SUCC;
}

/****************************************************************************************
 *	从模型列表中找到对应的模型段
 ****************************************************************************************/
long	lGetAlgoService(dbDisc *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDisc *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("无效的费率模型列表(0X%08X)或个数(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  模型不止一条
	{
		for(i = 0; i < lAlgoCnt; i ++)
		{
			if(dTxAmt <= pstAlgoList[i].upper_amount)
			{
				iFind = 1;
				memcpy(pstOut, &pstAlgoList[i], sizeof(dbDisc));
				break;
			}
		}
	}
	else if(1 == lAlgoCnt)	//  只有一条，默认
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDisc));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("费率模型个数(%d), 未找到交易金额(%.2f)的该段费率模型!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	从模型列表中找到对应的模型段
 ****************************************************************************************/
long	lGetAlgoModel(dbDiscAlgo *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDiscAlgo *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("无效的费率模型列表(0X%08X)或个数(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  模型不止一条
	{
		for(i = 0; i < lAlgoCnt; i ++)
		{
			if(dTxAmt <= pstAlgoList[i].upper_amount)
			{
				iFind = 1;
				memcpy(pstOut, &pstAlgoList[i], sizeof(dbDiscAlgo));
				break;
			}
		}
	}
	else if(1 == lAlgoCnt)	//  只有一条，默认
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDiscAlgo));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("费率模型个数(%d), 未找到交易金额(%.2f)的该段费率模型!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	获取DISC_ID所有分段的费率	
 ****************************************************************************************/
long	lGetDiscAlgo(char *pszDiscId, long lActType, long lMask, dbDiscAlgo **ppstAlgo, long *plOut) 
{
	long		lRet = 0, lCount = 0;
	dbDiscAlgo	stAlgo;
	void		*pszOut = NULL;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszDiscId);
	if(lActType <= 0)
	{
		vSetErrMsg("无效的卡类型(%d)!", lActType);
		return RC_FAIL;	
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	strcpy(stAlgo.disc_id, pszDiscId);
	sTrimAll(stAlgo.disc_id);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_PUB_DISC_ALGO;
	pstRun->lSize = sizeof(stAlgo);
	pstRun->pstVoid = &stAlgo;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	遍历所有记录
	while(1)
	{
		memset(&stAlgo, 0, sizeof(stAlgo));
		lRet = lTableFetch(TBL_PUB_DISC_ALGO, &stAlgo, sizeof(stAlgo));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		//	判断是否是所需的卡类型
		lGetEXLong(&stAlgo.card_type);
		if(FIELD_MATCH == lMask)
		{
			if(RC_SUCC != lActTypeCmp(stAlgo.card_type, lActType))
				continue;
		}
		else
		{
			if(lActType != stAlgo.card_type)
				continue;
		}

		if(0 == lCount)
			pszOut = (char *)malloc(pstRun->lSize);
 		else
			pszOut = (char *)realloc(pszOut, pstRun->lSize * (lCount + 1));
		if(NULL == pszOut)
		{
			vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
			vSetErrMsg("分配大小(%d)内存失败!", pstRun->lSize * (lCount + 1));
			return RC_FAIL;
		}
		memcpy((char *)pszOut + lCount * pstRun->lSize, &stAlgo, sizeof(dbDiscAlgo));
		lGetEXLong(&((dbDiscAlgo *)pszOut + lCount)->id);
		lGetEXLong(&((dbDiscAlgo *)pszOut + lCount)->index_num);
		lGetEXLong(&((dbDiscAlgo *)pszOut + lCount)->flag);
		lCount ++;
	}
	vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
	*ppstAlgo = pszOut;
	*plOut = lCount;

	if(0 == lCount)
	{
		vSetErrMsg("未找到卡号(%d)对应的费率模型(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	费率模型类型表数据转换	
 ****************************************************************************************/
void	vEXchangeAlgo(dbDisc *pstDisc, dbDiscAlgo *pstAlgo)
{
	pstAlgo->id = pstDisc->id;
	memcpy(pstAlgo->disc_id, pstDisc->disc_id, sizeof(pstAlgo->disc_id) - 1);
	pstAlgo->index_num = pstDisc->index_num;   
	pstAlgo->min_fee = pstDisc->min_fee;	 
	pstAlgo->max_fee = pstDisc->max_fee;	 
	pstAlgo->floor_amount = pstDisc->floor_amount;
	pstAlgo->upper_amount = pstDisc->upper_amount;
	pstAlgo->flag = pstDisc->flag;		
	pstAlgo->fee_value = pstDisc->fee_value;   
	pstAlgo->card_type = pstDisc->card_type;   
}

/****************************************************************************************
 *	获取DISC_ID服务费所有分段的费率	
 ****************************************************************************************/
long	lGetServiceDisc(char *pszDiscId, long lActType, long lMask, dbDisc **ppstAlgo, long *plOut) 
{
	long		lRet = 0, lCount = 0;
	dbDisc		stAlgo;
	void		*pszOut = NULL;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszDiscId);
/*	//	服务费，暂时不考虑卡类型
	if(lActType <= 0)
	{
		vSetErrMsg("无效的卡类型(%d)!", lActType);
		return RC_FAIL;	
	}
*/
	memset(&stAlgo, 0, sizeof(stAlgo));
	strcpy(stAlgo.disc_id, pszDiscId);
	sTrimAll(stAlgo.disc_id);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_DISC_ALGO;
	pstRun->lSize = sizeof(stAlgo);
	pstRun->pstVoid = &stAlgo;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	遍历所有记录
	while(1)
	{
		memset(&stAlgo, 0, sizeof(stAlgo));
		lRet = lTableFetch(TBL_DISC_ALGO, &stAlgo, sizeof(stAlgo));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		//	判断是否是所需的卡类型
		lGetEXLong(&stAlgo.card_type);
		if(0 == lCount)
			pszOut = (char *)malloc(pstRun->lSize);
 		else
			pszOut = (char *)realloc(pszOut, pstRun->lSize * (lCount + 1));
		if(NULL == pszOut)
		{
			vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
			vSetErrMsg("分配大小(%d)内存失败!", pstRun->lSize * (lCount + 1));
			return RC_FAIL;
		}
		memcpy((char *)pszOut + lCount * pstRun->lSize, &stAlgo, sizeof(dbDisc));
		lGetEXLong(&((dbDisc *)pszOut + lCount)->id);
		lGetEXLong(&((dbDisc *)pszOut + lCount)->index_num);
		lGetEXLong(&((dbDisc *)pszOut + lCount)->flag);
		lCount ++;
	}
	vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
	*ppstAlgo = pszOut;
	*plOut = lCount;

	if(0 == lCount)
	{
		vSetErrMsg("未找到卡号(%d)对应的费率模型(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
	获取服务配置模型
 ****************************************************************************************/
long	lGetServiceCfg(dbSvceCfg *pstCfg, long lId)
{
	long			lRet = 0;
	dbSvceCfg	   stCfg;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_SERVICE_CONFIG;

	memset(&stCfg, 0, sizeof(stCfg));
	stCfg.id = lSetEXLong(lId);
	pstRun->lSize = sizeof(stCfg);
	pstRun->pstVoid = (char *)&stCfg.id;

//	lRet = lQuerySpeed(pstRun, pstCfg);
	lRet = lQuerySpeedAt(pstRun, pstCfg);
	if(RC_SUCC != lRet)
		return lRet;

	lGetEXLong(&pstCfg->id);
	return RC_SUCC;
}

/****************************************************************************************
 *	搜索内存中商户对应的服务费模型
 ****************************************************************************************/
long	lGetMchtService(char *pszMchtNo, char *pszCode, char *pszOutDate, dbSvceCfg *pstCfg)
{
	long			lRet = 0, lCount = 0;
	dbSvceCfg	   stCfg;
	dbResigter	  stRegist;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszCode);
	IBPStrIsNull(pszMchtNo);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_SERVICE_REGISTER;

	memset(&stRegist, 0, sizeof(stRegist));
	stRegist.service_object_type[0] = '1';
	strcpy(stRegist.service_object_id, pszMchtNo);
	sTrimAll(stRegist.service_object_id);
	pstRun->lSize = sizeof(stRegist);
	pstRun->pstVoid = &stRegist;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return lRet;

	while(1)
	{
		memset(&stRegist, 0, sizeof(stRegist));
		lRet = lTableFetch(TBL_SERVICE_REGISTER, &stRegist, sizeof(stRegist));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_SERVICE_REGISTER, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		lRet = lGetServiceCfg(pstCfg, lSetEXLong(stRegist.service_id));
		if(RC_SUCC != lRet)
		{	
			vTableClose(TBL_SERVICE_REGISTER, SHM_DISCONNECT);
			return lRet;
		}

		if(!strcmp(pstCfg->service_code, pszCode))
		{
			lGetEXLong(&pstCfg->id);
			lGetEXLong(&pstCfg->prob_days);	
			sTrimAll(stRegist.reg_datetime);
			strcpy(pszOutDate, stRegist.reg_datetime);
			vTableClose(TBL_SERVICE_REGISTER, SHM_DISCONNECT);
			return RC_SUCC;
		}
	}
	vTableClose(TBL_SERVICE_REGISTER, SHM_DISCONNECT);
	return RC_NOTFOUND;
}

/****************************************************************************************
 *	搜索内存表中指定索引字段的极值 FIELD_MAX  FIELD_MIN
 ****************************************************************************************/
long	lGetVersCfg(dbSvceCfg *pstCfg, char *pszSevceCode, long lMask)
{
	long		lRet = 0, lInit = 0, i = 0;
	void		*p = NULL;
	dbSvceCfg	stSvceCfg;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszSevceCode);
	memset(&stSvceCfg, 0, sizeof(stSvceCfg));
	strcpy(stSvceCfg.service_code, pszSevceCode);
	sTrimAll(stSvceCfg.service_code);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_SERVICE_CONFIG;
	pstRun->lSize = sizeof(stSvceCfg);
	pstRun->pstVoid = &stSvceCfg;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	遍历所有记录
	while(1)
	{
		memset(&stSvceCfg, 0, sizeof(stSvceCfg));
		lRet = lTableFetch(TBL_SERVICE_CONFIG, &stSvceCfg, sizeof(stSvceCfg));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		if(0 == lInit)
		{
			lInit = 1;
			memcpy(pstCfg, &stSvceCfg, sizeof(stSvceCfg));
		}

		lGetEXLong(&stSvceCfg.prob_days);
		if(FIELD_MAX == lMask)
		{
			if(0 < memcmp(stSvceCfg.service_version, pstCfg->service_version, 
				strlen(stSvceCfg.service_version)))
				memcpy(pstCfg, &stSvceCfg, sizeof(stSvceCfg));
		}
		else if(FIELD_MIN == lMask)
		{
			if(0 > memcmp(stSvceCfg.service_version, pstCfg->service_version, 
				strlen(stSvceCfg.service_version)))
				memcpy(pstCfg, &stSvceCfg, sizeof(stSvceCfg));
		}
		else
		{
			vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);
			vSetErrMsg("未识别的匹配定义(%d)", lMask);
			return RC_FAIL;
		}
	}
	vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);

	if(0 == lInit)
		return RC_NOTFOUND;
	return RC_SUCC;
}

/****************************************************************************************
 * 	group by cups_no功能	
 ****************************************************************************************/
long	lGroupCupsNo(CMList **pstRoot, long lMode)
{
	long			lRet = 0, lLen = 0;
	dbMchtCupsInf   stCups;
	CMList			*pstNode = NULL;
	RunTime		 *pstRun = (RunTime *)pInitRunTime();
	long			lCount = 0;

	lLen = sizeof(stCups.cups_no);
	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return lRet;

	while(1)
	{
		memset(&stCups, 0, sizeof(stCups));
		lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stCups, sizeof(stCups));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		sTrimAll(stCups.cups_no);
		++lCount;

		//	已存在，下条
		if(pSearchNode(pstNode, stCups.cups_no, lLen))
			continue;

		pstNode = pInsertList(pstNode, stCups.cups_no, lLen);
		if(!pstRoot)
		{
			vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
			return RC_FAIL;
		}
	}
	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	*pstRoot = pstNode;

	return RC_SUCC;
}

/****************************************************************************************
 * 	向渠道金额加入数据
 ****************************************************************************************/
long	lInsertCupsAmt(EXTCupAmt *pstCupAmt)
{
	return RC_SUCC;
}

/****************************************************************************************
 *	code end
 ****************************************************************************************/
