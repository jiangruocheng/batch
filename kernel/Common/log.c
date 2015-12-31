/****************************************************************************
 *
 *  ����: ��ӡ��־
 *
 *  Edit History:
 *
 **************************************************************************/
#include	"Ibp.h"
#include	"load.h"
#include	"BatErr.h"

/* ��־�ļ�·���ĳ��ȵ����ֵ */
#define LOG_PATH_LEN_MAX		128
/* ��־�ļ����ĳ��ȵ����ֵ */
#define LOG_NAME_LEN_MAX		32

/* ��־�л�ģʽ, ���ļ���С������ */
#define LOG_SWITCH_MODE_SIZE	1
#define LOG_SWITCH_MODE_DATE	2

#define LOG_SIZE_UNIT			1000000

#define LOG_DEFAULT_NAME		"ibp.log"
#define SINGLE_LINE				"--------------------------------------------------------------------------------\n"

FILE	*fp;

int ConvertEnv(char *str);
int ReplaceEnvVar(char *str);

/*****************************************************************************/
/* FUNC:   int HtLog (char *sLogName, int nLogMode, char *sFileName,		 */
/*					int nLine, char *sFmt, ...);						   */
/* INPUT:  sLogName: ��־��, ����·��										*/
/*		 nLogMode: ��־����,											   */
/*				   HT_LOG_MODE_ERROR,HT_LOG_MODE_NORMAL,HT_LOG_MODE_DEBUG  */
/*		 sFileName: �����Դ������										 */
/*		 nLine: �����Դ������к�										 */
/*		 sFmt: ������Ϣ													*/
/* OUTPUT: ��																*/
/* RETURN: 0: �ɹ�, ����: ʧ��											   */
/* DESC:   ����LOG_MODE, ���ü���֮�µ���־��¼����־�ļ���,				 */
/*****************************************************************************/
int HtLog(char *sLogName, int nLogMode, char *sFileName, int nLine, char *sFmt, ...)
{
	char	sLogTime[128];
	char	sDateTime[16];
	char	sMilliTM[4];
	char	sParamLogFilePath[LOG_PATH_LEN_MAX];
	int	 nParamLogMode;
	int	 nParamLogSwitchMode;
	int	 nParamLogSize; /* in megabytes */
	int	 nReturnCode;
	long	lTimeTick;
	va_list ap;
	time_t  lTime;
	struct tm	*tTmLocal;
	struct tms	tTMS;
	struct timeb tTimeB;
	
	/* get config parameter, LOG_MODE, LOG_FILE_PATH, LOG_SWITCH_MODE, LOG_SIZE */
	if (getenv("LOG_MODE") == NULL)
		nParamLogMode = HT_LOG_MODE_NORMAL;
	else   
		nParamLogMode = atoi(getenv("LOG_MODE"));
	if (getenv("LOG_SWITCH_MODE") == NULL)
		nParamLogSwitchMode = LOG_SWITCH_MODE_SIZE;
	else
		nParamLogSwitchMode = atoi(getenv("LOG_SWITCH_MODE"));
	if (getenv("LOG_SIZE") == NULL)
		nParamLogSize = LOG_SIZE_DEFAULT;
	else
		nParamLogSize = atoi(getenv("LOG_SIZE"));
	
	memset(sParamLogFilePath, 0, sizeof (sParamLogFilePath));
	if (getenv("LOG_FILE_PATH"))
		strncpy(sParamLogFilePath, (char *)getenv("LOG_FILE_PATH"), sizeof(sParamLogFilePath) - 1);
	if (strlen(sParamLogFilePath) == 0)
		strcpy(sParamLogFilePath, ".");
	
	/* check whether the msg should be saved in log file */
	if (nParamLogMode < nLogMode)
		return 0;

	/* get current time */
	memset(sLogTime, 0x00, sizeof(sLogTime));
	memset(sDateTime, 0x00, sizeof(sDateTime));

	memset(&tTimeB, 0, sizeof(tTimeB));
	ftime(&tTimeB);
	sprintf(sMilliTM, "%03d", tTimeB.millitm);
	tTmLocal = localtime(&tTimeB.time);
	strftime (sLogTime, sizeof(sLogTime), "%Y-%m-%d %H:%M:%S", tTmLocal);
	strcat(sLogTime, ":");
	strcat(sLogTime, sMilliTM);
	strftime(sDateTime, sizeof(sDateTime), "%Y%m%d%H%M%S", tTmLocal);

	/* open log file */
	nReturnCode = OpenLogFile(sParamLogFilePath, sLogName, nParamLogSwitchMode, nParamLogSize, sDateTime);
	if (nReturnCode)
		return (nReturnCode);
		
	/* save log msg in file */
	fprintf(fp, "[%s][%s][%d]", sLogTime,sFileName,nLine); 

	va_start(ap, sFmt);
	vfprintf(fp, sFmt, ap);
	va_end(ap);

	fprintf(fp, "\n");
	fflush(fp);

	/* close file */
	fclose (fp);

	return (0);	
}

