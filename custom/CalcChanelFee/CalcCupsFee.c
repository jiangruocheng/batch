/*****************************************************************
 *  ����: ��������������(�̻��ʽ����)
 *	Author��DeffPuzzL
 *  Edit History:
 ****************************************************************/
#include "Ibp.h"
#include "pos_stlm.h"
#include "DbsApi.h"
#include "load.h"

#include <assert.h>

#define	WFJP_SERVICE_RATE		0.02
#define ONLINE_FIX_RATE	  	0.6

/************************************************************************************/
/*   ����  �� double dGetAlgoFee(dbDiscAlgo *pstAlgo, double dTxAmt)				*/
/*   ����  �� ����Ӧ���̻�������													*/
/*  ����ֵ �� RC_SUCC	   --  �ɹ�												*/
/*			RC_FAIL	   --  ʧ��												*/
/************************************************************************************/
double  dGetAlgoFee(dbDiscAlgo *pstAlgo, double dTxAmt)
{
	double	dFee = 0.00, dCalFee = 0.00;
	
	//�̶��շ�
	if(pstAlgo->flag == DISC_ALGO_FIXED)
	{
		dCalFee = pstAlgo->fee_value;
	}
	//�����շ�,�б��ף��ⶥ
	else if(pstAlgo->flag == DISC_ALGO_RATE)	
	{
		dFee = dTxAmt * pstAlgo->fee_value / 100;
		if(pstAlgo->min_fee > 0) 
		{
			dFee = (pstAlgo->min_fee >= dFee) ? pstAlgo->min_fee : dFee;
		}
		if(pstAlgo->max_fee > 0)
		{
			dFee = (pstAlgo->max_fee <= dFee) ? pstAlgo->max_fee : dFee;
		}

		dCalFee = dFee;
	}
	//������ʽδ����,Ĭ��Ϊ0
	else if(pstAlgo->flag > DISC_ALGO_FIXED)	
	{
		dCalFee = 0.00;
	}

	return dCalFee;
}

/************************************************************************************/
/*   ����  �� int nCalcCupsFee((TStlmSucc *pstSucc, double *pdFee)					*/
/*   ����  �� ������������ͨ���ɱ�													*/
/*  ����ֵ �� RC_SUCC		--	�ɹ�												*/ 
/*			  RC_FAIL		--	ʧ��												*/
/************************************************************************************/
int		nCalcCupsFee(TStlmSucc *pstSucc, double *pdFee, double totalAmt)
{
	int		iRet = RC_SUCC;

	if(memcmp(pstSucc->cups_no, "wfjp", 4) == 0)
		iRet = nCalculateWfjp(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "wfjn", 4) == 0)
		iRet = nCalculateWfjp(pstSucc, pdFee);
    else if(memcmp(pstSucc->cups_no, "wfja", 4) == 0)//add by zxd@20151208
		iRet = nCalculateWfja(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "alpy", 4) == 0)
		iRet = nCalculateOnline(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "bjnp", 4) == 0)
		iRet = nCalculateWfjp(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "bjys", 4) == 0)
		iRet = nCalculateWfjp(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "cilk", 4) == 0)
		iRet = nCalculateWfjp(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "cntp", 4) == 0)
		iRet = nCalculateCntp(pstSucc, pdFee, totalAmt);
	else if(memcmp(pstSucc->cups_no, "cofp", 4) == 0)
		iRet = nCalculateFix(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "cpnr", 4) == 0)
		iRet = nCalculateFixVirtual(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "hdsy", 4) == 0)
		iRet = nCalculateFix(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "hkub", 4) == 0)
		iRet = nCalculateFixVirtual(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "hxnp", 4) == 0)
		iRet = nCalculateFix(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "spdb", 4) == 0)
		iRet = nCalculateSpdb(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "tftp", 4) == 0)
		iRet = nCalculateFix(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "tlpy", 4) == 0)
		iRet = nCalculateFix(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "wcht", 4) == 0)
		iRet = nCalculateOnline(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "xcpy", 4) == 0)
		iRet = nCalculateFix(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "xcpa", 4) == 0)//add by zxd@20151117
		iRet = nCalculateCoexit(pstSucc, pdFee);
    else if(memcmp(pstSucc->cups_no, "hdsx", 4) == 0)//add by zxd@20151124
        iRet = nCalculateCoexit(pstSucc, pdFee);
    else if(memcmp(pstSucc->cups_no, "hsna", 4) == 0)//add by zxd@20151208
        iRet = nCalculateCoexit(pstSucc, pdFee);
    else if(memcmp(pstSucc->cups_no, "cdia", 4) == 0)//add by zxd@20151208
        iRet = nCalculateCoexit(pstSucc, pdFee);
    else if(memcmp(pstSucc->cups_no, "cntn", 4) == 0)//add by zxd@20151215
		iRet = nCalculateCntp(pstSucc, pdFee, totalAmt);
	else if(memcmp(pstSucc->cups_no, "ysep", 4) == 0)
		iRet = nCalculateFix(pstSucc, pdFee);
	else if(memcmp(pstSucc->cups_no, "ysea", 4) == 0)//add by zxd@20151216
		iRet = nCalculateCoexit(pstSucc, pdFee);
	else		
	{
		iRet = nCalculateFix(pstSucc, pdFee);
	}

	return iRet;
}

