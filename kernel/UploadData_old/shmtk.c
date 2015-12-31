#include	"load.h"

/****************************************************************************************
 *	ȫ�ֱ��������� 
 ****************************************************************************************/
IBPShm		g_ShmRun = {0};
RunTime		g_RunTime = {0};

/****************************************************************************************
 * 	��Ҫ�ڲ��̻�����	
 ****************************************************************************************/
extern  long	lGetIdxNum(TABLE table);
extern  void	vSetTblDef(char *p);
extern  TblDef* pGetTblDef(TABLE t);
extern  TblKey* pGetTblIdx(TABLE t);
void	vDetachShm();

/****************************************************************************************
 * 	��ʼ����ѯ�ṹ
 ****************************************************************************************/
void*	pInitRunTime()
{
	memset(&g_RunTime, 0, sizeof(g_RunTime));
	return &g_RunTime;
}

/****************************************************************************************
 * 	��ȡ��ѯ�ṹ
 ****************************************************************************************/
void*	pGetRunTime()
{
	return &g_RunTime;
}

/****************************************************************************************
 * 	��ȡ���Ӵ��������ڴ��KEYֵ
 ****************************************************************************************/
void*	pGetShmRun()
{
	return &g_ShmRun;
}

/****************************************************************************************
 * 	�ý��̱������ڴ������
 ****************************************************************************************/
void	vHoldConnect()
{
//	���鲻Ҫ����ʹ�ã� ��Щϵͳֻ��attchһ�£� ��ѯ���øò����Ͳ��ܹرչ����ڴ�����
//	�����ٴ����ӳ��� 12:Cannot allocate memory 
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_CONNECT;
}

/****************************************************************************************
 * 	�˿��빲���ڴ������
 ****************************************************************************************/
void	vDisConnect()
{
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_DISCONNECT;
	vDetachShm();
}

/****************************************************************************************
 *	�Ƿ񱣳�����
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
 * 	��ȡ���Ӵ��������ڴ��KEYֵ
 ****************************************************************************************/
long	lGetShmId()
{
	if(g_ShmRun.m_shmID <= 0)	
	{
		vSetErrMsg("�����ڴ滹δ��ʼ�������ȳ�ʼ��!");
		return RC_FAIL;
	}
	return g_ShmRun.m_shmID;
}

/****************************************************************************************
 * 	��ȡ���Ӵ��������ڴ��KEYֵ
 ****************************************************************************************/
void*	pGetShmAddr()
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("�����ڴ滹δ��ʼ�������ȳ�ʼ��!");
		return NULL;
	}
	return g_ShmRun.m_pszAddr;
}

/****************************************************************************************
	ָ������ݵĵ�ַ
 ****************************************************************************************/
void*	pGetTblAddr(long nOffset)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("�����ڴ滹δ��ʼ�������ȳ�ʼ��!");
		return NULL;
	}
	return g_ShmRun.m_pszAddr + g_ShmRun.m_lOffset + nOffset;
}

/****************************************************************************************
 * 	��ȡ���Ӵ��������ڴ��KEYֵ
 ****************************************************************************************/
key_t	yGetShmKey()
{
	return g_ShmRun.m_yKey;
}

/****************************************************************************************
 * 	��ȡ���Ӵ��������ڴ��KEYֵ
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
		vSetErrMsg("��ȡ�ڴ�Keyֵʧ��(%s):(%s)!", szKeyPath, strerror(errno));
		return RC_FAIL;
	}

	return yKey;
}

/****************************************************************************************
 * 	����  :���ò��������еĴ�������
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
 * 	����  :���������еĴ�������
 ****************************************************************************************/
char*	sGetError()
{
	return g_ShmRun.m_szMsg;
}

/****************************************************************************************
 * 	����  :���������ڴ�
 * 	����  :
 ****************************************************************************************/