/*****************************************************************************/
/* FUNC:   int HtDebugString (char *sLogName, int nLogMode, char *sFileName, */
/*							int nLine, char *psBuf, int nBufLen);		  */
/* INPUT:  sLogName: ��־��, ����·��										*/
/*		 nLogMode: ��־����,											   */
/*				   HT_LOG_MODE_ERROR,HT_LOG_MODE_NORMAL,HT_LOG_MODE_DEBUG  */
/*		 sFileName: �����Դ������										 */
/*		 nLine: �����Դ������к�										 */
/*		 psBuf: �������buffer											 */
/*		 nBufLen: buffer�ĳ���											 */
/* OUTPUT: ��																*/
/* RETURN: 0: �ɹ�, ����: ʧ��											   */
/* DESC:   ����LOG_MODE, ���ü���֮�µ���־��¼����־�ļ���,				 */
/*		 ���������buffer��16����ֵ										*/
/*****************************************************************************/
int HtDebugString (char *sLogName, int nLogMode, char *sFileName, int nLine, char *psBuf, int nBufLen)
{
	register int i,j=0;
	char	 sLine[100], sTemp[6];
	char	 sLogTime[128];
	char	 sDateTime[16];
	char	 sMilliTM[4];
	char	 sParamLogFilePath[LOG_PATH_LEN_MAX];
	int	  nParamLogMode;
	int	  nParamLogSwitchMode;
	int	  nParamLogSize; /* in megabytes */
	int	  nReturnCode;
	va_list  ap;
	time_t   lTime;
	struct tm	*tTmLocal;
	struct timeb tTimeB;
	
	/* get config parameter, LOG_MODE, LOG_FILE_PATH, LOG_SWITCH_MODE, LOG_SIZE */
	if(getenv("LOG_MODE") == NULL)
		nParamLogMode = HT_LOG_MODE_OFF;
	else   
		nParamLogMode = atoi(getenv ("LOG_MODE"));

	if (getenv("LOG_SWITCH_MODE") == NULL)
		nParamLogSwitchMode = LOG_SWITCH_MODE_SIZE;
	else
		nParamLogSwitchMode = atoi(getenv("LOG_SWITCH_MODE"));
	if (getenv("LOG_SIZE") == NULL)
		nParamLogSize = LOG_SIZE_DEFAULT;
	else
		nParamLogSize = atoi(getenv("LOG_SIZE"));
	
	memset(sParamLogFilePath, 0, sizeof (sParamLogFilePath));
	if (getenv("LOG_FILE_PATH"))
		strncpy(sParamLogFilePath, (char *)getenv("LOG_FILE_PATH"), sizeof(sParamLogFilePath) - 1);
	if (strlen(sParamLogFilePath) == 0)
		strcpy(sParamLogFilePath, ".");
	
	/* check whether the msg should be saved in log file */
	if (nParamLogMode < nLogMode)
		return 0;

	/* get current time */
	memset(sLogTime, 0x00, sizeof(sLogTime));
	memset(sDateTime, 0x00, sizeof(sDateTime));

	memset(&tTimeB, 0, sizeof(tTimeB));
	ftime(&tTimeB);
	sprintf(sMilliTM, "%03d", tTimeB.millitm);
	tTmLocal = localtime (&tTimeB.time);
	strftime(sLogTime, sizeof(sLogTime), "%Y-%m-%d %H:%M:%S", tTmLocal);
	strcat(sLogTime, ":");
	strcat(sLogTime, sMilliTM);
	strftime(sDateTime, sizeof(sDateTime), "%Y%m%d%H%M%S", tTmLocal);

	/* open log file */
	nReturnCode = OpenLogFile(sParamLogFilePath, sLogName, nParamLogSwitchMode, nParamLogSize, sDateTime);
	if (nReturnCode)
		return (nReturnCode);
		
	fprintf(fp, "[%s][%s][%d]len=%d\n", sLogTime,sFileName,nLine,nBufLen); 
	fprintf(fp, "%80.80s\n", SINGLE_LINE); 

	/* save log msg in file */
	for (i=0; i < nBufLen; i++) {
		/* initialize a new line */
		if (j == 0) {
			memset( sLine, ' ', sizeof(sLine));
			sprintf(sTemp, "%04d:", i );
			memcpy (sLine, sTemp, 5);
			sprintf(sTemp, ":%04d", i+15 );
			memcpy(sLine+72, sTemp, 5);
		}
		
		/* output psBuf value in hex */
		sprintf(sTemp, "%02X ", (unsigned char)psBuf[i]);
		memcpy(&sLine[j*3+5+(j>7)], sTemp, 3);
		
		/* output psBuf in ascii */
		if (isprint((unsigned char)psBuf[i])) {
			sLine[j+55+(j>7)]=psBuf[i];
		} else {
			sLine[j+55+(j>7)]='.';
		}
		j++;
		
		/* output the line to file */
		if (j == 16) {
			sLine[77] = 0;
			fprintf(fp, "%s\n", sLine);
			j=0;
		}
	}
	
	/* last line */
	if (j) {
		sLine[77]=0;
		fprintf(fp, "%s\n", sLine);
	}
	fprintf(fp, "%80.80s\n", SINGLE_LINE); 
	fflush(fp);

	/* close file */
	fclose (fp);
	return (0);	
}

