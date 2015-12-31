#ifndef		__TOOL_DEFINE_HEADER_FILE__XXXX_
#define		__TOOL_DEFINE_HEADER_FILE__XXXX_
typedef	struct	__LOGINFO
{
	long	lLogMode;			//	��־���ģʽ
	long	lLogGloble;			//	������־����
	long	lLogLevel;			//	��ǰ��¼��־����
	char	szLog[512];			//	��־����
	char	szRptName[512];		//	��������
}LogInfo;

/*��Ϣ�ṹ*/
typedef	struct	__LOGMSG
{
    long	m_lType;			/* ��Ϣ��ʶ�� */
    char	m_szMsg[1024];		/* ��Ϣ���� */
}LogMsg;

typedef	struct	__TRADECOUNT
{
	long	lRunStep;			//	��鲽��
	LogInfo	stLog;				//	��־�ṹ��
	LogInfo	stBakLog;			//	��־����
	long	lSetStep;			//	ֻ�����õĲ��������־
	long	lIsNStepNeed;		//	����ָ���ܵĲ��裬�б���Ҫ�ܵ�ǰ�Ჽ��
	long	lRetCode;			//	������
	long	lMaxStep;			//	�������Ŀ
	char	szBatNum[5];		//	����ģʽ TN00 T001
	char	szDate[10];			//	��ѯ����
	long	lCount;				//	�ܱ���
	double	dTxAmt;				//	�ܽ��
	long	lRealCount;			//	T+0����
	double	dRealAmt;			//	T+0���
	long	lSETCount;			//	�����ܱ���
	double	dSETTxAmt;			//	�����ܽ��
	long	lGAPCount;			//	���б���
	double	dGAPTxAmt;			//	���н��
	long	lErrCount;			//	������
	double	dErrTxAmt;			//	�����
	long	lLastCount;			//	�������������
	double	dLastTxAmt;			//	������������
	long	lLastGAPCnt;		//	����δ����ı���
	double	dLastGAPAmt;		//	����δ����Ľ��
	long	lSettleCount;		//	�����������
	double	dSettleAmt;			//	����������
	long	lTNDlyCount;		//	����T+N N > 1�ӳ��������
	double	dTNDlyAmt;			//	����T+N N > 1�ӳ�������
	long	lTNSetCount;		//	����T+N N > 1�ڱ������������
	double	dTNSetAmt;			//	����T+N N > 1�ڱ�����������
	long	lSumCount;			//	������ܸ���
	double	dSumTxAmt;			//	������ܽ��
	long	lCupsCount;			//	������������ˮ����
	double	dCupsAmt;			//	������������ˮ���
	long	lCupsNCnt;			//	������������ˮ����
	double	dCupsNAmt;			//	������������ˮ���
}TrdCnt;

typedef	struct	__SETTLEINFO
{
	long	lCount;				//	���ս��ױ���
	double	dTxAmt;				//	���ս��׽��
	double	dFeeAmt;			//	���ս���������
	double	dBaseAmt;			//	���ջ�׼������
	double	dCupAmt;			//	��������������
	double	dBrhFee;			//	���ջ���������
	double	dSettleAmt;			//	����������
	double	dOtherAmt;			//	��������
	double	dRePairAmt;			//	���ʽ��
	double	dUpRePairAmt;		//	���ʲ���������
	double	dUpErrAmt;			//	���˴������������
	double	dRate;				//	����
	char	szFlag[2];			//	�����ʶ
	char	szId[26];			//	PK
	char	szMchtNo[26];		//	�̻�
	char	szTraceNo[21];		//	�̻�
	char	szBrhNo[26];		//	������
	long    lCountLast;         //  ����T+N(N>1)����
	double  dTxAmtLast;         //  ����T+N(N>1)���׽��
	long    lCountCurr;         //  ����T+N(N>1)����
    double  dTxAmtCurr;         //  ����T+N(N>1)���׽��
}AlgoInfo, SettleInfo, SettleTxn;

typedef struct _RUNSTEP
{
    long    lRunStep;
    int     (*pfRunStep)();
}RunStep;

#endif	//__TOOL_DEFINE_HEADER_FILE__XXXX_

