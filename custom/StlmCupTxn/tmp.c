/*****************************************************************************/
/* PROGRAM NAME: StlmCupTxn.c												   */
/* DESCRIPTIONS: ����														*/
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

double	g_TXAmt = 0.00f;		// ͳ�������ޱ����У��ڵڶ�����ϸ�У�ͳ���������
double	g_CUPFee = 0.00f;
/************************************************************************************/
/*   ����  �� int nCreateChnnAct(char *pszDate)									 */
/*   ����  �� �����������������Ϣ												  */
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
/************************************************************************************/
int nCreateChnnAct(char *pszDate, char *pszCupsNo)
{
	int		iRet = 0, iCount = 0;
	double	dDFee = g_TXAmt - g_CUPFee, dCupAmt = 0.00, dCupFee = 0.00;
	
	DbsBegin();
	HtLog(NML_PARAM, "������(%s)���������������Ϣ!", pszCupsNo);
	iRet = sql_execute("delete from tbl_channel_account where stlm_date = '%s' and cups_no = '%s'", 
		pszDate, pszCupsNo);
	if(iRet)
	{
		HtLog(ERR_PARAM, "��������(%s)������Ϣʧ��, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}
	DbsCommit();		// ��������������

	iCount = sql_count("tbl_stlm_succ", "stlm_date = '%s' and cups_no = '%s' and result_flag > '%c'", 
		pszDate, pszCupsNo, STLM_SUCC_FLG_OK);
	if(0 == iCount)	 return RC_SUCC;

	// ��ֹ�� 4 ��״̬
	iRet = sql_sum_double("tbl_cup_succ", "tx_amt", &dCupAmt, "stlm_date in (select to_char(to_date(trans_date,'yyyymmdd') + 1, 'yyyymmdd') from "
		"tbl_stlm_date where stlm_date = '%s') and cups_no = '%s' and result_flag in ('%c', '%c', '%c')", pszDate, pszCupsNo, TRADE_TXN_INIT, 
		TRADE_TXN_NOTMATCH, TRADE_TXN_SUCCESS);
	if(iRet)
	{
		HtLog(ERR_PARAM, "ͳ������(%s)�˵������ܽ��ʧ��, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}

	iRet = sql_sum_double("tbl_cup_succ", "cup_fee", &dCupFee, "stlm_date in (select to_char(to_date(trans_date,'yyyymmdd') + 1, 'yyyymmdd') from "
		"tbl_stlm_date where stlm_date = '%s') and cups_no = '%s' and result_flag in ('%c', '%c', '%c')", pszDate, pszCupsNo, TRADE_TXN_INIT, 
		TRADE_TXN_NOTMATCH, TRADE_TXN_SUCCESS);
	if(iRet)
	{
		HtLog(ERR_PARAM, "ͳ������(%s)�˵�������������ʧ��, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}
	
	HtLog(NML_PARAM, "����(%s)������(%.2f)������(%.2f), �������������ļ����׽��(%.2f)������(%.2f)", 
		pszCupsNo, g_TXAmt, g_CUPFee, dCupAmt, dCupFee);
	iRet = sql_execute("insert into tbl_channel_account(id,stlm_date,cups_no,cups_nm,tx_amt,cup_fee,in_amt,real_amt,recv_flag,resv_amt1) "	
		"(select stlm_date||cups_no, stlm_date, cups_no, '%s', sum(tx_amt) + %.2f, sum(cup_fee) + %.2f, sum(tx_amt)- sum(cup_fee) + %.2f, "
 		"sum(tx_amt)- sum(cup_fee) + %.2f, '0', %.2f from tbl_stlm_succ where stlm_date = '%s' and cups_no = '%s' and "
		"result_flag = '%c' group by cups_no, stlm_date)", sGetCupsName(pszCupsNo), g_TXAmt, g_CUPFee, dDFee, dDFee, dCupAmt - dCupFee, 
		pszDate, pszCupsNo, STLM_SUCC_FLG_INIT);
	if(iRet)
	{
		HtLog(ERR_PARAM, "��������(%s)������Ϣʧ��, err:(%d)(%s)", pszCupsNo, iRet, DbsError());
		DbsRollback();
		return RC_FAIL;  
	}

	DbsCommit();

	return RC_SUCC;
}

/************************************************************************************/
/*   ��������					   ����������									  */
/************************************************************************************/
/*   ����  �� int main(int argc  , char ** argv )								   */
/*   ����  �� ��ɶ�������														  */
/*   ����  �� argc (�������� -- ����)											   */
/*		 �� argv (�����б� -- ���)											   */
/*			argv[1] --  ��������(��ѡ)											*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
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
	/*����*/
	iRet = StlmCupTxn(szDate, szCupsNo, argv[2]);
	if(iRet)
	{
		HtLog(ERR_PARAM, "����(%s)����ʧ��, err:%d", szCupsNo, iRet);
		DbsDisconnect(); 
		return RC_FAIL;
	}

	/*��������������Ϣ��*/
	iRet = nCreateChnnAct(szDate, szCupsNo);
	if(iRet)
	{
		HtLog(ERR_PARAM, "���ɽ���(%s)����������Ϣ��ʧ��, err:%d", szDate, iRet);
		DbsDisconnect();
		return RC_FAIL;
	}

	DbsDisconnect();		
	HtLog(NML_PARAM, "����(%s)�������!", szCupsNo);
	return RC_SUCC;
}	  

/************************************************************************************/
/*   ����  �� int nGetSettleDate(tbl_cup_succ_c pstCup, tbl_txn_succ_c pstTxn)	  */
/*   ����  �� �������ʲ��� ���ʲ�ƽ��ϸ��ͳɹ���								   */
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
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
	pstStlmErr->cups_amt = pstCup->tx_amt;	//	�������
	pstStlmErr->cup_fee = pstCup->cup_fee;
	memcpy(pstStlmErr->tx_name, pstCup->tx_code, sizeof(pstCup->tx_code)-1);
	memcpy(pstStlmErr->sys_ref_no, pstCup->sys_ref_no, sizeof(pstStlmErr->sys_ref_no)-1);
	strcpy(pstStlmErr->err_desc, "�������б�����");
	
	memset(&stTxnSucc, 0, sizeof(stTxnSucc));
	if(!memcmp(pstCup->cups_no, "alpy", 4) || !memcmp(pstCup->cups_no, "wcht", 4))
	{
		HtLog(NML_PARAM, "��������:��������(%s)������(%s)(%s)", pstCup->tx_date, 
			pstCup->cup_key + 4, pstCup->cups_no);
		iRet = Tbl_txn_succ_Sel(&stTxnSucc, "(tx_sub_code = '31' or tx_sub_code = '51') and "
			"order_no = '%s' and tx_date = '%s' and cups_no = '%s'", pstCup->cup_key + 4, 
			pstCup->tx_date, pstCup->cups_no);
	}
	else
	{
		// ����鲻�� ����date + trace_no  +  ���ǵ�date + trace_no
		HtLog(NML_PARAM, "��������:��������(%s)��ˮ(%s)", pstCup->tx_date, pstCup->trace_no);
		iRet = Tbl_txn_succ_Sel(&stTxnSucc, "(fd13 = '%s' or tx_date = '%s') and fd11 = '%s'", 
			pstCup->tx_date + 4, pstCup->tx_date, pstCup->trace_no + 6);
	}
   	if(RC_SUCC != iRet)
   	{
		HtLog(ERR_PARAM, "��ѯ���ؽ�����ˮʧ��, err:(%d)(%s)", iRet, DbsError());
		return RC_FAIL;
	}

	memcpy(pstStlmErr->id, stTxnSucc.id, sizeof(pstStlmErr->id) - 1);

	memset((char *)&stStlmSucc, 0, sizeof(stStlmSucc));
	pstStlmErr->disc_cycle = stTxnSucc.disc_cycle;   						// ��������
	pstStlmErr->expand = stTxnSucc.expand;
	memcpy(pstStlmErr->brno, stTxnSucc.brno, sizeof(pstStlmErr->brno)-1);	// ҳ���Ϸֻ�����ʾ
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
	pstStlmErr->local_amt = stTxnSucc.tx_amt;		//	���ؽ��(ҳ��չʾ��)
	memcpy(pstStlmErr->tx_sub_code, stTxnSucc.tx_sub_code, sizeof(pstStlmErr->tx_sub_code)-1);
	memcpy(pstStlmErr->fdxx, stTxnSucc.fdxx, sizeof(pstStlmErr->fdxx)-1);
	memcpy(pstStlmErr->fdxxx, stTxnSucc.fdxxx, sizeof(pstStlmErr->fdxxx)-1);

	stStlmSucc.result_flag[0] = STLM_SUCC_FLG_ERR;		//	�в��
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
	stStlmSucc.cup_fee = pstCup->cup_fee;		//	cups���ص� ������

//	memcpy(stStlmSucc.ac_bank_no, stTxnSucc.ac_bank_no, sizeof(stStlmSucc.ac_bank_no)-1);
	memcpy(stStlmSucc.tx_code, stTxnSucc.tx_code, sizeof(stStlmSucc.tx_code)-1);
//	memcpy(stStlmSucc.tx_name, stTxnSucc.tx_name, sizeof(stStlmSucc.tx_name)-1);
	strcpy(stStlmSucc.tx_name, "POS����");
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
	snprintf(stStlmSucc.fdxxx, sizeof(stStlmSucc.fdxxx), "�������б�����, ����[%.2f]", pstCup->tx_amt);
	CommonGetCurrentTime(stStlmSucc.rec_crt_ts);
	CommonGetCurrentTime(stStlmSucc.rec_upd_ts);

	CommonAllTrim(stStlmSucc.ibox42);

	memcpy(stStlmSucc.servetype, stTxnSucc.servetype, sizeof(stStlmSucc.servetype) - 1);
	memcpy(stStlmSucc.promtsmode, stTxnSucc.promtsmode, sizeof(stStlmSucc.promtsmode) - 1);
	stStlmSucc.promtsid = stTxnSucc.promtsid;
	stStlmSucc.promtsvalue = stTxnSucc.promtsvalue;

/*	���˲�ƽ �����̻�״̬�����˹�����
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
	
	//����"������ϸ��"
	iRet = Tbl_stlm_succ_Ins(&stStlmSucc);
	if(iRet)
	{
	   	HtLog(ERR_PARAM, "��¼�����б�������Ϣʧ��, err:(%d)(%s), id(%s)", iRet, DbsError(), stStlmSucc.id);
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int nChkStlmErr(struct tbl_stlm_err_c *pstStlmErr)					*/
/*   ����  �� ��Ա����С������ޣ�������T+1֮��Ž���ϸ��������						*/
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
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
		HtLog(ERR_PARAM, "��ѯ���ײ���ʧ��, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
		return RC_FAIL;
	}

	memset(szTime, 0, sizeof(szTime));
	CommonGetCurrentTime(szTime);
	(*pnDoFlag) = 1;
	HtLog(NML_PARAM, "���ڼ�¼ID[%s]�ñʽ����ڱ��ڴ���", pszId);
	if(ERR_CUP_FLG_RISKCRTL == stStlmErr.err_type[0] || ERR_CUP_FLG_MANUAL == stStlmErr.err_type[0])
	{
		// ״̬2  ����Ҫ�޸�stlm_succ���������ѣ���ʱstlm_succ��δ���ɼ�¼
		(*pnDoFlag) = 2;

		// ��ֹ�쳣���ж���ʽ����ڳɹ������Ƿ���ڣ��п��ܷ���ӳ��Ѿ��Թ�����
		iCount = sql_count("tbl_stlm_succ", "id = '%s'", pszId);
		if(iCount > 0)
			(*pnDoFlag) = 1;	// �ö��˲�ƽ��״̬������2������������

		HtLog(NML_PARAM, "ID[%s]�ñʽ������ڷ�ػ��˹��ӳٽ���", pszId);
		if(STLM_FLG_OK == pszFlag[0])	//	����ƽ, ������  ���˹�����
		{
			if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0] && '1' == pszFlag[1])	// �����˹�ѡ���������Ľ��ײ����̻������� ��غ��ֹ��ӳ���������
			{
				if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])	//	ֻ����������ſ��ܽ���2�ζ��ˣ� �������ڲ����޸�
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, "
						"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
						STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				else
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_date = '%s', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, "
						"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, pszDate, ERR_CUP_MCHT_UNNORMAL, 
						STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
			}
			else	// �˹������δ���� ���������־
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
				HtLog(ERR_PARAM, "�����ֹ��ӳٻ��ز����״̬ʧ��, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
				return RC_FAIL;
			}

			if(1 == *pnDoFlag)	// ����ǰ�ڶ��˲�ƽ�����ڶ���ƽ�Ĵ���
			{
				iRet = sql_execute("update tbl_stlm_succ set cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", dCupFee, szTime, pszId);
				if(iRet)
				{
					HtLog(ERR_PARAM, "�����ֹ��ӳٻ��ز����״̬ʧ��, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
		   		 	return iRet;
				}	
			}
			return RC_SUCC;
		}

		if(!pszErrDesc && STLM_FLG_OK != pszFlag[0])
		{
			HtLog(NML_PARAM, "���˲�ƽ���Գɹ����д��ڸñ�[%s]���ף��ڴ�����д���", pszId);
			return RC_SUCC;
		}

		// �˹�ѡ��1-�����μ����� ��λ�����ڶ��˲�ƽ����'9'-δ����
		if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0])
		{
			HtLog(NML_PARAM, "��صĽ���(%s)���뽻��, ��ƽ״̬(%c)", pszId, stStlmErr.stlm_type[0]);
			//  ����ǶԹ��ʵģ������޸Ķ�������, ����ֻ�б����������޵�����Żᷢ����
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, "
					"cups_amt = %.2f, err_desc = '%s', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, chErrType, chStlmType, dCupFee, dTxnAmt,
					dCupAmt, pszErrDesc, szTime, pszId);
			else
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_date = '%s', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, "
					"cups_amt = %.2f, err_desc = '%s', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, chErrType, pszDate, chStlmType, dCupFee, dTxnAmt,
					dCupAmt, pszErrDesc, szTime, pszId);
		}
		else	//  �� 1-�����μ�����, �����˹�����״̬
		{
			HtLog(NML_PARAM, "���·�صĽ���(%s), ��ƽ״̬(%c)", pszId, stStlmErr.stlm_type[0]);
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])
				iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, err_desc = '%s', "
					"rec_upd_ts = '%s' where id = '%s'", chStlmType, dCupFee, dTxnAmt, dCupAmt, pszErrDesc, szTime, pszId);
			else
				iRet = sql_execute("update tbl_stlm_err set stlm_date = '%s', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
					"err_desc = '%s', rec_upd_ts = '%s' where id = '%s'", pszDate, chStlmType, dCupFee, dTxnAmt, dCupAmt, pszErrDesc, szTime, pszId);
		}
		if(iRet)
		{
			HtLog(ERR_PARAM, "�����ֹ��ӳٻ��ز����״̬ʧ��, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
			return RC_FAIL;
		}
	}
	else
	{
		if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	������
		{
			if(!pszErrDesc && STLM_FLG_OK != pszFlag[0])
			{
				HtLog(NML_PARAM, "���˲�ƽ���Գɹ����д��ڸñ�[%s]���ף��ڴ�����д���", pszId);
				return RC_SUCC;
			}

			// ��9-δ����״̬�����������˹�ѡ���������, �˹�ѡ���1-�����������
			if(STLM_CUP_NOT_DEAL != stStlmErr.result_flag[0] && STLM_CUP_TO_DONE != stStlmErr.result_flag[0])
			{
				//	ת���˹���������״̬
				HtLog(NML_PARAM, "ID[%s]�ñʽ����Ѿ�ת���˹��������˸ñʲ�����, ���±�Ҫ�ֶ�!", pszId);
	 			iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
						"rec_upd_ts = '%s' where id = '%s'", chStlmType, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				if(iRet)
				{
					HtLog(ERR_PARAM, "���²������Ϣ��ʧ��, err:(%d)(%s)", iRet, DbsError());
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
					HtLog(NML_PARAM, "���ڼ�¼ID[%s]�ñʽ����ڱ����Ѷ�ƽ�����̻���������������!", pszId);
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', cup_fee = %.2f, local_amt = %.2f, "
							"cups_amt = %.2f, err_desc = '�Ѷ�ƽ�����̻�������', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
							STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				}
				else
				{
					HtLog(NML_PARAM, "���ڼ�¼ID[%s]�ñʽ����ڱ����Ѷ�ƽ��ֱ�Ӳ�������!", pszId);
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', err_desc = '�Ѷ�ƽ���Զ���������', cup_fee = %.2f, "
							"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_TO_DONE, STLM_FLG_OK_UNNORMAL, dCupFee, dTxnAmt, 
							dCupAmt, szTime, pszId);
				}
			}
	 		else 	//  ���˲�ƽ
			{
				if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0])
				{
					HtLog(NML_PARAM, "���ڼ�¼ID[%s]�ñʽ���,�˹��������㣬�����˲�ƽ", pszId);
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', stlm_type = '%c', err_desc = '%s', cup_fee = %.2f, "
							"local_amt = %.2f, cups_amt = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, chErrType, chStlmType, pszErrDesc, 
							dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				}
				else
				{
					HtLog(NML_PARAM, "���ڼ�¼ID[%s]�ñʽ����ڱ���δ��ƽ���޸Ĵ���״̬", pszId);
					iRet = sql_execute("update tbl_stlm_err set err_type = '%c', stlm_type = '%c', err_desc = '%s', cup_fee = %.2f, local_amt = %.2f, cups_amt = %.2f, "
							"rec_upd_ts = '%s' where id = '%s'", chErrType, chStlmType, pszErrDesc, dCupFee, dTxnAmt, dCupAmt, szTime, pszId);
				}
			}
			if(iRet)
			{
				HtLog(ERR_PARAM, "���²���ױ�����ʧ��, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
				return iRet;
			}
		}
		else	// �������˲�ƽ����	
		{
			//  �������˲�ƽ�����������˹�ѡ��������㣬�����̻��������������ñ�
			if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0] && '1' == pszFlag[1])
			{
				HtLog(NML_PARAM, "���ڼ�¼ID[%s]�ñʽ���,�˹��������㣬 ���̻���������������!", pszId);
				iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', err_desc = '�̻�������, ���ܴ���', "
					"stlm_type = '%c', rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
					STLM_FLG_OK_UNNORMAL, szTime, pszId);
				if(iRet)
				{
					HtLog(ERR_PARAM, "���²���ױ�����ʧ��, err:(%d)(%s), Id(%s)", iRet, DbsError(), pszId);
					return iRet;
				}
			}
		}
	}
	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int StlmErrDo(tbl_cup_succ_c stCup, tbl_txn_succ_c stTxn, 			*/
/*					char *psDate, char *psFalg)										*/
/*   ����  �� ���� ���ʲ�ƽ��ϸ��												   */
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
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

	stStlmErr.disc_cycle = stTxn.disc_cycle;   // T+0 �����ֶ�
	//	Ŀǰֻ��дT+N�Ľ��׵�settle_num, T+0�������ID��algo��֪
	if(stStlmErr.disc_cycle > 0)
		memcpy(stStlmErr.settle_num, pszSettleNum, 4);

	if(0 == stTxn.disc_cycle)
	{
		HtLog(NML_PARAM, "�����б����޴����߼�");
		iRet = nTStlmErrHandle(&stStlmErr, &stCup, &stTxn, psDate, psFlag, pszSettleNum);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "����T+0���������б����޳���, err:(%d)", iRet);
			return RC_FAIL;
		}
		return RC_SUCC;
	}

