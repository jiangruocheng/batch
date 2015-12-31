#include	"load.h" 

/********************************************************************************/
/* ���㵱ǰʱ���ƫ��lMin���ʱ�䣬������									 	*/
/********************************************************************************/
char*	sGetTimeAlgo(long lMin)
{
	long	lTime = 0;
	long	ctm;
	struct	tm	*tm;
	static	char	szTime[10];

	time(&ctm);
	tm = localtime(&ctm);

	memset(szTime, 0, sizeof(szTime));
	
	lTime = tm->tm_hour * 3600 + tm->tm_min * 60 + lMin * 60;
	if(lTime < 0)	lTime = 0;
	snprintf(szTime, sizeof(szTime), "%02d%02d00", lTime / 3600, lTime % 3600 / 60);
	
	return szTime;
}

/********************************************************************************/
/* ����ָ��ʱ���ƫ��lMin���ʱ�䣬������									 	*/
/********************************************************************************/
char*	sGetMinsOfs(char *pszTime, long lOfs)
{
	char	szTemp[3];
	long	lTime = 0;
	struct	tm	tm;
	static	char	szTime[10];

	memset(szTime, 0, sizeof(szTime));
	if(6 != strlen(pszTime))	return pszTime;

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, pszTime, 2);
	tm.tm_hour = atoi(szTemp);

	memcpy(szTemp, pszTime + 2, 2);
	tm.tm_min = atoi(szTemp);

	memcpy(szTemp, pszTime + 4, 2);
	tm.tm_sec = atoi(szTemp);

	lTime = tm.tm_hour * 60 + tm.tm_min + lOfs % (60 * 24);
	if(lTime < 0)	lTime = 0;
	snprintf(szTime, sizeof(szTime), "%02d%02d%02d", lTime / 60, lTime % 60, tm.tm_sec);

	return szTime;
}

/********************************************************************************/
/*   ����  �� ��ȡ��ǰ����														*/
/*   ����  �� ��																*/
/*  ����ֵ �� ��ǰ�����ַ���													*/
/********************************************************************************/
char*	sGetDate()
{  
	time_t current;
	struct tm *tm = NULL;
	static	char	szDate[9];
   
	time(&current);
	tm = localtime(&current);

	memset(szDate, 0, sizeof(szDate));
	snprintf(szDate, sizeof(szDate), "%4d%02d%02d", tm->tm_year + 1900, 
		   tm->tm_mon + 1, tm->tm_mday);

	return szDate;
}

/********************************************************************************/
/*   ����  �� ��ȡ��ǰʱ��,ʱ���ʽYYYYMMDDhhmmss								*/
/*   ����  �� ��																*/
/*  ����ֵ �� ��ǰ�����ַ���													*/
/********************************************************************************/
char*	sGetCurrentTime()
{  
	time_t	current;
	struct	tm *tm = NULL;
	static	char	szTime[15];
   
	tzset();
	time(&current);
	tm = localtime(&current);

	memset(szTime, 0, sizeof(szTime));
	snprintf(szTime, sizeof(szTime), "%4d%02d%02d%02d%02d%02d", tm->tm_year + 1900, 
		tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return szTime;
}

/********************************************************************************/
/* FUNC:   double DecRound(double value,int dot)								*/
/* INPUT:  value: Ҫ����ת����˫���ȸ�������									*/
/*		   dot	: ��������Ҫ������λ��											*/
/* OUTPUT: NULL															  		*/
/* RETURN: �ɹ���ת���������													*/
/* DESC	 : ��������ת������												  		*/
/********************************************************************************/
double	dDecRound(double dValue, long lDot)
{
	char	szDot[40], szDec[40];
	double	dAa;

	memset(szDot, 0, sizeof(szDot));
	memset(szDec, 0, sizeof(szDec));
	sprintf(szDec, "%s%dlf", "%.", lDot);	
	if(dValue >= 0)
		dAa = dValue + 0.00000001;
	else
		dAa = dValue - 0.00000001;

	snprintf(szDot, sizeof(szDot), szDec, dAa);	
	dAa = atof(szDot);

	return dAa;
} 

/********************************************************************************/
/*   ����  �� char* sGetChgDate(char *pszInDay, long lDay)						*/
/*   ����  �� ȡָ��������nDay���������										*/
/*   ����  �� pszInDay --ָ��ԭ����												*/
/*		 �� lDay	 --�������												  	*/
/*  ����ֵ �� 0--  �ɹ�															*/
/*		   -1--  ʧ��															*/
/********************************************************************************/
char*	sGetChgDate(char *pszInDay, long lDay)
{
	time_t	tCrtTime;
	struct tm  stm, *ptTm;
	char	szTmp[8];
	static	char	szDstDate[10];

	memset(&stm , 0 , sizeof(stm));
	memset(szTmp, 0, sizeof(szTmp));
	memset(szDstDate, 0, sizeof(szDstDate));

	memcpy(szTmp, pszInDay, 4);
	stm.tm_year = atoi(szTmp) - 1900;

	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszInDay + 4, 2);
	stm.tm_mon = atoi(szTmp) - 1;

	memset(szTmp, 0, sizeof(szTmp));
	memcpy(szTmp, pszInDay + 6, 2);
	stm.tm_mday = atoi(szTmp);
	
	tCrtTime = mktime(&stm) + lDay * 24l * 3600l;

	memset(&ptTm, 0, sizeof(ptTm));
	ptTm = localtime (&tCrtTime);

	snprintf(szDstDate, sizeof(szDstDate), "%04d%02d%02d", ptTm->tm_year+1900, 
		ptTm->tm_mon+1, ptTm->tm_mday);

	return szDstDate;
}

