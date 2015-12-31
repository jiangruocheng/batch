#include	"load.h"

/****************************************************************************************
 *	Ӧ�ò�
 ****************************************************************************************/
/****************************************************************************************
 *	�����̻���������ȡ�̻���Ϣ
 ****************************************************************************************/
long	lGetMchtBase(dbMchtBase *pstMcht, char *pszMchtNo)
{
	long		lRet = 0;
	dbMchtBase	stDbMcht;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_BASE)))
		return RC_FAIL;

	IBPStrIsNull(pszMchtNo);
	memset(&stDbMcht, 0, sizeof(stDbMcht));
	strcpy(stDbMcht.mcht_no, pszMchtNo);
	sTrimAll(stDbMcht.mcht_no);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_BASE;
	pstRun->lSize = sizeof(stDbMcht);
//	pstRun->pstVoid = &stDbMcht;
//	lRet = lSelectSpeed(pstRun, pstMcht);
	pstRun->pstVoid = stDbMcht.mcht_no;
	lRet = lQuerySpeed(pstRun, pstMcht);
	if(lRet !=  RC_SUCC)
		return lRet;

	lGetEXLong(&pstMcht->id);
	lGetEXLong(&pstMcht->mcht_resv1);
	lGetEXLong(&pstMcht->mcht_expand);

	return RC_SUCC;
}

/****************************************************************************************
 *	�����̻����û���Ϣ
 ****************************************************************************************/
long	lGetMchtUser(dbMchtUser *pstUser, long lUserId)
{
	long		lRet = 0;
	dbMchtUser  stUser;
	RunTime	 *pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_USER)))
		return RC_FAIL;

	memset(&stUser, 0, sizeof(stUser));
	stUser.user_id = lSetEXLong(lUserId);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_USER;

	pstRun->lSize = sizeof(stUser);
	pstRun->pstVoid = (char *)&stUser.user_id;
	lRet = lQuerySpeed(pstRun, pstUser);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstUser->user_id);

	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ�����̻���Ϣ		
 ****************************************************************************************/
long	lGetMCardDisc(dbMCard *pstCard, char *pszMchtNo)
{
	long		lRet = 0;
	dbMCard		stCard;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszMchtNo);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_CARD)))
		return RC_FAIL;

	memset(&stCard, 0, sizeof(stCard));
	strcpy(stCard.mcht_no, pszMchtNo);
	sTrimAll(stCard.mcht_no);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_CARD;
	pstRun->lSize = sizeof(stCard);
	pstRun->pstVoid = stCard.mcht_no;
	lRet = lQuerySpeed(pstRun, pstCard);
	if(lRet !=  RC_SUCC)
		return lRet;

	return RC_SUCC;
}

/****************************************************************************************
 *	������չԱ��Ϣ
 ****************************************************************************************/
long	lGetOprInfo(dbOprInf *pstOInf, long lExpand)
{
	long		lRet = 0;
	dbOprInf	stOInf;
	RunTime		*pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_OPR_INFO)))
		return RC_FAIL;

	memset(&stOInf, 0, sizeof(stOInf));
	stOInf.opr_id = lSetEXLong(lExpand);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_OPR_INFO;

	pstRun->lSize = sizeof(stOInf);
	pstRun->pstVoid = (char *)&stOInf.opr_id;
	lRet = lQuerySpeed(pstRun, pstOInf);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstOInf->opr_id);

	return RC_SUCC;
}
/****************************************************************************************
 *	�����̻���������ȡ�̻���Ϣ
 ****************************************************************************************/
long	lGetPorConfig(dbProCfg *pstPro, char *pszId)
{
	long		lRet = 0;
	dbProCfg	stPro;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszId);

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_PRO_CONFIG)))
		return RC_FAIL;

	memset(&stPro, 0, sizeof(stPro));
	strcpy(stPro.id, pszId);
	sTrimAll(stPro.id);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_PRO_CONFIG;
	pstRun->lSize = sizeof(stPro);
	pstRun->pstVoid = stPro.id;
	lRet = lQuerySpeed(pstRun, pstPro);
	if(lRet !=  RC_SUCC)
		return lRet;

	lGetEXLong(&pstPro->pay_day);
	lGetEXLong(&pstPro->tx_seq);

	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ�����̻��������
 ****************************************************************************************/
