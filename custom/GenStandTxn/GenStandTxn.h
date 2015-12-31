#ifndef         __XX__HH_MCHTTXNFILE__HHH
#define         __XX__HH_MCHTTXNFILE__HHH

typedef struct __CHKLIST
{
	char	m_szSettleDate[9];
	char	m_szTxName[61];
	char	m_szTxDate[9];
	char	m_szTxTime[9];	
	char	m_szBrno[21];	
	char	m_szIbox42[26];	
	char	m_szMchtNo[26];	
	char	m_szIbox41[16];
	char	m_szAcNo[20];	
	char	m_szIbox11[7];
	char	m_szFd39[7];
	char	m_szMcc[5];
	char	m_szFd49[4];	
	double	m_dTxAmt;
	double	m_dFeeAmt;
	double	m_dOtherFee;
	double	m_dFreeFee;
	char	m_szFd37[13];	
	char	m_IboxBatchNo[7];
	char	m_szOrderNo[21];
}IFileTxn;

#endif  //		__XX__HH_MCHTTXNFILE__HHH
