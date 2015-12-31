#include	"load.h"

#define ZCHAR(x)	(x == 0x00)?(x = ' '):x

long	vTest()
{
	long			i, lRet = 0, nRecord = 0, lCount = 0;
	BOOL			bRet = false;
	char			szTemp[100], szAfx[100], szTerm[100];
	TABLE			*pTable = NULL;
	dbMchtBase		stBase;
	dbBrhInfo		stBrh;
	dbMchtUser		stUser;
	dbMchtMccInf	stMcc;
	dbProCfg		stPro;
	dbSpeMcht		stSpec;
	dbMchtAlgo		stMtAlog;
	dbPfitInfo		stPfitIf;
	dbPfitDef		stDbPfitDef;
	dbAcctInfo		stAct;
	dbMchtCupsInf   stCups;
	dbNMchtMatch	stMatch;
	dbSvceCfg		stCfg;
	BSysIdx			stSys, stSIdx;
	dbBrhAlgo		*pstBAlgo = NULL;
	dbDiscAlgo		*pstAlgo = NULL;
	dbDisc			*pstDisc = NULL;
	CMList			*pstRoot = NULL, *pstNode = NULL;
	RunTime			*pstRun = NULL;	

	memset(&stBase, 0, sizeof(stBase));
	memset(&stBrh, 0, sizeof(stBrh));
	memset(&stUser, 0, sizeof(stUser));
	memset(&stPro, 0, sizeof(stPro));
	memset(&stSpec, 0, sizeof(stSpec));
	memset(&stMcc, 0, sizeof(stMcc));
	memset(&stMtAlog, 0, sizeof(stMtAlog));
	memset(&stPfitIf, 0, sizeof(stPfitIf));
	memset(&stDbPfitDef, 0, sizeof(stDbPfitDef));
	memset(&stAct, 0, sizeof(stAct));
	memset(&stCups, 0, sizeof(stCups));
	memset(&stMatch, 0, sizeof(stMatch));

	memset(szTemp, 0, sizeof(szTemp));
	memset(szAfx, 0, sizeof(szAfx));
	memset(szTerm, 0, sizeof(szTerm));
	strcpy(szTemp, "017001001001001001");
	strcpy(szAfx, "310442350130002");
	strcpy(szTerm, "122010000158");

	fprintf(stdout, "sizeof(%d)\n", sizeof(dbMchtBase));
	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "008422858140002");

		if(RC_SUCC != lGetMchtBase(&stBase, szTemp))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%9d|%15s|%40s|%4s|%6s|%15s|%6s|%40s|%5s|%18s|%10d|%18s|%8s|%8s|%8s|%8d|\n",
			nRecord, stBase.id, stBase.mcht_no, stBase.mcht_nm, stBase.mcht_status, stBase.mcht_kind,
			stBase.mcht_up_no, stBase.mcht_area_no, stBase.mcht_addr, stBase.mcht_mcc, stBase.mcht_brh,
			stBase.mcht_expand, stBase.mcht_stlm_brh, stBase.mcht_disc_id0, stBase.mcht_disc_id,
			stBase.mcht_opr_flag, stBase.mcht_resv1);

		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "008107");
		if(RC_SUCC != lGetBrhInfo(&stBrh, szTemp))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%10d|%10d|%20s|%-70s|%14s|\t%-9s|\t\t%c|\t%5c|\n", nRecord, 
			stBrh.brh_id, stBrh.brh_level, stBrh.brh_code, stBrh.brh_name, 
			stBrh.brh_stlm_flag, stBrh.brh_disc_id, ZCHAR(stBrh.brh_tag[0]), 
			ZCHAR(stBrh.brh_file_flag[0]));
		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		if(RC_SUCC != lGetMchtUser(&stUser, 11000882))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%10d|%15s|%50s|%30s|%11s|%30s|%18s|\n", nRecord, stUser.user_id, 
			stUser.mcht_no, stUser.user_name, stUser.user_engname, stUser.user_phone, stUser.user_email, 
			stUser.user_card_no);
		usleep(50000);
	}
