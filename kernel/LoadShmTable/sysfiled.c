#include	"Ibp.h"
#include	"DbsApi.h"
#include	"pos_stlm.h"
#include	"load.h"
#include	"table.h"

#define	CREATE_BEGIN(t)				vCreate_Table(TRUE, t);
#define	CREATE_END					vCreate_Table(FALSE, 0);
#define	FIELD(t, f, fd, a)			vCreate_Field(FiledPos(t, f), FiledLen(t, f), fd, a, TRUE);

BOOL	g_Benum = false;
TABLE   g_table = 0;
CMList  *g_pstRoot = NULL;

/****************************************************************************************
	获取表SYS_FILED_DEF启动内存大小
 ****************************************************************************************/
long	lDefine_SysFieldDef(TABLE t, char *pszSettleDate, char *pszSettleNum)
{
	char	szTable[100];
	long	lSize = 0, lCount = 0;

	memset(szTable, 0, sizeof(szTable));
	strcpy(szTable, sGetTableName(t));

	lSize = sizeof(TblDef);

	//	默认最大表数 * 每张表 30个字段
	lCount = SHM_TABLE_NUM * 30;
	if(RC_SUCC != lInitTblSize(t, lCount, sizeof(BFieldDef), &lSize))
		return RC_FAIL;

	fprintf(stdout, "开始建立表:%s\n", szTable);
	HtLog(NML_PARAM, "初始化表(%s)索引(m_table)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_table, FIELD_LONG))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_table)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "初始化表(%s)索引(m_szOwner)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_szOwner, FIELD_CHAR))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_szOwner)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "初始化表(%s)索引(m_szTable)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_szTable, FIELD_CHAR))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_szTable)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "初始化表(%s)索引(m_szField)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_szField, FIELD_CHAR))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_szField)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "初始化表(%s)索引(m_lKeyAttr)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_lKeyAttr, FIELD_LONG))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_lKeyAttr)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "初始化表(%s)索引(m_lKeyOffSet)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_lKeyOffSet, FIELD_LONG))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_lKeyOffSet)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "初始化表(%s)索引(m_lKeyLen)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_lKeyLen, FIELD_LONG))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_lKeyLen)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "初始化表(%s)索引(m_lIsPk)", szTable);
	if(RC_SUCC != IBPSetIdx(t, BFieldDef, m_lIsPk, FIELD_LONG))
	{
		HtLog(ERR_PARAM, "设置表(%s)字段(m_lIsPk)失败, err:(%s)", szTable, sGetError());
		return RC_FAIL;
	}

	return lSize;
}

/****************************************************************************************
	新增字段数据放入系统字段表 
 ****************************************************************************************/
long	lInsertFiled()
{
	long		lRec = 0;
	BFieldDef   stField;
	CMList	  *pstList = NULL;

	//  初始化字段系统表
	pstList = g_pstRoot;
	while(pstList)
	{
//		memset(&stField, 0, sizeof(stField));
		memcpy(&stField, pstList->pstData, sizeof(stField));
		stField.m_table		= lSetEXLong(stField.m_table);
		stField.m_lSeq		= lSetEXLong(++ lRec);
		stField.m_lKeyAttr  = lSetEXLong(stField.m_lKeyAttr);
		stField.m_lKeyOffSet= lSetEXLong(stField.m_lKeyOffSet);
		stField.m_lKeyLen   = lSetEXLong(stField.m_lKeyLen);
		stField.m_lIsPk		= lSetEXLong(stField.m_lIsPk);

		if(RC_SUCC != IBPInsertData(SYS_FILED_DEF, &stField, sizeof(stField)))
		{
			HtLog(ERR_PARAM, "新增表(%s)数据(%s)失败, 原因:%s", stField.m_szTable,
				stField.m_szField, sGetError());
			return RC_FAIL;
		}

		pstList = pstList->pstNext;
	}

	fprintf(stdout, "(SYS_FILED_DEF)装载表(%s)空间总记录(%d), 有效记录(%d)\n", 
		sGetTableName(g_table), lGetTblCount(SYS_FILED_DEF), pGetTblDef(SYS_FILED_DEF)->lValid);

	return RC_SUCC;
}

