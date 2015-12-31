#include	"load.h"
#include	"pos_stlm.h"
#include	"Ibp.h"
#include	"DbsApi.h"

/****************************************************************************************
	������
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

	//�жϲ�������
	if(argc < 5)
	{
		HtLog(ERR_PARAM, MAIN_ERR, argc, argv[0]);
		return RC_FAIL;
	}

	memset(szSettleDate, 0, sizeof(szSettleDate));
	memset(szSettleNum, 0, sizeof(szSettleNum));
	memcpy(szSettleDate, argv[1], 8);
	memcpy(szSettleNum, argv[2], 4);

	HtLog(NML_PARAM, "װ������(%s)װ������(%s), װ�ر�ʶ(%s)(%d)", szSettleDate, 
		szSettleNum, argv[4], getpid());

	if(RC_SUCC != lGetLoadList(&pTable, (long *)&lCount))
	{
		fprintf(stdout, "��ѯװ�ر��б�ʧ��, err:%s\n", sGetError());
		return RC_FAIL;
	}

	//	��ȫ��װ��
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
			fprintf(stderr, "δ����˱�(%s)װ�ط���\n", argv[4]);
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
			HtLog(NML_PARAM, "�ڴ������ʧ���ؽ��ڴ������!");
  			if(RC_SUCC != lBuildSysParam())
			{
				IBPFree(pTable);
				fprintf(stderr, "�����ڴ����ݳ���, err:%s", sGetError());
 				return RC_FAIL;
			}
		}
	}

	for(i; i < lCount; i ++)
	{
		lPid = fork();
		if(lPid  < 0)
		{
			HtLog(ERR_PARAM, "fork �ӽ���ʧ��, err:(%d)(%s)", errno, strerror(errno));
			exit(-1);
		}
		else if(lPid > 0)
			continue;
		else 	//	lPid == 0 success
			;

		if(RC_SUCC != lDbsConnect(lGetConnect(pTable[i])))
		{
			HtLog(ERR_PARAM, "�������ݿ�ʧ��, err:(%d)(%s)(%s)", lGetConnect(pTable[i]), 
				sGetError(), sDbsError());
			return RC_FAIL;
		}

		if(RC_SUCC != lCreateTable(szSettleDate, szSettleNum, pTable[i]))
		{
			fprintf(stderr, "�����ڴ����ݳ���, err:%s\n", sGetError());
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