long	lInitCreateShm(long	lSize)
{
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath();
	if(((IBPShm *)pGetShmRun())->m_yKey <= RC_FAIL)
		return RC_FAIL;

	if(lSize <= 0)
	{
		vSetErrMsg("����, �����ռ��С(%d)<0", lSize);
		return RC_FAIL;
	}

	//	�����ؽ������ڴ棬ÿ�ε���������û�����Ƶ�
	((IBPShm *)pGetShmRun())->m_shmID = shmget(((IBPShm *)pGetShmRun())->m_yKey, lSize, 
		IPC_CREAT|IPC_EXCL|0660);
	if(RC_FAIL >= ((IBPShm *)pGetShmRun())->m_shmID)
	{
		vSetErrMsg("���������ڴ�ʧ��:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr = (void *)shmat(((IBPShm *)pGetShmRun())->m_shmID, NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr || (void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr)
	{	   
		vSetErrMsg("���ӹ����ڴ�ʧ��:(%s)", strerror(errno));
		return RC_FAIL;
	}

	memset(((IBPShm *)pGetShmRun())->m_pszAddr, 0, lSize);
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr);
	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
	((IBPShm *)pGetShmRun())->m_bInit = 0;

	return RC_SUCC;
}

/****************************************************************************************
	��ʼ���ڴ�ָ��
 ****************************************************************************************/
long	lInitTblShm()
{
	long	bAttch = ((IBPShm *)pGetShmRun())->m_bAttch;

	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = bAttch;

	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath();
	if(((IBPShm *)pGetShmRun())->m_yKey == RC_FAIL)
	{
		vSetErrMsg("�����ڴ��Key����Ч!");
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_shmID = shmget(((IBPShm *)pGetShmRun())->m_yKey, 0, IPC_CREAT|0660);
	if(RC_FAIL == ((IBPShm *)pGetShmRun())->m_shmID)
	{
		vSetErrMsg("���ӹ����ڴ�ʧ��, ���ڴ���������ʧ:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
	�Ͽ��빲���ڴ������ 
 ****************************************************************************************/
void	vDetachShm()
{
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr);
	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
	((IBPShm *)pGetShmRun())->m_bInit = 0;
}

/****************************************************************************************
 * 	���ӹ����ڴ�	
 ****************************************************************************************/
long	lAttachShm()
{
	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}	   

	((IBPShm *)pGetShmRun())->m_pszAddr = (void* )shmat(lGetShmId(), NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr || (void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr)
	{	   
		vSetErrMsg("���ӹ����ڴ�ʧ��:(%d)(%d)(%s)!", lGetShmId(), errno, strerror(errno));
		return RC_FAIL;
	}
	((IBPShm *)pGetShmRun())->m_bInit = 1;
	return RC_SUCC;
}

/****************************************************************************************
 * 	ɾ�������ڴ�	
 ****************************************************************************************/
void	vDeleteShm()
{
	int		iRet = 0;

	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return ;
	}	   

	iRet = shmctl(((IBPShm *)pGetShmRun())->m_shmID, IPC_RMID, NULL);
	if(iRet)
	{	   
		vSetErrMsg("ɾ�������ڴ�ʧ��:(%s)!", strerror(errno));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr = NULL;
}

/****************************************************************************************
	ָ��Բ�ѯ�����Ľ�����׵�ַ	
 ****************************************************************************************/
char*	pGetSelectAddr(TABLE table)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return NULL;
	}
	return (char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[table];
}

/****************************************************************************************
 *	 ���ӹ����ڴ沢��ʼ����Ҫ����
 ****************************************************************************************/
long	lInitMemTable()
{
	long	lRet = 0;

	//	�빲���ڴ����ڱ���������
	if(1 == ((IBPShm *)pGetShmRun())->m_bInit)
	{
		//  ��ʼ��ͷ���
		vSetTblDef((char *)pGetShmAddr());
		return RC_SUCC;	
	}

	lRet = lInitTblShm();
	if(lRet)
		return RC_FAIL;

	//	��Щ���� ֻ��attchһ��
	lRet = lAttachShm();
	if(lRet) return RC_FAIL;

	//  ��ʼ��ͷ���
	vSetTblDef((char *)pGetShmAddr());
	
	//  �˳��ź�	vDetachShm();
	//  sigal(

	return RC_SUCC;
}

/****************************************************************************************
 * 	����ַ����Ƿ���0x00	
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
	��Ψһ����������֤
 ****************************************************************************************/
long	lTblIdxCompare(RunTime *pstRun, void *p, SHTree *pstTree)
{
	long	nCompare = 0, i = 0;
	TblKey  *pstIdx = pGetTblIdx(pstRun->tblName);

	nCompare = lGetIdxNum(pstRun->tblName);
	if(0 == nCompare)	   //  ����δ���������������۲���ʲô��ʽ��ѯֱ�ӷ��ز�ƥ��
		return RC_FAIL;

	for(i = 0; i < nCompare; i ++)  //  ����ȽϵĶ���Ψһ����
	{
		if(IDX_SELECT != pstIdx[i].m_lIsPk || pstTree->m_lIdxPos != pstIdx[i].m_lKeyOffSet)
			continue;

		if(!memcmp((char *)pstRun->pstVoid, (char *)p + pstTree->m_lIdxPos, pstTree->m_lIdxLen))
			return RC_SUCC;
	}

	return RC_FAIL;
}

/****************************************************************************************
 * 	����Ψһ������ѯ
 ****************************************************************************************/
long	lTblIdxMatch(RunTime *pstRun, void *p, long lFindType)
{
	long	lRet = 0, nCompare = 0, i = 0;
	char	szTemp[50];
	TblKey  *pstIdx = pGetTblIdx(pstRun->tblName);
	
	memset(szTemp, 0, sizeof(szTemp));
	nCompare = lGetIdxNum(pstRun->tblName);
	if(0 == nCompare && IDX_SELECT == lFindType)	//	����δ���������������۲���ʲô��ʽ��ѯֱ�ӷ��ز�ƥ��
		return RC_FAIL;

	for(i = 0; i < nCompare; i ++)	//	����ȽϵĶ���Ψһ����
	{
		//	���������������ѯ��������ѯ���������޳�
		//	������ǰ��ղ�ѯ�������飬��ô���в�ѯ��������ȥƥ��
		if(IDX_SELECT == lFindType)
		{
			if(pstRun->lFind != pstIdx[i].m_lIsPk)
				continue;
		}

		switch(pstIdx[i].m_lKeyAttr)
		{
		case FIELD_LONG:	//	�Ȳ���һ�´���
			//	�����Ƿ��ѯ����������������CHAR����δ��ֵ����ʾ����ѯ
			//	0����������Ϊ0����LONG_ZER
			if(!memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, szTemp, pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, (char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	����һ������������ֱ�ӷ���
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else
				return RC_FAIL;								//	����ʹ��������ƥ�仹�ǲ�ѯ������ƥ�䣬�Ƴ��ü�¼��ƥ��, �����������
			break;											//	�ĸ���ѯ����ƥ���ˣ�break������ ��Ҫƥ�������Ĳ�ѯ����
		case FIELD_CHAR:
			//	�����Ƿ��ѯ����������������CHAR����δ��ֵ����ʾ����ѯ
			if(RC_SUCC == lIsNRealZore((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, (char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	����һ������������ֱ�ӷ���
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else
				return RC_FAIL;								//	����ʹ��������ƥ�仹�ǲ�ѯ������ƥ�䣬�Ƴ��ü�¼��ƥ�� �����������
			break;
		default:
			vSetErrMsg("������ֶ�����(%d)", pstIdx[i].m_lKeyAttr);
			return RC_FAIL;
		}
	}
	if(IDX_SELECT == lFindType)	//	������ƥ����forѭ�����δ�ҵ�������ʧ��
		return RC_FAIL;

	//	��ѯ����������forѭ�����淵�أ����for����û�п�ƥ����ֶΣ�����Ĭ�ϲ�ѯƥ��
	return RC_SUCC;
}
/****************************************************************************************
	�����α���ʼ��ַ
 ****************************************************************************************/
long	lTableDeclare(RunTime *pstRun)
{
	long	lRet = 0;

	if(!pstRun)	return RC_FAIL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if((0 == pstRun->lSize && pstRun->pstVoid) || (0 != pstRun->lSize && !pstRun->pstVoid))
	{
		vSetErrMsg("���ò�ѯ��������(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	if((char *)g_ShmRun.stRunTime[pstRun->tblName].pstVoid)
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("�����α����, �α��ظ�����!");
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
	�ر��α�ָ��
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

	if(SHM_CONNECT == lType)	//	��������
		return ;
	vDetachShm();
}

/****************************************************************************************
	��ȡ��һ�¼�¼	
 ****************************************************************************************/
long	lTableFetch(TABLE table, void *pszVoid, long lSize)
{
	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	if(!(char *)(((IBPShm *)pGetShmRun())->m_pvCurAddr[table]))
	{
		vSetErrMsg("��Ч���α�����α�δ����(%d)", table);
		return RC_FAIL;
	}

	while(1)
	{
		if(lGetTblValid(table) < g_ShmRun.m_lCurLine[table])
		{
			vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
			return RC_NOTFOUND;
		}

		if((char *)g_ShmRun.stRunTime[table].pstVoid)
		{
			//	�α��ѯ�϶����ղ�ѯ������Ѱ��
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
	����������ȡ��ֵ
 ****************************************************************************************/
long	lGetExtremeIdx(RunTime *pstRun, void *pstVoid, long lMask)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("���ò�ѯ������������(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��(%d)δ����������δ���ã���ѯ��ʽ����֧��", pstRun->tblName);
		return RC_FAIL;
	}

	if(FIELD_MAX == lMask)
		pstTree = (SHTree *)pGetMaxSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)));
	else if(FIELD_MIN == lMask)
		pstTree = (SHTree *)pGetMinSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)));
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);
	vTblInitParam(pstRun->tblName);
	return RC_SUCC;
}

/****************************************************************************************
	��AVL��������Ψһ������¼
 ****************************************************************************************/
long	lQuerySpeed(RunTime *pstRun, void *pstVoid)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("���ò�ѯ������������(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��(%d)δ���ÿ��ٲ�ѯ�ڵ�!", pstRun->tblName);
		return RC_FAIL;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)), 0, pstRun->pstVoid);
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  ����������⣬�ٴ�ƥ��һ��, ֻƥ��������
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
 			vTblInitParam(pstRun->tblName);
			vSetErrMsg("���ش���������������ݲ�ƥ��!");
			return RC_FAIL;
		}
	}

	vTblInitParam(pstRun->tblName);
	return RC_SUCC;
}