long	lGetSpeMcht(dbSpeMcht *pstSpe, char *pszMchtNo, char *pszCode)
{
	long		lRet = 0;
	dbSpeMcht	stSpe;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszMchtNo);
	IBPStrIsNull(pszCode);

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_PRO_SPECIAL_MCHT)))
		return RC_FAIL;

	memset(&stSpe, 0, sizeof(stSpe));
	memcpy(stSpe.mcht_no, pszMchtNo, sizeof(stSpe.mcht_no) - 1);
	memcpy(stSpe.service_code, pszCode, sizeof(stSpe.service_code) - 1);
	sTrimAll(stSpe.mcht_no);
	sTrimAll(stSpe.service_code);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_PRO_SPECIAL_MCHT;
	pstRun->lSize = sizeof(stSpe);
	pstRun->pstVoid = &stSpe;

	lRet = lSelectTable(pstRun, pstSpe);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstSpe->id);

	return RC_SUCC;
}
/****************************************************************************************
 *	�����̻��������������
 ****************************************************************************************/
long	lIsMchtSettle(char *pszBrh, long *plRes)
{
	long		lRet = 0;
	dbBrhInfo	stDbBrh;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszBrh);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_BRH_INFO)))
		return RC_FAIL;

	memset(&stDbBrh, 0, sizeof(stDbBrh));
	strcpy(stDbBrh.brh_code, pszBrh);
	sTrimAll(stDbBrh.brh_code);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_INFO;
	pstRun->lSize = sizeof(stDbBrh);
//	pstRun->pstVoid = &stDbBrh;
//	lRet = lSelectSpeed(pstRun, &stDbBrh);
	pstRun->pstVoid = stDbBrh.brh_code;
	lRet = lQuerySpeed(pstRun, &stDbBrh);
	if(lRet !=  RC_SUCC)
		return lRet;

	if('1' == stDbBrh.brh_tag[0])
		*plRes = RC_SUCC;	
	else
		*plRes = RC_FAIL;	

	return RC_SUCC;
}

/****************************************************************************************
 *	���ݻ�����������ȡ������Ϣ
 ****************************************************************************************/
long	lIsBrhSettle(char *pszBrhCode, BOOL *pbRes)
{
	long		lRet = 0;
	dbBrhInfo	stDbBrh;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszBrhCode);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_BRH_INFO)))
		return RC_FAIL;

	memset(&stDbBrh, 0, sizeof(stDbBrh));
	strcpy(stDbBrh.brh_code, pszBrhCode);
	sTrimAll(stDbBrh.brh_code);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_INFO;
	pstRun->lSize = sizeof(stDbBrh);
//	pstRun->pstVoid = &stDbBrh;
//	lRet = lSelectSpeed(pstRun, &stDbBrh);
	pstRun->pstVoid = stDbBrh.brh_code;
	lRet = lQuerySpeed(pstRun, &stDbBrh);
	if(lRet != RC_SUCC)
		return lRet;

	if(BRH_STLM_YES == stDbBrh.brh_stlm_flag[0])
		*pbRes = TRUE;	
	else
		*pbRes = FALSE;	

	return RC_SUCC;
}

/****************************************************************************************
 *	���ݻ�����������ȡ������Ϣ
 ****************************************************************************************/
long	lGetBrhInfo(dbBrhInfo *pstBrhIf, char *pszBrhCode)
{
	long		lRet = 0;
	dbBrhInfo	stDbBrh;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszBrhCode);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_BRH_INFO)))
		return RC_FAIL;

	memset(&stDbBrh, 0, sizeof(stDbBrh));
	strcpy(stDbBrh.brh_code, pszBrhCode);
	sTrimAll(stDbBrh.brh_code);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_INFO;
	pstRun->lSize = sizeof(stDbBrh);
//	pstRun->pstVoid = &stDbBrh;
//	lRet = lSelectSpeed(pstRun, pstBrhIf);
	pstRun->pstVoid = stDbBrh.brh_code;
	lRet = lQuerySpeed(pstRun, pstBrhIf);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstBrhIf->brh_id);
	return RC_SUCC;
}

/****************************************************************************************
*	����MCC��������ȡMCC��Ϣ
 ****************************************************************************************/
