#ifndef		__TOOL_DEFINE_HEADER_FILE__XXXX_
#define		__TOOL_DEFINE_HEADER_FILE__XXXX_
typedef	struct	__LOGINFO
{
	long	lLogMode;			//	日志输出模式
	long	lLogGloble;			//	整个日志级别
	long	lLogLevel;			//	当前记录日志级别
	char	szLog[512];			//	日志名称
	char	szRptName[512];		//	报表名称
}LogInfo;

/*消息结构*/
typedef	struct	__LOGMSG
{
    long	m_lType;			/* 消息标识符 */
    char	m_szMsg[1024];		/* 消息内容 */
}LogMsg;

typedef	struct	__TRADECOUNT
{
	long	lRunStep;			//	检查步骤
	LogInfo	stLog;				//	日志结构体
	LogInfo	stBakLog;			//	日志备份
	long	lSetStep;			//	只对设置的步骤输出日志
	long	lIsNStepNeed;		//	对于指定跑的步骤，有必须要跑的前提步骤
	long	lRetCode;			//	错误码
	long	lMaxStep;			//	最大步骤数目
	char	szBatNum[5];		//	批量模式 TN00 T001
	char	szDate[10];			//	查询日期
	long	lCount;				//	总笔数
	double	dTxAmt;				//	总金额
	long	lRealCount;			//	T+0笔数
	double	dRealAmt;			//	T+0金额
	long	lSETCount;			//	清算总笔数
	double	dSETTxAmt;			//	清算总金额
	long	lGAPCount;			//	日切笔数
	double	dGAPTxAmt;			//	日切金额
	long	lErrCount;			//	差错笔数
	double	dErrTxAmt;			//	差错金额
	long	lLastCount;			//	差错参与清算笔数
	double	dLastTxAmt;			//	差错参与清算金额
	long	lLastGAPCnt;		//	上期未清算的笔数
	double	dLastGAPAmt;		//	上期未清算的金额
	long	lSettleCount;		//	当日清算笔数
	double	dSettleAmt;			//	当日清算金额
	long	lTNDlyCount;		//	对于T+N N > 1延迟清算笔数
	double	dTNDlyAmt;			//	对于T+N N > 1延迟清算金额
	long	lTNSetCount;		//	对于T+N N > 1在本周期清算笔数
	double	dTNSetAmt;			//	对于T+N N > 1在本周期清算金额
	long	lSumCount;			//	清算汇总个数
	double	dSumTxAmt;			//	清算汇总金额
	long	lCupsCount;			//	周期内渠道流水笔数
	double	dCupsAmt;			//	周期内渠道流水金额
	long	lCupsNCnt;			//	周期内渠道流水笔数
	double	dCupsNAmt;			//	周期内渠道流水金额
}TrdCnt;

typedef	struct	__SETTLEINFO
{
	long	lCount;				//	当日交易笔数
	double	dTxAmt;				//	当日交易金额
	double	dFeeAmt;			//	当日交易手续费
	double	dBaseAmt;			//	当日基准手续费
	double	dCupAmt;			//	当日渠道手续费
	double	dBrhFee;			//	当日机构手续费
	double	dSettleAmt;			//	当日清算金额
	double	dOtherAmt;			//	其他费用
	double	dRePairAmt;			//	补帐金额
	double	dUpRePairAmt;		//	补帐参与清算金额
	double	dUpErrAmt;			//	入账错误参与清算金额
	double	dRate;				//	费率
	char	szFlag[2];			//	清算标识
	char	szId[26];			//	PK
	char	szMchtNo[26];		//	商户
	char	szTraceNo[21];		//	商户
	char	szBrhNo[26];		//	机构号
	long    lCountLast;         //  上期T+N(N>1)笔数
	double  dTxAmtLast;         //  上期T+N(N>1)交易金额
	long    lCountCurr;         //  本期T+N(N>1)笔数
    double  dTxAmtCurr;         //  本期T+N(N>1)交易金额
}AlgoInfo, SettleInfo, SettleTxn;

typedef struct _RUNSTEP
{
    long    lRunStep;
    int     (*pfRunStep)();
}RunStep;

#endif	//__TOOL_DEFINE_HEADER_FILE__XXXX_

