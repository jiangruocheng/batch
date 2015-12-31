#include	"load.h"

/****************************************************************************************
 *	È«¾Ö±äÁ¿ÉêÇëÇø 
 ****************************************************************************************/
IBPShm		g_ShmRun = {0};
RunTime		g_RunTime = {0};

/****************************************************************************************
 * 	±ØÒªÄÚ²¿ÉÌ»§ÉêÃ÷	
 ****************************************************************************************/
extern  long	lGetIdxNum(TABLE table);
extern  void	vSetTblDef(char *p);
extern  TblDef* pGetTblDef(TABLE t);
extern  TblKey* pGetTblIdx(TABLE t);
void	vDetachShm();

/****************************************************************************************
 * 	³õÊ¼»¯²éÑ¯½á¹¹
 ****************************************************************************************/
void*	pInitRunTime()
{
	memset(&g_RunTime, 0, sizeof(g_RunTime));
	return &g_RunTime;
}

/****************************************************************************************
 * 	»ñÈ¡²éÑ¯½á¹¹
 ****************************************************************************************/
void*	pGetRunTime()
{
	return &g_RunTime;
}

/****************************************************************************************
 * 	»ñÈ¡Á¬½Ó´´½¨¹²ÏíÄÚ´æµÄKEYÖµ
 ****************************************************************************************/
void*	pGetShmRun()
{
	return &g_ShmRun;
}

/****************************************************************************************
 * 	ÈÃ½ø³Ì±£³ÖÓëÄÚ´æµÄÁ¬½Ó
 ****************************************************************************************/
void	vHoldConnect()
{
//	½¨Òé²»ÒªÇáÒ×Ê¹ÓÃ£¬ ÓÐÐ©ÏµÍ³Ö»ÄÜattchÒ»ÏÂ£¬ ²éÑ¯ÉèÖÃ¸Ã²ÎÊý¾Í²»ÄÜ¹Ø±Õ¹²ÏíÄÚ´æÁ¬½Ó
//	µ¼ÖÂÔÙ´ÎÁ¬½Ó³öÏÖ 12:Cannot allocate memory 
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_CONNECT;
}

/****************************************************************************************
 * 	¶Ë¿ªÓë¹²ÏíÄÚ´æµÄÁ¬½Ó
 ****************************************************************************************/
void	vDisConnect()
{
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_DISCONNECT;
	vDetachShm();
}

/****************************************************************************************
 *	ÊÇ·ñ±£³ÖÁ¬½Ó
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
 * 	»ñÈ¡Á¬½Ó´´½¨¹²ÏíÄÚ´æµÄKEYÖµ
 ****************************************************************************************/
long	lGetShmId()
{
	if(g_ShmRun.m_shmID <= 0)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æ»¹Î´³õÊ¼»¯£¬ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}
	return g_ShmRun.m_shmID;
}

/****************************************************************************************
 * 	»ñÈ¡Á¬½Ó´´½¨¹²ÏíÄÚ´æµÄKEYÖµ
 ****************************************************************************************/
void*	pGetShmAddr()
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æ»¹Î´³õÊ¼»¯£¬ÇëÏÈ³õÊ¼»¯!");
		return NULL;
	}
	return g_ShmRun.m_pszAddr;
}

/****************************************************************************************
	Ö¸Ïò±íÊý¾ÝµÄµØÖ·
 ****************************************************************************************/
void*	pGetTblAddr(long nOffset)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æ»¹Î´³õÊ¼»¯£¬ÇëÏÈ³õÊ¼»¯!");
		return NULL;
	}
	return g_ShmRun.m_pszAddr + g_ShmRun.m_lOffset + nOffset;
}

/****************************************************************************************
 * 	»ñÈ¡Á¬½Ó´´½¨¹²ÏíÄÚ´æµÄKEYÖµ
 ****************************************************************************************/
key_t	yGetShmKey()
{
	return g_ShmRun.m_yKey;
}

/****************************************************************************************
 * 	»ñÈ¡Á¬½Ó´´½¨¹²ÏíÄÚ´æµÄKEYÖµ
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
		vSetErrMsg("»ñÈ¡ÄÚ´æKeyÖµÊ§°Ü(%s):(%s)!", szKeyPath, strerror(errno));
		return RC_FAIL;
	}

	return yKey;
}

/****************************************************************************************
 * 	¹¦ÄÜ  :ÉèÖÃ²Ù×÷¹ý³ÌÖÐµÄ´íÎóÃèÊö
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
 * 	¹¦ÄÜ  :²Ù×÷¹ý³ÌÖÐµÄ´íÎóÃèÊö
 ****************************************************************************************/