long	lGetMccInfo(dbMchtMccInf *pstMcc, char *pszMchtTp)
{
	long			lRet = 0;
	dbMchtMccInf	stDbMcc;
	RunTime			*pstRun = NULL;

	IBPStrIsNull(pszMchtTp);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_MCC_INF)))
		return RC_FAIL;

	memset(&stDbMcc, 0, sizeof(stDbMcc));
	strcpy(stDbMcc.mchnt_tp, pszMchtTp);
	sTrimAll(stDbMcc.mchnt_tp);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_MCC_INF;
	pstRun->lSize = sizeof(stDbMcc);
//	pstRun->pstVoid = &stDbMcc;
//	lRet = lSelectSpeed(pstRun, pstMcc);
	pstRun->pstVoid = stDbMcc.mchnt_tp;
	lRet = lQuerySpeed(pstRun, pstMcc);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstMcc->id);
	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡDISC_ID���зֶεķ���	
 ****************************************************************************************/
long	lGetMchtAlgo(dbMchtAlgo *pstMAlgo, char *pszModeId)
{
	long		lRet = 0;
	dbMchtAlgo	stMAlgo;	
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszModeId);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_ALGO)))
		return RC_FAIL;

	memset(&stMAlgo, 0, sizeof(stMAlgo));
	strcpy(stMAlgo.model_id, pszModeId);
	sTrimAll(stMAlgo.model_id);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_ALGO;
	pstRun->lSize = sizeof(stMAlgo);
//	pstRun->pstVoid = &stMAlgo;
//	lRet = lSelectSpeed(pstRun, pstMAlgo);
	pstRun->pstVoid = stMAlgo.model_id;
	lRet = lQuerySpeed(pstRun, pstMAlgo);
	if(RC_SUCC != lRet)
		return lRet;

	lGetEXLong(&pstMAlgo->card_type);
	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ��������������Ϣ
 ****************************************************************************************/
long	lGetBrhPfitInf(dbPfitInfo *pstPfitInf, char *pszModeId)
{
	long			lRet = 0;
	dbPfitInfo		stPfitInf;	
	RunTime			*pstRun = NULL;

	IBPStrIsNull(pszModeId);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_BRH_PROFIT_INFO)))
		return RC_FAIL;

	memset(&stPfitInf, 0, sizeof(stPfitInf));
	strcpy(stPfitInf.model_id, pszModeId);
	sTrimAll(stPfitInf.model_id);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_INFO;
	pstRun->lSize = sizeof(stPfitInf);
//	pstRun->pstVoid = &stPfitInf;
//	return lSelectSpeed(pstRun, pstPfitInf);
	pstRun->pstVoid = stPfitInf.model_id;

	return lQuerySpeed(pstRun, pstPfitInf);
}

/****************************************************************************************
 *	��������ģ�Ͷ����
 ****************************************************************************************/
long	lGetBrhPfitDef(dbPfitDef *pstPfitDef, char *pszModeId)
{
	long			lRet = 0;
	dbPfitDef		stPfitDef;
	RunTime			*pstRun = NULL;

	IBPStrIsNull(pszModeId);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_BRH_PROFIT_DEF)))
		return RC_FAIL;

	memset(&stPfitDef, 0, sizeof(stPfitDef));
	strcpy(stPfitDef.model_id, pszModeId);
	sTrimAll(stPfitDef.model_id);

	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_DEF;
	pstRun->lSize = sizeof(stPfitDef);
//	pstRun->pstVoid = &stPfitDef;
//	return lSelectSpeed(pstRun, pstPfitDef);
	pstRun->pstVoid = stPfitDef.model_id;
	return lQuerySpeed(pstRun, pstPfitDef);
}

/****************************************************************************************
 *	���ݻ�����������ȡ������Ϣ
 ****************************************************************************************/
char*	sGetCupsName(char *pszCupsNo)
{
	long			lRet = 0;
	dbMchtCupsInf	stDbCupsIf;
	static	char	szCupsName[100];
	RunTime			*pstRun = NULL;

	memset(szCupsName, 0, sizeof(szCupsName));
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_CUPS_INF)))
		return szCupsName;

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	strcpy(stDbCupsIf.cups_no, pszCupsNo);
	sTrimAll(stDbCupsIf.cups_no);

	//	��ȡ�������ƣ�����ȡ��һ���͹��ˣ� ����CHK_SELECT
	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stDbCupsIf);
	pstRun->pstVoid = &stDbCupsIf;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return szCupsName;

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stDbCupsIf, sizeof(stDbCupsIf));
	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	if(RC_SUCC != lRet)
		return szCupsName;

	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	snprintf(szCupsName, sizeof(szCupsName), "%s", stDbCupsIf.cups_nm);
	return szCupsName;
}

