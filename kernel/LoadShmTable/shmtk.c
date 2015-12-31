#include	"load.h"

/****************************************************************************************
 *	ȫ�ֱ��������� 
 ****************************************************************************************/
IBPShm		g_ShmRun = {0};
RunTime		g_RunTime = {0};

/****************************************************************************************
 * 	��Ҫ�ڲ��̻�����	
 ****************************************************************************************/
void	vTblDisconnect(TABLE t);

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
IBPShm*	pGetShmRun()
{
	return &g_ShmRun;
}

/****************************************************************************************
 * 	��ʼ����ѯ�ṹ
 ****************************************************************************************/
void*	pInitRunTime(TABLE t)
{
	long		lRet = 0;
	BSysIdx		stSys;

	memset(&g_RunTime, 0, sizeof(g_RunTime));

	//	�����Ƿ�ϵͳ���������ϵͳ�������
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

	//	��ȡ�ͻ���ID�ɹ��󣬸��ÿͻ���ʽ��ѯ
	memset(&g_RunTime, 0, sizeof(g_RunTime));
//	g_RunTime.bSearch = stSys.m_lType;
	g_RunTime.bSearch = TYPE_CLIENT;

	return &g_RunTime;
}

/****************************************************************************************
	��ȡ����/ɾ����¼��
 ****************************************************************************************/
long	lGetEffect()
{
	return	((IBPShm *)pGetShmRun())->m_lEffect;
}

/****************************************************************************************
 * 	�ý��̱������ڴ������
 ****************************************************************************************/
void	vHoldConnect()
{
//	���鲻Ҫ����ʹ�ã� ��Щϵͳֻ��attch�̶������� ��ѯ���øò����Ͳ��ܹرչ����ڴ�����
//	�����ٴ����ӳ��� 12:Cannot allocate memory 
	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_CONNECT;
}

/****************************************************************************************
 * 	�˿��빲���ڴ������
 ****************************************************************************************/
void	vDisConnect(TABLE t)
{
	((IBPShm *)pGetShmRun())->m_bAttch = SHM_DISCONNECT;
	vTblDisconnect(t);
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
		vDisConnect(table);
}

/****************************************************************************************
 * 	��ȡ���Ӵ��������ڴ��KEYֵ
 ****************************************************************************************/
long	lGetShmId()
{
	if(g_ShmRun.m_shmID <= 0)	
	{
		vSetErrMsg("�����ڴ滹δ��ʼ�������ȳ�ʼ��");
		return RC_FAIL;
	}
	return g_ShmRun.m_shmID;
}

/****************************************************************************************
 * 	��ȡ���Ӵ��������ڴ��KEYֵ
 ****************************************************************************************/
void*	pGetShmAddr(TABLE t)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[t])
	{
		vSetErrMsg("�����ڴ滹δ��ʼ�������ȳ�ʼ��");
		return NULL;
	}

	return g_ShmRun.m_pszAddr[t];
}

/****************************************************************************************
	ָ������ݵĵ�ַ
 ****************************************************************************************/