/****************************************************************************************
	�Բ���attch2�ε�ϵͳ(�Ѿ�attchһ���ˣ���δ�ͷ�)��������⺯��
 ****************************************************************************************/
long	lQuerySpeedAt(RunTime *pstRun, void *pstVoid)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("���ò�ѯ������������(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(!g_ShmRun.m_bInit)
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	if(!((IBPShm *)pGetShmRun())->m_pszAddr)
	{
		vSetErrMsg("�����ڴ����ݿ�ָ����󣬿��ܻ�δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vSetErrMsg("��(%d)δ���ÿ��ٲ�ѯ�ڵ�!", pstRun->tblName);
		return RC_FAIL;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(lGetShmPos(pstRun->tblName)), 0, pstRun->pstVoid);
	if(!pstTree)
	{
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr() + pstTree->m_lData, pstRun->lSize);

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  ����������⣬�ٴ�ƥ��һ��, ֻƥ��������
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
			vSetErrMsg("���ش���������������ݲ�ƥ��!");
			return RC_FAIL;
		}
	}

	return RC_SUCC;
}

/****************************************************************************************
 * ���ٲ�ѯ�����ڴ����ƥ���¼���̷��أ�Ҫ������Ĳ�ѯ��������ʹ������	
 ****************************************************************************************/
