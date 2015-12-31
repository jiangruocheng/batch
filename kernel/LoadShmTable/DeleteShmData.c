#include	"pos_stlm.h"
#include	"Ibp.h"
#include	"load.h"
#include	"DbsApi.h"

/****************************************************************************************
	������
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
			HtLog(ERR_PARAM, "�����ڴ��Ѿ���ʧ���������!(%d)(%s)", iRet, sGetError());
			return RC_SUCC;
		}
		else
		{
			HtLog(ERR_PARAM, "���ӹ����ڴ�ʧ��, err:(%d)(%d)(%s)", iRet, errno, sGetError());
			return RC_FAIL;
		}
	}

	fprintf(stdout, "�����ڴ�����!\n");
	return RC_SUCC;
}
