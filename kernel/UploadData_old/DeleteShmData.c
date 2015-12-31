#include	"pos_stlm.h"
#include	"Ibp.h"
#include	"load.h"
#include	"DbsApi.h"

/****************************************************************************************
	主函数
 ****************************************************************************************/
int		main(int argc, char** argv)
{
	int		iRet = 0;

	memset(gsLogFile, 0, sizeof(gsLogFile));
	sprintf(gsLogFile, "DeleteShmData.log");

	//判断参数个数
	if(argc < 5)
	{
		HtLog(ERR_PARAM, "参数个数有误argc[%d].exec:%s stlm_date settle_num bat_id ", argc, argv[0]);
		return RC_FAIL;
	}
	
	iRet = lInitTblShm();
	if(iRet)
	{
		if(22 == errno)
		{
			HtLog(ERR_PARAM, "共享内存已经消失，处理完成!(%d)(%s)", iRet, sGetError());
			return RC_SUCC;
		}
		else
		{
			HtLog(ERR_PARAM, "连接共享内存失败, err:(%d)(%d)(%s)", iRet, errno, sGetError());
			return RC_FAIL;
		}
	}

	HtLog(NML_PARAM, "清理共享内存ID(%d), Key(0X%08X)", lGetShmId(), yGetShmKey());
	vDeleteShm();
	HtLog(NML_PARAM, "清理完毕!");
	return RC_SUCC;
}
