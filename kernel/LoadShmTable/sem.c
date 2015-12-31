#include	"load.h"

/****************************************************************************************
	信号量操作union结构
 ****************************************************************************************/
union semun {
	int					val;  
	struct	semid_ds	*buf;  
	ushort				*arr;
	struct	seminfo		*__buf;
};

/****************************************************************************************
	插入链表数据
 ****************************************************************************************/
long	lInitCreateSem(long lSems, long lValue, BOOL bCreate)
{
	union	semun	uSem;
	
	if(lSems <= 0)
	{
		vSetErrMsg("错误, 创建信号量个数(%d)<0", lSems);
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
		vSetErrMsg("创建共享内存失败:(%d)(%s)", errno, strerror(errno));
		return RC_FAIL;
	}

	uSem.val = lValue;
	semctl(((IBPShm *)pGetShmRun())->m_semID, 0, SETVAL, uSem);

  	return RC_SUCC;
}

/****************************************************************************************
 	删除信号量
 ****************************************************************************************/
void	vDeleteTableSem(long lSemId, TABLE t)
{
	int	 iRet = 0;
	union   semun   uSem;

	if(lSemId <= 0)
	{
		vSetErrMsg("表(%d)(%s)信号量还未初始化, 请先初始化!", t, sGetTableName(t));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_semID = lSemId;

	//	删除表t中第0个信号量, 目前每张表就一个信号量, 也就是删除本表的全部信号量
	iRet = semctl(((IBPShm *)pGetShmRun())->m_semID, 0, IPC_RMID, uSem);
	if(iRet)
	{
		vSetErrMsg("删除共享内存失败:(%s)!", strerror(errno));
		return ;
	}

	((IBPShm *)pGetShmRun())->m_semID = 0;
}

/****************************************************************************************
	信号量的操作
 ****************************************************************************************/
long	lSemOperate(long lSemId, long lSems, Benum evp)
{
	int		iRet = 0;
	struct	sembuf	sf; 

	if(lSemId <= 0)
	{
		vSetErrMsg("信号量(%d)还未初始化, 请先初始化!", lSemId);
		return RC_FAIL;
	}

	//	第一个信号量
	sf.sem_num = lSems;
	sf.sem_op  = evp;
	sf.sem_flg = SEM_UNDO;

	iRet = semop(lSemId, &sf, 1);
	if(RC_SUCC != iRet)
	{
		if(EINTR != errno)
			vSetErrMsg("信号量(%d)(%d)操作(%d)出错, err:(%d)(%s)", lSemId, 
				lSems, evp, errno, strerror(errno));
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
	code end
 ****************************************************************************************/