/****************************************************************************************
 *	���ݻ�����������ȡ������Ϣ
 ****************************************************************************************/
char*	sGetCupsStamp(char *pszCupsNo)
{
	long			lRet = 0;
	dbMchtCupsInf	stDbCupsIf;
	static	char	szStamp[64];
	RunTime			*pstRun = NULL;

	memset(szStamp, 0, sizeof(szStamp));
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_CUPS_INF)))
		return szStamp;

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	strcpy(stDbCupsIf.cups_no, pszCupsNo);
	sTrimAll(stDbCupsIf.cups_no);

	//	��ȡ�������ƣ�����ȡ��һ���͹��ˣ� ����CHK_SELECT
	pstRun->lFind = IDX_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stDbCupsIf);
	pstRun->pstVoid = &stDbCupsIf;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return szStamp;

	memset(&stDbCupsIf, 0, sizeof(stDbCupsIf));
	lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stDbCupsIf, sizeof(stDbCupsIf));
	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	if(RC_SUCC != lRet)
		return szStamp;

	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	snprintf(szStamp, sizeof(szStamp), "%s", stDbCupsIf.cup_stamp);
	return szStamp;
}

/****************************************************************************************
 *	��ȡ�˺���Ϣ
 ****************************************************************************************/
long	lGetAcctInfo(dbAcctInfo *pstAcctNo, char *pszAppId, long lAppType, long lAttrib)
{
	long			lRet = 0;
	dbAcctInfo		stActNo;
	RunTime			*pstRun = NULL;

	IBPStrIsNull(pszAppId);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_ACCT_INFO)))
		return RC_FAIL;

	memset(&stActNo, 0, sizeof(stActNo));
	memcpy(stActNo.app_id, pszAppId, sizeof(stActNo.app_id) - 1);
	stActNo.app_type = lAppType;
	stActNo.acct_default = lAttrib;

#if		0
	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_ACCT_INFO;
	pstRun->lSize = sizeof(dbAcctInfo);
	pstRun->pstVoid = &stActNo;
	lRet = lSelectTable(pstRun, pstAcctNo);
	if(lRet != RC_SUCC)
		return lRet;

	lGetEXLong(&pstAcctNo->acct_id);
	lGetEXLong(&pstAcctNo->app_type);
	lGetEXLong(&pstAcctNo->acct_default);
#else

	pstRun->lFind = IDX_SELECT;
	pstRun->lReMatch = MATCH_NO;
	pstRun->tblName = TBL_ACCT_INFO;
	pstRun->lSize = sizeof(stActNo);
	pstRun->pstVoid = sGetComIdx(stActNo.app_id, sizeof(stActNo.app_id) - 1,
		stActNo.app_type, stActNo.acct_default);
	lRet = lQuerySpeed(pstRun, pstAcctNo);
	if(lRet !=  RC_SUCC)
		return lRet;
	//	���������ٴκ˶�һ��
	if(strcmp(pstAcctNo->app_id, pszAppId))
	{
		vSetErrMsg("���ش���������������ݲ�ƥ��!");
		return RC_FAIL;
	}

	lGetEXLong(&pstAcctNo->acct_id);
	lGetEXLong(&pstAcctNo->app_type);
	lGetEXLong(&pstAcctNo->acct_default);
#endif

	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ��������001û���ն˺Ŷ����ļ�װ�ز�ѯ�̻���
 ****************************************************************************************/
long	lGetDefRoute(dbMchtCupsInf *pstCupsInf, char *pszCupsNo, char *pszMcht)
{
	long			lRet = 0;
	dbMchtCupsInf	stCupsInf;
	RunTime			*pstRun = NULL;

	IBPStrIsNull(pszCupsNo);
	IBPStrIsNull(pszMcht);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_CUPS_INF)))
		return RC_FAIL;

	memset(&stCupsInf, 0, sizeof(stCupsInf));
	memcpy(stCupsInf.cups_no, pszCupsNo, sizeof(stCupsInf.cups_no) - 1);
	memcpy(stCupsInf.mcht_cups_no, pszMcht, sizeof(stCupsInf.mcht_cups_no) - 1);
	sTrimAll(stCupsInf.cups_no);
	sTrimAll(stCupsInf.mcht_cups_no);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stCupsInf);
	pstRun->pstVoid = &stCupsInf;

	lRet = lSelectTable(pstRun, pstCupsInf);
	if(lRet !=  RC_SUCC)
		return lRet;

	lGetEXLong(&pstCupsInf->id);
	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ����·�ɱ���Ϣ������������ˮװ�غͼ������)	
 ****************************************************************************************/