long	lSelectSpeed(RunTime *pstRun, void *pstVoid)
{
	long	lRet = 0, lCount = 0;
	void	*p = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("���ò�ѯ������������(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vDetachShm();
		vSetErrMsg("��(%d)������δ���ã���ѯ��ʽ����֧��", pstRun->tblName);
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("��Ч���α�����α�δ����");
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
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		vTblInitParam(pstRun->tblName);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)p, pstRun->lSize);
	vTblInitParam(pstRun->tblName);

	return RC_SUCC;
}

/****************************************************************************************
 * 	��ѯ������¼
 ****************************************************************************************/
long	lSelectTable(RunTime *pstRun, void *pstVoid)
{
	long	lRet = 0, lCount = 0;
	void	*p = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid)
	{
		vSetErrMsg("���ò�ѯ��������(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("��Ч���α�����α�δ����");
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
				vSetErrMsg("��������(%d)���м�¼��ѡ��!", lCount);
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
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		vTblInitParam(pstRun->tblName);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)p, pstRun->lSize);
	vTblInitParam(pstRun->tblName);

	return RC_SUCC;
}

/****************************************************************************************
 *	�������ѯȫ�����������������м�¼
 ****************************************************************************************/
long	lRunSelect(RunTime *pstRun, void **ppszOut, long *plOut)
{
	long	lRet = 0, lCount = 0;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid)
	{
		vSetErrMsg("���ò�ѯ��������(%d)", pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable())
		return RC_FAIL;

	if(!g_ShmRun.m_bInit)	
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{
		vSetErrMsg("��Ч���α�����α�δ����");
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
				vSetErrMsg("�����С(%d)�ڴ�ʧ��!", pstRun->lSize * (lCount + 1));
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
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);

	return RC_SUCC;
}