void*	pGetTblAddr(TABLE t, long nOffset)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[t])
	{
		vSetErrMsg("�����ڴ滹δ��ʼ�������ȳ�ʼ��");
		return NULL;
	}

	return g_ShmRun.m_pszAddr[t] + g_ShmRun.m_lOffset + nOffset;
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
key_t	yGetIPCPath(Benum em)
{
	char	szKeyPath[512];
	key_t   yKey;

	memset(szKeyPath, 0, sizeof(szKeyPath));
	snprintf(szKeyPath, sizeof(szKeyPath), "%s/bin", getenv("HOME"));
	
	yKey = ftok(szKeyPath, em);
	if(yKey <= RC_FAIL)
	{
		vSetErrMsg("��ȡ�ڴ�Keyֵʧ��(%s):(%s)", szKeyPath, strerror(errno));
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
long	lInitCreateShm(TABLE t, long lSize, BOOL bCreate)
{
	if(lSize <= 0)
	{
		vSetErrMsg("����, �����ռ��С(%d)<0", lSize);
		return RC_FAIL;
	}

	memset(&g_ShmRun, 0, sizeof(g_ShmRun));

	//	�����ؽ������ڴ棬ÿ�ε���������û�����Ƶ�
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
		vSetErrMsg("���������ڴ�ʧ��:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr[t] = (void *)shmat(((IBPShm *)pGetShmRun())->m_shmID, NULL, 0);
	if(NULL == ((IBPShm *)pGetShmRun())->m_pszAddr[t] || 
		(void *)(-1) == ((IBPShm *)pGetShmRun())->m_pszAddr[t])
	{	   
		vSetErrMsg("���ӹ����ڴ�ʧ��:(%s)", strerror(errno));
		return RC_FAIL;
	}

	memset(((IBPShm *)pGetShmRun())->m_pszAddr[t], 0, lSize);
	shmdt(((IBPShm *)pGetShmRun())->m_pszAddr[t]);
	((IBPShm *)pGetShmRun())->m_pszAddr[t] = NULL;
	((IBPShm *)pGetShmRun())->m_bInit[t] = false;

	return RC_SUCC;
}

/****************************************************************************************
	��ʼ���ڴ�ָ��
 ****************************************************************************************/
long	lInitTblShm(long lType)
{
	long	bAttch = ((IBPShm *)pGetShmRun())->m_bAttch;

	//	����ǿͻ���ID��ϵͳ���еǼǣ��������Kyeֵ��ȡ
	if(TYPE_CLIENT == lType)	return RC_SUCC;

	memset(&g_ShmRun, 0, sizeof(g_ShmRun));
	((IBPShm *)pGetShmRun())->m_bAttch = bAttch;

	((IBPShm *)pGetShmRun())->m_yKey = yGetIPCPath(IPC_SHM);
	if(((IBPShm *)pGetShmRun())->m_yKey == RC_FAIL)
	{
		vSetErrMsg("�����ڴ��Key����Ч");
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
void	vTblDisconnect(TABLE t)
{
	if(((IBPShm *)pGetShmRun())->m_pszAddr[t])
		shmdt(((IBPShm *)pGetShmRun())->m_pszAddr[t]);
	((IBPShm *)pGetShmRun())->m_pszAddr[t] = NULL;
	((IBPShm *)pGetShmRun())->m_bInit[t] = false;
}

/****************************************************************************************
 * 	���ӹ����ڴ�	
 ****************************************************************************************/
long	lAttachShm(TABLE t)
{
	void	*p = NULL;

	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}	   

	p = (void* )shmat(lGetShmId(), NULL, 0);
	if(NULL == (char *)p || (void *)(-1) == (void *)p)
	{	   
		vSetErrMsg("���ӹ����ڴ�ʧ��:(%d)(%d)(%s)!", lGetShmId(), errno, strerror(errno));
		return RC_FAIL;
	}
	((IBPShm *)pGetShmRun())->m_pszAddr[t] = p;
	((IBPShm *)pGetShmRun())->m_bInit[t] = true;

	return RC_SUCC;
}

/****************************************************************************************
 * 	ɾ�������ڴ�	
 ****************************************************************************************/
void	vDeleteTable(long lShmId, TABLE t)
{
	int		iRet = 0;

	//	�ȶϿ���ɾ��
	vTblDisconnect(t);

	((IBPShm *)pGetShmRun())->m_shmID = lShmId;

	if(((IBPShm *)pGetShmRun())->m_shmID <= 0)
	{	   
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return ;
	}	   

	iRet = shmctl(((IBPShm *)pGetShmRun())->m_shmID, IPC_RMID, NULL);
	if(iRet)
	{	   
		vSetErrMsg("ɾ�������ڴ�ʧ��:(%s)!", strerror(errno));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_pszAddr[t] = NULL;
}

/****************************************************************************************
	ָ��Բ�ѯ�����Ľ�����׵�ַ	
 ****************************************************************************************/
char*	pGetSelectAddr(TABLE t)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[t])
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return NULL;
	}
	return (char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[t];
}

/****************************************************************************************
 *	 ���ӹ����ڴ沢��ʼ����Ҫ����
 ****************************************************************************************/
long	lInitMemTable(TABLE t, long lType)
{
	long	lRet = 0;

	lRet = lInitTblShm(lType);
	if(lRet)	return RC_FAIL;

	//	��Щ���� ֻ��attchһ��
	lRet = lAttachShm(t);
	if(lRet) return RC_FAIL;

	//  ��ʼ��ͷ���
	vSetTblDef(t, (char *)pGetShmAddr(t));
	
	//  �˳��ź�	vTblDisconnect();
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
	//	 ����δ���������������۲���ʲô��ʽ��ѯֱ�ӷ��ز�ƥ��
	if(0 == nCompare && IDX_SELECT == lFindType)
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

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, 
				(char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)					//	����һ������������ֱ�ӷ���
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else	//  ����ʹ��������ƥ�仹�ǲ�ѯ������ƥ�䣬�˳��ü�¼��ƥ��, �����������
				return RC_FAIL;	
			break;	//	ƥ��ò�ѯ�����ˣ�break������ ��Ҫƥ�������Ĳ�ѯ����
		case FIELD_CHAR:
			//	�����Ƿ��ѯ����������������CHAR����δ��ֵ����ʾ����ѯ
			if(RC_SUCC == lIsNRealZore((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, 
				pstIdx[i].m_lKeyLen))
				break;

			lRet = memcmp((char *)pstRun->pstVoid + pstIdx[i].m_lKeyOffSet, 
				(char *)p + pstIdx[i].m_lKeyOffSet, pstIdx[i].m_lKeyLen);
			if(!lRet)										//	����һ������������ֱ�ӷ���
			{
				if(IDX_SELECT == lFindType)	
					return RC_SUCC;
			}
			else	//	����ʹ��������ƥ�仹�ǲ�ѯ������ƥ�䣬�Ƴ��ü�¼��ƥ��?�����������
				return RC_FAIL;
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

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return RC_FAIL;
	}

	if((char *)g_ShmRun.stRunTime[pstRun->tblName].pstVoid)
	{
		vTableClose(pstRun->tblName, SHM_DISCONNECT);
		vSetErrMsg("�����α����, �α��ظ�����");
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
	vTblDisconnect(table);
}

/****************************************************************************************
	��ȡ��һ�¼�¼	
 ****************************************************************************************/
long	lTableFetch(TABLE table, void *pszVoid, long lSize)
{
	if(!((IBPShm *)pGetShmRun())->m_bInit[table])
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return RC_FAIL;
	}

	if(!(char *)(((IBPShm *)pGetShmRun())->m_pvCurAddr[table]))
	{
		vSetErrMsg("��Ч���α�����α�δ����(%d)", table);
		return RC_FAIL;
	}

	if(((IBPShm *)pGetShmRun())->m_lLSize[table] != lSize)
	{
		vSetErrMsg("�ڴ��(%s)�汾�������⣬(%d)(%d)����/������", sGetTableName(table),
			((IBPShm *)pGetShmRun())->m_lLSize[table], lSize);
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

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��(%d)δ������������ѯ��ʽ����֧��", pstRun->tblName);
		return RC_FAIL;
	}

	if(FIELD_MAX == lMask)
		pstTree = (SHTree *)pGetMaxSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)));
	else if(FIELD_MIN == lMask)
		pstTree = (SHTree *)pGetMinSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)));
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr(pstRun->tblName) + pstTree->m_lData, pstRun->lSize);
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

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != pstRun->lSize)
	{
		vSetErrMsg("�ڴ��(%s)�汾�������⣬(%d)(%d)����/������!", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return RC_FAIL;
	}

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��(%d)(%s)δ���ÿ��ٲ�ѯ�ڵ������δװ��", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_NOTFOUND;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)), 
		0, pstRun->pstVoid);
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

	memcpy((char *)pstVoid, (char *)pGetShmAddr(pstRun->tblName) + pstTree->m_lData, pstRun->lSize);

	if(MATCH_YES == pstRun->lReMatch)
	{
		//  ����������⣬�ٴ�ƥ��һ��, ֻƥ��������
		if(RC_SUCC != lTblIdxCompare(pstRun, pstVoid, pstTree))
		{
 			vTblInitParam(pstRun->tblName);
			vSetErrMsg("���ش���������������ݲ�ƥ��");
			return RC_FAIL;
		}
	}

	vTblInitParam(pstRun->tblName);

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

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != pstRun->lSize)
	{
		vSetErrMsg("�ڴ��(%s)�汾�������⣬(%d)(%d)����/������!", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], pstRun->lSize);
		return RC_FAIL;
	}

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return RC_FAIL;
	}

	if(0 == lGetIdxNum(pstRun->tblName))
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��(%d)����δ���ã���ѯ��ʽ����֧��", pstRun->tblName);
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(pstRun->tblName, lGetTblPos(pstRun->tblName));
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

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(pstRun->tblName, 
		lGetTblPos(pstRun->tblName));
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
	���¼�¼	ע:�޽���ά�����Ľṹ, ��˲��ܸ���AVL�ڵ�
 ****************************************************************************************/