long	lGetMchtRoute(dbMchtCupsInf *pstCupsInf, char *pszCupsNo, char *pszMcht, char *pszTerm)
{
	long			lRet = 0;
	dbMchtCupsInf	stCupsInf;
	RunTime			*pstRun = NULL;

	IBPStrIsNull(pszCupsNo);
	IBPStrIsNull(pszMcht);
	IBPStrIsNull(pszTerm);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_CUPS_INF)))
		return RC_FAIL;

	memset(&stCupsInf, 0, sizeof(stCupsInf));
	memcpy(stCupsInf.cups_no, pszCupsNo, sizeof(stCupsInf.cups_no) - 1);
	memcpy(stCupsInf.mcht_cups_no, pszMcht, sizeof(stCupsInf.mcht_cups_no) - 1);
	memcpy(stCupsInf.term_cups_no, pszTerm, sizeof(stCupsInf.term_cups_no) - 1);
	sTrimAll(stCupsInf.cups_no);
	sTrimAll(stCupsInf.mcht_cups_no);
	sTrimAll(stCupsInf.term_cups_no);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	pstRun->lSize = sizeof(stCupsInf);
	pstRun->pstVoid = &stCupsInf;

	lRet = lSelectTable(pstRun, pstCupsInf);
	if(lRet !=  RC_SUCC)
		return lRet;

	lGetEXLong(&pstCupsInf->id);
	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ�ַ�����Ϣ������������ˮװ�غͼ������)	
 ****************************************************************************************/
long	lGetSpdbRoute(dbNMchtMatch *pstMatch, char *pszCupsNo, char *pszMcht)
{
	long			lRet = 0;
	dbNMchtMatch	stMatch;
	RunTime			*pstRun = NULL;

	IBPStrIsNull(pszCupsNo);
	IBPStrIsNull(pszMcht);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_N_MCHT_MATCH)))
		return RC_FAIL;

	memset(&stMatch, 0, sizeof(stMatch));
	memcpy(stMatch.cups_no, pszCupsNo, sizeof(stMatch.cups_no) - 1);
	memcpy(stMatch.cups_mcht_no, pszMcht, sizeof(stMatch.cups_mcht_no) - 1);
	sTrimAll(stMatch.cups_no);
	sTrimAll(stMatch.cups_mcht_no);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_N_MCHT_MATCH;
	pstRun->lSize = sizeof(stMatch);
	pstRun->pstVoid = &stMatch;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return lRet;

	lRet = lTableFetch(TBL_N_MCHT_MATCH, pstMatch, sizeof(dbNMchtMatch));
	vTableClose(TBL_N_MCHT_MATCH, SHM_DISCONNECT);
	if(RC_SUCC != lRet)
		return lRet;

	lGetEXLong(&pstMatch->id);
	lGetEXLong(&pstMatch->resv_0);

	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡDISC_ID���зֶεķ���	
 ****************************************************************************************/
long	lActTypeCmp(long lAllType, long lActType)
{
/*
	lActType = (int)pow(2., lActType - 1);

	if(lActType != (lAllType & lActType))
		return RC_FAIL;
	return RC_SUCC;
*/
	register long	lMask = 1, i = 0;

	for(i = 1; i <= sizeof(int) * 8; i ++)
	{
		if(!(lAllType & lMask) && (lActType & lMask))
			return RC_FAIL;
		lMask = lMask << 1;
	}
	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡ�˺�����SQL IN�ַ���
 ****************************************************************************************/
char*	sGetActInl(long lActType)
{
	static	char	szOut[60];
	long	lMask = 1, i = 0;

	memset(szOut, 0, sizeof(szOut));
	for(i = 1; i <= sizeof(int) * 8; i ++)
	{
		if((lActType & lMask) > 0)
			sprintf(szOut + strlen(szOut), "%d,", i);
		lMask = lMask << 1;
	}
	szOut[strlen(szOut) - 1] = 0x00;

	return szOut;
}

/****************************************************************************************
 *	��ȡ�����������ñ��б�
 ****************************************************************************************/
long	lGetPfitAlgo(char *pszModel, dbBrhAlgo **ppstBAlgo, long *plOut)
{
	long		lRet = 0, i = 0;
	dbBrhAlgo	stBAlgo;	
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszModel);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_BRH_PROFIT_ALGO)))
		return RC_FAIL;

	memset(&stBAlgo, 0, sizeof(stBAlgo));
	strcpy(stBAlgo.model_id, pszModel);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_BRH_PROFIT_ALGO;
	pstRun->lSize = sizeof(stBAlgo);
	pstRun->pstVoid = &stBAlgo;

	lRet = lRunSelect(pstRun, (void *)ppstBAlgo, plOut);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	����ֵppstAlgo ָ����void���ͣ�����λ�Ʋ������·�ʽ
	for(i = 0; i < *plOut; i ++)
		lGetEXLong(&((dbBrhAlgo *)*ppstBAlgo + i)->card_type);

	return RC_SUCC;
}