int HtWriteLog(char *sLogName, int nLogMode, char *sFileName, int nLine, char *psBuf, int nBufLen)
{
	char	sLogTime[128];
	char	sDateTime[16];
	char	sMilliTM[4];
	char	sParamLogFilePath[LOG_PATH_LEN_MAX];
	int	 nParamLogMode;
	int	 nParamLogSwitchMode;
	int	 nParamLogSize;
	int	 nReturnCode;
	long	lTimeTick;
	va_list ap;
	time_t  lTime;
	struct tm	*tTmLocal;
	struct tms	tTMS;
	struct timeb tTimeB;
	char	sWriteBuf[1024];
	char	sWriteLen[5];
	
	/* get config parameter, LOG_MODE, LOG_FILE_PATH, LOG_SWITCH_MODE, LOG_SIZE */
	if (getenv("LOG_MODE") == NULL)
		nParamLogMode = HT_LOG_MODE_OFF;
	else   
		nParamLogMode = atoi(getenv("LOG_MODE"));
	if (getenv("LOG_SWITCH_MODE") == NULL)
		nParamLogSwitchMode = LOG_SWITCH_MODE_SIZE;
	else
		nParamLogSwitchMode = atoi(getenv("LOG_SWITCH_MODE"));
	if (getenv("LOG_SIZE") == NULL)
		nParamLogSize = LOG_SIZE_DEFAULT;
	else
		nParamLogSize = atoi(getenv("LOG_SIZE"));
	
	memset(sParamLogFilePath, 0, sizeof(sParamLogFilePath));
	if (getenv("LOG_FILE_PATH"))
		strncpy(sParamLogFilePath, (char *)getenv("LOG_FILE_PATH"), sizeof(sParamLogFilePath) - 1);
	if (strlen(sParamLogFilePath) == 0)
		strcpy(sParamLogFilePath, ".");
	
	/* check whether the msg should be saved in log file */
	if (nParamLogMode < nLogMode)
		return 0;

	/* get current time */
	memset(sLogTime, 0x00, sizeof(sLogTime));
	memset(sDateTime, 0x00, sizeof(sDateTime));

	memset(&tTimeB, 0, sizeof(tTimeB));
	ftime(&tTimeB);
	sprintf(sMilliTM, "%03d", tTimeB.millitm);
	tTmLocal = localtime (&tTimeB.time);
	strftime(sLogTime, sizeof(sLogTime), "%Y-%m-%d %H:%M:%S", tTmLocal);
	strcat(sLogTime, ":");
	strcat(sLogTime, sMilliTM);
	strftime(sDateTime, sizeof(sDateTime), "%Y%m%d%H%M%S", tTmLocal);

	/* open log file */
	nReturnCode = OpenLogFile(sParamLogFilePath, sLogName, nParamLogSwitchMode, nParamLogSize, sDateTime);
	if (nReturnCode)
		return (nReturnCode);
		
	memset(sWriteBuf, 0, sizeof(sWriteBuf));
	memset(sWriteLen, 0, sizeof(sWriteLen));
	sprintf(sWriteLen, "%04d", nBufLen);
	memcpy(sWriteBuf, sLogTime, 23);
	memcpy(sWriteBuf+23, sWriteLen, 4);
	memcpy(sWriteBuf+27, psBuf, nBufLen);

	/* save msg in file */
	fwrite(sWriteBuf,nBufLen+27,1,fp);
	fprintf(fp, "\n");

	fflush(fp);
	/* close file */
	fclose (fp);
	return (0);	
}