long	lUpdateQuick(RunTime *pstRun, void *pstVoid, long lSize)
{
	SHTree  *pstTree = NULL;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(pstRun->lSize <= 0 || NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{
		vSetErrMsg("���ò�ѯ������������(%d)(%d)", pstRun->lSize, pstRun->lFind);
		return RC_FAIL;
	}

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != lSize)
	{   
		vSetErrMsg("�ڴ��(%s)�汾�������⣬(%d)(%d)����/������", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], lSize);
		return RC_FAIL;
	}   

	if(((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("��(%d)(%s)�����ڴ����޷����´���", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_FAIL;
	}   

	//	P ����
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_P))
		return RC_FAIL;

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��");
		return RC_FAIL;
	}   

	if(0 == pGetTblDef(pstRun->tblName)->lTreeNode)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��(%d)(%s)δ���ÿ��ٲ�ѯ�ڵ������δװ��", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_NOTFOUND;
	}

	pstTree = (SHTree *)pSearchSHTree((void *)pGetTblAddr(pstRun->tblName, lGetShmPos(pstRun->tblName)),
		0, pstRun->pstVoid);
	if(!pstTree)
	{
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��ƥ���¼, (%d)", RC_NOTFOUND);
		return RC_NOTFOUND;
	}

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

	memcpy((char *)pGetShmAddr(pstRun->tblName) + pstTree->m_lData, (char *)pstVoid, pstRun->lSize);

	vTblInitParam(pstRun->tblName);

	//	V ����
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_V))
		return RC_FAIL;

	return RC_SUCC;
}


