#include	"load.h"

#define	BOOL(x)	(x == 0 ? false : true)	

/****************************************************************************************
	main
 ****************************************************************************************/
int		main(int argc, char** argv)
{
	long			lRet = 20000473;
	dbMchtUser		stUser;	
	CMList			*pstNode = NULL, *pstRoot = NULL;

	memset(&stUser, 0, sizeof(stUser));
	if(RC_SUCC != lGetMchtUser(&stUser, lRet))
	{
		fprintf(stderr, "≤È—ØTBL_MCHT_USER±Ì(%d)¥ÌŒÛ, err:(%s)\n", lRet, sGetError());
		return RC_FAIL;
	}

	fprintf(stdout, "[%d]:\t%10d|%15s|%50s|%30s|%11s|%20s|%18s|\n", 0, stUser.user_id, stUser.mcht_no,
		stUser.user_name, stUser.user_engname, stUser.user_phone, stUser.user_email, stUser.user_card_no);

	return RC_SUCC;
}