/*****************************************************************************/
/* FUNC:   int OpenLogFile (char *sLogFilePath, char *sLogName,			  */
/*						  int nLogSwitchMode, int nLogSize,				*/
/*						  char *sDate, FILE *fp)						   */
/* INPUT:  sLogFilePath: ��־·��											*/
/*		 sLogName: ��־�ļ���											  */
/*		 nLogSwitchMode: ��־�л�ģʽ									  */
/*				   LOG_SWITCH_MODE_SIZE, LOG_SWITCH_MODE_DATE			  */
/*		 nLogSize: LOG_SWITCH_MODE_SIZEģʽ���ļ���С					  */
/*		 sDateTime: ��ǰʱ��, YYYYMMDDhhmmss							   */
/* OUTPUT: fp: �򿪵���־�ļ���ָ��										  */
/* RETURN: 0: �ɹ�, ����: ʧ��											   */
/* DESC:   ����nLogSwitchMode, ����־�ļ�								  */
/*		 LOG_SWITCH_MODE_SIZE: ���ļ���С(M)�ﵽnLogSize, �л������ļ�,	*/
/*							   ԭ�ļ�����Ϊ�ļ����д���ʱ��				*/
/*							   xx.log.YYYYMMDDhhmmss					   */
/*		 LOG_SWITCH_MODE_DATE: ��־�ļ�����������, xx.log.YYYYMMDD		 */
/*****************************************************************************/
int OpenLogFile (char *sLogFilePath, char *sLogName, int nLogSwitchMode, int nLogSize, char *sDateTime )
{
	char		sExpFilePath[LOG_PATH_LEN_MAX];
	char		sFullLogName[LOG_PATH_LEN_MAX+LOG_NAME_LEN_MAX];
	char		sFullBakLogName[LOG_PATH_LEN_MAX+LOG_NAME_LEN_MAX];
	int			nReturnCode;
	struct stat	statbuf;
	
	memset (sFullLogName, 0x00, sizeof (sFullLogName));
	
	strcpy (sExpFilePath, sLogFilePath);
	nReturnCode = ReplaceEnvVar (sExpFilePath);
	
	/* set log file name */
	if (!sLogName || strlen(sLogName) == 0)
		sprintf (sFullLogName, "%s/%s", sExpFilePath, LOG_DEFAULT_NAME);
	else
		sprintf (sFullLogName, "%s/%s", sExpFilePath, sLogName);
	
	if (nLogSwitchMode == LOG_SWITCH_MODE_DATE)
	{
		/* append date in log file name */
		strncat (sFullLogName, sDateTime, 8);
	}
	else
	{
		/* this is LOG_SWITCH_MODE_SIZE */
		/* check file size */
		memset (&statbuf, 0x00, sizeof(statbuf));
		nReturnCode = stat (sFullLogName, &statbuf);
		if ( nReturnCode == 0 && statbuf.st_size >= LOG_SIZE_UNIT * nLogSize )
		{
			memset (sFullBakLogName, 0x00, sizeof(sFullBakLogName));
			sprintf (sFullBakLogName, "%s.%s", sFullLogName, sDateTime);
			rename (sFullLogName, sFullBakLogName);
		}
	}

	/* open log file */
	fp = fopen (sFullLogName, "a+");
	if (fp == NULL )
		return (ERR_CODE_LOG_BASE+errno);

	return 0;
}