/****************************************************************************************
 *	Ӧ�ò�
 ****************************************************************************************/
/****************************************************************************************
 *	�����̻���������ȡ�̻���Ϣ
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
 *	�����̻����û���Ϣ
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
 *	�����̻��������������
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
 *	���ݻ�����������ȡ������Ϣ
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
 *	���ݻ�����������ȡ������Ϣ
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
*	����MCC��������ȡMCC��Ϣ
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
 *	��ȡDISC_ID���зֶεķ���	
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
 *	��ȡ��������������Ϣ
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
 *	��������ģ�Ͷ����
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
 *	���ݻ�����������ȡ������Ϣ
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

	//	��ȡ�������ƣ�����ȡ��һ���͹��ˣ� ����CHK_SELECT
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
 *	���ݻ�����������ȡ������Ϣ
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

	//	��ȡ�������ƣ�����ȡ��һ���͹��ˣ� ����CHK_SELECT
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
 *	��ȡ�˺���Ϣ
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
	//	���������ٴκ˶�һ��
	if(strcmp(pstAcctNo->app_id, pszAppId))
	{
		vSetErrMsg("���ش���������������ݲ�ƥ��!");
		return RC_FAIL;
	}

	lGetEXLong(&pstAcctNo->acct_id);
	lGetEXLong(&pstAcctNo->app_type);
	lGetEXLong(&pstAcctNo->acct_default);
#endif

	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ��������001û���ն˺Ŷ����ļ�װ�ز�ѯ�̻���
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
 *	��ȡ����·�ɱ���Ϣ������������ˮװ�غͼ������)	
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
 *	��ȡ�ַ�����Ϣ������������ˮװ�غͼ������)	
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
 *	��ȡDISC_ID���зֶεķ���	
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
 *	��ȡ�˺�����SQL IN�ַ���
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
 *	��ȡ�����������ñ��б�
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

	//	����ֵppstAlgo ָ����void���ͣ�����λ�Ʋ������·�ʽ
	for(i = 0; i < *plOut; i ++)
		lGetEXLong(&((dbBrhAlgo *)*ppstBAlgo + i)->card_type);

	return RC_SUCC;
}

/****************************************************************************************
 *	��ģ���б����ҵ���Ӧ��ģ�Ͷ�
 ****************************************************************************************/
