/*****************************************************************************/
/* PROGRAM NAME: StlmCupTxn.c												   */
/* DESCRIPTIONS: 对账														*/
/*****************************************************************************/
/*						MODIFICATION LOG								   */
/* DATE		PROGRAMMER	 DESCRIPTION									*/
/*****************************************************************************/
#include <glob.h>
#include <math.h>
#include <unistd.h>
#include "Ibp.h"
#include "pos_stlm.h"
#include "DbsDef.h"
#include "tbl_txn_succ_c.h"
#include "tbl_cup_succ_c.h"
#include "tbl_stlm_succ_c.h"
#include "tbl_stlm_err_c.h"
#include "tbl_channel_account_c.h"
#include "tbl_algo_dtl_c.h"
#include "load.h"

#define INIT_LENGTH		1

double	g_TXAmt = 0.00f;		// 统计渠道无本地有，在第二天明细中，统计渠道金额
double	g_CUPFee = 0.00f;
/************************************************************************************/
/*   声明  ： int nCreateChnnAct(char *pszDate)									 */
/*   功能  ： 生成渠道账务汇总信息												  */
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int nCreateChnnAct(char *pszDate, char *pszCupsNo)
{
	int		iRet = 0, iCount = 0;
	double	dDFee = g_TXAmt - g_CUPFee, dCupAmt = 0.00, dCupFee = 0.00;
	
	DbsBegin();
	HtLog(NML_PARAM, "对渠道(%s)新增渠道账务表信息!", pszCupsNo);
	iRet = sql_execute("delete from tbl_channel_account where stlm_date = '%s' and cups_no = '%s'", 
		pszDate, pszCupsNo);
	if(iRet)
	{
		HtLog(ERR_PARAM, "新增渠道(%s)账务信息失败, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}
	DbsCommit();		// 多渠道并发对账

	iCount = sql_count("tbl_stlm_succ", "stlm_date = '%s' and cups_no = '%s' and result_flag > '%c'", 
		pszDate, pszCupsNo, STLM_SUCC_FLG_OK);
	if(0 == iCount)	 return RC_SUCC;

	// 防止有 4 的状态
	iRet = sql_sum_double("tbl_cup_succ", "tx_amt", &dCupAmt, "stlm_date in (select to_char(to_date(trans_date,'yyyymmdd') + 1, 'yyyymmdd') from "
		"tbl_stlm_date where stlm_date = '%s') and cups_no = '%s' and result_flag in ('%c', '%c', '%c')", pszDate, pszCupsNo, TRADE_TXN_INIT, 
		TRADE_TXN_NOTMATCH, TRADE_TXN_SUCCESS);
	if(iRet)
	{
		HtLog(ERR_PARAM, "统计渠道(%s)账单交易总金额失败, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}

	iRet = sql_sum_double("tbl_cup_succ", "cup_fee", &dCupFee, "stlm_date in (select to_char(to_date(trans_date,'yyyymmdd') + 1, 'yyyymmdd') from "
		"tbl_stlm_date where stlm_date = '%s') and cups_no = '%s' and result_flag in ('%c', '%c', '%c')", pszDate, pszCupsNo, TRADE_TXN_INIT, 
		TRADE_TXN_NOTMATCH, TRADE_TXN_SUCCESS);
	if(iRet)
	{
		HtLog(ERR_PARAM, "统计渠道(%s)账单交易总手续费失败, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}
	
	HtLog(NML_PARAM, "渠道(%s)补充金额(%.2f)手续费(%.2f), 当日渠道对账文件交易金额(%.2f)手续费(%.2f)", 
		pszCupsNo, g_TXAmt, g_CUPFee, dCupAmt, dCupFee);
	iRet = sql_execute("insert into tbl_channel_account(id,stlm_date,cups_no,cups_nm,tx_amt,cup_fee,in_amt,real_amt,recv_flag,resv_amt1) "	
		"(select stlm_date||cups_no, stlm_date, cups_no, '%s', sum(tx_amt) + %.2f, sum(cup_fee) + %.2f, sum(tx_amt)- sum(cup_fee) + %.2f, "
 		"sum(tx_amt)- sum(cup_fee) + %.2f, '0', %.2f from tbl_stlm_succ where stlm_date = '%s' and cups_no = '%s' and "
		"result_flag = '%c' group by cups_no, stlm_date)", sGetCupsName(pszCupsNo), g_TXAmt, g_CUPFee, dDFee, dDFee, dCupAmt - dCupFee, 
		pszDate, pszCupsNo, STLM_SUCC_FLG_INIT);
	if(iRet)
	{
		HtLog(ERR_PARAM, "新增渠道(%s)账务信息失败, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}

	DbsCommit();

	return RC_SUCC;
}

/************************************************************************************/
/*   函数名：					   对账主函数									  */
/************************************************************************************/
/*   声明  ： int main(int argc  , char ** argv )								   */
/*   功能  ： 完成对账流程														  */
/*   参数  ： argc (参数个数 -- 输入)											   */
/*		 ： argv (参数列表 -- 输出)											   */
/*			argv[1] --  清算日期(可选)											*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int main(int argc, char **argv)
{
	int		iRet = 0;
	char	szDate[9], szCupsNo[5];

	g_TXAmt = 0.00f;
	g_CUPFee = 0.00f;
	memset(gsLogFile,0x00,sizeof(gsLogFile));
	strcpy(gsLogFile, "StlmCupTxn.log");

	memset(szDate, 0, sizeof(szDate));
	memset(szCupsNo, 0, sizeof(szCupsNo));
	if(5 != argc) 
	{
		HtLog(ERR_PARAM, "lose parameters[%d]:%s stl_date stl_num bat_id matchstr", argc, argv[0]);
		return RC_FAIL;
	}

	strcpy(szDate, argv[1]);
	memcpy(szCupsNo, argv[4], sizeof(szCupsNo) - 1);
	sprintf(gsLogFile, "StlmCupTxn_%s.log", szCupsNo);
	iRet = DbsConnect();
	if(iRet)
	{
		HtLog(ERR_PARAM, "dbsconnect err:(%d)(%s)", iRet, DbsError());
		return RC_FAIL;
	}

	HtLog(NML_PARAM, "Date[%s], CupsNo[%s]", szDate, szCupsNo);
	/*对账*/
	iRet = StlmCupTxn(szDate, szCupsNo, argv[2]);
	if(iRet)
	{
		HtLog(ERR_PARAM, "渠道(%s)对账失败, err:%d", szCupsNo, iRet);
		DbsDisconnect(); 
		return RC_FAIL;
	}

	/*生成渠道账务信息表*/
	iRet = nCreateChnnAct(szDate, szCupsNo);
	if(iRet)
	{
		HtLog(ERR_PARAM, "生成今日(%s)渠道账务信息表失败, err:%d", szDate, iRet);
		DbsDisconnect();
		return RC_FAIL;
	}

	DbsDisconnect();		
	HtLog(NML_PARAM, "渠道(%s)对账完毕!", szCupsNo);
	return RC_SUCC;
}	  

/************************************************************************************/
/*   声明  ： int nGetSettleDate(tbl_cup_succ_c pstCup, tbl_txn_succ_c pstTxn)	  */
/*   功能  ： 渠道多帐插入 对帐不平明细表和成功表								   */
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int nGetSettleDate(struct tbl_stlm_err_c *pstStlmErr, struct tbl_cup_succ_c *pstCup)
{
	int		iRet = 0; 
	struct tbl_txn_succ_c	stTxnSucc;
	struct tbl_stlm_succ_c	stStlmSucc;
	
	memcpy(pstStlmErr->tx_date, pstCup->tx_date, 8);
	memcpy(pstStlmErr->tx_time, pstCup->tx_time, 8);
	memcpy(pstStlmErr->cups_no, pstCup->cups_no, sizeof(pstStlmErr->cups_no)-1);
	memcpy(pstStlmErr->trace_no, pstCup->trace_no, sizeof(pstStlmErr->trace_no) - 1);
	memcpy(pstStlmErr->ac_no, pstCup->ac_no, sizeof(pstStlmErr->ac_no)-1);
	memcpy(pstStlmErr->ac_bank_no, pstCup->ac_bank_no, sizeof(pstStlmErr->ac_bank_no)-1);
	pstStlmErr->tx_amt = pstCup->tx_amt;
	pstStlmErr->cups_amt = pstCup->tx_amt;	//	渠道金额
	pstStlmErr->cup_fee = pstCup->cup_fee;
	memcpy(pstStlmErr->tx_name, pstCup->tx_code, sizeof(pstCup->tx_code)-1);
	memcpy(pstStlmErr->sys_ref_no, pstCup->sys_ref_no, sizeof(pstStlmErr->sys_ref_no)-1);
	strcpy(pstStlmErr->err_desc, "第三方有本地无");
	
	memset(&stTxnSucc, 0, sizeof(stTxnSucc));
	if(!memcmp(pstCup->cups_no, "alpy", 4) || !memcmp(pstCup->cups_no, "wcht", 4))
	{
		HtLog(NML_PARAM, "渠道多帐:渠道日期(%s)订单号(%s)(%s)", pstCup->tx_date, 
			pstCup->cup_key + 4, pstCup->cups_no);
		iRet = Tbl_txn_succ_Sel(&stTxnSucc, "(tx_sub_code = '31' or tx_sub_code = '51') and "
			"order_no = '%s' and tx_date = '%s' and cups_no = '%s'", pstCup->cup_key + 4, 
			pstCup->tx_date, pstCup->cups_no);
	}
	else
	{
		// 如果查不到 他们date + trace_no  +  我们的date + trace_no
		HtLog(NML_PARAM, "渠道多帐:渠道日期(%s)流水(%s)", pstCup->tx_date, pstCup->trace_no);
		iRet = Tbl_txn_succ_Sel(&stTxnSucc, "(fd13 = '%s' or tx_date = '%s') and fd11 = '%s'", 
			pstCup->tx_date + 4, pstCup->tx_date, pstCup->trace_no + 6);
	}
   	if(RC_SUCC != iRet)
   	{
		HtLog(ERR_PARAM, "查询本地交易流水失败, err:(%d)(%s)", iRet, DbsError());
		return RC_FAIL;
	}

	memcpy(pstStlmErr->id, stTxnSucc.id, sizeof(pstStlmErr->id) - 1);

	memset((char *)&stStlmSucc, 0, sizeof(stStlmSucc));
	pstStlmErr->disc_cycle = stTxnSucc.disc_cycle;   						// 清算周期
	pstStlmErr->expand = stTxnSucc.expand;
	memcpy(pstStlmErr->brno, stTxnSucc.brno, sizeof(pstStlmErr->brno)-1);	// 页面上分机构显示
	memcpy(pstStlmErr->ibox_no, stTxnSucc.ibox_no, sizeof(pstStlmErr->ibox_no)-1);	
	memcpy(pstStlmErr->order_no, stTxnSucc.order_no, sizeof(pstStlmErr->order_no)-1);	
	memcpy(pstStlmErr->tx_code, stTxnSucc.tx_code, sizeof(pstStlmErr->tx_code)-1);	
	memcpy(pstStlmErr->longitude, stTxnSucc.longitude, sizeof(pstStlmErr->longitude)-1);
	memcpy(pstStlmErr->latitude, stTxnSucc.latitude, sizeof(pstStlmErr->latitude)-1);
	memcpy(pstStlmErr->ibox41, stTxnSucc.ibox41, sizeof(pstStlmErr->ibox41)-1);
	memcpy(pstStlmErr->ibox42, stTxnSucc.ibox42, sizeof(pstStlmErr->ibox42)-1);
	memcpy(pstStlmErr->ibox43, stTxnSucc.ibox43, sizeof(pstStlmErr->ibox43)-1);
	memcpy(pstStlmErr->fd41, stTxnSucc.fd41, sizeof(pstStlmErr->fd41)-1);
	memcpy(pstStlmErr->fd42, stTxnSucc.fd42, sizeof(pstStlmErr->fd42)-1);
	memcpy(pstStlmErr->fd43, stTxnSucc.fd43, sizeof(pstStlmErr->fd43)-1);
	memcpy(pstStlmErr->ac_no, stTxnSucc.ac_no, sizeof(pstStlmErr->ac_no)-1);
	pstStlmErr->fdx = stTxnSucc.fdx;
	pstStlmErr->local_amt = stTxnSucc.tx_amt;		//	本地金额(页面展示用)
	memcpy(pstStlmErr->tx_sub_code, stTxnSucc.tx_sub_code, sizeof(pstStlmErr->tx_sub_code)-1);
	memcpy(pstStlmErr->fdxx, stTxnSucc.fdxx, sizeof(pstStlmErr->fdxx)-1);
	memcpy(pstStlmErr->fdxxx, stTxnSucc.fdxxx, sizeof(pstStlmErr->fdxxx)-1);

	stStlmSucc.result_flag[0] = STLM_SUCC_FLG_ERR;		//	有差错
	snprintf(stStlmSucc.id, sizeof(stStlmSucc.id), "%s%s", stTxnSucc.tx_date, stTxnSucc.trace_no);
	memcpy(stStlmSucc.stlm_date, pstStlmErr->stlm_date, 8);
	memcpy(stStlmSucc.tx_date, stTxnSucc.tx_date, sizeof(stStlmSucc.tx_date)-1);
	memcpy(stStlmSucc.tx_time, stTxnSucc.tx_time, sizeof(stStlmSucc.tx_time)-1);
	memcpy(stStlmSucc.trace_no, stTxnSucc.trace_no, sizeof(stStlmSucc.trace_no)-1);
	memcpy(stStlmSucc.ibox_no, stTxnSucc.ibox_no, sizeof(stStlmSucc.ibox_no)-1);
	memcpy(stStlmSucc.order_no, stTxnSucc.order_no, sizeof(stStlmSucc.order_no)-1);
	memcpy(stStlmSucc.cups_no, stTxnSucc.cups_no, sizeof(stStlmSucc.cups_no)-1);
	memcpy(stStlmSucc.ac_no, stTxnSucc.ac_no, sizeof(stStlmSucc.ac_no)-1);
	stStlmSucc.user_id = stTxnSucc.user_id;
	stStlmSucc.disc_cycle = stTxnSucc.disc_cycle;
	stStlmSucc.expand = stTxnSucc.expand;
	stStlmSucc.ac_type = stTxnSucc.ac_type;
	stStlmSucc.tx_amt = stTxnSucc.tx_amt; 
	stStlmSucc.fee_amt = stTxnSucc.fee_amt;
	stStlmSucc.cup_fee = pstCup->cup_fee;		//	cups返回的 手续费

//	memcpy(stStlmSucc.ac_bank_no, stTxnSucc.ac_bank_no, sizeof(stStlmSucc.ac_bank_no)-1);
	memcpy(stStlmSucc.tx_code, stTxnSucc.tx_code, sizeof(stStlmSucc.tx_code)-1);
//	memcpy(stStlmSucc.tx_name, stTxnSucc.tx_name, sizeof(stStlmSucc.tx_name)-1);
	strcpy(stStlmSucc.tx_name, "POS消费");
	memcpy(stStlmSucc.tx_sub_code, stTxnSucc.tx_sub_code, sizeof(stStlmSucc.tx_sub_code)-1);
	memcpy(stStlmSucc.brno, stTxnSucc.brno, sizeof(stStlmSucc.brno)-1);
	memcpy(stStlmSucc.longitude, stTxnSucc.longitude, sizeof(stStlmSucc.longitude)-1);
	memcpy(stStlmSucc.latitude, stTxnSucc.latitude, sizeof(stStlmSucc.latitude)-1);
	memcpy(stStlmSucc.sw_respcd, stTxnSucc.sw_respcd, sizeof(stStlmSucc.sw_respcd)-1);
	memcpy(stStlmSucc.ibox_respcd, stTxnSucc.ibox_respcd, sizeof(stStlmSucc.ibox_respcd)-1);
	memcpy(stStlmSucc.fd3, stTxnSucc.fd3, sizeof(stStlmSucc.fd3)-1);
	memcpy(stStlmSucc.fd7, stTxnSucc.fd7, sizeof(stStlmSucc.fd7)-1);
	memcpy(stStlmSucc.ibox11, stTxnSucc.ibox11, sizeof(stStlmSucc.ibox11)-1);
	memcpy(stStlmSucc.fd11, stTxnSucc.fd11, sizeof(stStlmSucc.fd11)-1);
	memcpy(stStlmSucc.fd12, stTxnSucc.fd12, sizeof(stStlmSucc.fd12)-1);
	memcpy(stStlmSucc.fd13, stTxnSucc.fd13, sizeof(stStlmSucc.fd13)-1);
	memcpy(stStlmSucc.fd15, stTxnSucc.fd15, sizeof(stStlmSucc.fd15)-1);
	memcpy(stStlmSucc.ibox18, stTxnSucc.ibox18, sizeof(stStlmSucc.ibox18)-1);
	memcpy(stStlmSucc.fd28, stTxnSucc.fd28, sizeof(stStlmSucc.fd28)-1);
	memcpy(stStlmSucc.fd37, stTxnSucc.fd37, sizeof(stStlmSucc.fd37)-1);
	memcpy(stStlmSucc.fd38, stTxnSucc.fd38, sizeof(stStlmSucc.fd38)-1);
	memcpy(stStlmSucc.fd39, stTxnSucc.fd39, sizeof(stStlmSucc.fd39)-1);
	memcpy(stStlmSucc.ibox41, stTxnSucc.ibox41, sizeof(stStlmSucc.ibox41)-1);
	memcpy(stStlmSucc.ibox42, stTxnSucc.ibox42, sizeof(stStlmSucc.ibox42)-1);
	memcpy(stStlmSucc.ibox43, stTxnSucc.ibox43, sizeof(stStlmSucc.ibox43)-1);
	memcpy(stStlmSucc.fd41, stTxnSucc.fd41, sizeof(stStlmSucc.fd41)-1);
	memcpy(stStlmSucc.fd42, stTxnSucc.fd42, sizeof(stStlmSucc.fd42)-1);
	memcpy(stStlmSucc.fd43, stTxnSucc.fd43, sizeof(stStlmSucc.fd43)-1);
	memcpy(stStlmSucc.fd49, stTxnSucc.fd49, sizeof(stStlmSucc.fd49)-1);
	memcpy(stStlmSucc.fd60, stTxnSucc.fd60, sizeof(stStlmSucc.fd60)-1);
	memcpy(stStlmSucc.fd100, stTxnSucc.fd100, sizeof(stStlmSucc.fd100)-1);
	memcpy(stStlmSucc.fd102, stTxnSucc.fd102, sizeof(stStlmSucc.fd102)-1);
	memcpy(stStlmSucc.fd103, stTxnSucc.fd103, sizeof(stStlmSucc.fd103)-1);

	stStlmSucc.fdx = stTxnSucc.fdx;
	memcpy(stStlmSucc.fdxx, stTxnSucc.fdxx, sizeof(stStlmSucc.fdxx)-1);
	snprintf(stStlmSucc.fdxxx, sizeof(stStlmSucc.fdxxx), "第三方有本地无, 渠道[%.2f]", pstCup->tx_amt);
	CommonGetCurrentTime(stStlmSucc.rec_crt_ts);
	CommonGetCurrentTime(stStlmSucc.rec_upd_ts);

	CommonAllTrim(stStlmSucc.ibox42);

	memcpy(stStlmSucc.servetype, stTxnSucc.servetype, sizeof(stStlmSucc.servetype) - 1);
	memcpy(stStlmSucc.promtsmode, stTxnSucc.promtsmode, sizeof(stStlmSucc.promtsmode) - 1);
	stStlmSucc.promtsid = stTxnSucc.promtsid;
	stStlmSucc.promtsvalue = stTxnSucc.promtsvalue;

/*	对账不平 不管商户状态，让人工处理
	memset(&stMchtInfo, 0, sizeof(stMchtInfo));
	iRet = Tbl_mcht_base_Sel(&stMchtInfo, "mcht_no = '%s'", stStlmSucc.ibox42);
	if(iRet)
	{
		HtLog(ERR_PARAM, "tbl_mcht_base err:iRet(%d)(%s), ibox42(%s)", iRet, DbsError(), stStlmSucc.ibox42);
		return RC_FAIL;
	}

	if('0' != stMchtInfo.mcht_status[0])
		pstStlmErr->err_type[0] = ERR_CUP_MCHT_UNNORMAL;
*/
	
	//插入"对帐明细表"
	iRet = Tbl_stlm_succ_Ins(&stStlmSucc);
	if(iRet)
	{
	   	HtLog(ERR_PARAM, "记录渠道有本地无信息失败, err:(%d)(%s), id(%s)", iRet, DbsError(), stStlmSucc.id);
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int nChkStlmErr(struct tbl_stlm_err_c *pstStlmErr)					*/
/*   功能  ： 针对本地有、渠道无，渠道在T+1之后才将明细反馈回来						*/
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int nChkStlmErr(char *pszId, char chErrType, char chStlmType, char *pszErrDesc, 
				char *pszFlag, int *pnDoFlag, char *pszDate, double dCupFee, 
				double dTxnAmt, double dCupAmt)
{	 
	int		iRet = 0, iCount = 0;
	char	szTime[15];
	struct tbl_stlm_err_c stStlmErr;

	(*pnDoFlag) = 0;
	memset(&stStlmErr, 0, sizeof(stStlmErr));
	iRet = Tbl_stlm_err_Sel(&stStlmErr, "id = '%s'", pszId);
	if(DBS_NOTFOUND == iRet)
		return RC_SUCC;
	else if(RC_SUCC != iRet)
	{
		HtLog(ERR_PARAM, "查询交易差错表失败, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
		return RC_FAIL;
	}

	memset(szTime, 0, sizeof(szTime));
	CommonGetCurrentTime(szTime);
	(*pnDoFlag) = 1;
	HtLog(NML_PARAM, "上期记录ID[%s]该笔交易在本期存在", pszId);
	if(ERR_CUP_FLG_RISKCRTL == stStlmErr.err_type[0] || ERR_CUP_FLG_MANUAL == stStlmErr.err_type[0])
	{
		// 状态2  不需要修改stlm_succ渠道手续费，此时stlm_succ还未生成记录
		(*pnDoFlag) = 2;

		// 防止异常，判断这笔交易在成功表中是否存在，有可能风控延迟已经对过账了
		iCount = sql_count("tbl_stlm_succ", "id = '%s'", pszId);
		if(iCount > 0)
			(*pnDoFlag) = 1;	// 置对账不平的状态、对账2表都不新增数据

		HtLog(NML_PARAM, "ID[%s]该笔交易属于风控或人工延迟交易", pszId);
		if(STLM_FLG_OK == pszFlag[0])	//	对账平, 不处理  由人工处理
		{
			if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0] && '1' == pszFlag[1])	// 对于人工选择参与清算的交易并且商户不正常 风控和手工延迟描述保留
			{
				if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])	//	只有这种情况才可能进程2次对账， 对账日期不能修改
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, "
						"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
						STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				else
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_date = '%s', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, "
						"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, pszDate, ERR_CUP_MCHT_UNNORMAL, 
						STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
			}
			else	// 人工处理或未处理 保留处理标志
			{
				if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])
					iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
						"rec_upd_ts = '%s' where id = '%s'", STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				else
					iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
						"rec_upd_ts = '%s' where id = '%s'", pszDate, STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
			}
			if(iRet)
			{
				HtLog(ERR_PARAM, "更新手工延迟或风控差错交易状态失败, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
				return RC_FAIL;
			}

			if(1 == *pnDoFlag)	// 对于前期对账不平，本期对账平的处理
			{
				iRet = sql_execute("update tbl_stlm_succ set cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", dCupFee, szTime, pszId);
				if(iRet)
				{
					HtLog(ERR_PARAM, "更新手工延迟或风控差错交易状态失败, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
		   		 	return iRet;
				}	
			}
			return RC_SUCC;
		}

		if(!pszErrDesc && STLM_FLG_OK != pszFlag[0])
		{
			HtLog(NML_PARAM, "对账不平，对成功表中存在该笔[%s]交易，在错误表中处理", pszId);
			return RC_SUCC;
		}

		// 人工选择1-处理后参加清算 单位本周期对账不平，置'9'-未处理
		if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0])
		{
			HtLog(NML_PARAM, "风控的交易(%s)参与交易, 不平状态(%c)", pszId, stStlmErr.stlm_type[0]);
			//  如果是对过帐的，不能修改对账日期, 基本只有本地有渠道无的情况才会发生，
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, "
					"cups_amt = %.2f, err_desc = '%s', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, chErrType, chStlmType, dCupFee, dTxnAmt,
					dCupAmt, pszErrDesc, szTime, pszId);
			else
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_date = '%s', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, "
					"cups_amt = %.2f, err_desc = '%s', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, chErrType, pszDate, chStlmType, dCupFee, dTxnAmt,
					dCupAmt, pszErrDesc, szTime, pszId);
		}
		else	//  非 1-处理后参加清算, 保留人工处理状态
		{
			HtLog(NML_PARAM, "更新风控的交易(%s), 不平状态(%c)", pszId, stStlmErr.stlm_type[0]);
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])
				iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, err_desc = '%s', "
					"rec_upd_ts = '%s' where id = '%s'", chStlmType, dCupFee, dTxnAmt, dCupAmt, pszErrDesc, szTime, pszId);
			else
				iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
					"err_desc = '%s', rec_upd_ts = '%s' where id = '%s'", pszDate, chStlmType, dCupFee, dTxnAmt, dCupAmt, pszErrDesc, szTime, pszId);
		}
		if(iRet)
		{
			HtLog(ERR_PARAM, "更新手工延迟或风控差错交易状态失败, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
			return RC_FAIL;
		}
	}
	else
	{
		if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	本地有
		{
			if(!pszErrDesc && STLM_FLG_OK != pszFlag[0])
			{
				HtLog(NML_PARAM, "对账不平，对成功表中存在该笔[%s]交易，在错误表中处理", pszId);
				return RC_SUCC;
			}

			// 非9-未处理状态，不处理，有人工选择的来处理, 人工选择的1-参与清算除外
			if(STLM_CUP_NOT_DEAL != stStlmErr.result_flag[0] && STLM_CUP_TO_DONE != stStlmErr.result_flag[0])
			{
				//	转入人工处理，更新状态
				HtLog(NML_PARAM, "ID[%s]该笔交易已经转入人工处理，对账该笔不处理, 更新必要字段!", pszId);
	 			iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
						"rec_upd_ts = '%s' where id = '%s'", chStlmType, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				if(iRet)
				{
					HtLog(ERR_PARAM, "更新差错交易信息表失败, err:(%d)(%s)", iRet, DbsError());
					return iRet;
				}
				return RC_SUCC;
			}
 	
			if(STLM_FLG_OK == pszFlag[0])
			{
				g_TXAmt += stStlmErr.tx_amt;
				g_CUPFee += dCupFee;
				iRet = sql_execute("update tbl_stlm_succ set cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", dCupFee, szTime, pszId);
				if(iRet)
				{
					HtLog(ERR_PARAM, "update tbl_stlm_succ error:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
					return iRet;
				}
				if('1' == pszFlag[1])
				{
					HtLog(NML_PARAM, "上期记录ID[%s]该笔交易在本期已对平，但商户不正常，不处理!", pszId);
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, "
							"cups_amt = %.2f, err_desc = '已对平，但商户不正常', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
							STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				}
				else
				{
					HtLog(NML_PARAM, "上期记录ID[%s]该笔交易在本期已对平，直接参与清算!", pszId);
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', err_desc = '已对平，自动参与清算', cup_fee = %.2f, "
							"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_TO_DONE, STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, 
							dCupAmt, szTime, pszId);
				}
			}
	 		else 	//  对账不平
			{
				if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0])
				{
					HtLog(NML_PARAM, "上期记录ID[%s]该笔交易,人工参与清算，但对账不平", pszId);
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', err_desc = '%s', cup_fee = %.2f, "
							"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, chErrType, chStlmType, pszErrDesc, 
							dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				}
				else
				{
					HtLog(NML_PARAM, "上期记录ID[%s]该笔交易在本期未对平，修改错误状态", pszId);
					iRet = sql_execute("update tbl_stlm_err set err_type = '%c', stlm_type = '%c', err_desc = '%s', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
							"rec_upd_ts = '%s' where id = '%s'", chErrType, chStlmType, pszErrDesc, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				}
			}
			if(iRet)
			{
				HtLog(ERR_PARAM, "更新差错交易表数据失败, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
				return iRet;
			}
		}
		else	// 其他对账不平类型	
		{
			//  其他对账不平的情况，如果人工选择参与清算，但是商户不正常，截留该笔
			if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0] && '1' == pszFlag[1])
			{
				HtLog(NML_PARAM, "上期记录ID[%s]该笔交易,人工参与清算， 但商户不正常，不处理!", pszId);
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', err_desc = '商户不正常, 不能处理', "
					"stlm_type = '%c', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
					STLM_FLG_OK_UNNORMAL, szTime, pszId);
				if(iRet)
				{
					HtLog(ERR_PARAM, "更新差错交易表数据失败, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
					return iRet;
				}
			}
		}
	}
	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int StlmErrDo(tbl_cup_succ_c stCup, tbl_txn_succ_c stTxn, 			*/
/*					char *psDate, char *psFalg)										*/
/*   功能  ： 插入 对帐不平明细表												   */
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int StlmErrDo(struct tbl_cup_succ_c stCup, struct tbl_txn_succ_c stTxn, char *psDate, 
			char *psFlag, char *pszSettleNum)
{
	int		iRet, iDoFlag = 0;
	struct tbl_stlm_err_c  stStlmErr;
		 
	memset(&stStlmErr ,0x00, sizeof(struct tbl_stlm_err_c));
	memcpy(stStlmErr.stlm_date, psDate, 8);
	stStlmErr.result_flag[0] = STLM_CUP_NOT_DEAL;
	stStlmErr.err_type[0] = ERR_CUP_FLG_NOTMACHT;
	stStlmErr.stlm_type[0] = psFlag[0];
	stStlmErr.user_id = stTxn.user_id;

	stStlmErr.disc_cycle = stTxn.disc_cycle;   // T+0 新增字段
	//	目前只填写T+N的交易的settle_num, T+0必须根据ID从algo得知
	if(stStlmErr.disc_cycle > 0)
		memcpy(stStlmErr.settle_num, pszSettleNum, 4);

	if(0 == stTxn.disc_cycle)
	{
		HtLog(NML_PARAM, "渠道有本地无处理逻辑");
		iRet = nTStlmErrHandle(&stStlmErr, &stCup, &stTxn, psDate, psFlag, pszSettleNum);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "处理T+0或者渠道有本地无出错, err:(%d)", iRet);
			return RC_FAIL;
		}
		return RC_SUCC;
	}

