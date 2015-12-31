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

	//�жϲ�������
	if(argc < 5)
	{
		HtLog(ERR_PARAM, "������������argc[%d].exec:%s stlm_date settle_num bat_id ", argc, argv[0]);
		return RC_FAIL;
	}
	
	iRet = lInitTblShm();
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

	HtLog(NML_PARAM, "�������ڴ�ID(%d), Key(0X%08X)", lGetShmId(), yGetShmKey());
	vDeleteShm();
	HtLog(NML_PARAM, "�������!");
	return RC_SUCC;
}
