/*****************************************************************
 *  功能: 计算渠道手续费(商户资金清分)
 *	Author：DeffPuzzL
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
/*   声明  ： double dGetAlgoFee(dbDiscAlgo *pstAlgo, double dTxAmt)				*/
/*   功能  ： 计算应收商户手续费													*/
/*  返回值 ： RC_SUCC	   --  成功												*/
/*			RC_FAIL	   --  失败												*/
/************************************************************************************/
double  dGetAlgoFee(dbDiscAlgo *pstAlgo, double dTxAmt)
{
	double	dFee = 0.00, dCalFee = 0.00;
	
	//固定收费
	if(pstAlgo->flag == DISC_ALGO_FIXED)
	{
		dCalFee = pstAlgo->fee_value;
	}
	//比例收费,有保底，封顶
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
	//其他方式未定义,默认为0
	else if(pstAlgo->flag > DISC_ALGO_FIXED)	
	{
		dCalFee = 0.00;
	}

	return dCalFee;
}

/************************************************************************************/
/*   声明  ： int nCalcCupsFee((TStlmSucc *pstSucc, double *pdFee)					*/
/*   功能  ： 根据渠道计算通道成本													*/
/*  返回值 ： RC_SUCC		--	成功												*/ 
/*			  RC_FAIL		--	失败												*/
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
/*   声明  ： int nCalculateWfjp((TStlmSucc *pstSucc, double *pdFee					*/
/*   功能  ： 计算王府井渠道手续费													*/
/*  返回值 ： RC_SUCC		--	成功												*/ 
/*			  RC_FAIL		--	失败												*/
/************************************************************************************/
int		nCalculateWfjp(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;

	/*检查商户是否在本平台注册，对注册的处理，对未注册的不处理*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "查找路由表商户信息错误:(%s), %s,%s,%s", sGetError(), pstSucc->cups_no, pstSucc->fd42,pstSucc->fd41);
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
	stAlgo.fee_value = WFJP_SERVICE_RATE / 2;	//	银联品牌服务费和王府井平分
	(*pdFee) += dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}


//固定的，从tbl_mcht_cups_inf根据商户号，终端号取出汇率规则
int		nCalculateFix(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;

	/*检查商户是否在本平台注册，对注册的处理，对未注册的不处理*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "查找路由表商户信息错误:(%s)", sGetError());
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


//海口，汇付存在虚拟的商户号，终端号(全是8)
int		nCalculateFixVirtual(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	TStlmSucc	   stTmpSucc;
	int			 iRet;

	memset(&stTmpSucc, 0, sizeof(TStlmSucc));
	
	/*检查商户是否在本平台注册，对注册的处理，对未注册的不处理*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	iRet = lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, pstSucc->fd41);
	if(RC_NOTFOUND == iRet)
	{
		HtLog(ERR_PARAM, "[ ibox mcht find ]查找路由表商户信息错误:(%s)", sGetError());
		strcpy(stTmpSucc.cups_no, pstSucc->cups_no);
		strcpy(stTmpSucc.fd42, "888888888888888");
		strcpy(stTmpSucc.fd41, "88888888");
		iRet = lGetMchtRoute(&stCupsInf, stTmpSucc.cups_no, stTmpSucc.fd42, stTmpSucc.fd41);
		if (RC_NOTFOUND == iRet) 
		{
			HtLog(ERR_PARAM, "[ virtual mcht find ]查找路由表商户信息错误:(%s)", sGetError());
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


//支付宝，微信固定手续费0.6%
int nCalculateOnline(TStlmSucc *pstSucc, double *pdFee)
{
	dbDiscAlgo		stAlgo;

	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.max_fee = 0.00;
	stAlgo.fee_value = ONLINE_FIX_RATE;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

	return RC_SUCC;
}

//浦发
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

	//调用lGetSpdbRoute,需要注意第2，3个参数
	//iRet = lGetSpdbRoute(&stMatch, pstSucc->cups_no, pstSucc->fd42);
	iRet = lGetSpdbRoute(&stMatch, tmpStlmSucc.cups_no, tmpStlmSucc.fd42);
	if (RC_NOTFOUND == iRet)
	{
		HtLog(ERR_PARAM, "查找路由表商户信息错误:(%s)", sGetError());
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


//银视通内存里面加载的“1亿以上:  0.33	 0.72-24封顶 
//需要根据金额调整为 “ 1亿以下:  0.34	 0.78-26封顶
int		nCalculateCntp(TStlmSucc *pstSucc, double *pdFee, double totalAmt)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;

	/*检查商户是否在本平台注册，对注册的处理，对未注册的不处理*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, pstSucc->cups_no, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "查找路由表商户信息错误:(%s)", sGetError());
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

//add by zxd@20151214 银视通虚拟通道特殊处理
//银视通内存里面加载的“1亿以上:  0.33	 0.72-24封顶 
//需要根据金额调整为 “ 1亿以下:  0.34	 0.78-26封顶
int		nCalculateCnta(TStlmSucc *pstSucc, double *pdFee, double totalAmt)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	char            szCupsNo[4+1];
	char            szNewCupsNo[4+1];

    /*获取归类渠道名称*/
	memset(szCupsNo, 0, sizeof(szCupsNo));
	memset(szNewCupsNo, 0, sizeof(szNewCupsNo));
	memcpy(szCupsNo, pstSucc->cups_no, 4);
	snprintf(szNewCupsNo, sizeof(szNewCupsNo), "%s", sGetDbsEnv(sUpper(szCupsNo)));

	/*检查商户是否在本平台注册，对注册的处理，对未注册的不处理*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, szNewCupsNo, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "查找路由表商户信息错误:(%s)", sGetError());
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
//同一渠道有两种清算方式，从tbl_mcht_cups_inf根据商户号，终端号取出汇率规则
int		nCalculateCoexit(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	char            szCupsNo[4+1];
	char            szNewCupsNo[4+1];

    /*获取归类渠道名称*/
	memset(szCupsNo, 0, sizeof(szCupsNo));
	memset(szNewCupsNo, 0, sizeof(szNewCupsNo));
	memcpy(szCupsNo, pstSucc->cups_no, 4);
	snprintf(szNewCupsNo, sizeof(szNewCupsNo), "%s", sGetDbsEnv(sUpper(szCupsNo)));
	
	/*检查商户是否在本平台注册，对注册的处理，对未注册的不处理*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, szNewCupsNo, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "查找路由表商户信息错误:(%s)", sGetError());
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
/*   声明  ： int nCalculateWfja((TStlmSucc *pstSucc, double *pdFee					*/
/*   功能  ： 计算王府井虚拟渠道手续费													*/
/*  返回值 ： RC_SUCC		--	成功												*/ 
/*			  RC_FAIL		--	失败												*/
/************************************************************************************/
int		nCalculateWfja(TStlmSucc *pstSucc, double *pdFee)
{
	dbMchtCupsInf	stCupsInf;
	dbDiscAlgo		stAlgo;
	char            szCupsNo[4+1];
	char            szNewCupsNo[4+1];
	
	/*获取归类渠道名称*/
	memset(szCupsNo, 0, sizeof(szCupsNo));
	memset(szNewCupsNo, 0, sizeof(szNewCupsNo));
	memcpy(szCupsNo, pstSucc->cups_no, 4);
	snprintf(szNewCupsNo, sizeof(szNewCupsNo), "%s", sGetDbsEnv(sUpper(szCupsNo)));

	/*检查商户是否在本平台注册，对注册的处理，对未注册的不处理*/
	memset(&stCupsInf, 0, sizeof(stCupsInf));
	if(RC_SUCC != lGetMchtRoute(&stCupsInf, szNewCupsNo, pstSucc->fd42, 
		pstSucc->fd41))
	{
		HtLog(ERR_PARAM, "查找路由表商户信息错误:(%s), %s,%s,%s", sGetError(), pstSucc->cups_no, pstSucc->fd42,pstSucc->fd41);
		return RC_FAIL;
	}

	memset(&stAlgo, 0, sizeof(stAlgo));
	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.min_fee = 0.00;
	stAlgo.max_fee = stCupsInf.fee_val;
	stAlgo.fee_value = stCupsInf.fee_rate;
	(*pdFee) = dGetAlgoFee(&stAlgo, pstSucc->tx_amt);

//直清的不收品牌服务费
#if 0
	if(2 != pstSucc->ac_type && 3 != pstSucc->ac_type)
		return RC_SUCC;

	stAlgo.flag = DISC_ALGO_RATE;
	stAlgo.max_fee = 0.00;
	stAlgo.fee_value = WFJP_SERVICE_RATE / 2;	//	银联品牌服务费和王府井平分
	(*pdFee) += dGetAlgoFee(&stAlgo, pstSucc->tx_amt);
#endif
	return RC_SUCC;
}