char*	sGetError()
{
	return g_ShmRun.m_szMsg;
}

/****************************************************************************************
 * 	¹¦ÄÜ  :´´½¨¹²ÏíÄÚ´æ
 * 	²ÎÊý  :
 ****************************************************************************************/
long	lInitCreateShm(long	lSize)
{
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath();
	if(((IBPShm *)pGetShmRun())->m_yKey <= RC_FAIL)
		return RC_FAIL;

	if(lSize <= 0)
	{
		vSetErrMsg("´íÎó, ´´½¨¿Õ¼ä´óÐ¡(%d)<0", lSize);
		return RC_FAIL;
	}

	//	±ØÐëÖØ½¨¹²ÏíÄÚ´æ£¬Ã¿´ÎµÄÊýÁ¿¶¼ÊÇÃ»·¨¹À¼ÆµÄ
	((IBPShm *)pGetShmRun())->m_shmID = shmget(((IBPShm *)pGetShmRun())->m_yKey, lSize, 
		IPC_CREAT|IPC_EXCL|0660);
	if(RC_FAIL >= ((IBPShm *)pGetShmRun())->m_shmID)
	{
		vSetErrMsg("´´½¨¹²ÏíÄÚ´æÊ§°Ü:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr = (void *)shmat(((IBPShm *)pGetShmRun())->m_shmID, NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr || (void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr)
	{	   
		vSetErrMsg("Á¬½Ó¹²ÏíÄÚ´æÊ§°Ü:(%s)", strerror(errno));
		return RC_FAIL;
	}

	memset(((IBPShm *)pGetShmRun())->m_pszAddr, 0, lSize);
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr);
	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
	((IBPShm *)pGetShmRun())->m_bInit = 0;

	return RC_SUCC;
}

/****************************************************************************************
	³õÊ¼»¯ÄÚ´æÖ¸Õë
 ****************************************************************************************/
long	lInitTblShm()
{
	long	bAttch = ((IBPShm *)pGetShmRun())->m_bAttch;

	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = bAttch;

	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath();
	if(((IBPShm *)pGetShmRun())->m_yKey == RC_FAIL)
	{
		vSetErrMsg("Á¬½ÓÄÚ´æµÄKeyÒÑÎÞÐ§!");
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_shmID = shmget(((IBPShm *)pGetShmRun())->m_yKey, 0, IPC_CREAT|0660);
	if(RC_FAIL == ((IBPShm *)pGetShmRun())->m_shmID)
	{
		vSetErrMsg("Á¬½Ó¹²ÏíÄÚ´æÊ§°Ü, ¸ÃÄÚ´æ¿é¿ÉÄÜÒÑÏûÊ§:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
	¶Ï¿ªÓë¹²ÏíÄÚ´æµÄÁ´½Ó 
 ****************************************************************************************/
void	vDetachShm()
{
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr);
	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
	((IBPShm *)pGetShmRun())->m_bInit = 0;
}

/****************************************************************************************
 * 	Á´½Ó¹²ÏíÄÚ´æ	
 ****************************************************************************************/
long	lAttachShm()
{
	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}	   

	((IBPShm *)pGetShmRun())->m_pszAddr = (void* )shmat(lGetShmId(), NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr || (void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr)
	{	   
		vSetErrMsg("Á´½Ó¹²ÏíÄÚ´æÊ§°Ü:(%d)(%d)(%s)!", lGetShmId(), errno, strerror(errno));
		return RC_FAIL;
	}
	((IBPShm *)pGetShmRun())->m_bInit = 1;
	return RC_SUCC;
}

/****************************************************************************************
 * 	É¾³ý¹²ÏíÄÚ´æ	
 ****************************************************************************************/
void	vDeleteShm()
{
	int		iRet = 0;

	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return ;
	}	   

	iRet = shmctl(((IBPShm *)pGetShmRun())->m_shmID, IPC_RMID, NULL);
	if(iRet)
	{	   
		vSetErrMsg("É¾³ý¹²ÏíÄÚ´æÊ§°Ü:(%s)!", strerror(errno));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
}

/****************************************************************************************
	Ö¸Ïò¶Ô²éÑ¯³öÀ´µÄ½á¹ûµÄÊ×µØÖ·	
 ****************************************************************************************/
char*	pGetSelectAddr(TABLE table)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return NULL;
	}
	return (char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[table];
}

/****************************************************************************************
 *	 Á¬½Ó¹²ÏíÄÚ´æ²¢³õÊ¼»¯±ØÒªË÷Òý
 ****************************************************************************************/
long	lInitMemTable()
{
	long	lRet = 0;

	//	Óë¹²ÏíÄÚ´æÈÔÔÚ±£³ÖÁ¬½ÓÖÐ
	if(1 == ((IBPShm *)pGetShmRun())->m_bInit)
	{
		//  ³õÊ¼»¯Í·½áµã
		vSetTblDef((char *)pGetShmAddr());
		return RC_SUCC;	
	}

	lRet = lInitTblShm();
	if(lRet)
		return RC_FAIL;

	//	ÓÐÐ©»úÆ÷ Ö»ÄÜattchÒ»´Î
	lRet = lAttachShm();
	if(lRet) return RC_FAIL;

	//  ³õÊ¼»¯Í·½áµã
	vSetTblDef((char *)pGetShmAddr());
	
	//  ÍË³öÐÅºÅ	vDetachShm();
	//  sigal(

	return RC_SUCC;
}

/****************************************************************************************
 * 	¼ì²é×Ö·û´®ÊÇ·ñ¶¼ÊÇ0x00	
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
	¶ÔÎ¨Ò»Ë÷Òý½øÐÐÑéÖ¤
 ****************************************************************************************/
long	lTblIdxCompare(RunTime *pstRun, void *p, SHTree *pstTree)
{
	long	nCompare = 0, i = 0;
	TblKey  *pstIdx = pGetTblIdx(pstRun->tblName);

	nCompare = lGetIdxNum(pstRun->tblName);
	if(0 == nCompare)	   //  ¶ÔÓÚÎ´ÉèÖÃÖ÷Ë÷Òý£¬ÎÞÂÛ²ÉÓÃÊ²Ã´·½Ê½²éÑ¯Ö±½Ó·µ»Ø²»Æ¥Åä
		return RC_FAIL;

	for(i = 0; i < nCompare; i ++)  //  ÏÂÃæ±È½ÏµÄ¶¼ÊÇÎ¨Ò»Ë÷Òý
	{
		if(IDX_SELECT != pstIdx[i].m_lIsPk || pstTree->m_lIdxPos != pstIdx[i].m_lKeyOffSet)
			continue;

		if(!memcmp((char *)pstRun->pstVoid, (char *)p + pstTree->m_lIdxPos, pstTree->m_lIdxLen))
			return RC_SUCC;
	}

	return RC_FAIL;
}

/****************************************************************************************
 * 	±í°´ÕÕÎ¨Ò»Ë÷Òý²éÑ¯
 ****************************************************************************************/
long	lTblIdxMatch(RunTime *pstRun, void *p, long lFindType)
{
	long	lRet = 0, nCompare = 0, i = 0;
	char	szTemp[50];
	TblKey  *pstIdx = pGetTblIdx(pstRun->tblName);
	
	memset(szTemp, 0, sizeof(szTemp));
	nCompare = lGetIdxNum(pstRun->tblName);
	if(0 == nCompare && IDX_SELECT == lFindType)	//	¶ÔÓÚÎ´ÉèÖÃÖ÷Ë÷Òý£¬ÎÞÂÛ²ÉÓÃÊ²Ã´·½Ê½²éÑ¯Ö±½Ó·µ»Ø²»Æ¥Åä
		return RC_FAIL;

	for(i = 0; i < nCompare; i ++)	//	ÏÂÃæ±È½ÏµÄ¶¼ÊÇÎ¨Ò»Ë÷Òý
	{
		//	Èç¹û°´ÕÕË÷ÒýÀ´²éÑ¯£¬ÆäËû²éÑ¯Ìõ¼þ½«±»ÌÞ³ý
		//	Èç¹û²»ÊÇ°´ÕÕ²éÑ¯Ë÷ÒýÀ´²é£¬ÄÇÃ´ËùÓÐ²éÑ¯Ìõ¼þ½«»áÈ¥Æ¥Åä
		if(IDX_SELECT == lFindType)
		{
			if(pstRun->lFind != pstIdx[i].m_lIsPk)
				continue;
		}

		switch(pstIdx[i].m_lKeyAttr)
		{
		case FIELD_LONG:	//	ÏÈ²ÉÓÃÒ»ÖÂ´¦Àí
			//	ÎÞÂÛÊÇ·ñ²éÑ¯Ë÷Òý»¹ÊÇÖ÷Ë÷Òý£¬CHARÀàÐÍÎ´¸³Öµ£¬±íÊ¾²»²éÑ¯
			//	0²»´¦Àí£¬ÉèÖÃÎª0£¬ÓÃLONG_ZER
			if(!memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, szTemp, pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, (char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	ÈÎÒâÒ»ÌõÖ÷Ë÷Òý²éÕÒÖ±½Ó·µ»Ø
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else
				return RC_FAIL;								//	ÎÞÂÛÊ¹Ö÷Ë÷Òý²»Æ¥Åä»¹ÊÇ²éÑ¯Ë÷Òý²»Æ¥Åä£¬ÍÆ³ö¸Ã¼ÇÂ¼²»Æ¥Åä, ÎÞÐè¼ÌÐø²éÕÒ
			break;											//	¸Ä¸ö²éÑ¯Ë÷ÒýÆ¥ÅäÁË£¬break³öÀ´£¬ »¹ÒªÆ¥ÅäÆäËûµÄ²éÑ¯Ë÷Òý
		case FIELD_CHAR:
			//	ÎÞÂÛÊÇ·ñ²éÑ¯Ë÷Òý»¹ÊÇÖ÷Ë÷Òý£¬CHARÀàÐÍÎ´¸³Öµ£¬±íÊ¾²»²éÑ¯
			if(RC_SUCC == lIsNRealZore((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, (char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	ÈÎÒâÒ»ÌõÖ÷Ë÷Òý²éÕÒÖ±½Ó·µ»Ø
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else
				return RC_FAIL;								//	ÎÞÂÛÊ¹Ö÷Ë÷Òý²»Æ¥Åä»¹ÊÇ²éÑ¯Ë÷Òý²»Æ¥Åä£¬ÍÆ³ö¸Ã¼ÇÂ¼²»Æ¥Åä£ ÎÞÐè¼ÌÐø²éÕÒ
			break;
		default:
			vSetErrMsg("´íÎóµÄ×Ö¶ÎÀàÐÍ(%d)", pstIdx[i].m_lKeyAttr);
			return RC_FAIL;
		}
	}
	if(IDX_SELECT == lFindType)	//	Ö÷Ë÷ÒýÆ¥ÅäÓÐforÑ­»·Íê±ÏÎ´ÕÒµ½£¬·µ»ØÊ§°Ü
		return RC_FAIL;

	//	²éÑ¯Ë÷Òý²»ÄÜÔÚforÑ­»·ÀïÃæ·µ»Ø£¬Èç¹ûforÀïÃæÃ»ÓÐ¿ÉÆ¥ÅäµÄ×Ö¶Î£¬±¾ÌõÄ¬ÈÏ²éÑ¯Æ¥Åä
	return RC_SUCC;
}
/****************************************************************************************
	¶¨ÒåÓÎ±êÆðÊ¼µØÖ·
 ****************************************************************************************/
long	lTableDeclare(RunTime *pstRun)
{
	long	lRet = 0;

	if(!pstRun)	return RC_FAIL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if((0 == pstRun->lSize && pstRun->pstVoid) || (0 != pstRun->lSize && !pstRun->pstVoid))
	{
		vSetErrMsg("ÉèÖÃ²éÑ¯Ìõ¼þ´íÎó(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	if((char *)g_ShmRun.stRunTime[pstRun->tblName].pstVoid)
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("¶¨ÒåÓÎ±ê³ö´í, ÓÎ±êÖØ¸´¶¨Òå!");
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
	¹Ø±ÕÓÎ±êÖ¸Õë
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

	if(SHM_CONNECT == lType)	//	±£³ÖÁ¬½Ó
		return ;
	vDetachShm();
}

/****************************************************************************************
	»ñÈ¡ÏÂÒ»ÏÂ¼ÇÂ¼	
 ****************************************************************************************/
long	lTableFetch(TABLE table, void *pszVoid, long lSize)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	if(!(char *)(((IBPShm *)pGetShmRun())->m_pvCurAddr[table]))
	{
		vSetErrMsg("ÎÞÐ§µÄÓÎ±ê»òÕßÓÎ±êÎ´¶¨Òå(%d)", table);
		return RC_FAIL;
	}

	while(1)
	{
		if(lGetTblValid(table) < g_ShmRun.m_lCurLine[table])
		{
			vSetErrMsg("ÎÞÆ¥Åä¼ÇÂ¼, (%d)", RC_NOTFOUND);
			return RC_NOTFOUND;
		}

		if((char *)g_ShmRun.stRunTime[table].pstVoid)
		{
			//	ÓÎ±ê²éÑ¯¿Ï¶¨°´ÕÕ²éÑ¯Ë÷ÒýÀ´Ñ°ÕÒ
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
	°´ÕÕË÷Òý»ñÈ¡¼«Öµ
 ****************************************************************************************/
long	lGetExtremeIdx(RunTime *pstRun, void *pstVoid, long lMask)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("ÉèÖÃ²éÑ¯Ìõ¼þ»ò¹æÔò´íÎó(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("±í(%d)Î´ÉèÖÃÖ÷Ë÷ÒýÎ´ÉèÖÃ£¬²éÑ¯·½Ê½²»±»Ö§³Ö", pstRun->tblName);
		return RC_FAIL;
	}

	if(FIELD_MAX == lMask)
		pstTree = (SHTree *)pGetMaxSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)));
	else if(FIELD_MIN == lMask)
		pstTree = (SHTree *)pGetMinSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)));
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("ÎÞÆ¥Åä¼ÇÂ¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);
	vTblInitParam(pstRun->tblName);
	return RC_SUCC;
}

/****************************************************************************************
	ÔÚAVLÊ÷ÉÏËÑË÷Î¨Ò»Ë÷Òý¼ÇÂ¼
 ****************************************************************************************/
long	lQuerySpeed(RunTime *pstRun, void *pstVoid)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("ÉèÖÃ²éÑ¯Ìõ¼þ»ò¹æÔò´íÎó(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("±í(%d)Î´ÉèÖÃ¿ìËÙ²éÑ¯½Úµã!", pstRun->tblName);
		return RC_FAIL;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)), 0, pstRun->pstVoid);
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("ÎÞÆ¥Åä¼ÇÂ¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  ±ÜÃâ³öÏÖÎÊÌâ£¬ÔÙ´ÎÆ¥ÅäÒ»ÏÂ, Ö»Æ¥ÅäÖ÷Ë÷Òý
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
 			vTblInitParam(pstRun->tblName);
			vSetErrMsg("ÑÏÖØ´íÎó£¬Ë÷ÒýÓë±íÖÐÊý¾Ý²»Æ¥Åä!");
			return RC_FAIL;
		}
	}

	vTblInitParam(pstRun->tblName);
	return RC_SUCC;
}

/****************************************************************************************
	¶Ô²»ÄÜattch2´ÎµÄÏµÍ³(ÒÑ¾­attchÒ»´ÎÁË£¬²¢Î´ÊÍ·Å)£¬Ìí¼ÓÌØÊâº¯Êý
 ****************************************************************************************/
long	lQuerySpeedAt(RunTime *pstRun, void *pstVoid)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("ÉèÖÃ²éÑ¯Ìõ¼þ»ò¹æÔò´íÎó(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(!g_ShmRun.m_bInit)
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	if(!((IBPShm *)pGetShmRun())->m_pszAddr)
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿âÖ¸Õë´íÎó£¬¿ÉÄÜ»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vSetErrMsg("±í(%d)Î´ÉèÖÃ¿ìËÙ²éÑ¯½Úµã!", pstRun->tblName);
		return RC_FAIL;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)), 0, pstRun->pstVoid);
	if(!pstTree)
	{
		vSetErrMsg("ÎÞÆ¥Åä¼ÇÂ¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  ±ÜÃâ³öÏÖÎÊÌâ£¬ÔÙ´ÎÆ¥ÅäÒ»ÏÂ, Ö»Æ¥ÅäÖ÷Ë÷Òý
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
			vSetErrMsg("ÑÏÖØ´íÎó£¬Ë÷ÒýÓë±íÖÐÊý¾Ý²»Æ¥Åä!");
			return RC_FAIL;
		}
	}

	return RC_SUCC;
}