/****************************************************************************************
 *	��ģ���б����ҵ���Ӧ��ģ�Ͷ�
 ****************************************************************************************/
long	lGetAlgoService(dbDisc *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDisc *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("��Ч�ķ���ģ���б�(0X%08X)�����(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  ģ�Ͳ�ֹһ��
	{
		for(i = 0; i < lAlgoCnt; i ++)
		{
			if(dTxAmt <= pstAlgoList[i].upper_amount)
			{
				iFind = 1;
				memcpy(pstOut, &pstAlgoList[i], sizeof(dbDisc));
				break;
			}
		}
	}
	else if(1 == lAlgoCnt)	//  ֻ��һ����Ĭ��
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDisc));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("����ģ�͸���(%d), δ�ҵ����׽��(%.2f)�ĸöη���ģ��!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	��ģ���б����ҵ���Ӧ��ģ�Ͷ�
 ****************************************************************************************/
long	lGetAlgoModel(dbDiscAlgo *pstAlgoList, long lAlgoCnt, double dTxAmt, dbDiscAlgo *pstOut)
{
	long	i = 0, iFind = 0;

	if(0 == lAlgoCnt || !pstAlgoList)
	{
		vSetErrMsg("��Ч�ķ���ģ���б�(0X%08X)�����(%d)", lAlgoCnt, pstAlgoList);
		return RC_FAIL;
	}

	if(lAlgoCnt > 1)  //  ģ�Ͳ�ֹһ��
	{
		for(i = 0; i < lAlgoCnt; i ++)
		{
			if(dTxAmt <= pstAlgoList[i].upper_amount)
			{
				iFind = 1;
				memcpy(pstOut, &pstAlgoList[i], sizeof(dbDiscAlgo));
				break;
			}
		}
	}
	else if(1 == lAlgoCnt)	//  ֻ��һ����Ĭ��
	{
		iFind = 1;
		memcpy(pstOut, &pstAlgoList[i], sizeof(dbDiscAlgo));
	}
	else	//  lAlgoCnt <= 0
		iFind = 0;

	if(0 == iFind)
	{
		vSetErrMsg("����ģ�͸���(%d), δ�ҵ����׽��(%.2f)�ĸöη���ģ��!",
			lAlgoCnt, dTxAmt);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	��ȡDISC_ID���зֶεķ���	
 ****************************************************************************************/
long	lGetDiscAlgo(char *pszDiscId, long lActType, long lMask, dbDiscAlgo **ppstAlgo, long *plOut) 
{
	long		lRet = 0, lCount = 0;
	dbDiscAlgo	stAlgo;
	void		*pszOut = NULL;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszDiscId);
	if(lActType <= 0)
	{
		vSetErrMsg("��Ч�Ŀ�����(%d)!", lActType);
		return RC_FAIL;	
	}

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_PUB_DISC_ALGO)))
		return RC_FAIL;

	memset(&stAlgo, 0, sizeof(stAlgo));
	strcpy(stAlgo.disc_id, pszDiscId);
	sTrimAll(stAlgo.disc_id);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_PUB_DISC_ALGO;
	pstRun->lSize = sizeof(stAlgo);
	pstRun->pstVoid = &stAlgo;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	�������м�¼
	while(1)
	{
		memset(&stAlgo, 0, sizeof(stAlgo));
		lRet = lTableFetch(TBL_PUB_DISC_ALGO, &stAlgo, sizeof(stAlgo));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		//	�ж��Ƿ�������Ŀ�����
		lGetEXLong(&stAlgo.card_type);
		if(FIELD_MATCH == lMask)
		{
			if(RC_SUCC != lActTypeCmp(stAlgo.card_type, lActType))
				continue;
		}
		else
		{
			if(lActType != stAlgo.card_type)
				continue;
		}

		if(0 == lCount)
			pszOut = (char *)malloc(pstRun->lSize);
 		else
			pszOut = (char *)realloc(pszOut, pstRun->lSize * (lCount + 1));
		if(NULL == pszOut)
		{
			vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
			vSetErrMsg("�����С(%d)�ڴ�ʧ��!", pstRun->lSize * (lCount + 1));
			return RC_FAIL;
		}
		memcpy((char *)pszOut + lCount * pstRun->lSize, &stAlgo, sizeof(dbDiscAlgo));
		lGetEXLong(&((dbDiscAlgo *)pszOut + lCount)->id);
		lGetEXLong(&((dbDiscAlgo *)pszOut + lCount)->index_num);
		lGetEXLong(&((dbDiscAlgo *)pszOut + lCount)->flag);
		lCount ++;
	}
	vTableClose(TBL_PUB_DISC_ALGO, SHM_DISCONNECT);
	*ppstAlgo = pszOut;
	*plOut = lCount;

	if(0 == lCount)
	{
		vSetErrMsg("δ�ҵ�����(%d)��Ӧ�ķ���ģ��(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	����ģ�����ͱ�����ת��	
 ****************************************************************************************/
void	vEXchangeAlgo(dbDisc *pstDisc, dbDiscAlgo *pstAlgo)
{
	pstAlgo->id = pstDisc->id;
	memcpy(pstAlgo->disc_id, pstDisc->disc_id, sizeof(pstAlgo->disc_id) - 1);
	pstAlgo->index_num = pstDisc->index_num;   
	pstAlgo->min_fee = pstDisc->min_fee;	 
	pstAlgo->max_fee = pstDisc->max_fee;	 
	pstAlgo->floor_amount = pstDisc->floor_amount;
	pstAlgo->upper_amount = pstDisc->upper_amount;
	pstAlgo->flag = pstDisc->flag;		
	pstAlgo->fee_value = pstDisc->fee_value;   
	pstAlgo->card_type = pstDisc->card_type;   
}

/****************************************************************************************
 *	��ȡDISC_ID��������зֶεķ���	
 ****************************************************************************************/
long	lGetServiceDisc(char *pszDiscId, long lActType, long lMask, dbDisc **ppstAlgo, long *plOut) 
{
	long		lRet = 0, lCount = 0;
	dbDisc		stAlgo;
	void		*pszOut = NULL;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszDiscId);
/*	//	����ѣ���ʱ�����ǿ�����
	if(lActType <= 0)
	{
		vSetErrMsg("��Ч�Ŀ�����(%d)!", lActType);
		return RC_FAIL;	
	}
*/
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_DISC_ALGO)))
		return RC_FAIL;

	memset(&stAlgo, 0, sizeof(stAlgo));
	strcpy(stAlgo.disc_id, pszDiscId);
	sTrimAll(stAlgo.disc_id);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_DISC_ALGO;
	pstRun->lSize = sizeof(stAlgo);
	pstRun->pstVoid = &stAlgo;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	�������м�¼
	while(1)
	{
		memset(&stAlgo, 0, sizeof(stAlgo));
		lRet = lTableFetch(TBL_DISC_ALGO, &stAlgo, sizeof(stAlgo));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		//	�ж��Ƿ�������Ŀ�����
		lGetEXLong(&stAlgo.card_type);
		if(0 == lCount)
			pszOut = (char *)malloc(pstRun->lSize);
 		else
			pszOut = (char *)realloc(pszOut, pstRun->lSize * (lCount + 1));
		if(NULL == pszOut)
		{
			vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
			vSetErrMsg("�����С(%d)�ڴ�ʧ��!", pstRun->lSize * (lCount + 1));
			return RC_FAIL;
		}
		memcpy((char *)pszOut + lCount * pstRun->lSize, &stAlgo, sizeof(dbDisc));
		lGetEXLong(&((dbDisc *)pszOut + lCount)->id);
		lGetEXLong(&((dbDisc *)pszOut + lCount)->index_num);
		lGetEXLong(&((dbDisc *)pszOut + lCount)->flag);
		lCount ++;
	}
	vTableClose(TBL_DISC_ALGO, SHM_DISCONNECT);
	*ppstAlgo = pszOut;
	*plOut = lCount;

	if(0 == lCount)
	{
		vSetErrMsg("δ�ҵ�����(%d)��Ӧ�ķ���ģ��(%s)", lActType, pszDiscId);
		return RC_NOTFOUND;
	}

	return RC_SUCC;
}

