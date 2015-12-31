#include	"load.h"

/****************************************************************************************
	�ź�������union�ṹ
 ****************************************************************************************/
union semun {
	int					val;  
	struct	semid_ds	*buf;  
	ushort				*arr;
	struct	seminfo		*__buf;
};

/****************************************************************************************
	������������
 ****************************************************************************************/
long	lInitCreateSem(long lSems, long lValue, BOOL bCreate)
{
	union	semun	uSem;
	
	if(lSems <= 0)
	{
		vSetErrMsg("����, �����ź�������(%d)<0", lSems);
		return RC_FAIL;	
	}

	if(bCreate)
	{
		((IBPShm *)pGetShmRun())->m_ySey = yGetIPCPath(IPC_SEM);
		if(((IBPShm *)pGetShmRun())->m_ySey <= RC_FAIL)
			return RC_FAIL;

		((IBPShm *)pGetShmRun())->m_semID = semget(((IBPShm *)pGetShmRun())->m_ySey, 
			lSems, IPC_CREAT|0660);
	}
	else
		((IBPShm *)pGetShmRun())->m_semID = semget(IPC_PRIVATE, lSems, IPC_CREAT|0660);
	if(RC_FAIL >= ((IBPShm *)pGetShmRun())->m_semID)
	{
		vSetErrMsg("���������ڴ�ʧ��:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	uSem.val = lValue;
	semctl(((IBPShm *)pGetShmRun())->m_semID, 0, SETVAL, uSem);

  	return RC_SUCC;
}

/****************************************************************************************
 	ɾ���ź���
 ****************************************************************************************/
void	vDeleteTableSem(long lSemId, TABLE t)
{
	int	 iRet = 0;
	union   semun   uSem;

	if(lSemId <= 0)
	{
		vSetErrMsg("��(%d)(%s)�ź�����δ��ʼ��, ���ȳ�ʼ��!", t, sGetTableName(t));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_semID = lSemId;

	//	ɾ����t�е�0���ź���, Ŀǰÿ�ű��һ���ź���, Ҳ����ɾ�������ȫ���ź���
	iRet = semctl(((IBPShm *)pGetShmRun())->m_semID, 0, IPC_RMID, uSem);
	if(iRet)
	{
		vSetErrMsg("ɾ�������ڴ�ʧ��:(%s)!", strerror(errno));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_semID = 0;
}

/****************************************************************************************
	�ź����Ĳ���
 ****************************************************************************************/
long	lSemOperate(long lSemId, long lSems, Benum evp)
{
	int		iRet = 0;
	struct	sembuf	sf; 

	if(lSemId <= 0)
	{
		vSetErrMsg("�ź���(%d)��δ��ʼ��, ���ȳ�ʼ��!", lSemId);
		return RC_FAIL;
	}

	//	��һ���ź���
	sf.sem_num = lSems;
	sf.sem_op  = evp;
	sf.sem_flg = SEM_UNDO;

	iRet = semop(lSemId, &sf, 1);
	if(RC_SUCC != iRet)
	{
		if(EINTR != errno)
			vSetErrMsg("�ź���(%d)(%d)����(%d)����, err:(%d)(%s)", lSemId, 
				lSems, evp, errno, strerror(errno));
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
	code end
 ****************************************************************************************/