/****************************************************************************************
	创建表预
 ****************************************************************************************/
void	vCreate_Table(BOOL bf, TABLE t)
{
	long	lRet = 0;

	g_Benum = bf;
	if(TRUE == g_Benum)
	{
		g_table = t;
		return ;
	}
	else	//  END
		;

	//  插入表
	if(RC_SUCC != lInsertFiled())
	{
		vDestroyList(g_pstRoot);
		g_pstRoot = NULL;
		return ;
	}
	vDestroyList(g_pstRoot);
	g_pstRoot = NULL;

	return ;
}

/****************************************************************************************
	初始化字段属性
 ****************************************************************************************/
void	vCreate_Field(long lOfs, long lLen, char *f, long lAttr, long lPk)
{
	BFieldDef   stField;

	if(false == g_Benum)	return ;

	memset(&stField, 0, sizeof(stField));
	stField.m_table = g_table;
	snprintf(stField.m_szTable, sizeof(stField.m_szTable), "%s", sGetTableName(g_table));
	snprintf(stField.m_szOwner, sizeof(stField.m_szOwner), "%s", getenv("LOGNAME"));
	sTrimAll(stField.m_szTable);
	sTrimAll(stField.m_szOwner);

	strcpy(stField.m_szField, f);
	sUpper(stField.m_szField);
	
	stField.m_lKeyAttr = lAttr;
	stField.m_lKeyOffSet = lOfs;
	stField.m_lKeyLen = lLen;
	stField.m_lIsPk = lPk;

	g_pstRoot = pInsertList(g_pstRoot, &stField, sizeof(BFieldDef));
	if(!g_pstRoot)
	{
		vDestroyList(g_pstRoot);
		g_Benum = false;
		return ;
	}

	return ;
}


/****************************************************************************************
	加载客户表SYS_FILED_DEF
 ****************************************************************************************/