/****************************************************************************************
 *	�����ڴ����ָ�������ֶεļ�ֵ FIELD_MAX  FIELD_MIN
 ****************************************************************************************/
long	lGetVersCfg(dbSvceCfg *pstCfg, char *pszSevceCode, long lMask)
{
	long		lRet = 0, lInit = 0, i = 0;
	void		*p = NULL;
	dbSvceCfg	stSvceCfg;
	RunTime		*pstRun = NULL;

	IBPStrIsNull(pszSevceCode);
	if(!(pstRun = (RunTime *)pInitRunTime(TBL_SERVICE_CONFIG)))
		return RC_FAIL;

	memset(&stSvceCfg, 0, sizeof(stSvceCfg));
	strcpy(stSvceCfg.service_code, pszSevceCode);
	sTrimAll(stSvceCfg.service_code);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_SERVICE_CONFIG;
	pstRun->lSize = sizeof(stSvceCfg);
	pstRun->pstVoid = &stSvceCfg;

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return RC_FAIL;

	//	�������м�¼
	while(1)
	{
		memset(&stSvceCfg, 0, sizeof(stSvceCfg));
		lRet = lTableFetch(TBL_SERVICE_CONFIG, &stSvceCfg, sizeof(stSvceCfg));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		if(0 == lInit)
		{
			lInit = 1;
			memcpy(pstCfg, &stSvceCfg, sizeof(stSvceCfg));
		}

		lGetEXLong(&stSvceCfg.prob_days);
		if(FIELD_MAX == lMask)
		{
			if(0 < memcmp(stSvceCfg.service_version, pstCfg->service_version, 
				strlen(stSvceCfg.service_version)))
				memcpy(pstCfg, &stSvceCfg, sizeof(stSvceCfg));
		}
		else if(FIELD_MIN == lMask)
		{
			if(0 > memcmp(stSvceCfg.service_version, pstCfg->service_version, 
				strlen(stSvceCfg.service_version)))
				memcpy(pstCfg, &stSvceCfg, sizeof(stSvceCfg));
		}
		else
		{
			vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);
			vSetErrMsg("δʶ���ƥ�䶨��(%d)", lMask);
			return RC_FAIL;
		}
	}
	vTableClose(TBL_SERVICE_CONFIG, SHM_DISCONNECT);

	if(0 == lInit)
		return RC_NOTFOUND;
	return RC_SUCC;
}