/****************************************************************************************
	���¼�¼
 ****************************************************************************************/
long	lUpdateSpeed(RunTime *pstRun, void *pstVoid, long lSize)
{
	long	lRet = 0;

	if(RC_SUCC != lTblIsNExist(pstRun->tblName))
		return RC_FAIL;

	if(NULL == (char *)pstRun->pstVoid || IDX_SELECT != pstRun->lFind)
	{   
		vSetErrMsg("���ò�ѯ������������(%d)", pstRun->lFind);
		return RC_FAIL;
	}   

	if(((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName] != lSize)
	{   
		vSetErrMsg("�ڴ��(%s)�汾�������⣬(%d)(%d)����/������!", sGetTableName(pstRun->tblName),
			((IBPShm *)pGetShmRun())->m_lLSize[pstRun->tblName], lSize);
		return RC_FAIL;
	}   

	if(((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("��(%d)(%s)�����ڴ����޷����´���!", pstRun->tblName, 
			sGetTableName(pstRun->tblName));
		return RC_FAIL;
	}   

	//	P ����
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_P))
		return RC_FAIL;

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{   
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}   

	if(0 == lGetIdxNum(pstRun->tblName))
	{   
		vTblInitParam(pstRun->tblName);
		vSetErrMsg("��(%d)����δ���ã���ѯ��ʽ����֧��", pstRun->tblName);
		return RC_FAIL;
	}   

	((IBPShm *)pGetShmRun())->m_lEffect  = 0;
	((IBPShm *)pGetShmRun())->m_lCurLine[pstRun->tblName] = 1;
	((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName] = 
		(void *)pGetTblAddr(pstRun->tblName, lGetTblPos(pstRun->tblName));
	if(!(char *)((IBPShm *)pGetShmRun())->m_pvCurAddr[pstRun->tblName])
	{   
		vSetErrMsg("��Ч���α�����α�δ����");
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

	//	V ����
	if(RC_SUCC != lSemOperate(((IBPShm *)pGetShmRun())->m_semID, 0, SEM_O_V))
		return RC_FAIL;

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

	if(RC_SUCC != lInitMemTable(pstRun->tblName, pstRun->bSearch))
		return RC_FAIL;

	if(!((IBPShm *)pGetShmRun())->m_bInit[pstRun->tblName])
	{
		vSetErrMsg("�����ڴ����ݿ⻹δ��ʼ��, ���ȳ�ʼ��!");
		return RC_FAIL;
	}

	g_ShmRun.m_lCurLine[pstRun->tblName] = 1;
	g_ShmRun.m_pvCurAddr[pstRun->tblName] = (void *)pGetTblAddr(pstRun->tblName, lGetTblPos(pstRun->tblName));
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
	code end 
 ****************************************************************************************/
