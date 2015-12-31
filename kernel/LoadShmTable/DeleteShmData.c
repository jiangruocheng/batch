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

	iRet = lDropTables(TABLE_ALL, 0, true);
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

	fprintf(stdout, "清理内存表完毕!\n");
	return RC_SUCC;
}