long	lGetAlgoService(dbDisc *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDisc *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("��Ч�ķ���ģ���б�(0X%08X)�����(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  ģ�Ͳ�ֹһ��
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
	else if(1 == lAlgoCnt)	//  ֻ��һ����Ĭ��
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDisc));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("����ģ�͸���(%d), δ�ҵ����׽��(%.2f)�ĸöη���ģ��!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	��ģ���б����ҵ���Ӧ��ģ�Ͷ�
 ****************************************************************************************/
long	lGetAlgoModel(dbDiscAlgo *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDiscAlgo *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("��Ч�ķ���ģ���б�(0X%08X)�����(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  ģ�Ͳ�ֹһ��
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
	else if(1 == lAlgoCnt)	//  ֻ��һ����Ĭ��
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDiscAlgo));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("����ģ�͸���(%d), δ�ҵ����׽��(%.2f)�ĸöη���ģ��!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡDISC_ID���зֶεķ���	
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
		vSetErrMsg("��Ч�Ŀ�����(%d)!", lActType);
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

	//	�������м�¼
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

		//	�ж��Ƿ�������Ŀ�����
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
			vSetErrMsg("�����С(%d)�ڴ�ʧ��!", pstRun->lSize * (lCount + 1));
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
		vSetErrMsg("δ�ҵ�����(%d)��Ӧ�ķ���ģ��(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	����ģ�����ͱ�����ת��	
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
 *	��ȡDISC_ID��������зֶεķ���	
 ****************************************************************************************/
long	lGetServiceDisc(char *pszDiscId, long lActType, long lMask, dbDisc **ppstAlgo, long *plOut) 
{
	long		lRet = 0, lCount = 0;
	dbDisc		stAlgo;
	void		*pszOut = NULL;
	RunTime		*pstRun = (RunTime *)pInitRunTime();

	IBPStrIsNull(pszDiscId);
/*	//	����ѣ���ʱ�����ǿ�����
	if(lActType <= 0)
	{
		vSetErrMsg("��Ч�Ŀ�����(%d)!", lActType);
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

	//	�������м�¼
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

		//	�ж��Ƿ�������Ŀ�����
		lGetEXLong(&stAlgo.card_type);
		if(0 == lCount)
			pszOut = (char *)malloc(pstRun->lSize);
 		else
			pszOut = (char *)realloc(pszOut, pstRun->lSize * (lCount + 1));
		if(NULL == pszOut)
		{
			vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
			vSetErrMsg("�����С(%d)�ڴ�ʧ��!", pstRun->lSize * (lCount + 1));
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
		vSetErrMsg("δ�ҵ�����(%d)��Ӧ�ķ���ģ��(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
	��ȡ��������ģ��
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
 *	�����ڴ����̻���Ӧ�ķ����ģ��
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
 *	�����ڴ����ָ�������ֶεļ�ֵ FIELD_MAX  FIELD_MIN
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

	//	�������м�¼
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
			vSetErrMsg("δʶ���ƥ�䶨��(%d)", lMask);
			return RC_FAIL;
		}
	}
	vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);

	if(0 == lInit)
		return RC_NOTFOUND;
	return RC_SUCC;
}

/****************************************************************************************
 * 	group by cups_no����	
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

		//	�Ѵ��ڣ�����
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
 * 	����������������
 ****************************************************************************************/
long	lInsertCupsAmt(EXTCupAmt *pstCupAmt)
{
	return RC_SUCC;
}

/****************************************************************************************
 *	code end
 ****************************************************************************************/
