#include	"load.h"
#include	"Ibp.h"

/****************************************************************************************
 *  *  Function:  sGetFileValue	从配置文件获取数据
 *  *  Description:				获取文件有效数据 name=value
 ****************************************************************************************/
char*   sGetFileValue(char *pszPath, char *pszName, char *pszKey)
{
	static  char	szValue[256];
	char	szLine[1024], szName[256];
	FILE	*fp = NULL;

	memset(szLine, 0, sizeof(szLine));
	memset(szValue, 0, sizeof(szValue));
	if(NULL == (fp = fopen(pszPath, "rb")))
		return NULL;

	while(fgets(szLine, sizeof(szLine), fp))
	{
		sTrimAll(szLine);
		if(0 == strlen(szLine) || '#' ==  szLine[0] || ('/' == szLine[0] && '/' == szLine[1]))
		{
			memset(szLine, 0, sizeof(szLine));
			continue;
		}

		memset(szName, 0, sizeof(szName));
		strcpy(szName, sGetValueByIdx(szLine, pszKey, 1));
		sTrimAll(szName);

		if(!strcmp(szName, pszName))
		{
			strcpy(szValue, sGetValueByIdx(szLine, pszKey, 2));
			sTrimCRLF(szValue);
			break;
		}
	}

	fclose(fp);
	return szValue;
}

/****************************************************************************************
 *  *  Function:  sGetFileValue	从配置文件获取数据
 *   *  Description:				获取文件有效数据 name=value
****************************************************************************************/
char*   sGetDbsEnv(char *pszName)
{
	static  char	szValue[256];
	char	szPath[512];

	memset(szValue, 0, sizeof(szValue));
	memset(szPath, 0, sizeof(szPath));

	snprintf(szPath, sizeof(szPath), "%s", getenv("BATCHCFG"));

	strcpy(szValue, sGetFileValue(szPath, pszName, "="));
	return szValue;
}

/**********************************************************************************
 *  Function:  sGetRealFile		获取去掉路径的真实文件名称
 *  Description:				不改变输入字符串的值
 **********************************************************************************/
char*	sGetRealFile(char *p)
{
	long	i = 0;	
	char	*q = NULL;

	if(!p || !strlen(p))
		return p;

	i = strlen(p) - 1;

	for(i; i >= 0; i --)
	{
#if(defined(__WIN32__) || defined(_WIN32))
		if('\\' == p[i])
#else
		if('/' == p[i])
#endif
			break;
	}
	q = p + i + 1;
	return q;
}

/**********************************************************************************
 *  Function:  sGetLastCwd		返回文件上层路径	
 *  Description:				返回文件上层路径字符串
 **********************************************************************************/
char*   sGetLastCwd(char *s)
{
	char	*p = NULL;
	static	char	szPath[2048];

	memset(szPath, 0, sizeof(szPath));
	if(!s || !strlen(s))
		return szPath;

	if(!(p = strrchr(s, '/')))
		return szPath;

	memcpy(szPath, s, p - s);
	szPath[sizeof(szPath) - 1] = 0x00;

	return szPath;
}