/************************************************************************************/
/*   ����  �� int nCalculateWfjp((TStlmSucc *pstSucc, double *pdFee					*/
/*   ����  �� ��������������������													*/
/*  ����ֵ �� RC_SUCC		--	�ɹ�												*/ 
/*			  RC_FAIL		--	ʧ��												*/
/************************************************************************************/
int		nCalculateWfjp(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;

	/*����̻��Ƿ��ڱ�ƽ̨ע�ᣬ��ע��Ĵ�����δע��Ĳ�����*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "����·�ɱ��̻���Ϣ����:(%s), %s,%s,%s", sGetError(), pstSucc->cups_no, pstSucc->fd42,pstSucc->fd41);
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	stAlgo.max_fee = stCupsInf.fee_val;
	stAlgo.fee_value = stCupsInf.fee_rate;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	if(2 != pstSucc->ac_type && 3 != pstSucc->ac_type)
		return RC_SUCC;

	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.max_fee = 0.00;
	stAlgo.fee_value = WFJP_SERVICE_RATE / 2;	//	����Ʒ�Ʒ���Ѻ�������ƽ��
	(*pdFee) += dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}


//�̶��ģ���tbl_mcht_cups_inf�����̻��ţ��ն˺�ȡ�����ʹ���
int		nCalculateFix(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;

	/*����̻��Ƿ��ڱ�ƽ̨ע�ᣬ��ע��Ĵ�����δע��Ĳ�����*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "����·�ɱ��̻���Ϣ����:(%s)", sGetError());
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	stAlgo.max_fee = stCupsInf.fee_val;
	stAlgo.fee_value = stCupsInf.fee_rate;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}


//���ڣ��㸶����������̻��ţ��ն˺�(ȫ��8)
int		nCalculateFixVirtual(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	TStlmSucc	   stTmpSucc;
	int			 iRet;

	memset(&stTmpSucc, 0, sizeof(TStlmSucc));
	
	/*����̻��Ƿ��ڱ�ƽ̨ע�ᣬ��ע��Ĵ�����δע��Ĳ�����*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	iRet = lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, pstSucc->fd41);
	if(RC_NOTFOUND == iRet)
	{
		HtLog(ERR_PARAM, "[ ibox mcht find ]����·�ɱ��̻���Ϣ����:(%s)", sGetError());
		strcpy(stTmpSucc.cups_no, pstSucc->cups_no);
		strcpy(stTmpSucc.fd42, "888888888888888");
		strcpy(stTmpSucc.fd41, "88888888");
		iRet = lGetMchtRoute(&stCupsInf, stTmpSucc.cups_no, stTmpSucc.fd42, stTmpSucc.fd41);
		if (RC_NOTFOUND == iRet) 
		{
			HtLog(ERR_PARAM, "[ virtual mcht find ]����·�ɱ��̻���Ϣ����:(%s)", sGetError());
			return RC_FAIL;
		}
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	stAlgo.max_fee = stCupsInf.fee_val;
	stAlgo.fee_value = stCupsInf.fee_rate;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);


	return RC_SUCC;
}


//֧������΢�Ź̶�������0.6%
int nCalculateOnline(TStlmSucc *pstSucc, double *pdFee)
{
	dbDiscAlgo		stAlgo;

	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.max_fee = 0.00;
	stAlgo.fee_value = ONLINE_FIX_RATE;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}

//�ַ�
int		nCalculateSpdb(TStlmSucc *pstSucc, double *pdFee)
{
	dbNMchtMatch	stMatch;
	dbDiscAlgo		stAlgo;
	TStlmSucc		tmpStlmSucc;
	int 			iRet = 0;

	memset(&tmpStlmSucc, 0, sizeof(tmpStlmSucc));
	strcpy(tmpStlmSucc.cups_no, pstSucc->cups_no);
	strcpy(tmpStlmSucc.fd42, pstSucc->fd42);

	memset(&stMatch, 0, sizeof(stMatch));

	//����lGetSpdbRoute,��Ҫע���2��3������
	//iRet = lGetSpdbRoute(&stMatch, pstSucc->cups_no, pstSucc->fd42);
	iRet = lGetSpdbRoute(&stMatch, tmpStlmSucc.cups_no, tmpStlmSucc.fd42);
	if (RC_NOTFOUND == iRet)
	{
		HtLog(ERR_PARAM, "����·�ɱ��̻���Ϣ����:(%s)", sGetError());
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	stAlgo.max_fee = stMatch.fee_val;
	stAlgo.fee_value = stMatch.fee_rate;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}


//����ͨ�ڴ�������صġ�1������:  0.33	 0.72-24�ⶥ 
//��Ҫ���ݽ�����Ϊ �� 1������:  0.34	 0.78-26�ⶥ
int		nCalculateCntp(TStlmSucc *pstSucc, double *pdFee, double totalAmt)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;

	/*����̻��Ƿ��ڱ�ƽ̨ע�ᣬ��ע��Ĵ�����δע��Ĳ�����*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "����·�ɱ��̻���Ϣ����:(%s)", sGetError());
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	if (fabs(totalAmt - 100000000.00) <= 0.0001 && stCupsInf.fee_val > 0)
	{
		stAlgo.fee_value = stCupsInf.fee_rate + 0.06;
		stAlgo.max_fee = stCupsInf.fee_val + 2.00;
	}
	else if (fabs(totalAmt - 100000000.00) <= 0.001)
	{
		stAlgo.fee_value = stCupsInf.fee_rate + 0.01;
		stAlgo.max_fee = 0;
	}
	else
	{
		stAlgo.fee_value = stCupsInf.fee_rate;
		stAlgo.max_fee = stCupsInf.fee_val;
	}

	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}

//add by zxd@20151214 ����ͨ����ͨ�����⴦��
//����ͨ�ڴ�������صġ�1������:  0.33	 0.72-24�ⶥ 
//��Ҫ���ݽ�����Ϊ �� 1������:  0.34	 0.78-26�ⶥ
int		nCalculateCnta(TStlmSucc *pstSucc, double *pdFee, double totalAmt)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	char            szCupsNo[4+1];
	char            szNewCupsNo[4+1];

    /*��ȡ������������*/
	memset(szCupsNo, 0, sizeof(szCupsNo));
	memset(szNewCupsNo, 0, sizeof(szNewCupsNo));
	memcpy(szCupsNo, pstSucc->cups_no, 4);
	snprintf(szNewCupsNo, sizeof(szNewCupsNo), "%s", sGetDbsEnv(sUpper(szCupsNo)));

	/*����̻��Ƿ��ڱ�ƽ̨ע�ᣬ��ע��Ĵ�����δע��Ĳ�����*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, szNewCupsNo, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "����·�ɱ��̻���Ϣ����:(%s)", sGetError());
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	if (fabs(totalAmt - 100000000.00) <= 0.0001 && stCupsInf.fee_val > 0)
	{
		stAlgo.fee_value = stCupsInf.fee_rate + 0.06;
		stAlgo.max_fee = stCupsInf.fee_val + 2.00;
	}
	else if (fabs(totalAmt - 100000000.00) <= 0.001)
	{
		stAlgo.fee_value = stCupsInf.fee_rate + 0.01;
		stAlgo.max_fee = 0;
	}
	else
	{
		stAlgo.fee_value = stCupsInf.fee_rate;
		stAlgo.max_fee = stCupsInf.fee_val;
	}

	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}

//add by zxd@20151117
//ͬһ�������������㷽ʽ����tbl_mcht_cups_inf�����̻��ţ��ն˺�ȡ�����ʹ���
int		nCalculateCoexit(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	char            szCupsNo[4+1];
	char            szNewCupsNo[4+1];

    /*��ȡ������������*/
	memset(szCupsNo, 0, sizeof(szCupsNo));
	memset(szNewCupsNo, 0, sizeof(szNewCupsNo));
	memcpy(szCupsNo, pstSucc->cups_no, 4);
	snprintf(szNewCupsNo, sizeof(szNewCupsNo), "%s", sGetDbsEnv(sUpper(szCupsNo)));
	
	/*����̻��Ƿ��ڱ�ƽ̨ע�ᣬ��ע��Ĵ�����δע��Ĳ�����*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, szNewCupsNo, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "����·�ɱ��̻���Ϣ����:(%s)", sGetError());
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	stAlgo.max_fee = stCupsInf.fee_val;
	stAlgo.fee_value = stCupsInf.fee_rate;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}

/************************************************************************************/
/*   ����  �� int nCalculateWfja((TStlmSucc *pstSucc, double *pdFee					*/
/*   ����  �� ������������������������													*/
/*  ����ֵ �� RC_SUCC		--	�ɹ�												*/ 
/*			  RC_FAIL		--	ʧ��												*/
/************************************************************************************/
int		nCalculateWfja(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	char            szCupsNo[4+1];
	char            szNewCupsNo[4+1];
	
	/*��ȡ������������*/
	memset(szCupsNo, 0, sizeof(szCupsNo));
	memset(szNewCupsNo, 0, sizeof(szNewCupsNo));
	memcpy(szCupsNo, pstSucc->cups_no, 4);
	snprintf(szNewCupsNo, sizeof(szNewCupsNo), "%s", sGetDbsEnv(sUpper(szCupsNo)));

	/*����̻��Ƿ��ڱ�ƽ̨ע�ᣬ��ע��Ĵ�����δע��Ĳ�����*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, szNewCupsNo, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "����·�ɱ��̻���Ϣ����:(%s), %s,%s,%s", sGetError(), pstSucc->cups_no, pstSucc->fd42,pstSucc->fd41);
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	stAlgo.max_fee = stCupsInf.fee_val;
	stAlgo.fee_value = stCupsInf.fee_rate;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

//ֱ��Ĳ���Ʒ�Ʒ����
#if 0
	if(2 != pstSucc->ac_type && 3 != pstSucc->ac_type)
		return RC_SUCC;

	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.max_fee = 0.00;
	stAlgo.fee_value = WFJP_SERVICE_RATE / 2;	//	����Ʒ�Ʒ���Ѻ�������ƽ��
	(*pdFee) += dGetAlgoFee(&stAlgo, pstSucc->tx_amt);
#endif
	return RC_SUCC;
}