/****************************************************************************************
 * ¿ìËÙ²éÑ¯£¬ÔÚÄÚ´æ±íÖÐÆ¥Åä¼ÇÂ¼Á¢¿Ì·µ»Ø£¬ÒªÇóÊäÈëµÄ²éÑ¯Ìõ¼þ±ØÐëÊ¹Ö÷Ë÷Òý	
 ****************************************************************************************/
long	lSelectSpeed(RunTime *pstRun, void *pstVoid)
{
	long	lRet = 0, lCount = 0;
	void	*p = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("ÉèÖÃ²éÑ¯Ìõ¼þ»ò¹æÔò´íÎó(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vDetachShm();
		vSetErrMsg("±í(%d)Ö÷Ë÷ÒýÎ´ÉèÖÃ£¬²éÑ¯·½Ê½²»±»Ö§³Ö", pstRun->tblName);
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("ÎÞÐ§µÄÓÎ±ê»òÕßÓÎ±êÎ´¶¨Òå");
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
		vSetErrMsg("ÎÞÆ¥Åä¼ÇÂ¼, (%d)", RC_NOTFOUND);
		vTblInitParam(pstRun->tblName);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)p, pstRun->lSize);
	vTblInitParam(pstRun->tblName);

	return RC_SUCC;
}

/****************************************************************************************
 * 	²éÑ¯µ¥Ìõ¼ÇÂ¼
 ****************************************************************************************/