/*
	for(i = 0; i < 20; i ++)
	{
		if(RC_SUCC != lGetPorConfig(&stPro, szTemp))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%10d|%15s|%50s|%30s|%11s|%30s|%18s|\n", nRecord, stUser.user_id, 
			stUser.mcht_no, stUser.user_name, stUser.user_engname, stUser.user_phone, stUser.user_email, 
			stUser.user_card_no);
		usleep(50000);
	}
*/

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		memset(szAfx, 0, sizeof(szAfx));
		strcpy(szTemp, "888888888888881");
		strcpy(szAfx, "real");
		if(RC_SUCC != lGetSpeMcht(&stSpec, szTemp, szAfx))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:%8d|%20s|%10s|%4s|%8s|%8s|\n", nRecord, stSpec.id, stSpec.mcht_no,
			stSpec.service_code, stSpec.flag, stSpec.pro_rate, stSpec.pro_fix);
		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "034001");
		if(RC_SUCC != lIsMchtSettle(szTemp, (long *)&lRet))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:机构(%s)是否为机构记录(%d)\n", nRecord, szTemp, lRet);
		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "018001");
		if(RC_SUCC != lIsBrhSettle(szTemp, &bRet))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:机构(%s)是否清算机构(%d)\n", nRecord, szTemp, bRet);
		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "9211");
		if(RC_SUCC != lGetMccInfo(&stMcc, szTemp))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%5d|\t%5s|\t%5s|\n", nRecord, stMcc.id, stMcc.mchnt_tp,
			stMcc.mchnt_tp_grp);
		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "MPN20003");
		if(RC_SUCC != lGetMchtAlgo(&stMtAlog, szTemp))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%8s|%8s|%8s|%12s|%4s|%8s|%6s|%6d|\n", nRecord,
			stMtAlog.model_id, stMtAlog.model_type, stMtAlog.cycle_type, stMtAlog.stlm_flag,
			stMtAlog.model_flag, stMtAlog.trans_type, stMtAlog.mcht_grp, stMtAlog.card_type);
		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "J0000001");
		if(RC_SUCC != lGetBrhPfitInf(&stPfitIf, szTemp))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%10s|\t%10s|\t%10s|\t%10s|\n", nRecord, stPfitIf.model_id,
			stPfitIf.treaty_id, stPfitIf.standard_id, stPfitIf.other_id);
		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "PA200002");
		if(RC_SUCC != lGetBrhPfitDef(&stDbPfitDef, szTemp))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%-9s|\t%18s|\t%4s|\t%12s|\t%9s|\n", nRecord, stDbPfitDef.model_id,
			stDbPfitDef.model_brh, stDbPfitDef.model_flag, stDbPfitDef.unite_ratio, stDbPfitDef.profit_ratio);

		usleep(50000);
	}

	for(i = 0; i < 20; i ++)
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, "045610151390001");
		if(RC_SUCC != lGetAcctInfo(&stAct, szTemp, 2, 1))
		{
			fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
			return RC_FAIL;
		}

		fprintf(stdout, "[%d]:\t%10d|%4d|%15s|%4s|%4d|%25s|%30s|%12s|%20s|%12s|%48s|%21s|%8s|%12s|%12s|\n",
			nRecord, stAct.acct_id, stAct.app_type, stAct.app_id, stAct.acct_type, stAct.acct_default,
			stAct.acct_no, stAct.acct_nm, stAct.acct_bank_no, stAct.acct_bank_nm, stAct.acct_zbank_no,
			stAct.acct_zbank_nm, stAct.acct_zbank_addr, stAct.acct_zbank_code, stAct.acct_net_no,
			stAct.acct_union_no);

		usleep(10000);
	}