//	该段可以选择去掉
	HtLog(NML_PARAM, "进入T+N差错交易处理");
	if(psFlag[0] == STLM_CUP_FLG_CUPS)	//   渠道多帐
	{ 
		/* 渠道多帐 成功表和错误表都插入数据*/
		iRet = nGetSettleDate(&stStlmErr, &stCup);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "获取渠道多帐对应的本地流水错误, err:(%d)", iRet);
			return RC_FAIL;
		}
	}
	else
	{
		memcpy(stStlmErr.id, stTxn.id, sizeof(stStlmErr.id) - 1);
		memcpy(stStlmErr.tx_date, stTxn.tx_date,sizeof(stStlmErr.tx_date)-1);
		memcpy(stStlmErr.tx_time, stTxn.tx_time,sizeof(stStlmErr.tx_time)-1);
		memcpy(stStlmErr.trace_no, stTxn.trace_no,sizeof(stStlmErr.trace_no)-1);
		memcpy(stStlmErr.ibox_no, stTxn.ibox_no,sizeof(stStlmErr.ibox_no)-1);
		memcpy(stStlmErr.order_no, stTxn.order_no,sizeof(stStlmErr.order_no)-1);
		memcpy(stStlmErr.cups_no, stTxn.cups_no,sizeof(stStlmErr.cups_no)-1);
		memcpy(stStlmErr.ac_no, stCup.ac_no, sizeof(stStlmErr.ac_no) - 1);
		stStlmErr.ac_type = stTxn.ac_type;
		stStlmErr.tx_amt = stCup.tx_amt;		// 存在金额不同以渠道为准 和 本地有渠道无
		stStlmErr.local_amt = stTxn.tx_amt; 	// 本地金额
		stStlmErr.cups_amt = stCup.tx_amt; 		// 渠道金额
		stStlmErr.cup_fee = stCup.cup_fee;
		stStlmErr.expand = stTxn.expand;
		memcpy(stStlmErr.tx_code, stTxn.tx_code, sizeof(stStlmErr.tx_code)-1);
		memcpy(stStlmErr.tx_name, stCup.tx_code, sizeof(stCup.tx_code)-1);
		memcpy(stStlmErr.tx_sub_code, stTxn.tx_sub_code,sizeof(stStlmErr.tx_sub_code)-1);
		memcpy(stStlmErr.brno, stTxn.brno,sizeof(stStlmErr.brno)-1);
		memcpy(stStlmErr.longitude, stTxn.longitude,sizeof(stStlmErr.longitude)-1);
		memcpy(stStlmErr.latitude, stTxn.latitude,sizeof(stStlmErr.latitude)-1);
		memcpy(stStlmErr.sys_ref_no, stTxn.fd37,sizeof(stStlmErr.sys_ref_no)-1);
		memcpy(stStlmErr.ibox41, stTxn.ibox41,sizeof(stStlmErr.ibox41)-1);
		memcpy(stStlmErr.ibox42, stTxn.ibox42,sizeof(stStlmErr.ibox42)-1);
		memcpy(stStlmErr.ibox43, stTxn.ibox43,sizeof(stStlmErr.ibox43)-1);
		memcpy(stStlmErr.fd41, stTxn.fd41,sizeof(stStlmErr.fd41)-1);
		memcpy(stStlmErr.fd42, stTxn.fd42,sizeof(stStlmErr.fd42)-1);
		memcpy(stStlmErr.fd43, stTxn.fd43,sizeof(stStlmErr.fd43)-1);
		if(psFlag[0] == STLM_CUP_FLG_POSP)
		{
			stStlmErr.tx_amt = stTxn.tx_amt;
			memcpy(stStlmErr.ac_no, stTxn.ac_no, strlen(stTxn.ac_no));
			strcpy(stStlmErr.err_desc, "第三方无本地有");
			strcpy(stStlmErr.tx_name, "POS消费");
		}
		else if(psFlag[0] ==  STLM_CUP_FLG_ACCT)
			sprintf(stStlmErr.err_desc, "帐户不一致:本地%s", stTxn.ac_no);
		else if(psFlag[0] == STLM_CUP_FLG_AMT)
			sprintf(stStlmErr.err_desc, "金额不一致,本地金额[%.2f]", stTxn.tx_amt);
		else if(STLM_FLG_OK_UNNORMAL == psFlag[0])
			sprintf(stStlmErr.err_desc, "流水已对平");

		//	对平且商户不正常 差错标志置4, 其他差错类，差错类型优先等级高于商户不正常
		if(psFlag[0] == STLM_FLG_OK_UNNORMAL && psFlag[1] == '1')
			stStlmErr.err_type[0] = ERR_CUP_MCHT_UNNORMAL;

		stStlmErr.fdx = stTxn.fdx;
		memcpy(stStlmErr.fdxx, stTxn.fdxx, sizeof(stStlmErr.fdxx)-1);
		memcpy(stStlmErr.fdxxx, stTxn.fd103, sizeof(stStlmErr.fdxxx)-1);
	}
	CommonGetCurrentTime(stStlmErr.rec_crt_ts);
	CommonGetCurrentTime(stStlmErr.rec_upd_ts);

	iRet = nChkStlmErr(stStlmErr.id, stStlmErr.err_type[0], stStlmErr.stlm_type[0], stStlmErr.err_desc, psFlag, 
			&iDoFlag, psDate, stCup.cup_fee, stTxn.tx_amt, stCup.tx_amt);
	if(iRet)
	{
	   	HtLog(ERR_PARAM, "nChkStlmErr err:iRet(%d)", iRet);
		return RC_FAIL;
	}
	if(1 == iDoFlag || 2 == iDoFlag)	return RC_SUCC;
	
	HtLog(NML_PARAM, "Id(%s)tx_date(%s)trace_no(%s)", stStlmErr.id, stStlmErr.tx_date, stStlmErr.trace_no);

	//插入"对帐不平明细表"
	iRet = Tbl_stlm_err_Ins(&stStlmErr);
	if(iRet)
	{
	   	HtLog(ERR_PARAM, "新增差错交易失败, err:iRet(%d)(%s),id(%s)", iRet, DbsError(), stStlmErr.id);
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int StlmSuccDo(tbl_cup_succ_c stCup, tbl_txn_succ_c stTxn, 			*/
/*					char *psDate, char *psFalg)										*/
/*   功能  ： 插入 对帐平明细表													 */
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int StlmSuccDo(struct tbl_cup_succ_c stCup ,struct tbl_txn_succ_c stTxn ,char *psDate, char *psFlag)
{
	int		iRet = 0;
	struct tbl_stlm_succ_c stStlmSucc;
	 
	memset((char *)&stStlmSucc ,0x00, sizeof(struct tbl_stlm_succ_c));

	if(psFlag[0] == '0')
		stStlmSucc.result_flag[0] = STLM_SUCC_FLG_INIT;			//	未清分
	else
		stStlmSucc.result_flag[0] = STLM_SUCC_FLG_ERR;			//	对账不平 

	memcpy(stStlmSucc.id, stTxn.id, sizeof(stStlmSucc.id) - 1);
	memcpy(stStlmSucc.stlm_date, psDate, 8);
	memcpy(stStlmSucc.tx_date, stTxn.tx_date,  sizeof(stStlmSucc.tx_date)-1);
	memcpy(stStlmSucc.tx_time, stTxn.tx_time,  sizeof(stStlmSucc.tx_time)-1);
	memcpy(stStlmSucc.trace_no, stTxn.trace_no,  sizeof(stStlmSucc.trace_no)-1);
	memcpy(stStlmSucc.ibox_no, stTxn.ibox_no,  sizeof(stStlmSucc.ibox_no)-1);
	memcpy(stStlmSucc.order_no, stTxn.order_no,  sizeof(stStlmSucc.order_no)-1);
	memcpy(stStlmSucc.cups_no, stTxn.cups_no,  sizeof(stStlmSucc.cups_no)-1);
	memcpy(stStlmSucc.ac_no, stTxn.ac_no, sizeof(stStlmSucc.ac_no) - 1);
	stStlmSucc.user_id = stTxn.user_id;
	stStlmSucc.disc_cycle = stTxn.disc_cycle;
	stStlmSucc.ac_type = stTxn.ac_type;
	memcpy(stStlmSucc.ac_bank_no, stCup.ac_bank_no,  sizeof(stStlmSucc.ac_bank_no)-1);
	stStlmSucc.fee_amt = stTxn.fee_amt;
	stStlmSucc.cup_fee = stCup.cup_fee;		//	cups返回的 手续费

	memcpy(stStlmSucc.tx_code, stTxn.tx_code, sizeof(stStlmSucc.tx_code)-1);
	strcpy(stStlmSucc.tx_name, "POS消费");
	memcpy(stStlmSucc.tx_sub_code, stTxn.tx_sub_code, sizeof(stStlmSucc.tx_sub_code)-1);
	memcpy(stStlmSucc.brno, stTxn.brno, sizeof(stStlmSucc.brno)-1);
	memcpy(stStlmSucc.longitude, stTxn.longitude,  sizeof(stStlmSucc.longitude)-1);
	memcpy(stStlmSucc.latitude, stTxn.latitude,  sizeof(stStlmSucc.latitude)-1);
	memcpy(stStlmSucc.sw_respcd, stTxn.sw_respcd, sizeof(stStlmSucc.sw_respcd)-1);
	memcpy(stStlmSucc.ibox_respcd, stTxn.ibox_respcd, sizeof(stStlmSucc.ibox_respcd)-1);
	memcpy(stStlmSucc.fd3, stTxn.fd3, sizeof(stStlmSucc.fd3)-1);
	memcpy(stStlmSucc.fd7, stTxn.fd7, sizeof(stStlmSucc.fd7)-1);
	memcpy(stStlmSucc.ibox11, stTxn.ibox11, sizeof(stStlmSucc.ibox11)-1);
	memcpy(stStlmSucc.fd11, stTxn.fd11, sizeof(stStlmSucc.fd11)-1);
	memcpy(stStlmSucc.fd12, stTxn.fd12, sizeof(stStlmSucc.fd12)-1);
	memcpy(stStlmSucc.fd13, stTxn.fd13, sizeof(stStlmSucc.fd13)-1);
	memcpy(stStlmSucc.fd15, stTxn.fd15, sizeof(stStlmSucc.fd15)-1);
	memcpy(stStlmSucc.ibox18, stTxn.ibox18, sizeof(stStlmSucc.ibox18)-1);
	memcpy(stStlmSucc.fd28, stTxn.fd28, sizeof(stStlmSucc.fd28)-1);
	memcpy(stStlmSucc.fd37, stTxn.fd37, sizeof(stStlmSucc.fd37)-1);
	memcpy(stStlmSucc.fd38, stTxn.fd38, sizeof(stStlmSucc.fd38)-1);
	memcpy(stStlmSucc.fd39, stTxn.fd39, sizeof(stStlmSucc.fd39)-1);
	memcpy(stStlmSucc.ibox41, stTxn.ibox41, sizeof(stStlmSucc.ibox41)-1);
	memcpy(stStlmSucc.ibox42, stTxn.ibox42, sizeof(stStlmSucc.ibox42)-1);
	memcpy(stStlmSucc.ibox43, stTxn.ibox43, sizeof(stStlmSucc.ibox43)-1);
	memcpy(stStlmSucc.fd41, stTxn.fd41, sizeof(stStlmSucc.fd41)-1);
	memcpy(stStlmSucc.fd42, stTxn.fd42, sizeof(stStlmSucc.fd42)-1);
	memcpy(stStlmSucc.fd43, stTxn.fd43, sizeof(stStlmSucc.fd43)-1);
	memcpy(stStlmSucc.fd49, stTxn.fd49, sizeof(stStlmSucc.fd49)-1);
	memcpy(stStlmSucc.fd60, stTxn.fd60, sizeof(stStlmSucc.fd60)-1);
	memcpy(stStlmSucc.fd100, stTxn.fd100, sizeof(stStlmSucc.fd100)-1);
	memcpy(stStlmSucc.fd102, stTxn.fd102, sizeof(stStlmSucc.fd102)-1);
	memcpy(stStlmSucc.fd103, stTxn.fd103, sizeof(stStlmSucc.fd103)-1);

	stStlmSucc.fdx = stTxn.fdx;
	memcpy(stStlmSucc.fdxx, stTxn.fdxx, sizeof(stStlmSucc.fdxx)-1);
	stStlmSucc.expand = stTxn.expand;

	memcpy(stStlmSucc.servetype, stTxn.servetype, sizeof(stStlmSucc.servetype) - 1);
	memcpy(stStlmSucc.promtsmode, stTxn.promtsmode, sizeof(stStlmSucc.promtsmode) - 1);
	stStlmSucc.promtsid = stTxn.promtsid;
	stStlmSucc.promtsvalue = stTxn.promtsvalue;

	if(STLM_CUP_FLG_AMT == psFlag[0])
	{
		snprintf(stStlmSucc.fdxxx, sizeof(stStlmSucc.fdxxx), "本地金额[%.2f]渠道金额[%.2f],以渠道为准", 
			stTxn.tx_amt, stCup.tx_amt);
		stStlmSucc.tx_amt = stCup.tx_amt; 
	}
	else
	{
		memcpy(stStlmSucc.fdxxx, stTxn.fdxxx, sizeof(stStlmSucc.fdxxx)-1);
		stStlmSucc.tx_amt = stTxn.tx_amt; 
	}
	CommonGetCurrentTime(stStlmSucc.rec_crt_ts);
	CommonGetCurrentTime(stStlmSucc.rec_upd_ts);
	
	//插入"对帐明细表"
	iRet = Tbl_stlm_succ_Ins(&stStlmSucc);
	if(iRet)
	{
	   	HtLog(ERR_PARAM, "新增对账成功信息失败, err:(%d)(%s),Id(%s)", iRet, DbsError(), stStlmSucc.id);
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int nGetFeeAmt(char *pszDisc, double dTxAmt, double *pdFeeAmt)		*/
/*   功能  ： T+0计算手续费															*/
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int		nGetFeeAmt(char *pszDisc, double dTxAmt, long lActType, double *pdFeeAmt)
{
	double		dFeeAmt = 0.00;
	int	 	iRet = 0, lCount = 0;
	dbDiscAlgo  *pstAlgo = NULL, stAlgo;

	HtLog(NML_PARAM, "DISC_CD[%s], tx_amt(%.2f)", pszDisc, dTxAmt);
	iRet = lGetDiscAlgo(pszDisc, lActType, FIELD_MATCH, &pstAlgo, (long *)&lCount);
	if(iRet)
	{
		HtLog(ERR_PARAM, "获取费率模型列表失败 err:(%d)(%s)", iRet, sGetError());
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	iRet = lGetAlgoModel(pstAlgo, lCount, dTxAmt, &stAlgo);
	if(iRet)
	{
		IBPFree(pstAlgo);
		HtLog(ERR_PARAM, "从模型列表(%d)中获取金额(%.2f)对应费率失败, err:(%d)(%s)", lCount, dTxAmt, 
			iRet, sGetError());
		return RC_FAIL;
	}
	IBPFree(pstAlgo);

	if(stAlgo.flag == DISC_ALGO_FIXED)  /*** 按笔(固定)收费**/
		*pdFeeAmt = stAlgo.fee_value;
	else if(stAlgo.flag == DISC_ALGO_RATE)   /*** 按比收费**/
	{
		dFeeAmt = dTxAmt * stAlgo.fee_value / 100;
		if(stAlgo.min_fee > 0)		   //  取最小最大区间的费用
  			dFeeAmt = (stAlgo.min_fee >= dFeeAmt) ? stAlgo.min_fee : dFeeAmt;
		if(stAlgo.max_fee > 0)
			dFeeAmt = (stAlgo.max_fee <= dFeeAmt) ? stAlgo.max_fee : dFeeAmt;
  		*pdFeeAmt = dFeeAmt;
	}
	else if(stAlgo.flag > DISC_ALGO_FIXED)	//	  固定收费
  		*pdFeeAmt = 0.00;

	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int nTFailTrade(struct tbl_stlm_err_c *pstStlmErr, char chState)		*/
/*   功能  ： T+0对账错误表处理														*/
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int		nTFailTrade(struct tbl_stlm_err_c *pstStlmErr)
{
	int		iRet = 0, iChkAlgo = 0;
	char	szTime[20];
	struct tbl_stlm_err_c	stErr;
	struct tbl_algo_dtl_c	stAlgo;
	
	memset(szTime, 0, sizeof(szTime));
	CommonGetCurrentTime(szTime);
	memset(&stAlgo, 0, sizeof(stAlgo));
	iChkAlgo = Tbl_algo_dtl_Sel(&stAlgo, "id = '%s'", pstStlmErr->id);
	if(iChkAlgo && DBS_NOTFOUND != iChkAlgo)
	{
		HtLog(ERR_PARAM, "查询清分表(%s)错误, err:(%d)(%s),id(%s)", pstStlmErr->id, 
			iRet, DbsError());
		return RC_FAIL;
	}
	else if(0 == iChkAlgo)
	{
		CommonAllTrim(stAlgo.settle_disc);
		iRet = nGetFeeAmt(stAlgo.settle_disc, pstStlmErr->cups_amt, stAlgo.ac_type, 
			&pstStlmErr->fee_amt);
		if(iRet)
		{
			HtLog(ERR_PARAM, "根据渠道金额计算手续费出错, err:(%d)id(%s)", iRet, 
				pstStlmErr->id);
			return RC_FAIL;
		}
	}

	memset(&stErr, 0, sizeof(stErr));
	iRet = Tbl_stlm_err_Sel(&stErr, "id = '%s'", pstStlmErr->id);
	if(iRet && DBS_NOTFOUND != iRet)
	{
		HtLog(ERR_PARAM, "查询差错信息失败, err:(%d)(%s)id(%s)", iRet, DbsError(), 
			pstStlmErr->id);
		return RC_FAIL;
	}
	else if(DBS_NOTFOUND == iRet)	// 新增对账不平记录
	{
		//	T+N 渠道有，本地无的流水走的这里。
		HtLog(WAR_PARAM, "周期(%d)交易有差错, Id(%s)", pstStlmErr->disc_cycle, pstStlmErr->id);
		iRet = Tbl_stlm_err_Ins(pstStlmErr);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "新增差错流水出错, err:(%d)(%s),ID(%s)", iRet, DbsError(), 
				pstStlmErr->id);
			return RC_FAIL;
		}
		return RC_SUCC;
	}
//	else	find it

	//	对账不平对明细处理逻辑
	if(ERR_CUP_FLG_RISKCRTL == stErr.err_type[0] || ERR_CUP_FLG_MANUAL == stErr.err_type[0])
	{
		//	该笔交易如果未处理或者参入清算后并已清分、处理后参加清算
		if(STLM_CUP_TO_DONE == stErr.result_flag[0] || STLM_CUP_SETTLE_SUCC == stErr.result_flag[0] ||
			STLM_CUP_NOT_DEAL == stErr.result_flag[0])	// 准备清分或者已经清分
		{
			HtLog(NML_PARAM, "T+0交易延迟标志(%c), 处理标志(%c)，状态重置, Id(%s)", 
				stErr.err_type[0], stErr.result_flag[0], pstStlmErr->id);
			if(STLM_CUP_FLG_POSP == stErr.stlm_type[0])	 //  已经对过账，不能修改对账日期
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', "
					"cups_amt = %.2f, fee_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
					STLM_CUP_NOT_DEAL, pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, pstStlmErr->fee_amt, 
					pstStlmErr->cup_fee, szTime, pstStlmErr->id);
			else
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_date = '%s', "
					"stlm_type = '%c', cups_amt = %.2f, fee_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' "
					"where id = '%s'", STLM_CUP_NOT_DEAL, pstStlmErr->stlm_date, pstStlmErr->stlm_type[0], 
					pstStlmErr->cups_amt, pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, pstStlmErr->id);
		}
		else	// 人工处理（ 这里指更新状态)
		{
			HtLog(NML_PARAM, "T+0交易延迟标志(%c), 人工已处理, 处理标志(%c)，更新对账标识, id(%s)", 
				stErr.err_type[0], stErr.result_flag[0], pstStlmErr->id);
			if(STLM_CUP_FLG_POSP == stErr.stlm_type[0])	 //  已经对过账，不能修改对账日期
				iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cups_amt = %.2f, fee_amt = %.2f, "
					"cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pstStlmErr->stlm_type[0], 
					pstStlmErr->cups_amt, pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, pstStlmErr->id);
			else
				iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', stlm_type = '%c', cups_amt = %.2f, "
					"fee_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pstStlmErr->stlm_date, 
					pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, pstStlmErr->fee_amt, pstStlmErr->cup_fee, 
					szTime, pstStlmErr->id);
		}
	}
	else if(ERR_CUP_MCHT_UNNORMAL == stErr.err_type[0])
	{
		HtLog(NML_PARAM, "T+0交易商户不正常,更新对账标识, Id(%s)", pstStlmErr->id);
		if(STLM_CUP_FLG_POSP == stErr.stlm_type[0])	 //  已经对过账，不能修改对账日期
			iRet = sql_execute("update tbl_stlm_err set err_type ='%c', stlm_type = '%c', cups_amt = %.2f, "
				"fee_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pstStlmErr->err_type[0], 
				pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, 
				pstStlmErr->id);
		else
			iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', err_type = '%c', stlm_type = '%c', "
				"cups_amt = %.2f, fee_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
				pstStlmErr->stlm_date, pstStlmErr->err_type[0], pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, 
				pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, pstStlmErr->id);
	}
	else	//	对账不平的交易
	{
		HtLog(NML_PARAM, "T+0上期对账不平交易标识(%c), 处理标志(%c)，状态重置, Id(%s)", stErr.stlm_type[0], 
			stErr.result_flag[0], pstStlmErr->id);
		
		//	走到这里肯定已经对过账了
		//	该笔交易如果未处理或者参入清算后并已清分、处理后参加清算
		if(STLM_CUP_TO_DONE == stErr.result_flag[0] || STLM_CUP_SETTLE_SUCC == stErr.result_flag[0] ||
			STLM_CUP_NOT_DEAL == stErr.result_flag[0])	// 准备清分或者已经清分
		{
			iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', cups_amt = %.2f, "
				"fee_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, 
				pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, 
				pstStlmErr->id);
		}
		else	// 人工处理（ 这里指更新状态)
		{
			iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cups_amt = %.2f, fee_amt = %.2f, "
				"cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, 
				pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, pstStlmErr->id);
		}
	}
	if(iRet)
	{
		HtLog(ERR_PARAM, "更新差错交易表明细出错, err:(%d)(%s), id(%s)", iRet, DbsError(), pstStlmErr->id);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int nTSuccTrade(tbl_txn_succ_c *pstTxnSucc, char chState)			 */
/*   功能  ： T+0对账平交易的处理												   */
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int	nTSuccTrade(struct tbl_txn_succ_c *pstTxnSucc, double dCupFee, char chState, char *pszDate)
{
	int		iRet = 0, iChkAlgo = 0;
	char	szTime[20];
	struct tbl_stlm_err_c stStlmErr;
	struct tbl_algo_dtl_c stAlgo;

	memset(szTime, 0, sizeof(szTime));
	CommonGetCurrentTime(szTime);
	memset(&stAlgo, 0, sizeof(stAlgo));
	iChkAlgo = Tbl_algo_dtl_Sel(&stAlgo, "id = '%s'", pstTxnSucc->id);
	if(iChkAlgo && DBS_NOTFOUND != iChkAlgo)
	{
		HtLog(ERR_PARAM, "查询清分表失败, err:(%d)(%s), id(%s)", iRet, DbsError(), pstTxnSucc->id);
		return RC_FAIL;
	}
	else if(0 == iChkAlgo)	//	更新algo表数据
	{
		HtLog(NML_PARAM, "更新清分表手续费字段, id(%s)", pstTxnSucc->id);
		iRet = sql_execute("update tbl_algo_dtl set cup_fee = %.2f, other_fee = fee_amt - %.2f, "
			"rec_upd_ts = '%s' where id = '%s'", dCupFee, dCupFee, szTime, pstTxnSucc->id);
		if(iRet)
		{
			HtLog(ERR_PARAM, "对T+0交易更新清分表必要字段失败, err:(%d)(%s), id(%s)", iRet, DbsError(), pstTxnSucc->id);
			return RC_FAIL;
		}
	}

	memset(&stStlmErr, 0, sizeof(stStlmErr));
	iRet = Tbl_stlm_err_Sel(&stStlmErr, "id = '%s'", pstTxnSucc->id);
	if(iRet && DBS_NOTFOUND != iRet)
	{
		HtLog(ERR_PARAM, "Tbl_stlm_err_Sel err:iRet(%d)(%s), id(%s)", iRet, DbsError(), pstTxnSucc->id);
		return RC_FAIL;
	}
	else if(DBS_NOTFOUND == iRet)
		return RC_SUCC;
//	else  	//	GET IT

	if(ERR_CUP_FLG_RISKCRTL == stStlmErr.err_type[0] || ERR_CUP_FLG_MANUAL == stStlmErr.err_type[0])
	{
		HtLog(NML_PARAM, "T+0风控或手工延迟清算的交易");
		//	人工选择参与清算，说明该笔肯定还未清算, 但是商户此时非正常
		if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0] && '1' == chState)	
		{
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	已经对过账，不能修改对账日期
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', "
					"stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
					STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, 
					dCupFee, szTime, pstTxnSucc->id);
			else
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_date = '%s', "
					"err_type = '%c', stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' "
					"where id = '%s'", STLM_CUP_NOT_DEAL, pszDate, ERR_CUP_MCHT_UNNORMAL, STLM_FLG_OK_UNNORMAL, 
					pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
		}
		else	//	参与清算但商户正常，或者人工处理 或者已经清算了
		{
			if(0 == iChkAlgo)   //  algo表存在
			{
				if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	已经对过账，不能修改对账日期
				{
					if(STLM_CUP_NOT_DEAL == stStlmErr.stlm_type[0])	//	还未处理
						iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', "
							"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
							STLM_CUP_SETTLE_SUCC, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, 
							szTime, pstTxnSucc->id);
					else	//	人工处理了或者已经清分了， 不可能是 1、参与清算状态
						iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cups_amt = %.2f, "
							"cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_FLG_OK_UNNORMAL, 
							pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				}
				else
				{
					if(STLM_CUP_NOT_DEAL == stStlmErr.stlm_type[0]) //  还未处理
						iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_date = '%s', "
							"stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
							STLM_CUP_SETTLE_SUCC, pszDate, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, 
							szTime, pstTxnSucc->id);
					else
						iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', stlm_type = '%c', "
							"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pszDate, 
							STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				}
			}
			else	//	algo 不存在该记录
			{
				if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	已经对过账，不能修改对账日期
					iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cups_amt = %.2f, "
						"cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_FLG_OK_UNNORMAL, 
						pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				else
					iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', stlm_type = '%c', "
						"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pszDate, 
						STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
			}
		}
	}
	else if(ERR_CUP_MCHT_UNNORMAL == stStlmErr.err_type[0])	//	页面上不能对商户不正常的进行处理
	{
		HtLog(NML_PARAM, "T+0商户不正常延迟清算的交易");
		if(0 == iChkAlgo)	//	algo表存在
		{
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])	 //  已经对过账，不能修改对账日期
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', "
					"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
					STLM_CUP_SETTLE_SUCC, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, 
					szTime, pstTxnSucc->id);
			else
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_date = '%s', "
					"stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
					STLM_CUP_SETTLE_SUCC, pszDate, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, 
					szTime, pstTxnSucc->id);
		}
		else
		{
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])	 //  已经对过账，不能修改对账日期
				iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cups_amt = %.2f, "
					"cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_FLG_OK_UNNORMAL, 
					pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
			else
				iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', stlm_type = '%c', "
					"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pszDate, 
					STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
		}
	}
	else	// ERR_CUP_FLG_NOTMACHT
	{
		// 人工可能已经选择补帐了, 已经对账的不能修改对账日期
		if(STLM_CUP_NOT_DEAL == stStlmErr.result_flag[0] || STLM_CUP_TO_DONE == stStlmErr.result_flag[0])	// 该笔人工还未处理
		{
			if(DBS_NOTFOUND == iChkAlgo)	// 未找到，对平，可以清算
			{
				if('1' == chState)
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', "
						"err_desc = '已对平，商户不正常', stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, "
						"rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
						STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				else
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_desc = '已对平，自动参与清算', "
						"stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
						STLM_CUP_TO_DONE, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
			}
			else	// Alog表存在记录, 已经对平的交易，直接改掉处理状态 0、参入清算后并已清分
			{
				//	此情况不可能是 1、参与清算状态 (alog有记录不能选1状态), 这里还是判断一下
				if(STLM_CUP_NOT_DEAL == stStlmErr.result_flag[0])
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_desc = '流水已对平', "
						"stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
						STLM_CUP_SETTLE_SUCC, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				else
					HtLog(ERR_PARAM, "页面处理错误，警告处理!");
			}
		}
		else	// 该笔人工已经处理、 仅仅更新状态	不能修改对账日期
		{
			iRet = sql_execute("update tbl_stlm_err set err_desc = '流水已对平', stlm_type = '%c', "
				"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_FLG_OK_UNNORMAL, 
				pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
		}
	}
	if(iRet)
	{
		HtLog(ERR_PARAM, "更新差错交易表出错, err:(%d)(%s)ID(%s)", iRet, DbsError(), pstTxnSucc->id);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int nTStlmErrHandle(struct tbl_cup_succ_c *pstCup,					*/ 
/*				struct tbl_txn_succ_c *pstTxnSucc, char *pszDate, char *pszFlag)	*/
/*   功能  ： T+0对账平交易的处理												   */
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int		nTStlmErrHandle(struct tbl_stlm_err_c *pstStlmErr, struct tbl_cup_succ_c *pstCup, 
			struct tbl_txn_succ_c *pstTxnSucc, char *pszDate, char *pszFlag, char *pszSettleNum)
{
	int		iRet = 0;
	
	if(pszFlag[0] == STLM_CUP_FLG_CUPS)	//   渠道多帐
	{ 
		/* 渠道多帐 成功表和错误表都插入数据*/
		iRet = nGetSettleDate(pstStlmErr, pstCup);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "获取渠道多帐对应的本地流水错误, err:(%d)", iRet);
			return RC_FAIL;
		}
	}
	else
   	{ 
		memcpy(pstStlmErr->id, pstTxnSucc->id, sizeof(pstStlmErr->id) - 1);
		memcpy(pstStlmErr->tx_date, pstTxnSucc->tx_date, sizeof(pstStlmErr->tx_date)-1);
		memcpy(pstStlmErr->tx_time, pstTxnSucc->tx_time, sizeof(pstStlmErr->tx_time)-1);
		memcpy(pstStlmErr->trace_no, pstTxnSucc->trace_no, sizeof(pstStlmErr->trace_no)-1);
		memcpy(pstStlmErr->ibox_no, pstTxnSucc->ibox_no, sizeof(pstStlmErr->ibox_no)-1);
		memcpy(pstStlmErr->order_no, pstTxnSucc->order_no, sizeof(pstStlmErr->order_no)-1);
		memcpy(pstStlmErr->cups_no, pstTxnSucc->cups_no, sizeof(pstStlmErr->cups_no)-1);
		memcpy(pstStlmErr->ac_no, pstCup->ac_no, sizeof(pstStlmErr->ac_no) - 1);
		pstStlmErr->ac_type = pstTxnSucc->ac_type;
		pstStlmErr->tx_amt = pstCup->tx_amt;			// 存在金额不同以渠道为准 和 本地有渠道无
		pstStlmErr->local_amt = pstTxnSucc->tx_amt; 	// 本地金额
		pstStlmErr->cups_amt = pstCup->tx_amt; 			// 渠道金额
		pstStlmErr->cup_fee = pstCup->cup_fee;
		pstStlmErr->expand = pstTxnSucc->expand;
		memcpy(pstStlmErr->tx_code, pstTxnSucc->tx_code, sizeof(pstStlmErr->tx_code)-1);
		memcpy(pstStlmErr->tx_name, pstCup->tx_code, sizeof(pstCup->tx_code)-1);
		memcpy(pstStlmErr->tx_sub_code, pstTxnSucc->tx_sub_code, sizeof(pstStlmErr->tx_sub_code)-1);
		memcpy(pstStlmErr->brno, pstTxnSucc->brno, sizeof(pstStlmErr->brno)-1);
		memcpy(pstStlmErr->longitude, pstTxnSucc->longitude, sizeof(pstStlmErr->longitude)-1);
		memcpy(pstStlmErr->latitude, pstTxnSucc->latitude, sizeof(pstStlmErr->latitude)-1);
		memcpy(pstStlmErr->sys_ref_no, pstTxnSucc->fd37, sizeof(pstStlmErr->sys_ref_no)-1);
		memcpy(pstStlmErr->ibox41, pstTxnSucc->ibox41, sizeof(pstStlmErr->ibox41)-1);
		memcpy(pstStlmErr->ibox42, pstTxnSucc->ibox42, sizeof(pstStlmErr->ibox42)-1);
		memcpy(pstStlmErr->ibox43, pstTxnSucc->ibox43, sizeof(pstStlmErr->ibox43)-1);
		memcpy(pstStlmErr->fd41, pstTxnSucc->fd41, sizeof(pstStlmErr->fd41)-1);
		memcpy(pstStlmErr->fd42, pstTxnSucc->fd42, sizeof(pstStlmErr->fd42)-1);
		memcpy(pstStlmErr->fd43, pstTxnSucc->fd43, sizeof(pstStlmErr->fd43)-1);
		if(pszFlag[0] == STLM_CUP_FLG_POSP)
		{
			pstStlmErr->tx_amt = pstTxnSucc->tx_amt;
			memcpy(pstStlmErr->ac_no, pstTxnSucc->ac_no, strlen(pstTxnSucc->ac_no));
   			strcpy(pstStlmErr->err_desc, "第三方无本地有");
   			strcpy(pstStlmErr->tx_name, "POS消费");
   		}
   		else if(pszFlag[0] == STLM_CUP_FLG_ACCT)
   			sprintf(pstStlmErr->err_desc, "帐户不一致:本地%s", pstTxnSucc->ac_no);
   		else if(pszFlag[0] == STLM_CUP_FLG_AMT)
   			sprintf(pstStlmErr->err_desc, "金额不一致,本地金额[%.2f]", pstTxnSucc->tx_amt);
		else if(STLM_FLG_OK_UNNORMAL == pszFlag[0])
   			sprintf(pstStlmErr->err_desc, "流水已对平");
	
//	T+0 交易判断商户状态无意义
//	if(pszFlag[1] == '1')	// 商户不正常
//		pstStlmErr->err_type[0] = ERR_CUP_MCHT_UNNORMAL;
	
   		pstStlmErr->fdx = pstTxnSucc->fdx;
   		memcpy(pstStlmErr->fdxx, pstTxnSucc->fdxx, sizeof(pstStlmErr->fdxx)-1);
   		memcpy(pstStlmErr->fdxxx, pstTxnSucc->fd103, sizeof(pstStlmErr->fdxxx)-1);
	}
   	CommonGetCurrentTime(pstStlmErr->rec_crt_ts);
   	CommonGetCurrentTime(pstStlmErr->rec_upd_ts);
	if(pstStlmErr->disc_cycle > 0)
		memcpy(pstStlmErr->settle_num, pszSettleNum, 4);

	iRet = nTFailTrade(pstStlmErr);
	if(iRet)
	{
		HtLog(ERR_PARAM, "处理差错交易出错, err:(%d)(%s)", iRet, pstStlmErr->id);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int nSetErrSettleNum(char *pszSettleNum, char *pstStlmData)			*/
/*   功能  ： 修改stlm_err表中的settle_num字段										*/
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
int		nSetErrSettleNum(char *pszSettleNum, char *pstStlmData)
{

	int		iRet = 0, lCount = 0;
	struct	tbl_stlm_err_c	stStlmErr;	
	struct	tbl_algo_dtl_c	stAlgoDtl;	

	iRet = Tbl_stlm_err_Dec_Sel("stlm_date = '%s' and disc_cycle = 0 and settle_num is null", pstStlmData);
	if(iRet)
	{
		HtLog(ERR_PARAM, "查询差错交易信息表失败, err:(%d)(%s)", iRet, DbsError());
		return RC_FAIL;
	}

	while(1)
	{
		memset(&stStlmErr, 0, sizeof(stStlmErr));
		iRet = Tbl_stlm_err_Fet_Sel(&stStlmErr);
		if(RC_SUCC != iRet && iRet != DBS_NOTFOUND)
		{
			HtLog(ERR_PARAM, "查询差错交易信息表失败, err:(%d)(%s)", iRet, DbsError());
			Tbl_stlm_err_Clo_Sel();
			DbsRollback();
			return RC_FAIL;
		}
		else if(iRet == DBS_NOTFOUND)
		{
			HtLog(NML_PARAM, "修改差错交易表中T + 0记录批次序号完成!");
			break;
		}

		CommonAllTrim(stStlmErr.id);
		memset(&stAlgoDtl, 0, sizeof(stAlgoDtl));
		iRet = Tbl_algo_dtl_Sel(&stAlgoDtl, "id = '%s'", stStlmErr.id);
		if(iRet && DBS_NOTFOUND != iRet)
		{
			HtLog(ERR_PARAM, "查询清分表失败, err:(%d)(%s), id(%d)", iRet, DbsError(), stStlmErr.id);
			return RC_FAIL;
		}
		else if(DBS_NOTFOUND == iRet)		//	没找到说明该笔还未清算，置默认TN00
			strcpy(stStlmErr.settle_num, "TN00");
		else
			memcpy(stStlmErr.settle_num, stAlgoDtl.settle_num, sizeof(stStlmErr.settle_num) - 1);

		iRet = sql_execute("update tbl_stlm_err set settle_num = '%s' where id = '%s'", stStlmErr.settle_num, stStlmErr.id);
		if(iRet)
		{
			HtLog(ERR_PARAM, "更新差错交易表中T + 0记录批次序号失败, err:(%d)(%s), id(%d)", iRet, DbsError(), stStlmErr.id);
			return RC_FAIL;
		}
	}
	Tbl_stlm_err_Clo_Sel();

	return RC_SUCC;
}

/************************************************************************************/
/*   声明  ： int StlmCupTxn(char *psDate)										  */
/*   功能  ： 对账主调函数														  */
/*   参数  ： 输入无																*/
/*		 ： 输出无																*/
/*  返回值 ： 0--  成功															 */
/*		   -1--  失败															 */
/************************************************************************************/
/*******对账主调函数**********/
int StlmCupTxn(char *pszStlmDate, char *pszCupsNo, char *pszSettleNum)
{	
	int	 iRet = 0, iDoFlag = 0, lCount = 0;
	char	sFlag[2], sMaxDate[9];
	char	szTime[15] = {0};
	struct tbl_cup_succ_c	stCupSucc;
	struct tbl_txn_succ_c	stTxnSucc;
	dbMchtBase				stMchtInfo;

	memset(sFlag, 0, sizeof(sFlag));
	memset(sMaxDate, 0, sizeof(sMaxDate));
	GetChgDate(pszStlmDate, sMaxDate, -1);
	DbsBegin();
	/*获取银联本代他当前工作日对应的记录首次对帐*/
	iRet = Tbl_cup_succ_Dec_Sel("stlm_date <= '%s' and cups_no = '%s' and result_flag = '%c' "
		"and tx_date < '%s' order by tx_date, tx_time ", pszStlmDate, pszCupsNo, TRADE_TXN_INIT, 
		pszStlmDate);
	if(iRet)
	{
		HtLog(ERR_PARAM, "查询渠道流水失败, err:(%d)(%s)", iRet, DbsError());
		return RC_FAIL;
	}

	while(1)
	{
		memset(&stCupSucc, 0, sizeof(stCupSucc));
		sFlag[0] = '0';
		sFlag[1] = '0';

		iRet = Tbl_cup_succ_Fet_Sel(&stCupSucc);
		if(RC_SUCC != iRet && iRet != DBS_NOTFOUND)
		{
			HtLog(ERR_PARAM, "获取渠道流水失败, err:(%d)(%s)", iRet, DbsError());
			Tbl_cup_succ_Clo_Sel();
			DbsRollback();
			return RC_FAIL;
		}
		else if(iRet == DBS_NOTFOUND)
		{
			HtLog(NML_PARAM, "依据渠道流水核对本地处理完毕!");
			Tbl_cup_succ_Clo_Sel();
			break;
		}
		
		lCount ++;
		CommonGetCurrentTime(szTime);
		CommonAllTrim(stCupSucc.cup_key);
		//查询收单成功交易流水表中交易
		memset(&stTxnSucc, 00, sizeof(struct tbl_txn_succ_c));
		iRet = Tbl_txn_succ_Sel(&stTxnSucc, "cup_key = '%s' ", stCupSucc.cup_key);
		if(iRet != 0 && iRet != DBS_NOTFOUND)
		{
			HtLog(ERR_PARAM, "查找Tbl_txn_succ明细失败, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), 
				stCupSucc.cup_key);
			Tbl_cup_succ_Clo_Sel();
			DbsRollback();
			return RC_FAIL; 
		}
		else if(iRet == DBS_NOTFOUND)	/*第三方有本地无*/
		{
			HtLog(NML_PARAM, "第三方有本地无(%s)", stCupSucc.cup_key);
			//	检查该笔交易是否在本地垮天了
			//	倘若渠道交易时间是23000，并且渠道多帐，我们可能垮天，有可能本期不对账
			if(0 < memcmp(stCupSucc.tx_time, IBP_SETTLE_TIME, 6) && !memcmp(stCupSucc.tx_date, sMaxDate, 8))
			{
				continue;
				
				/*tlb_n_txn目前不在批量表中，txn_suss也没当日的交易，因此对着这种交易只能第二天对账*/
				/*
				memset(&stNTxn, 0, sizeof(stNTxn));
				iRet = Tbl_n_txn_Sel(&stNTxn, "(fd13 = '%s' or tx_date = '%s') and fd11 = '%s' and (tx_sub_code='31' or tx_sub_code = '51') and stat = '0'", 
						stCupSucc.tx_date + 4, stCupSucc.tx_date, stCupSucc.trace_no + 6);
				if(iRet != 0 && iRet != DBS_NOTFOUND)
				{
					HtLog(ERR_PARAM, "Tbl_n_txn_Sel err:iRet(%d)(%s), cup_key(%s)", iRet, DbsError(), stCupSucc.cup_key);
					Tbl_cup_succ_Clo_Sel();
					DbsRollback();
					return RC_FAIL;
				}
				else if(RC_SUCC == iRet)
				{
					//	如果在本地并且是成功的交易并且本地该笔交易在第二天	本条不处理, 下周期处理
					if(!memcmp(stNTxn.tx_date, pszStlmDate, 8)) 
						continue;
				}
				*/
				//else	DBS_NOTFOUND 
			}

			memcpy(stCupSucc.result_flag, "1", 1);
			sFlag[0] = STLM_CUP_FLG_CUPS;

			iRet = sql_execute("update tbl_cup_succ set result_flag = '%c', rec_upd_ts = '%s' where rowid = '%s' ", 
				TRADE_TXN_NOTMATCH, szTime, stCupSucc.rowid);
			if(iRet)
			{
				HtLog(ERR_PARAM, "更新tbl_cup_succ失败, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stCupSucc.cup_key);
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}
		}
		else if(iRet == 0)			/* 在本地表中找到记录 **/
		{
			stCupSucc.result_flag[0] = STLM_FLG_OK;

			if(memcmp(stCupSucc.cups_no, "wcht", 4) && memcmp(stCupSucc.cups_no, "alpy", 4 ) && memcmp(stCupSucc.cups_no, "hkub", 4))
			{
				/***改为对前六后四******/
				CommonAllTrim(stCupSucc.ac_no);
				CommonAllTrim(stTxnSucc.ac_no);
				if( memcmp(stCupSucc.ac_no, stTxnSucc.ac_no, 6) != 0 || 
					memcmp(stCupSucc.ac_no + strlen(stCupSucc.ac_no) - 4, stTxnSucc.ac_no + strlen(stTxnSucc.ac_no) - 4, 4) != 0)
				{
  					HtLog(ERR_PARAM, "对账不平, 账号不匹配:(%s)(%s), error!", stCupSucc.ac_no, stTxnSucc.ac_no);
  					memcpy(stCupSucc.result_flag, "1", 1);
					sFlag[0] = STLM_CUP_FLG_ACCT;
 				}
			}

			/***核对金额***/	  	 
			if(fabs(stCupSucc.tx_amt - stTxnSucc.tx_amt) > 0.005)
			{
				HtLog(ERR_PARAM, "对账不平，金额不一致:(%.2f)(%.2f), error!", stCupSucc.tx_amt, stTxnSucc.tx_amt);
				memcpy(stCupSucc.result_flag, "1", 1);
				sFlag[0] = STLM_CUP_FLG_AMT;
			}

			CommonAllTrim(stTxnSucc.ibox42);
			memset(&stMchtInfo, 0, sizeof(stMchtInfo));
			iRet = lGetMchtBase(&stMchtInfo, stTxnSucc.ibox42);
			if(iRet)
			{
				HtLog(ERR_PARAM, "获取商户(%s)信息失败, err:(%d)(%s)", stTxnSucc.ibox42, iRet, sGetError());
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}
			// 0、2、5认为正常需要清分的商户
			if('0' != stMchtInfo.mcht_status[0] && '2' != stMchtInfo.mcht_status[0] && '5' != stMchtInfo.mcht_status[0])		//   非正常延迟清算
				sFlag[1] = '1'; 

			iRet = sql_execute("update tbl_cup_succ set result_flag = '%s', rec_upd_ts = '%s' where rowid = '%s' ", 
				stCupSucc.result_flag, szTime, stCupSucc.rowid);
			if(iRet)
			{
				HtLog(ERR_PARAM, "更新tbl_cup_succ状态失败, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stCupSucc.cup_key);
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}

			iRet = sql_execute("update tbl_txn_succ set result_flag = '%s' where rowid = '%s' ", stCupSucc.result_flag, 
				stTxnSucc.rowid);
			if(iRet)
			{
				HtLog(ERR_PARAM, "更新tbl_txn_succ失败, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stCupSucc.cup_key);
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}

			//对平的交易
			if(STLM_FLG_OK == stCupSucc.result_flag[0] && 0 == stTxnSucc.disc_cycle)
			{
				//	修改风控手工延迟的交易状态，和对商户不正常的交易处理
				iRet = nTSuccTrade(&stTxnSucc, stCupSucc.cup_fee, sFlag[1], pszStlmDate);
				if(iRet)
				{
					HtLog(ERR_PARAM, "处理T0对账平流水失败, err:(%d)Id(%s)", iRet, stTxnSucc.id);
					Tbl_cup_succ_Clo_Sel();
	 				DbsRollback();
	 				return RC_FAIL;	
				}
				continue;
			}
		}

		/*对账平的情况插入到对账明细表*/ 
		if(sFlag[0] != STLM_CUP_FLG_CUPS)
		{
			/* 对账平的情况，将上期本地有，渠道无在错误表中的记录改成参与清算处理,*/
			iRet = nChkStlmErr(stTxnSucc.id, '0', '0', NULL, sFlag, &iDoFlag, pszStlmDate, stCupSucc.cup_fee, 
				stTxnSucc.tx_amt, stCupSucc.tx_amt);
			if(iRet)
			{
				HtLog(ERR_PARAM, "检查记录(%s)失败, err:(%d)", stTxnSucc.id, iRet);
	 			DbsRollback();
				return RC_FAIL;
			}
			/*上期err表中无，或者上期err表中是风控和延迟清算的，succ表需要添加记录行, 对账不平和商户不正常，succ表肯定有记录*/
			if((0 == iDoFlag || 2 == iDoFlag) && (0 != stTxnSucc.disc_cycle))
			{
				//	T+0的交易stlm_succ 已经有了，不能插入succ表中
		   		iRet = StlmSuccDo(stCupSucc, stTxnSucc, pszStlmDate, sFlag);			
		   		if(RC_SUCC != iRet)
		   		{				 
		   			HtLog(ERR_PARAM, "StlmSuccDo Error:iRet(%d).", iRet);
			  		Tbl_cup_succ_Clo_Sel();
				   	DbsRollback(); 
				   	return RC_FAIL;
				} 
			}
		}
		if(sFlag[0] != STLM_FLG_OK || sFlag[1] == '1')   /*不一致的情况插入到对账不平明细表*/
		{
			if(STLM_FLG_OK == sFlag[0] && (1 == iDoFlag || 2 == iDoFlag))   // 防止商户不正常的明细
				HtLog(NML_PARAM, "该笔[%s]交易在上去错误表中，在本地中核对平", stTxnSucc.id);
			else
 			{
				HtLog(NML_PARAM, "对账不平,插入对账不平明细表");
				/**对账金额OR卡号不一样,不平,插入对账明细表**/
				iRet = StlmErrDo(stCupSucc, stTxnSucc, pszStlmDate, sFlag, pszSettleNum); 
				if(RC_SUCC != iRet)
				{				 
					HtLog(ERR_PARAM, "StlmErrDo Error:iRet(%d).", iRet);
					Tbl_cup_succ_Clo_Sel();
					DbsRollback(); 
					return RC_FAIL;
		   		} 
			}
		}

		if(lCount % 1000 == 0) 
		   DbsCommit();
	}	
	DbsCommit();

	lCount = 0;
	/*查找POSP收单成功交易流水表中核对的交易为POSP有，CUPS没有*/	
	iRet = Tbl_txn_succ_Dec_Sel("((tx_date = '%s' and tx_time <= '%s') or tx_date < '%s') and "
		"cups_no = '%s' and result_flag = '%c'", sMaxDate, IBP_SETTLE_TIME, sMaxDate, pszCupsNo, 
		TRADE_TXN_INIT); 
	if(iRet)
	{				 
		HtLog(ERR_PARAM, "查询本地多渠道无流水失败, err:(%d)(%s)", iRet, DbsError());
		return RC_FAIL;
	}

	while(1)
	{
		memset(&stTxnSucc, 0, sizeof(struct tbl_txn_succ_c));
		memset(&stCupSucc, 0, sizeof(struct tbl_cup_succ_c));
		iRet = Tbl_txn_succ_Fet_Sel(&stTxnSucc);
		if(RC_SUCC != iRet && iRet != DBS_NOTFOUND)
		{
			HtLog(ERR_PARAM, "Tbl_txn_succ_Fet_Sel Err:iRet[%d][%s].", iRet, DbsError());
			Tbl_txn_succ_Clo_Sel();
			DbsRollback();
			return RC_FAIL;
		}
		else if(iRet == DBS_NOTFOUND)
		{
			HtLog(NML_PARAM, "本地对流水笔数(%d), 处理本地多渠道无流水完成!", lCount);
			Tbl_txn_succ_Clo_Sel();
			break;
		}

		lCount ++;
		sFlag[0] = STLM_CUP_FLG_POSP;
		sFlag[1] = '0';
 		CommonAllTrim(stTxnSucc.ibox42);
		memset(&stMchtInfo, 0, sizeof(stMchtInfo));
		iRet = lGetMchtBase(&stMchtInfo, stTxnSucc.ibox42);
		if(iRet)
		{
			HtLog(ERR_PARAM, "查找商户(%s)信息失败, err:(%d)(%s)", stTxnSucc.ibox42, iRet, sGetError());
			Tbl_txn_succ_Clo_Sel();
 			DbsRollback(); 
  			return RC_FAIL;
 		}

		if('0' != stMchtInfo.mcht_status[0] && '2' != stMchtInfo.mcht_status[0] && '5' != stMchtInfo.mcht_status[0])
			sFlag[1] = '1'; 

		HtLog(NML_PARAM, "POSP单边账, 商户[%s]状态[%c]", stMchtInfo.mcht_no, stMchtInfo.mcht_status[0]);

		//	非T+0的流程
		if(0 < stTxnSucc.disc_cycle)
		{
			iRet = nChkStlmErr(stTxnSucc.id, '1', '0', NULL, sFlag, &iDoFlag, pszStlmDate, 
				stCupSucc.cup_fee, stTxnSucc.tx_amt, stCupSucc.tx_amt);
			if(iRet)
			{
				HtLog(ERR_PARAM, "nChkStlmErr err:iRet(%d)", iRet);
  				DbsRollback(); 
				return RC_FAIL;
			}
			if(0 == iDoFlag || 2 == iDoFlag)
			{
				/***同时也插入成功表***/
 				iRet = StlmSuccDo(stCupSucc, stTxnSucc, pszStlmDate, sFlag);			
				if(iRet)
  				{				 
					HtLog(ERR_PARAM, "StlmSuccDo error:iRet(%d)", iRet);
					Tbl_txn_succ_Clo_Sel();
 					DbsRollback(); 
 					return RC_FAIL;
				} 
			}
		}
	
		/* 调用StlmErrDo判断更stlm_err表数据, 
		0 - 插入err表  
		1 - (有错误记录) 更新记录  
		2 -(风控延迟) 更新记录
		*/
		iRet = StlmErrDo(stCupSucc,stTxnSucc,pszStlmDate, sFlag, pszSettleNum); 
		if(iRet)
		{				 
			HtLog(ERR_PARAM, "处理差错交易失败, err:(%d).", iRet);
			Tbl_txn_succ_Clo_Sel();
			DbsRollback(); 
			return RC_FAIL;
		} 

		/*更新当日POSP成功交易流水表的对账结果POSP单边帐*/
		iRet = sql_execute("update tbl_txn_succ set result_flag = '%c' where rowid = '%s' ", TRADE_TXN_NOTMATCH, 
			stTxnSucc.rowid);
 		if(iRet)
		{				 
			HtLog(ERR_PARAM, "更新本地交易流水失败, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stTxnSucc.cup_key);
			Tbl_txn_succ_Clo_Sel();
			DbsRollback();
			return RC_FAIL;
		}
	}
	DbsCommit();

	//	针对stlm_err表settle_num字段的处理
	iRet = nSetErrSettleNum(pszSettleNum, pszStlmDate);
	if(iRet)
	{				 
		HtLog(ERR_PARAM, "nSetErrSettleNum err:iRet(%d)", iRet);
   		DbsRollback();
		return RC_FAIL;
	}
	DbsCommit();

	return RC_SUCC;
}