long	lSelectTable(RunTime *pstRun, void *pstVoid)
{
	long	lRet = 0, lCount = 0;
	void	*p = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid)
	{
		vSetErrMsg("ÉèÖÃ²éÑ¯Ìõ¼þ´íÎó(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("ÎÞÐ§µÄÓÎ±ê»òÕßÓÎ±êÎ´¶¨Òå");
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
				vSetErrMsg("´æÔÚÖÁÉÙ(%d)¶àÐÐ¼ÇÂ¼±»Ñ¡Ôñ!", lCount);
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
		vSetErrMsg("ÎÞÆ¥Åä¼ÇÂ¼, (%d)", RC_NOTFOUND);
		vTblInitParam(pstRun->tblName);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)p, pstRun->lSize);
	vTblInitParam(pstRun->tblName);

	return RC_SUCC;
}

/****************************************************************************************
 *	°´¹æÔò²éÑ¯È«±í£¬·µ»ØÂú×ãÌõ¼þËùÓÐ¼ÇÂ¼
 ****************************************************************************************/
long	lRunSelect(RunTime *pstRun, void **ppszOut, long *plOut)
{
	long	lRet = 0, lCount = 0;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid)
	{
		vSetErrMsg("ÉèÖÃ²éÑ¯Ìõ¼þ´íÎó(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("¹²ÏíÄÚ´æÊý¾Ý¿â»¹Î´³õÊ¼»¯, ÇëÏÈ³õÊ¼»¯!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("ÎÞÐ§µÄÓÎ±ê»òÕßÓÎ±êÎ´¶¨Òå");
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
				vSetErrMsg("·ÖÅä´óÐ¡(%d)ÄÚ´æÊ§°Ü!", pstRun->lSize * (lCount + 1));
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
		vSetErrMsg("ÎÞÆ¥Åä¼ÇÂ¼, (%d)", RC_NOTFOUND);

	return RC_SUCC;
}

/****************************************************************************************
 *	Ó¦ÓÃ²ã
 ****************************************************************************************/
/****************************************************************************************
 *	¸ù¾ÝÉÌ»§ºÅË÷Òý»ñÈ¡ÉÌ»§ÐÅÏ¢
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
 *	¸ù¾ÝÉÌ»§ºÅÓÃ»§ÐÅÏ¢
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
 *	¸ù¾ÝÉÌ»§ºÅË÷ÒýÇåËã»ú¹¹
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
 *	¸ù¾Ý»ú¹¹ºÅË÷Òý»ñÈ¡»ú¹¹ÐÅÏ¢
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
 *	¸ù¾Ý»ú¹¹ºÅË÷Òý»ñÈ¡»ú¹¹ÐÅÏ¢
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
*	¸ù¾ÝMCCÂëË÷Òý»ñÈ¡MCCÐÅÏ¢
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
 *	»ñÈ¡DISC_IDËùÓÐ·Ö¶ÎµÄ·ÑÂÊ	
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
 *	»ñÈ¡»ú¹¹·ÖÈóÖ÷ÌåÐÅÏ¢
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
 *	»ú¹¹·ÑÂÊÄ£ÐÍ¶¨Òå±í
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
 *	¸ù¾Ý»ú¹¹ºÅË÷Òý»ñÈ¡»ú¹¹ÐÅÏ¢
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

	//	»ñÈ¡ÇþµÀÃû³Æ£¬ËæÒâÈ¡µÚÒ»Ìõ¾Í¹»ÁË£¬ ²»ÓÃCHK_SELECT
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
 *	¸ù¾Ý»ú¹¹ºÅË÷Òý»ñÈ¡»ú¹¹ÐÅÏ¢
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

	//	»ñÈ¡ÇþµÀÃû³Æ£¬ËæÒâÈ¡µÚÒ»Ìõ¾Í¹»ÁË£¬ ²»ÓÃCHK_SELECT
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
 *	»ñÈ¡ÕËºÅÐÅÏ¢
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
	//	ÔÚÕâÀïÔÚÔÙ´ÎºË¶ÔÒ»ÏÂ
	if(strcmp(pstAcctNo->app_id, pszAppId))
	{
		vSetErrMsg("ÑÏÖØ´íÎó£¬Ë÷ÒýÓë±íÖÐÊý¾Ý²»Æ¥Åä!");
		return RC_FAIL;
	}

	lGetEXLong(&pstAcctNo->acct_id);
	lGetEXLong(&pstAcctNo->app_type);
	lGetEXLong(&pstAcctNo->acct_default);
#endif

	return RC_SUCC;
}

/****************************************************************************************
 *	»ñÈ¡ÀàËÆÉçÇø001Ã»ÓÐÖÕ¶ËºÅ¶ÔÕËÎÄ¼þ×°ÔØ²éÑ¯ÉÌ»§ºÅ
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
 *	»ñÈ¡½»Ò×Â·ÓÉ±íÐÅÏ¢£¨ÓÃÓÚÇþµÀÁ÷Ë®×°ÔØºÍ¼ÆËã·ÑÂÊ)	
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
 *	»ñÈ¡ÆÖ·¢±íÐÅÏ¢£¨ÓÃÓÚÇþµÀÁ÷Ë®×°ÔØºÍ¼ÆËã·ÑÂÊ)	
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
 *	»ñÈ¡DISC_IDËùÓÐ·Ö¶ÎµÄ·ÑÂÊ	
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
 *	»ñÈ¡ÕËºÅÀàÐÍSQL IN×Ö·û´®
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
 *	»ñÈ¡»ú¹¹·ÖÈóÅäÖÃ±íÁÐ±í
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

	//	·µ»ØÖµppstAlgo Ö¸¶¨ÁËvoidÀàÐÍ£¬ÌõÊýÎ»ÒÆ²ÉÓÃÒÔÏÂ·½Ê½
	for(i = 0; i < *plOut; i ++)
		lGetEXLong(&((dbBrhAlgo *)*ppstBAlgo + i)->card_type);

	return RC_SUCC;
}

/****************************************************************************************
 *	´ÓÄ£ÐÍÁÐ±íÖÐÕÒµ½¶ÔÓ¦µÄÄ£ÐÍ¶Î
 ****************************************************************************************/
long	lGetAlgoService(dbDisc *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDisc *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("ÎÞÐ§µÄ·ÑÂÊÄ£ÐÍÁÐ±í(0X%08X)»ò¸öÊý(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  Ä£ÐÍ²»Ö¹Ò»Ìõ
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
	else if(1 == lAlgoCnt)	//  Ö»ÓÐÒ»Ìõ£¬Ä¬ÈÏ
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDisc));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("·ÑÂÊÄ£ÐÍ¸öÊý(%d), Î´ÕÒµ½½»Ò×½ð¶î(%.2f)µÄ¸Ã¶Î·ÑÂÊÄ£ÐÍ!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	´ÓÄ£ÐÍÁÐ±íÖÐÕÒµ½¶ÔÓ¦µÄÄ£ÐÍ¶Î
 ****************************************************************************************/
long	lGetAlgoModel(dbDiscAlgo *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDiscAlgo *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("ÎÞÐ§µÄ·ÑÂÊÄ£ÐÍÁÐ±í(0X%08X)»ò¸öÊý(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  Ä£ÐÍ²»Ö¹Ò»Ìõ
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
	else if(1 == lAlgoCnt)	//  Ö»ÓÐÒ»Ìõ£¬Ä¬ÈÏ
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDiscAlgo));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("·ÑÂÊÄ£ÐÍ¸öÊý(%d), Î´ÕÒµ½½»Ò×½ð¶î(%.2f)µÄ¸Ã¶Î·ÑÂÊÄ£ÐÍ!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	»ñÈ¡DISC_IDËùÓÐ·Ö¶ÎµÄ·ÑÂÊ	
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
		vSetErrMsg("ÎÞÐ§µÄ¿¨ÀàÐÍ(%d)!", lActType);
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

	//	±éÀúËùÓÐ¼ÇÂ¼
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

		//	ÅÐ¶ÏÊÇ·ñÊÇËùÐèµÄ¿¨ÀàÐÍ
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
			vSetErrMsg("·ÖÅä´óÐ¡(%d)ÄÚ´æÊ§°Ü!", pstRun->lSize * (lCount + 1));
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
		vSetErrMsg("Î´ÕÒµ½¿¨ºÅ(%d)¶ÔÓ¦µÄ·ÑÂÊÄ£ÐÍ(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	·ÑÂÊÄ£ÐÍÀàÐÍ±íÊý¾Ý×ª»»	
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
 *	»ñÈ¡DISC_ID·þÎñ·ÑËùÓÐ·Ö¶ÎµÄ·ÑÂÊ	
 ****************************************************************************************/
long	lGetServiceDisc(char *pszDiscId, long lActType, long lMask, dbDisc **ppstAlgo, long *plOut) 
{
	long		lRet = 0, lCount = 0;
	dbDisc		stAlgo;
	void		*pszOut = NULL;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszDiscId);
/*	//	·þÎñ·Ñ£¬ÔÝÊ±²»¿¼ÂÇ¿¨ÀàÐÍ
	if(lActType <= 0)
	{
		vSetErrMsg("ÎÞÐ§µÄ¿¨ÀàÐÍ(%d)!", lActType);
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

	//	±éÀúËùÓÐ¼ÇÂ¼
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

		//	ÅÐ¶ÏÊÇ·ñÊÇËùÐèµÄ¿¨ÀàÐÍ
		lGetEXLong(&stAlgo.card_type);
		if(0 == lCount)
			pszOut = (char *)malloc(pstRun->lSize);
 		else
			pszOut = (char *)realloc(pszOut, pstRun->lSize * (lCount + 1));
		if(NULL == pszOut)
		{
			vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
			vSetErrMsg("·ÖÅä´óÐ¡(%d)ÄÚ´æÊ§°Ü!", pstRun->lSize * (lCount + 1));
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
		vSetErrMsg("Î´ÕÒµ½¿¨ºÅ(%d)¶ÔÓ¦µÄ·ÑÂÊÄ£ÐÍ(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
	»ñÈ¡·þÎñÅäÖÃÄ£ÐÍ
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
 *	ËÑË÷ÄÚ´æÖÐÉÌ»§¶ÔÓ¦µÄ·þÎñ·ÑÄ£ÐÍ
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
 *	ËÑË÷ÄÚ´æ±íÖÐÖ¸¶¨Ë÷Òý×Ö¶ÎµÄ¼«Öµ FIELD_MAX  FIELD_MIN
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

	//	±éÀúËùÓÐ¼ÇÂ¼
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
			vSetErrMsg("Î´Ê¶±ðµÄÆ¥Åä¶¨Òå(%d)", lMask);
			return RC_FAIL;
		}
	}
	vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);

	if(0 == lInit)
		return RC_NOTFOUND;
	return RC_SUCC;
}

/****************************************************************************************
 * 	group by cups_no¹¦ÄÜ	
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

		//	ÒÑ´æÔÚ£¬ÏÂÌõ
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
 * 	ÏòÇþµÀ½ð¶î¼ÓÈëÊý¾Ý
 ****************************************************************************************/
long	lInsertCupsAmt(EXTCupAmt *pstCupAmt)
{
	return RC_SUCC;
}

/****************************************************************************************
 *	code end
 ****************************************************************************************/
