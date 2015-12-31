#include	"load.h"

/*************************************************************************************************
	ȫ�ֱ�������
 *************************************************************************************************/
IBPLog	g_stLog = {0};

/*************************************************************************************************
	�ṹ��ָ��
 *************************************************************************************************/
IBPLog*	pGetLog()
{
	return &g_stLog;
}

/*************************************************************************************************
	��ȡ���ڼ���	
 *************************************************************************************************/
void	vSetLogLevel(int n)
{
	if(n > LOG_MOST)
		n = LOG_MOST;
	g_stLog.lLogLevel = n;	
}

/*************************************************************************************************
	��ȡ���ڼ���	
 *************************************************************************************************/
int		nGetLogLevel()
{
	return g_stLog.lLogLevel;
}

/*************************************************************************************************
	�ر���־	
 *************************************************************************************************/
void	vDisableLog()
{
	g_stLog.lLogBak = g_stLog.lLogGloble;
	g_stLog.lLogGloble = LOG_MOST * 2 + 1;
}

/*************************************************************************************************
	������־
 *************************************************************************************************/
void	vEnableLog()
{
	if(g_stLog.lLogBak > LOG_MOST)
		g_stLog.lLogGloble = LOG_DEBUG;
	else
		g_stLog.lLogGloble = g_stLog.lLogBak;
}

/*************************************************************************************************
	����ϵͳ���ڵȼ�
 *************************************************************************************************/
void	vSetSysLog(int n)
{
	g_stLog.lLogGloble = n;
}

/*************************************************************************************************
	��ȡϵͳ��־�ȼ�	
 *************************************************************************************************/
int		nGetSysLog()
{
	return g_stLog.lLogGloble;
}

/*************************************************************************************************
	��ȡ��־����
 *************************************************************************************************/
char*	sGetLogName()
{
	if(!strlen(g_stLog.szLog))
		strcpy(g_stLog.szLog, LOGNAME);
	return g_stLog.szLog;
}

/*************************************************************************************************
	��������
 *************************************************************************************************/
void	vSetLogName(char *s, ...)
{
	va_list ap;

	memset(g_stLog.szLog, 0, sizeof(g_stLog.szLog));
	va_start(ap, s);
	vsnprintf(g_stLog.szLog, sizeof(g_stLog.szLog), s, ap);
	va_end(ap);
}

/*************************************************************************************************
	�������ͷ
 *************************************************************************************************/
void	vPrintHead(char *pszFile, long lLine, int nMode)
{
	FILE	*fp = NULL;
	char	szLog[1024];
	time_t	cur;
	struct	tm *tm;

	if(nGetLogLevel() < nGetSysLog())
		return ;

	memset(szLog, 0, sizeof(szLog));
	snprintf(szLog, sizeof(szLog), "%s/%s", getenv("LOG_FILE_PATH"), sGetLogName());

	if(NULL == (fp = fopen(szLog, "a+")))
	{
		fprintf(stderr, "���ļ�%sʧ��:[%s]", szLog, strerror(errno));
		exit(-1);
	}

	time(&cur);
	tm = localtime(&cur);
	fprintf(fp, "F=%-10s L=%-5d P=%-6d D=%04d-%02d-%02d  T=%02d:%02d:%02d  ", pszFile, lLine, getpid(), 
		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	fclose(fp);

	vSetLogLevel(nMode);
}

/*************************************************************************************************
	�����־
 *************************************************************************************************/
void	vPrintLog(char *pszFmt, ...)
{
	va_list ap;
	FILE	*fp = NULL;
	char	szLog[1024], szMsg[10240];

	memset(szLog, 0, sizeof(szLog));
   	snprintf(szLog, sizeof(szLog), "%s/%s", getenv("LOG_FILE_PATH"), sGetLogName());

	if(nGetLogLevel() < nGetSysLog())
		return ;

	if(NULL == (fp = fopen(szLog, "a+")))
	{
		fprintf(stderr, "���ļ�%sʧ��:[%s]", szLog, strerror(errno));
		exit(-1);
	}

	if(LOG_WARM == nGetLogLevel())
		fprintf(fp, "���棺");
	else if(LOG_ERROR == nGetLogLevel())
		fprintf(fp, "����");

	memset(szMsg, 0, sizeof(szMsg));
	va_start(ap, pszFmt);
	vsnprintf(szMsg, sizeof(szMsg), pszFmt, ap);
	va_end(ap);

	strcat(szMsg, "\n");
	szMsg[sizeof(szMsg) - 1 ] = 0x00;

	fprintf(fp, szMsg);
	fclose(fp);
}

/******************************************************************************
 *  ����   :		��ӡ��־
 ******************************************************************************/
void	vPrintHex(char *s, int k, long len)
{
	char	szHex[200], szTest[200];
	int		i, j, n = 0;

	memset(szTest, 0x20, sizeof(szTest));
	szTest[sizeof(szTest) - 1] = 0x00;
	for(i = 0; i < k; n ++)
	{
		memset(szHex, 0, sizeof(szHex));
		sprintf(szHex + strlen(szHex), "%08d - %08d: ", n * len, 
			(n + 1) * len > k ? (k - 1) : ((n + 1) * len - 1));
		for(j = 0; j < len && i < k; j ++, i++)
			sprintf(szHex + strlen(szHex), "%02X ", (uchar)s[i]);
		sprintf(szHex + strlen(szHex), "%.*s\t", (len - j) * 3, szTest);

		for(j; j > 0; j --)
		{
			if(32 <= (uchar)s[i - j] && (uchar)s[i - j] <= 126)
				sprintf(szHex + strlen(szHex), "%c", (uchar)s[i - j]);
			else
				sprintf(szHex + strlen(szHex), ".");
		}

		vPrintLog("%s", szHex);
		if(i >= k)	return ;
	}

	return ;
}