//	if(RC_SUCC != lGetDefRoute(&stCups, szTemp, szAfx))
//	if(RC_SUCC != lGetMchtRoute(&stCups, szTemp, szAfx, szTerm))
//	if(RC_SUCC != lGetSpdbRoute(&stMatch, szTemp, szAfx))
//	if(RC_SUCC != lGetVersCfg(&stCfg, szTemp, FIELD_MIN))
//	if(RC_SUCC != lGetPfitAlgo(szTemp, &pstBAlgo, (long *)&lCount))
//	if(RC_SUCC != lGetDiscAlgo(szTemp, 15, FIELD_MATCH, &pstAlgo, (long *)&lCount))
//
	memset(szTemp, 0, sizeof(szTemp));
	strcpy(szTemp, "serv6003");
	if(RC_SUCC != lGetServiceDisc(szTemp, 15, FIELD_MATCH, &pstDisc, (long *)&lCount))
	{
		fprintf(stdout, "查询商户表失败, err:%s\n", sGetError());
		return RC_FAIL;
	}

	for(i = 0; i < lCount; i ++)
	{
		fprintf(stdout, "[%d]:\t%10d|%8s|%4d|%12.2f|%13.2f|%15.2f|%15.2f|%8d|%12.2f|%6d|\n",
			i + 1, pstDisc[i].id, pstDisc[i].disc_id, pstDisc[i].index_num, pstDisc[i].min_fee, pstDisc[i].max_fee,
			pstDisc[i].floor_amount, pstDisc[i].upper_amount, pstDisc[i].flag, pstDisc[i].fee_value, pstDisc[i].card_type);
	}

	IBPFree(pstDisc);

	if(RC_SUCC != lGetLoadList(&pTable, (long *)&lCount))
	{
		fprintf(stdout, "查询装载表列表失败, err:%s\n", sGetError());
		return RC_FAIL;
	}

	for(i = 0; i < lCount; i ++)
		fprintf(stdout, "[%d]\t%4d\n", i + 1, pTable[i]);

	IBPFree(pTable);
/*
	memset(szTemp, 0, sizeof(szTemp));
	strcpy(szTemp, "MHN10242");
	if(RC_SUCC != lGetDiscAlgo(szTemp, 2, FIELD_MATCH, &pstAlgo, (long *)&lCount))
	{
		fprintf(stdout, "查询商户表失败, err:%s\n", sGetError());
		return RC_FAIL;
	}

	for(i = 0; i < lCount; i ++)
	{
		fprintf(stdout, "[%d]:\t%10d|%8s|%4d|%12.2f|%13.2f|%15.2f|%15.2f|%8d|%12.2f|%6d|\n",
			i + 1, pstAlgo[i].id, pstAlgo[i].disc_id, pstAlgo[i].index_num, pstAlgo[i].min_fee, pstAlgo[i].max_fee,
			pstAlgo[i].floor_amount, pstAlgo[i].upper_amount, pstAlgo[i].flag, pstAlgo[i].fee_value, pstAlgo[i].card_type);
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	lRet = lGetAlgoModel(pstAlgo, lCount, 4980.00, &stAlgo);
	if(lRet)
	{
		IBPFree(pstAlgo);
		fprintf(stdout, "从费率表中查询(%s)金额(%.2f)段费率出错, err:(%d)(%s)",
			szTemp, 4980.00, lRet, sGetError());
		return RC_FAIL;
	}

	fprintf(stdout, "[选中]:\t%10d|%8s|%4d|%12.2f|%13.2f|%15.2f|%15.2f|%8d|%12.2f|%6d|\n",
		stAlgo.id, stAlgo.disc_id, stAlgo.index_num, stAlgo.min_fee, stAlgo.max_fee,
		stAlgo.floor_amount, stAlgo.upper_amount, stAlgo.flag, stAlgo.fee_value, stAlgo.card_type);

	IBPFree(pstAlgo);
*/
	return RC_SUCC;
}

/****************************************************************************************
	main
 ****************************************************************************************/