/****************************************************************************************
 * 	group by cups_no����	
 ****************************************************************************************/
long	lGroupCupsNo(CMList **pstRoot, long lMode)
{
	long			lRet = 0, lLen = 0;
	dbMchtCupsInf   stCups;
	CMList			*pstNode = NULL;
	RunTime		 *pstRun = NULL;
	long			lCount = 0;

	if(!(pstRun = (RunTime *)pInitRunTime(TBL_MCHT_CUPS_INF)))
		return RC_FAIL;

	lLen = sizeof(stCups.cups_no);
	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = TBL_MCHT_CUPS_INF;
	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
		return lRet;

	while(1)
	{
		memset(&stCups, 0, sizeof(stCups));
		lRet = lTableFetch(TBL_MCHT_CUPS_INF, &stCups, sizeof(stCups));
		if(RC_FAIL == lRet)
		{
			vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
			return RC_FAIL;
		}
		else if(RC_NOTFOUND == lRet)
			break;

		sTrimAll(stCups.cups_no);
		++lCount;

		//	�Ѵ��ڣ�����
		if(pSearchNode(pstNode, stCups.cups_no, lLen))
			continue;

		pstNode = pInsertList(pstNode, stCups.cups_no, lLen);
		if(!pstNode)
		{
			vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
			return RC_FAIL;
		}
	}
	vTableClose(TBL_MCHT_CUPS_INF, SHM_DISCONNECT);
	*pstRoot = pstNode;

	return RC_SUCC;
}

/****************************************************************************************
 *	code end
 ****************************************************************************************/