int	 ConvertEnv(char *str)
{
	char envbuf[LOG_PATH_LEN_MAX],buf[LOG_PATH_LEN_MAX];
	char *ptr=NULL,*ptr1=NULL;
	int  len,choice=0;
	
	memset(buf, 0, sizeof(buf));

	ptr=(char *)strpbrk((char *)str,"./");
	if ( ptr != NULL )
	{
		choice = 1;
		memset(envbuf, 0, sizeof(envbuf));
		memcpy(envbuf, &str[1], ptr-str-1);
	}
	else
	{
		choice = 2;
		memset(envbuf, 0, sizeof(envbuf));
		strcpy(envbuf,&str[1]);
	}

	ptr1=(char *)getenv(envbuf);
	if ( ptr1 == (char *)NULL )
	{
		return -1;
	}
	else
	{
		if ( choice == 1 )
		{
			strcat(buf, ptr1);
			strcat(buf, ptr);
		}
		else if ( choice == 2 )
			strcat(buf, ptr1);
	}

	len=strlen(buf);

	memset(str, 0, sizeof(str));
	strcpy(str, buf);

	return 0;
}

/*****************************************************************************/
/* FUNC:   int ReplaceEnvVar(char *str)									  */
/* INPUT:  str: �ַ���, ���ܰ�����������									 */
/*			  ��: $(HOME)/src/$TERM										*/
/* OUTPUT: str: �ַ���, ���������Ѿ��ö�Ӧ��ֵ�滻						   */
/* RETURN: 0: �ɹ�, ����: ʧ��											   */
/* DESC:   ��������ַ���, �����еĻ��������ö�Ӧ��ֵ�滻					*/
/*		 ��: $(HOME)/src/$TERM, ���е�$(HOME)��$TERM�ᱻ�滻			   */
/*		 ����ʱ�뱣֤str��Ӧ��buffer�㹻��, �������滻���Խ��			 */
/*****************************************************************************/
int ReplaceEnvVar(char *str)
{
	char *ptr=NULL;
	char buf[LOG_PATH_LEN_MAX], field[LOG_PATH_LEN_MAX];
	int	 len=0,nRet,flag;
	int  count=1,i,j;

	/********************************************************************/
	/*		 delete	 characters ' ','(',')','\t' from string "str"		*/
	/********************************************************************/
	len=strlen(str);
	memset(buf,0,sizeof(buf));

	for(i=0,j=0;i<len;i++)
	if ( str[i] != ' ' && str[i] != '\t' && str[i] != '(' && str[i] != ')' )
		buf[j++]=str[i];

	memset(str,0, len);
	strcpy(str,buf);

	/********************************************************/
	/*		 distinguish if first character is '$' or not	*/
	/********************************************************/
	if ( str[0] == '$' )
		flag = 1;
	else
		flag = 2;

	memset(buf, 0, sizeof(buf));
	if ( flag == 1 )
	{
		   ptr=(char *)strtok((char *)str,"$");
		   while ( ptr != NULL )
		{
			memset(field, 0, sizeof(field));
			sprintf(field,"%c%s",'$',ptr);
			nRet = ConvertEnv(field);
			if ( nRet != 0 )
				return -1;
	
			strcat(buf, field);

			ptr=(char *)strtok((char *)NULL,"$");
		}
	}
	else if ( flag == 2 )
	{
		   ptr=(char *)strtok((char *)str,"$");
		   while ( ptr != NULL )
		{
			count++;
			if ( count != 2)
			{
				memset(field, 0, sizeof(field));
				sprintf(field,"%c%s",'$',ptr);
				nRet = ConvertEnv(field);
				if ( nRet != 0 )
					return -1;
			}
			else
			{
				memset(field, 0, sizeof(field));
				sprintf(field,"%s",ptr);
			}

			strcat(buf, field);

			ptr=(char *)strtok((char *)NULL,"$");
		}
	}

	len = strlen(buf);
	memcpy(str, buf, len+1);

	return 0;
}
