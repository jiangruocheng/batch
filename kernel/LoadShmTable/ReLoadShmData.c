#include	"load.h"
#include	"pos_stlm.h"
#include	"Ibp.h"
#include	"DbsApi.h"

/****************************************************************************************
	主函数
 ****************************************************************************************/
int		main(int argc, char** argv)
{
	long	i, lRet = 0, lCount = 0;
	pid_t	lPid = 0;
	BOOL	bFind = false;
	TABLE	table, *pTable = NULL;
	char	szSettleDate[15], szSettleNum[5];

	memset(gsLogFile, 0, sizeof(gsLogFile));
	sprintf(gsLogFile, "ReLoadShmData.log");

	//判断参数个数
	if(argc < 5)
	{
		HtLog(ERR_PARAM, MAIN_ERR, argc, argv[0]);
		return RC_FAIL;
	}

	memset(szSettleDate, 0, sizeof(szSettleDate));
	memset(szSettleNum, 0, sizeof(szSettleNum));
	memcpy(szSettleDate, argv[1], 8);
	memcpy(szSettleNum, argv[2], 4);

	HtLog(NML_PARAM, "装载日期(%s)装载批次(%s), 装载标识(%s)(%d)", szSettleDate, 
		szSettleNum, argv[4], getpid());

	if(RC_SUCC != lGetLoadList(&pTable, (long *)&lCount))
	{
		fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
		return RC_FAIL;
	}

	//	非全量装载
	if(strcmp("*", argv[4]) && strcmp("all", argv[4]))
	{
		table = atol(argv[4]);
		for(i = 0; i < lCount; i ++)
		{
			if(table == pTable[i])	
			{
				bFind = true;
				break;
			}
		}
		if(!bFind)
		{
			IBPFree(pTable);
			fprintf(stderr, "未定义此表(%s)装载方法\n", argv[4]);
			return RC_SUCC;
		}
		lCount = i + 1;
	}
	else
		i = 0;


	if(RC_SUCC != lInitTblShm(TYPE_SYSTEM))
	{
		if(22 == errno)
		{
			HtLog(NML_PARAM, "内存表已消失，重建内存表索引!");
  			if(RC_SUCC != lBuildSysParam())
			{
				IBPFree(pTable);
				fprintf(stderr, "加载内存数据出错, err:%s", sGetError());
 				return RC_FAIL;
			}
		}
	}

	for(i; i < lCount; i ++)
	{
		lPid = fork();
		if(lPid  < 0)
		{
			HtLog(ERR_PARAM, "fork 子进程失败, err:(%d)(%s)", errno, strerror(errno));
			exit(-1);
		}
		else if(lPid > 0)
			continue;
		else 	//	lPid == 0 success
			;

		if(RC_SUCC != lDbsConnect(lGetConnect(pTable[i])))
		{
			HtLog(ERR_PARAM, "连接数据库失败, err:(%d)(%s)(%s)", lGetConnect(pTable[i]), 
				sGetError(), sDbsError());
			return RC_FAIL;
		}

		if(RC_SUCC != lCreateTable(szSettleDate, szSettleNum, pTable[i]))
		{
			fprintf(stderr, "加载内存数据出错, err:%s\n", sGetError());
			vDbsDisconnect();
			return RC_FAIL;
		}

		IBPFree(pTable);
		vDbsDisconnect();
		return RC_SUCC;
	}

	IBPFree(pTable);

	return RC_SUCC;
}