int		main(int argc, char** argv)
{
	long		i, lRet = 0, nRecord= 0, lCount = 0;
	BOOL		bRet = false;
	char		szTemp[100], szAfx[100], szTerm[100];
	BFieldDef   stField, *pstField = NULL;
	RunTime	 *pstRun = NULL;
	dbAcctInfo  stAct;

/*
	if(!(pstRun = (RunTime *)pInitRunTime(SYS_FILED_DEF)))
	{
		fprintf(stderr, "初始化表(%s)内存块失败, err:(%s)\n", sGetTableName(SYS_FILED_DEF),
			 sGetError());
		return ;
	}


	memset(&stField, 0, sizeof(stField));
	stField.m_table = lSetEXLong(TBL_MCHT_BASE);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = SYS_FILED_DEF;
	pstRun->lSize = sizeof(stField);
	pstRun->pstVoid = &stField;

	if(RC_SUCC != lRunSelect(pstRun, (void **)&pstField, &lCount))
	{
		fprintf(stdout, "查询错误, err:(%s)\n", sGetError());
		return RC_FAIL;
	}

	for(i = 0; i < lCount; i ++)
	{
		lGetEXLong((long *)&pstField[i].m_table);
		lGetEXLong(&pstField[i].m_lKeyAttr);
		lGetEXLong(&pstField[i].m_lKeyOffSet);
		lGetEXLong(&pstField[i].m_lKeyLen);
		lGetEXLong(&pstField[i].m_lIsPk);
		lGetEXLong(&pstField[i].m_lSeq);

		fprintf(stdout, "[%d]:\t%4d|%12s|%-25s|%8d|%20s|%8d|%8d|%8d|%8d|\n", nRecord, pstField[i].m_table,
			pstField[i].m_szOwner, pstField[i].m_szTable, pstField[i].m_lSeq, pstField[i].m_szField, pstField[i].m_lKeyAttr,
			pstField[i].m_lKeyOffSet, pstField[i].m_lKeyLen, pstField[i].m_lIsPk);
	}
	IBPFree(pstField);
*/
	memset(&stAct, 0, sizeof(stAct));
	memset(szTemp, 0, sizeof(szTemp));
	strcpy(szTemp, "008230154510599");
	if(RC_SUCC != lGetAcctInfo(&stAct, szTemp, ACCT_INFO_MCHT, ACCT_INFO_DEFAULT))
	{
		fprintf(stderr, "err:(%s)", sGetError());
		return RC_FAIL;
   	} 

	fprintf(stdout, "line\t账号序号 应用类型  应用标示号   类型 是否默认\t账户账号\t"
		"\t\t账户名称\t   账户开户行号\t	账户开户行名称  账户开户支行号\t   账户开户支行名称"
		"\t\t\t\t开户支行地址\t支行地区号 网银支付号 账户联行号\n");
	fprintf(stdout, "-------------------------------------------------------------------"
		"-------------------------------------------------------------------------------"
		"-----------------------------------------------------------------------------"
		"-------------------------------------\n");

	fprintf(stdout, "[%d]:\t%10d|%4d|%15s|%4s|%4d|%25s|%30s|%12s|%20s|%12s|%48s|%21s|%8s|%12s|%12s|\n",
		0, stAct.acct_id, stAct.app_type, stAct.app_id, stAct.acct_type, stAct.acct_default,
		stAct.acct_no, stAct.acct_nm, stAct.acct_bank_no, stAct.acct_bank_nm, stAct.acct_zbank_no,
		stAct.acct_zbank_nm, stAct.acct_zbank_addr, stAct.acct_zbank_code, stAct.acct_net_no,
		stAct.acct_union_no);

	fprintf(stdout, "(%s)(%s)(%s)(%s)(%s)\n", stAct.acct_no, stAct.acct_nm, stAct.acct_bank_no,
		stAct.acct_zbank_no, stAct.acct_union_no);

	return RC_SUCC;
}