//	�öο���ѡ��ȥ��
	HtLog(NML_PARAM, "����T+N����״���");
	if(psFlag[0] == STLM_CUP_FLG_CUPS)	//   ��������
	{ 
		/* �������� �ɹ���ʹ������������*/
		iRet = nGetSettleDate(&stStlmErr, &stCup);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "��ȡ�������ʶ�Ӧ�ı�����ˮ����, err:(%d)", iRet);
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
		stStlmErr.tx_amt = stCup.tx_amt;		// ���ڽ�ͬ������Ϊ׼ �� ������������
		stStlmErr.local_amt = stTxn.tx_amt; 	// ���ؽ��
		stStlmErr.cups_amt = stCup.tx_amt; 		// �������
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
			strcpy(stStlmErr.err_desc, "�������ޱ�����");
			strcpy(stStlmErr.tx_name, "POS����");
		}
		else if(psFlag[0] ==  STLM_CUP_FLG_ACCT)
			sprintf(stStlmErr.err_desc, "�ʻ���һ��:����%s", stTxn.ac_no);
		else if(psFlag[0] == STLM_CUP_FLG_AMT)
			sprintf(stStlmErr.err_desc, "��һ��,���ؽ��[%.2f]", stTxn.tx_amt);
		else if(STLM_FLG_OK_UNNORMAL == psFlag[0])
			sprintf(stStlmErr.err_desc, "��ˮ�Ѷ�ƽ");

		//	��ƽ���̻������� ����־��4, ��������࣬����������ȵȼ������̻�������
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

	//����"���ʲ�ƽ��ϸ��"
	iRet = Tbl_stlm_err_Ins(&stStlmErr);
	if(iRet)
	{
	   	HtLog(ERR_PARAM, "���������ʧ��, err:iRet(%d)(%s),id(%s)", iRet, DbsError(), stStlmErr.id);
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int StlmSuccDo(tbl_cup_succ_c stCup, tbl_txn_succ_c stTxn, 			*/
/*					char *psDate, char *psFalg)										*/
/*   ����  �� ���� ����ƽ��ϸ��													 */
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
/************************************************************************************/
int StlmSuccDo(struct tbl_cup_succ_c stCup ,struct tbl_txn_succ_c stTxn ,char *psDate, char *psFlag)
{
	int		iRet = 0;
	struct tbl_stlm_succ_c stStlmSucc;
	 
	memset((char *)&stStlmSucc ,0x00, sizeof(struct tbl_stlm_succ_c));

	if(psFlag[0] == '0')
		stStlmSucc.result_flag[0] = STLM_SUCC_FLG_INIT;			//	δ���
	else
		stStlmSucc.result_flag[0] = STLM_SUCC_FLG_ERR;			//	���˲�ƽ 

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
	stStlmSucc.cup_fee = stCup.cup_fee;		//	cups���ص� ������

	memcpy(stStlmSucc.tx_code, stTxn.tx_code, sizeof(stStlmSucc.tx_code)-1);
	strcpy(stStlmSucc.tx_name, "POS����");
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
		snprintf(stStlmSucc.fdxxx, sizeof(stStlmSucc.fdxxx), "���ؽ��[%.2f]�������[%.2f],������Ϊ׼", 
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
	
	//����"������ϸ��"
	iRet = Tbl_stlm_succ_Ins(&stStlmSucc);
	if(iRet)
	{
	   	HtLog(ERR_PARAM, "�������˳ɹ���Ϣʧ��, err:(%d)(%s),Id(%s)", iRet, DbsError(), stStlmSucc.id);
		return RC_FAIL;
	}
	
	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int nGetFeeAmt(char *pszDisc, double dTxAmt, double *pdFeeAmt)		*/
/*   ����  �� T+0����������															*/
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
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
		HtLog(ERR_PARAM, "��ȡ����ģ���б�ʧ�� err:(%d)(%s)", iRet, sGetError());
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	iRet = lGetAlgoModel(pstAlgo, lCount, dTxAmt, &stAlgo);
	if(iRet)
	{
		IBPFree(pstAlgo);
		HtLog(ERR_PARAM, "��ģ���б�(%d)�л�ȡ���(%.2f)��Ӧ����ʧ��, err:(%d)(%s)", lCount, dTxAmt, 
			iRet, sGetError());
		return RC_FAIL;
	}
	IBPFree(pstAlgo);

	if(stAlgo.flag == DISC_ALGO_FIXED)  /*** ����(�̶�)�շ�**/
		*pdFeeAmt = stAlgo.fee_value;
	else if(stAlgo.flag == DISC_ALGO_RATE)   /*** �����շ�**/
	{
		dFeeAmt = dTxAmt * stAlgo.fee_value / 100;
		if(stAlgo.min_fee > 0)		   //  ȡ��С�������ķ���
  			dFeeAmt = (stAlgo.min_fee >= dFeeAmt) ? stAlgo.min_fee : dFeeAmt;
		if(stAlgo.max_fee > 0)
			dFeeAmt = (stAlgo.max_fee <= dFeeAmt) ? stAlgo.max_fee : dFeeAmt;
  		*pdFeeAmt = dFeeAmt;
	}
	else if(stAlgo.flag > DISC_ALGO_FIXED)	//	  �̶��շ�
  		*pdFeeAmt = 0.00;

	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int nTFailTrade(struct tbl_stlm_err_c *pstStlmErr, char chState)		*/
/*   ����  �� T+0���˴������														*/
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
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
		HtLog(ERR_PARAM, "��ѯ��ֱ�(%s)����, err:(%d)(%s),id(%s)", pstStlmErr->id, 
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
			HtLog(ERR_PARAM, "�������������������ѳ���, err:(%d)id(%s)", iRet, 
				pstStlmErr->id);
			return RC_FAIL;
		}
	}

	memset(&stErr, 0, sizeof(stErr));
	iRet = Tbl_stlm_err_Sel(&stErr, "id = '%s'", pstStlmErr->id);
	if(iRet && DBS_NOTFOUND != iRet)
	{
		HtLog(ERR_PARAM, "��ѯ�����Ϣʧ��, err:(%d)(%s)id(%s)", iRet, DbsError(), 
			pstStlmErr->id);
		return RC_FAIL;
	}
	else if(DBS_NOTFOUND == iRet)	// �������˲�ƽ��¼
	{
		//	T+N �����У������޵���ˮ�ߵ����
		HtLog(WAR_PARAM, "����(%d)�����в��, Id(%s)", pstStlmErr->disc_cycle, pstStlmErr->id);
		iRet = Tbl_stlm_err_Ins(pstStlmErr);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "���������ˮ����, err:(%d)(%s),ID(%s)", iRet, DbsError(), 
				pstStlmErr->id);
			return RC_FAIL;
		}
		return RC_SUCC;
	}
//	else	find it

	//	���˲�ƽ����ϸ�����߼�
	if(ERR_CUP_FLG_RISKCRTL == stErr.err_type[0] || ERR_CUP_FLG_MANUAL == stErr.err_type[0])
	{
		//	�ñʽ������δ������߲������������֡������μ�����
		if(STLM_CUP_TO_DONE == stErr.result_flag[0] || STLM_CUP_SETTLE_SUCC == stErr.result_flag[0] ||
			STLM_CUP_NOT_DEAL == stErr.result_flag[0])	// ׼����ֻ����Ѿ����
		{
			HtLog(NML_PARAM, "T+0�����ӳٱ�־(%c), �����־(%c)��״̬����, Id(%s)", 
				stErr.err_type[0], stErr.result_flag[0], pstStlmErr->id);
			if(STLM_CUP_FLG_POSP == stErr.stlm_type[0])	 //  �Ѿ��Թ��ˣ������޸Ķ�������
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
		else	// �˹����� ����ָ����״̬)
		{
			HtLog(NML_PARAM, "T+0�����ӳٱ�־(%c), �˹��Ѵ���, �����־(%c)�����¶��˱�ʶ, id(%s)", 
				stErr.err_type[0], stErr.result_flag[0], pstStlmErr->id);
			if(STLM_CUP_FLG_POSP == stErr.stlm_type[0])	 //  �Ѿ��Թ��ˣ������޸Ķ�������
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
		HtLog(NML_PARAM, "T+0�����̻�������,���¶��˱�ʶ, Id(%s)", pstStlmErr->id);
		if(STLM_CUP_FLG_POSP == stErr.stlm_type[0])	 //  �Ѿ��Թ��ˣ������޸Ķ�������
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
	else	//	���˲�ƽ�Ľ���
	{
		HtLog(NML_PARAM, "T+0���ڶ��˲�ƽ���ױ�ʶ(%c), �����־(%c)��״̬����, Id(%s)", stErr.stlm_type[0], 
			stErr.result_flag[0], pstStlmErr->id);
		
		//	�ߵ�����϶��Ѿ��Թ�����
		//	�ñʽ������δ������߲������������֡������μ�����
		if(STLM_CUP_TO_DONE == stErr.result_flag[0] || STLM_CUP_SETTLE_SUCC == stErr.result_flag[0] ||
			STLM_CUP_NOT_DEAL == stErr.result_flag[0])	// ׼����ֻ����Ѿ����
		{
			iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', cups_amt = %.2f, "
				"fee_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, 
				pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, 
				pstStlmErr->id);
		}
		else	// �˹����� ����ָ����״̬)
		{
			iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cups_amt = %.2f, fee_amt = %.2f, "
				"cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", pstStlmErr->stlm_type[0], pstStlmErr->cups_amt, 
				pstStlmErr->fee_amt, pstStlmErr->cup_fee, szTime, pstStlmErr->id);
		}
	}
	if(iRet)
	{
		HtLog(ERR_PARAM, "���²���ױ���ϸ����, err:(%d)(%s), id(%s)", iRet, DbsError(), pstStlmErr->id);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int nTSuccTrade(tbl_txn_succ_c *pstTxnSucc, char chState)			 */
/*   ����  �� T+0����ƽ���׵Ĵ���												   */
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
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
		HtLog(ERR_PARAM, "��ѯ��ֱ�ʧ��, err:(%d)(%s), id(%s)", iRet, DbsError(), pstTxnSucc->id);
		return RC_FAIL;
	}
	else if(0 == iChkAlgo)	//	����algo������
	{
		HtLog(NML_PARAM, "������ֱ��������ֶ�, id(%s)", pstTxnSucc->id);
		iRet = sql_execute("update tbl_algo_dtl set cup_fee = %.2f, other_fee = fee_amt - %.2f, "
			"rec_upd_ts = '%s' where id = '%s'", dCupFee, dCupFee, szTime, pstTxnSucc->id);
		if(iRet)
		{
			HtLog(ERR_PARAM, "��T+0���׸�����ֱ��Ҫ�ֶ�ʧ��, err:(%d)(%s), id(%s)", iRet, DbsError(), pstTxnSucc->id);
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
		HtLog(NML_PARAM, "T+0��ػ��ֹ��ӳ�����Ľ���");
		//	�˹�ѡ��������㣬˵���ñʿ϶���δ����, �����̻���ʱ������
		if(STLM_CUP_TO_DONE == stStlmErr.result_flag[0] && '1' == chState)	
		{
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	�Ѿ��Թ��ˣ������޸Ķ�������
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
		else	//	�������㵫�̻������������˹����� �����Ѿ�������
		{
			if(0 == iChkAlgo)   //  algo�����
			{
				if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	�Ѿ��Թ��ˣ������޸Ķ�������
				{
					if(STLM_CUP_NOT_DEAL == stStlmErr.stlm_type[0])	//	��δ����
						iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', stlm_type = '%c', "
							"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
							STLM_CUP_SETTLE_SUCC, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, 
							szTime, pstTxnSucc->id);
					else	//	�˹������˻����Ѿ�����ˣ� �������� 1����������״̬
						iRet = sql_execute("update tbl_stlm_err set stlm_type = '%c', cups_amt = %.2f, "
							"cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_FLG_OK_UNNORMAL, 
							pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				}
				else
				{
					if(STLM_CUP_NOT_DEAL == stStlmErr.stlm_type[0]) //  ��δ����
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
			else	//	algo �����ڸü�¼
			{
				if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])		//	�Ѿ��Թ��ˣ������޸Ķ�������
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
	else if(ERR_CUP_MCHT_UNNORMAL == stStlmErr.err_type[0])	//	ҳ���ϲ��ܶ��̻��������Ľ��д���
	{
		HtLog(NML_PARAM, "T+0�̻��������ӳ�����Ľ���");
		if(0 == iChkAlgo)	//	algo�����
		{
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])	 //  �Ѿ��Թ��ˣ������޸Ķ�������
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
			if(STLM_CUP_FLG_POSP == stStlmErr.stlm_type[0])	 //  �Ѿ��Թ��ˣ������޸Ķ�������
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
		// �˹������Ѿ�ѡ������, �Ѿ����˵Ĳ����޸Ķ�������
		if(STLM_CUP_NOT_DEAL == stStlmErr.result_flag[0] || STLM_CUP_TO_DONE == stStlmErr.result_flag[0])	// �ñ��˹���δ����
		{
			if(DBS_NOTFOUND == iChkAlgo)	// δ�ҵ�����ƽ����������
			{
				if('1' == chState)
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_type = '%c', "
						"err_desc = '�Ѷ�ƽ���̻�������', stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, "
						"rec_upd_ts = '%s' where id = '%s'", STLM_CUP_NOT_DEAL, ERR_CUP_MCHT_UNNORMAL, 
						STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				else
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_desc = '�Ѷ�ƽ���Զ���������', "
						"stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
						STLM_CUP_TO_DONE, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
			}
			else	// Alog����ڼ�¼, �Ѿ���ƽ�Ľ��ף�ֱ�Ӹĵ�����״̬ 0����������������
			{
				//	������������� 1����������״̬ (alog�м�¼����ѡ1״̬), ���ﻹ���ж�һ��
				if(STLM_CUP_NOT_DEAL == stStlmErr.result_flag[0])
					iRet = sql_execute("update tbl_stlm_err set result_flag = '%c', err_desc = '��ˮ�Ѷ�ƽ', "
						"stlm_type = '%c', cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", 
						STLM_CUP_SETTLE_SUCC, STLM_FLG_OK_UNNORMAL, pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
				else
					HtLog(ERR_PARAM, "ҳ�洦����󣬾��洦��!");
			}
		}
		else	// �ñ��˹��Ѿ����� ��������״̬	�����޸Ķ�������
		{
			iRet = sql_execute("update tbl_stlm_err set err_desc = '��ˮ�Ѷ�ƽ', stlm_type = '%c', "
				"cups_amt = %.2f, cup_fee = %.2f, rec_upd_ts = '%s' where id = '%s'", STLM_FLG_OK_UNNORMAL, 
				pstTxnSucc->tx_amt, dCupFee, szTime, pstTxnSucc->id);
		}
	}
	if(iRet)
	{
		HtLog(ERR_PARAM, "���²���ױ����, err:(%d)(%s)ID(%s)", iRet, DbsError(), pstTxnSucc->id);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int nTStlmErrHandle(struct tbl_cup_succ_c *pstCup,					*/ 
/*				struct tbl_txn_succ_c *pstTxnSucc, char *pszDate, char *pszFlag)	*/
/*   ����  �� T+0����ƽ���׵Ĵ���												   */
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
/************************************************************************************/
int		nTStlmErrHandle(struct tbl_stlm_err_c *pstStlmErr, struct tbl_cup_succ_c *pstCup, 
			struct tbl_txn_succ_c *pstTxnSucc, char *pszDate, char *pszFlag, char *pszSettleNum)
{
	int		iRet = 0;
	
	if(pszFlag[0] == STLM_CUP_FLG_CUPS)	//   ��������
	{ 
		/* �������� �ɹ���ʹ������������*/
		iRet = nGetSettleDate(pstStlmErr, pstCup);
		if(iRet)
		{
	   		HtLog(ERR_PARAM, "��ȡ�������ʶ�Ӧ�ı�����ˮ����, err:(%d)", iRet);
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
		pstStlmErr->tx_amt = pstCup->tx_amt;			// ���ڽ�ͬ������Ϊ׼ �� ������������
		pstStlmErr->local_amt = pstTxnSucc->tx_amt; 	// ���ؽ��
		pstStlmErr->cups_amt = pstCup->tx_amt; 			// �������
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
   			strcpy(pstStlmErr->err_desc, "�������ޱ�����");
   			strcpy(pstStlmErr->tx_name, "POS����");
   		}
   		else if(pszFlag[0] == STLM_CUP_FLG_ACCT)
   			sprintf(pstStlmErr->err_desc, "�ʻ���һ��:����%s", pstTxnSucc->ac_no);
   		else if(pszFlag[0] == STLM_CUP_FLG_AMT)
   			sprintf(pstStlmErr->err_desc, "��һ��,���ؽ��[%.2f]", pstTxnSucc->tx_amt);
		else if(STLM_FLG_OK_UNNORMAL == pszFlag[0])
   			sprintf(pstStlmErr->err_desc, "��ˮ�Ѷ�ƽ");
	
//	T+0 �����ж��̻�״̬������
//	if(pszFlag[1] == '1')	// �̻�������
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
		HtLog(ERR_PARAM, "�������׳���, err:(%d)(%s)", iRet, pstStlmErr->id);
		return RC_FAIL;
	}

	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int nSetErrSettleNum(char *pszSettleNum, char *pstStlmData)			*/
/*   ����  �� �޸�stlm_err���е�settle_num�ֶ�										*/
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
/************************************************************************************/
int		nSetErrSettleNum(char *pszSettleNum, char *pstStlmData)
{

	int		iRet = 0, lCount = 0;
	struct	tbl_stlm_err_c	stStlmErr;	
	struct	tbl_algo_dtl_c	stAlgoDtl;	

	iRet = Tbl_stlm_err_Dec_Sel("stlm_date = '%s' and disc_cycle = 0 and settle_num is null", pstStlmData);
	if(iRet)
	{
		HtLog(ERR_PARAM, "��ѯ�������Ϣ��ʧ��, err:(%d)(%s)", iRet, DbsError());
		return RC_FAIL;
	}

	while(1)
	{
		memset(&stStlmErr, 0, sizeof(stStlmErr));
		iRet = Tbl_stlm_err_Fet_Sel(&stStlmErr);
		if(RC_SUCC != iRet && iRet != DBS_NOTFOUND)
		{
			HtLog(ERR_PARAM, "��ѯ�������Ϣ��ʧ��, err:(%d)(%s)", iRet, DbsError());
			Tbl_stlm_err_Clo_Sel();
			DbsRollback();
			return RC_FAIL;
		}
		else if(iRet == DBS_NOTFOUND)
		{
			HtLog(NML_PARAM, "�޸Ĳ���ױ���T + 0��¼����������!");
			break;
		}

		CommonAllTrim(stStlmErr.id);
		memset(&stAlgoDtl, 0, sizeof(stAlgoDtl));
		iRet = Tbl_algo_dtl_Sel(&stAlgoDtl, "id = '%s'", stStlmErr.id);
		if(iRet && DBS_NOTFOUND != iRet)
		{
			HtLog(ERR_PARAM, "��ѯ��ֱ�ʧ��, err:(%d)(%s), id(%d)", iRet, DbsError(), stStlmErr.id);
			return RC_FAIL;
		}
		else if(DBS_NOTFOUND == iRet)		//	û�ҵ�˵���ñʻ�δ���㣬��Ĭ��TN00
			strcpy(stStlmErr.settle_num, "TN00");
		else
			memcpy(stStlmErr.settle_num, stAlgoDtl.settle_num, sizeof(stStlmErr.settle_num) - 1);

		iRet = sql_execute("update tbl_stlm_err set settle_num = '%s' where id = '%s'", stStlmErr.settle_num, stStlmErr.id);
		if(iRet)
		{
			HtLog(ERR_PARAM, "���²���ױ���T + 0��¼�������ʧ��, err:(%d)(%s), id(%d)", iRet, DbsError(), stStlmErr.id);
			return RC_FAIL;
		}
	}
	Tbl_stlm_err_Clo_Sel();

	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int StlmCupTxn(char *psDate)										  */
/*   ����  �� ������������														  */
/*   ����  �� ������																*/
/*		 �� �����																*/
/*  ����ֵ �� 0--  �ɹ�															 */
/*		   -1--  ʧ��															 */
/************************************************************************************/
/*******������������**********/
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
	/*��ȡ������������ǰ�����ն�Ӧ�ļ�¼�״ζ���*/
	iRet = Tbl_cup_succ_Dec_Sel("stlm_date <= '%s' and cups_no = '%s' and result_flag = '%c' "
		"and tx_date < '%s' order by tx_date, tx_time ", pszStlmDate, pszCupsNo, TRADE_TXN_INIT, 
		pszStlmDate);
	if(iRet)
	{
		HtLog(ERR_PARAM, "��ѯ������ˮʧ��, err:(%d)(%s)", iRet, DbsError());
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
			HtLog(ERR_PARAM, "��ȡ������ˮʧ��, err:(%d)(%s)", iRet, DbsError());
			Tbl_cup_succ_Clo_Sel();
			DbsRollback();
			return RC_FAIL;
		}
		else if(iRet == DBS_NOTFOUND)
		{
			HtLog(NML_PARAM, "����������ˮ�˶Ա��ش������!");
			Tbl_cup_succ_Clo_Sel();
			break;
		}
		
		lCount ++;
		CommonGetCurrentTime(szTime);
		CommonAllTrim(stCupSucc.cup_key);
		//��ѯ�յ��ɹ�������ˮ���н���
		memset(&stTxnSucc, 00, sizeof(struct tbl_txn_succ_c));
		iRet = Tbl_txn_succ_Sel(&stTxnSucc, "cup_key = '%s' ", stCupSucc.cup_key);
		if(iRet != 0 && iRet != DBS_NOTFOUND)
		{
			HtLog(ERR_PARAM, "����Tbl_txn_succ��ϸʧ��, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), 
				stCupSucc.cup_key);
			Tbl_cup_succ_Clo_Sel();
			DbsRollback();
			return RC_FAIL; 
		}
		else if(iRet == DBS_NOTFOUND)	/*�������б�����*/
		{
			HtLog(NML_PARAM, "�������б�����(%s)", stCupSucc.cup_key);
			//	���ñʽ����Ƿ��ڱ��ؿ�����
			//	������������ʱ����23000�������������ʣ����ǿ��ܿ��죬�п��ܱ��ڲ�����
			if(0 < memcmp(stCupSucc.tx_time, IBP_SETTLE_TIME, 6) && !memcmp(stCupSucc.tx_date, sMaxDate, 8))
			{
				continue;
				
				/*tlb_n_txnĿǰ�����������У�txn_sussҲû���յĽ��ף���˶������ֽ���ֻ�ܵڶ������*/
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
					//	����ڱ��ز����ǳɹ��Ľ��ײ��ұ��ظñʽ����ڵڶ���	����������, �����ڴ���
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
				HtLog(ERR_PARAM, "����tbl_cup_succʧ��, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stCupSucc.cup_key);
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}
		}
		else if(iRet == 0)			/* �ڱ��ر����ҵ���¼ **/
		{
			stCupSucc.result_flag[0] = STLM_FLG_OK;

			if(memcmp(stCupSucc.cups_no, "wcht", 4) && memcmp(stCupSucc.cups_no, "alpy", 4 ) && memcmp(stCupSucc.cups_no, "hkub", 4))
			{
				/***��Ϊ��ǰ������******/
				CommonAllTrim(stCupSucc.ac_no);
				CommonAllTrim(stTxnSucc.ac_no);
				if( memcmp(stCupSucc.ac_no, stTxnSucc.ac_no, 6) != 0 || 
					memcmp(stCupSucc.ac_no + strlen(stCupSucc.ac_no) - 4, stTxnSucc.ac_no + strlen(stTxnSucc.ac_no) - 4, 4) != 0)
				{
  					HtLog(ERR_PARAM, "���˲�ƽ, �˺Ų�ƥ��:(%s)(%s), error!", stCupSucc.ac_no, stTxnSucc.ac_no);
  					memcpy(stCupSucc.result_flag, "1", 1);
					sFlag[0] = STLM_CUP_FLG_ACCT;
 				}
			}

			/***�˶Խ��***/	  	 
			if(fabs(stCupSucc.tx_amt - stTxnSucc.tx_amt) > 0.005)
			{
				HtLog(ERR_PARAM, "���˲�ƽ����һ��:(%.2f)(%.2f), error!", stCupSucc.tx_amt, stTxnSucc.tx_amt);
				memcpy(stCupSucc.result_flag, "1", 1);
				sFlag[0] = STLM_CUP_FLG_AMT;
			}

			CommonAllTrim(stTxnSucc.ibox42);
			memset(&stMchtInfo, 0, sizeof(stMchtInfo));
			iRet = lGetMchtBase(&stMchtInfo, stTxnSucc.ibox42);
			if(iRet)
			{
				HtLog(ERR_PARAM, "��ȡ�̻�(%s)��Ϣʧ��, err:(%d)(%s)", stTxnSucc.ibox42, iRet, sGetError());
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}
			// 0��2��5��Ϊ������Ҫ��ֵ��̻�
			if('0' != stMchtInfo.mcht_status[0] && '2' != stMchtInfo.mcht_status[0] && '5' != stMchtInfo.mcht_status[0])		//   �������ӳ�����
				sFlag[1] = '1'; 

			iRet = sql_execute("update tbl_cup_succ set result_flag = '%s', rec_upd_ts = '%s' where rowid = '%s' ", 
				stCupSucc.result_flag, szTime, stCupSucc.rowid);
			if(iRet)
			{
				HtLog(ERR_PARAM, "����tbl_cup_succ״̬ʧ��, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stCupSucc.cup_key);
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}

			iRet = sql_execute("update tbl_txn_succ set result_flag = '%s' where rowid = '%s' ", stCupSucc.result_flag, 
				stTxnSucc.rowid);
			if(iRet)
			{
				HtLog(ERR_PARAM, "����tbl_txn_succʧ��, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stCupSucc.cup_key);
				Tbl_cup_succ_Clo_Sel();
				DbsRollback(); 
				return RC_FAIL;
			}

			//��ƽ�Ľ���
			if(STLM_FLG_OK == stCupSucc.result_flag[0] && 0 == stTxnSucc.disc_cycle)
			{
				//	�޸ķ���ֹ��ӳٵĽ���״̬���Ͷ��̻��������Ľ��״���
				iRet = nTSuccTrade(&stTxnSucc, stCupSucc.cup_fee, sFlag[1], pszStlmDate);
				if(iRet)
				{
					HtLog(ERR_PARAM, "����T0����ƽ��ˮʧ��, err:(%d)Id(%s)", iRet, stTxnSucc.id);
					Tbl_cup_succ_Clo_Sel();
	 				DbsRollback();
	 				return RC_FAIL;	
				}
				continue;
			}
		}

		/*����ƽ��������뵽������ϸ��*/ 
		if(sFlag[0] != STLM_CUP_FLG_CUPS)
		{
			/* ����ƽ������������ڱ����У��������ڴ�����еļ�¼�ĳɲ������㴦��,*/
			iRet = nChkStlmErr(stTxnSucc.id, '0', '0', NULL, sFlag, &iDoFlag, pszStlmDate, stCupSucc.cup_fee, 
				stTxnSucc.tx_amt, stCupSucc.tx_amt);
			if(iRet)
			{
				HtLog(ERR_PARAM, "����¼(%s)ʧ��, err:(%d)", stTxnSucc.id, iRet);
	 			DbsRollback();
				return RC_FAIL;
			}
			/*����err�����ޣ���������err�����Ƿ�غ��ӳ�����ģ�succ����Ҫ��Ӽ�¼��, ���˲�ƽ���̻���������succ��϶��м�¼*/
			if((0 == iDoFlag || 2 == iDoFlag) && (0 != stTxnSucc.disc_cycle))
			{
				//	T+0�Ľ���stlm_succ �Ѿ����ˣ����ܲ���succ����
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
		if(sFlag[0] != STLM_FLG_OK || sFlag[1] == '1')   /*��һ�µ�������뵽���˲�ƽ��ϸ��*/
		{
			if(STLM_FLG_OK == sFlag[0] && (1 == iDoFlag || 2 == iDoFlag))   // ��ֹ�̻�����������ϸ
				HtLog(NML_PARAM, "�ñ�[%s]��������ȥ������У��ڱ����к˶�ƽ", stTxnSucc.id);
			else
 			{
				HtLog(NML_PARAM, "���˲�ƽ,������˲�ƽ��ϸ��");
				/**���˽��OR���Ų�һ��,��ƽ,���������ϸ��**/
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
	/*����POSP�յ��ɹ�������ˮ���к˶ԵĽ���ΪPOSP�У�CUPSû��*/	
	iRet = Tbl_txn_succ_Dec_Sel("((tx_date = '%s' and tx_time <= '%s') or tx_date < '%s') and "
		"cups_no = '%s' and result_flag = '%c'", sMaxDate, IBP_SETTLE_TIME, sMaxDate, pszCupsNo, 
		TRADE_TXN_INIT); 
	if(iRet)
	{				 
		HtLog(ERR_PARAM, "��ѯ���ض���������ˮʧ��, err:(%d)(%s)", iRet, DbsError());
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
			HtLog(NML_PARAM, "���ض���ˮ����(%d), �����ض���������ˮ���!", lCount);
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
			HtLog(ERR_PARAM, "�����̻�(%s)��Ϣʧ��, err:(%d)(%s)", stTxnSucc.ibox42, iRet, sGetError());
			Tbl_txn_succ_Clo_Sel();
 			DbsRollback(); 
  			return RC_FAIL;
 		}

		if('0' != stMchtInfo.mcht_status[0] && '2' != stMchtInfo.mcht_status[0] && '5' != stMchtInfo.mcht_status[0])
			sFlag[1] = '1'; 

		HtLog(NML_PARAM, "POSP������, �̻�[%s]״̬[%c]", stMchtInfo.mcht_no, stMchtInfo.mcht_status[0]);

		//	��T+0������
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
				/***ͬʱҲ����ɹ���***/
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
	
		/* ����StlmErrDo�жϸ�stlm_err������, 
		0 - ����err��  
		1 - (�д����¼) ���¼�¼  
		2 -(����ӳ�) ���¼�¼
		*/
		iRet = StlmErrDo(stCupSucc,stTxnSucc,pszStlmDate, sFlag, pszSettleNum); 
		if(iRet)
		{				 
			HtLog(ERR_PARAM, "��������ʧ��, err:(%d).", iRet);
			Tbl_txn_succ_Clo_Sel();
			DbsRollback(); 
			return RC_FAIL;
		} 

		/*���µ���POSP�ɹ�������ˮ��Ķ��˽��POSP������*/
		iRet = sql_execute("update tbl_txn_succ set result_flag = '%c' where rowid = '%s' ", TRADE_TXN_NOTMATCH, 
			stTxnSucc.rowid);
 		if(iRet)
		{				 
			HtLog(ERR_PARAM, "���±��ؽ�����ˮʧ��, err:(%d)(%s), cup_key(%s)", iRet, DbsError(), stTxnSucc.cup_key);
			Tbl_txn_succ_Clo_Sel();
			DbsRollback();
			return RC_FAIL;
		}
	}
	DbsCommit();

	//	���stlm_err��settle_num�ֶεĴ���
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