long	lLoad_SysFieldDef(TABLE t, Tree **pstNode, char *pszSettleDate, char *pszSettleNum)
{

	//	首先对表进行初始化
	IBPInitTable(t);

//	对客户表字段的建立
//	创建表:TBL_REGION_CODE
	CREATE_BEGIN(TBL_REGION_CODE)
	FIELD(dbReginCode, id,				"id",				FIELD_LONG)
	FIELD(dbReginCode, region_code,		"region_code",		FIELD_CHAR)
	FIELD(dbReginCode, region_name,		"region_name",		FIELD_CHAR)
	CREATE_END

//	创建表:TBL_BRH_INFO
	CREATE_BEGIN(TBL_BRH_INFO)
	FIELD(dbBrhInfo, brh_id,			"brh_id",			FIELD_LONG)
	FIELD(dbBrhInfo, brh_level,			"brh_level",		FIELD_LONG)
	FIELD(dbBrhInfo, brh_code,			"brh_code",			FIELD_CHAR)
	FIELD(dbBrhInfo, brh_name,			"brh_name",			FIELD_CHAR)
	FIELD(dbBrhInfo, brh_stlm_flag,		"brh_stlm_flag",	FIELD_CHAR)
	FIELD(dbBrhInfo, brh_disc_id,		"brh_disc_id",		FIELD_CHAR)
	FIELD(dbBrhInfo, brh_tag,			"brh_tag",			FIELD_CHAR)
	FIELD(dbBrhInfo, brh_file_flag,		"brh_file_flag",	FIELD_CHAR)
	CREATE_END

//	创建表:TBL_BRH_PROFIT_INFO
	CREATE_BEGIN(TBL_BRH_PROFIT_INFO)
	FIELD(dbPfitInfo, model_id,			"model_id",			FIELD_CHAR)
	FIELD(dbPfitInfo, treaty_id,		"treaty_id",		FIELD_CHAR)
	FIELD(dbPfitInfo, standard_id,		"standard_id",		FIELD_CHAR)
	FIELD(dbPfitInfo, other_id,			"other_id",			FIELD_CHAR)
	CREATE_END

//	创建表:TBL_MCHT_MCC_INF
	CREATE_BEGIN(TBL_MCHT_MCC_INF)
	FIELD(dbMchtMccInf, id,				"id",				FIELD_LONG)
	FIELD(dbMchtMccInf, mchnt_tp,		"mchnt_tp",			FIELD_CHAR)
	FIELD(dbMchtMccInf, mchnt_tp_grp,	"mchnt_tp_grp",		FIELD_CHAR)
	CREATE_END

//	创建表:TBL_BRH_PROFIT_DEF
	CREATE_BEGIN(TBL_BRH_PROFIT_DEF)
	FIELD(dbPfitDef, model_id,			"model_id",			FIELD_CHAR)
	FIELD(dbPfitDef, model_brh,			"model_brh",		FIELD_CHAR)
	FIELD(dbPfitDef, model_flag,		"model_flag",		FIELD_CHAR)
	FIELD(dbPfitDef, unite_ratio,		"unite_ratio",		FIELD_CHAR)
	FIELD(dbPfitDef, profit_ratio,		"profit_ratio",		FIELD_CHAR)
	CREATE_END

//	创建表:TBL_BRH_PROFIT_ALGO
	CREATE_BEGIN(TBL_BRH_PROFIT_ALGO)
	FIELD(dbBrhAlgo, model_id,			"model_id",			FIELD_CHAR)
	FIELD(dbBrhAlgo, trans_type,		"trans_type",		FIELD_CHAR)
	FIELD(dbBrhAlgo, mcht_grp,			"mcht_grp",			FIELD_CHAR)
	FIELD(dbBrhAlgo, card_type,			"card_type",		FIELD_LONG)	
	FIELD(dbBrhAlgo, base_fee,			"base_fee",			FIELD_CHAR)
	FIELD(dbBrhAlgo, profit_ratio,		"profit_ratio",		FIELD_CHAR)	
	CREATE_END

//	创建表:TBL_ACCT_INFO
	CREATE_BEGIN(TBL_ACCT_INFO)
	FIELD(dbAcctInfo, acct_id,			"acct_id",			FIELD_LONG)
	FIELD(dbAcctInfo, app_type,			"app_type",			FIELD_LONG)	
	FIELD(dbAcctInfo, app_id,			"app_id",			FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_type,		"acct_type",		FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_default,		"acct_default",		FIELD_LONG)			
	FIELD(dbAcctInfo, acct_no,			"acct_no",			FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_nm,			"acct_nm",			FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_bank_no,		"acct_bank_no",		FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_bank_nm,		"acct_bank_nm",		FIELD_CHAR)		
	FIELD(dbAcctInfo, acct_zbank_no,	"acct_zbank_no",	FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_zbank_nm,	"acct_zbank_nm",	FIELD_CHAR)		
	FIELD(dbAcctInfo, acct_zbank_addr,	"acct_zbank_addr",	FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_zbank_code,	"acct_zbank_code",	FIELD_CHAR)		
	FIELD(dbAcctInfo, acct_net_no,		"acct_net_no",		FIELD_CHAR)	
	FIELD(dbAcctInfo, acct_union_no,	"acct_union_no",	FIELD_CHAR)		
	CREATE_END

//	创建表:TBL_OPR_INFO
	CREATE_BEGIN(TBL_OPR_INFO)
	FIELD(dbOprInf, opr_id,				"opr_id",			FIELD_LONG)
	FIELD(dbOprInf, opr_code,			"opr_code",			FIELD_CHAR)	
	FIELD(dbOprInf, opr_name,			"opr_name",			FIELD_CHAR)		
	CREATE_END

//	创建表:TBL_SPDB_BRH
	CREATE_BEGIN(TBL_SPDB_BRH)
	FIELD(dbSpdbBrh, id,				"id",				FIELD_LONG)
	FIELD(dbSpdbBrh, region_name,		"region_name",		FIELD_CHAR)	
	FIELD(dbSpdbBrh, brh_area_no,		"brh_area_no",		FIELD_CHAR)		
	CREATE_END

//	创建表:TBL_MCHT_BASE
	CREATE_BEGIN(TBL_MCHT_BASE)
	FIELD(dbMchtBase, id,				"id",				FIELD_LONG)
	FIELD(dbMchtBase, mcht_no,			"mcht_no",			FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_nm,			"mcht_nm",			FIELD_CHAR)		
	FIELD(dbMchtBase, mcht_status,		"mcht_status",		FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_kind,		"mcht_kind",		FIELD_CHAR)			
	FIELD(dbMchtBase, mcht_type,		"mcht_type",		FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_up_no,		"mcht_up_no",		FIELD_CHAR)		
	FIELD(dbMchtBase, mcht_area_no,		"mcht_area_no",		FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_addr,		"mcht_addr",		FIELD_CHAR)		
	FIELD(dbMchtBase, mcht_grp,			"mcht_grp",			FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_mcc,			"mcht_mcc",			FIELD_CHAR)		
	FIELD(dbMchtBase, mcht_brh,			"mcht_brh",			FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_expand,		"mcht_expand",		FIELD_LONG)
	FIELD(dbMchtBase, mcht_stlm_brh,	"mcht_stlm_brh",	FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_disc_id0,	"mcht_disc_id0",	FIELD_CHAR)		
	FIELD(dbMchtBase, mcht_disc_id,		"mcht_disc_id",		FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_disc_this,	"mcht_disc_this",	FIELD_CHAR)		
	FIELD(dbMchtBase, mcht_opr_flag,	"mcht_opr_flag",	FIELD_CHAR)	
	FIELD(dbMchtBase, mcht_resv1,		"mcht_resv1",		FIELD_LONG)
	CREATE_END

//	创建表:TBL_MCHT_ALGO
	CREATE_BEGIN(TBL_MCHT_ALGO)
	FIELD(dbMchtAlgo, model_id,			"model_id",			FIELD_CHAR)	
	FIELD(dbMchtAlgo, model_type,		"model_type",		FIELD_CHAR)	
	FIELD(dbMchtAlgo, cycle_type,		"cycle_type",		FIELD_CHAR)	
	FIELD(dbMchtAlgo, stlm_flag,		"stlm_flag",		FIELD_CHAR)	
	FIELD(dbMchtAlgo, model_flag,		"model_flag",		FIELD_CHAR)	
	FIELD(dbMchtAlgo, trans_type,		"trans_type",		FIELD_CHAR)	
	FIELD(dbMchtAlgo, mcht_grp,			"mcht_grp",			FIELD_CHAR)	
	FIELD(dbMchtAlgo, card_type,		"card_type",		FIELD_LONG)	
	CREATE_END

//	创建表:TBL_PUB_DISC_ALGO
	CREATE_BEGIN(TBL_PUB_DISC_ALGO)
	FIELD(dbDiscAlgo, id,				"id",				FIELD_LONG)
	FIELD(dbDiscAlgo, disc_id,			"disc_id",			FIELD_CHAR)	
	FIELD(dbDiscAlgo, index_num,		"index_num",		FIELD_LONG)	
	FIELD(dbDiscAlgo, min_fee,			"min_fee",			FIELD_LONG)		
	FIELD(dbDiscAlgo, max_fee,			"max_fee",			FIELD_LONG)		
	FIELD(dbDiscAlgo, floor_amount,		"floor_amount",		FIELD_LONG)		
	FIELD(dbDiscAlgo, upper_amount,		"upper_amount",		FIELD_LONG)		
	FIELD(dbDiscAlgo, flag,				"flag",				FIELD_LONG)		
	FIELD(dbDiscAlgo, fee_value,		"fee_value",		FIELD_LONG)		
	FIELD(dbDiscAlgo, card_type,		"card_type",		FIELD_LONG)		
	CREATE_END

//	创建表:TBL_DISC_ALGO
	CREATE_BEGIN(TBL_DISC_ALGO)
	FIELD(dbDisc, id,					"id",				FIELD_LONG)
	FIELD(dbDisc, disc_id,				"disc_id",			FIELD_CHAR)	
	FIELD(dbDisc, index_num,			"index_num",		FIELD_LONG)	
	FIELD(dbDisc, min_fee,				"min_fee",			FIELD_LONG)		
	FIELD(dbDisc, max_fee,				"max_fee",			FIELD_LONG)		
	FIELD(dbDisc, floor_amount,			"floor_amount",		FIELD_LONG)		
	FIELD(dbDisc, upper_amount,			"upper_amount",		FIELD_LONG)		
	FIELD(dbDisc, flag,					"flag",				FIELD_LONG)		
	FIELD(dbDisc, fee_value,			"fee_value",		FIELD_LONG)		
	FIELD(dbDisc, card_type,			"card_type",		FIELD_LONG)		
	CREATE_END

//	创建表:TBL_MCHT_USER
	CREATE_BEGIN(TBL_MCHT_USER)
	FIELD(dbMchtUser, user_id,			"user_id",			FIELD_LONG)
	FIELD(dbMchtUser, mcht_no,			"mcht_no",			FIELD_CHAR)	
	FIELD(dbMchtUser, user_name,		"user_name",		FIELD_CHAR)		
	FIELD(dbMchtUser, user_engname,		"user_engname",		FIELD_CHAR)		
	FIELD(dbMchtUser, user_phone,		"user_phone",		FIELD_CHAR)		
	FIELD(dbMchtUser, user_email,		"user_email",		FIELD_CHAR)		
	FIELD(dbMchtUser, user_card_no,		"user_card_no",		FIELD_CHAR)		
	CREATE_END

//	创建表:TBL_MCHT_CUPS_INF
	CREATE_BEGIN(TBL_MCHT_CUPS_INF)
	FIELD(dbMchtCupsInf, id,			"id",				FIELD_LONG)	
	FIELD(dbMchtCupsInf, cups_no,		"cups_no",			FIELD_CHAR)	
	FIELD(dbMchtCupsInf, cups_nm,		"cups_nm",			FIELD_CHAR)		
	FIELD(dbMchtCupsInf, mcht_cups_no,	"mcht_cups_no",		FIELD_CHAR)		
	FIELD(dbMchtCupsInf, term_cups_no,	"term_cups_no",		FIELD_CHAR)		
	FIELD(dbMchtCupsInf, fee_rate,		"fee_rate",			FIELD_LONG)		
	FIELD(dbMchtCupsInf, fee_val,		"fee_val",			FIELD_LONG)		
	FIELD(dbMchtCupsInf, cup_stamp,		"cup_stamp",		FIELD_CHAR)	
	CREATE_END

//	创建表:TBL_N_MCHT_MATCH
	CREATE_BEGIN(TBL_N_MCHT_MATCH)
	FIELD(dbNMchtMatch, id,				"id",				FIELD_LONG)	
	FIELD(dbNMchtMatch, cups_no,		"cups_no",			FIELD_CHAR)	
	FIELD(dbNMchtMatch, mcht_stat,		"mcht_stat",		FIELD_CHAR)	
	FIELD(dbNMchtMatch, export_stat,	"export_stat",		FIELD_CHAR)	
	FIELD(dbNMchtMatch, local_mcht_no,	"local_mcht_no",	FIELD_CHAR)	
	FIELD(dbNMchtMatch, cups_mcht_sort_no,"cups_mcht_sort_no",FIELD_CHAR)	
	FIELD(dbNMchtMatch, cups_mcht_no,	"cups_mcht_no",		FIELD_CHAR)	
	FIELD(dbNMchtMatch, reason,			"reason",			FIELD_CHAR)	
	FIELD(dbNMchtMatch, resv_0,			"resv_0",			FIELD_LONG)	
	FIELD(dbNMchtMatch, resv_1,			"resv_1",			FIELD_CHAR)		
	FIELD(dbNMchtMatch, fee_rate,		"fee_rate",			FIELD_LONG)	
	FIELD(dbNMchtMatch, fee_val,		"fee_val",			FIELD_LONG)		
	FIELD(dbNMchtMatch, mcht_nm,		"mcht_nm",			FIELD_CHAR)	
	FIELD(dbNMchtMatch, export_date,	"export_date",		FIELD_CHAR)	
	FIELD(dbNMchtMatch, mcht_grp,		"mcht_grp",			FIELD_CHAR)	
	FIELD(dbNMchtMatch, mcht_mcc,		"mcht_mcc",			FIELD_CHAR)	
	FIELD(dbNMchtMatch, mcht_disc_id,	"mcht_disc_id",		FIELD_CHAR)	
	CREATE_END

//	创建表:TBL_SERVICE_CONFIG
	CREATE_BEGIN(TBL_SERVICE_CONFIG)
	FIELD(dbSvceCfg, id,				"id",				FIELD_LONG)	
	FIELD(dbSvceCfg, service_code,		"service_code",		FIELD_CHAR)	
	FIELD(dbSvceCfg, service_name,		"service_name",		FIELD_CHAR)
	FIELD(dbSvceCfg, service_version,	"service_version",	FIELD_CHAR)	
	FIELD(dbSvceCfg, fee_rate,			"fee_rate",			FIELD_CHAR)	
	FIELD(dbSvceCfg, prob_concess_rate,	"prob_concess_rate",FIELD_CHAR)	
	FIELD(dbSvceCfg, prob_days,			"prob_days",		FIELD_LONG)	
	FIELD(dbSvceCfg, plan_starttime,	"plan_starttime",	FIELD_CHAR)	
	FIELD(dbSvceCfg, plan_endtime,		"plan_endtime",		FIELD_CHAR)
	FIELD(dbSvceCfg, task_status,		"task_status",		FIELD_CHAR)	
	FIELD(dbSvceCfg, profit_rate,		"profit_rate",		FIELD_CHAR)	
	FIELD(dbSvceCfg, profit_brno,		"profit_brno",		FIELD_CHAR)		
	FIELD(dbSvceCfg, profile_flag,		"profile_flag",		FIELD_CHAR)		
	CREATE_END

//	创建表:TBL_PRO_CONFIG
	CREATE_BEGIN(TBL_PRO_CONFIG)
	FIELD(dbProCfg, id,					"id",				FIELD_CHAR)	
	FIELD(dbProCfg, tx_date,			"tx_date",			FIELD_CHAR)	
	FIELD(dbProCfg, service_code,		"service_code",		FIELD_CHAR)
	FIELD(dbProCfg, pay_day,			"pay_day",			FIELD_LONG)	
	FIELD(dbProCfg, tx_seq,				"tx_seq",			FIELD_LONG)	
	FIELD(dbProCfg, stlm_date,			"stlm_date",		FIELD_CHAR)	
	FIELD(dbProCfg, flag,				"flag",				FIELD_CHAR)	
	FIELD(dbProCfg, pro_rate,			"pro_rate",			FIELD_CHAR)	
	FIELD(dbProCfg, pro_fix,			"pro_fix",			FIELD_CHAR)
	CREATE_END

//	创建表:TBL_PRO_SPECIAL_MCHT
	CREATE_BEGIN(TBL_PRO_SPECIAL_MCHT)
	FIELD(dbSpeMcht, id,				"id",				FIELD_LONG)	
	FIELD(dbSpeMcht, mcht_no,			"mcht_no",			FIELD_CHAR)	
	FIELD(dbSpeMcht, service_code,		"service_code",		FIELD_CHAR)	
	FIELD(dbSpeMcht, flag,				"flag",				FIELD_CHAR)
	FIELD(dbSpeMcht, pro_rate,			"pro_rate",			FIELD_CHAR)	
	FIELD(dbSpeMcht, pro_fix,			"pro_fix",			FIELD_CHAR)	
	FIELD(dbSpeMcht, pro_rsv,			"pro_rsv",			FIELD_CHAR)	
	FIELD(dbSpeMcht, rsv1,				"rsv1",				FIELD_CHAR)	
	FIELD(dbSpeMcht, remark,			"remark",			FIELD_CHAR)	
	CREATE_END	

//	创建表:TBL_MCHT_CARD
	CREATE_BEGIN(TBL_MCHT_CARD)
	FIELD(dbMCard,	mcht_no,			"mcht_no",			FIELD_CHAR)	
	FIELD(dbMCard,	disc_id,			"disc_id",			FIELD_CHAR)	
	CREATE_END	

	return RC_SUCC;
}

/****************************************************************************************
	code end
 ****************************************************************************************/
