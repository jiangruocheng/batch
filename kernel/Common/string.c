#include	"Ibp.h"
#include	"load.h"

/**********************************************************************************
 *  *  Function:  sUpper		将英文字符串改成大写
 *  *  Description:				改变输入字符串的值
 **********************************************************************************/
char	*sUpper(char *s)
{
	long	i, l = strlen(s);

	for(i=0; i < l; i++)
	{
		if(s[i] >= 'a' && s[i] <= 'z')
			s[i] = s[i]- 32;
	}
	return s;
}

/**********************************************************************************
 *  *  Function:  sLower		将英文字符串改成小写
 *  *  Description:				改变输入字符串的值
 **********************************************************************************/
char	*sLower(char *s)
{
	long	i, l = strlen(s);

	for(i = 0; i < l; i++)
	{
		if(s[i] >= 'A' && s[i] <= 'Z')
			s[i] = s[i] + 32;
	}

	return s;
}

/**********************************************************************************
 *  *  Function:  sTrimCRLF		去掉字符串中的回车换行
 *  *  Description:				改变输入字符串的值
 **********************************************************************************/
char*   sTrimCRLF(char *p)
{
	int	 i = 0, j = 0, m = 0;
	char	*s = NULL;

	if(!p)		   return p;
	if(!strlen(p))   return p;

	s = p;
	m = strlen(p);
	for(i = 0; i < m; i ++)
	{
		if(p[i] == 10 || p[i] == 13)
			continue;
		s[j] = p[i];
		j ++;
	}
	s[j] = 0x00;
	return s;
}
/**********************************************************************************
 *  *  Function:  sTrimLeft		去掉字符串中的回车换行
 *  *  Description:				改变输入字符串的值
 **********************************************************************************/
char*	sTrimLeft(char *p)
{
	long	i, k = 0;

	if(!p || !strlen(p))
		return p;

	for(i = 0; i < (long)strlen(p); i ++)
	{
		if(p[i] ==' ')
		{
			k ++;
			continue;
		}
		break;
	}
	return p + k;
}

/**********************************************************************************
 *  *  Function:  sTrimRight		去掉字符串中的回车换行
 *  *  Description:				改变输入字符串的值
 **********************************************************************************/
char*	sTrimRight(char *p)
{
	long	i, k = 0, l = 0;

	if(!p || !strlen(p))
		return p;

	l = strlen(p);
	for(i = strlen(p) - 1; i >= 0; i --)
	{
		if(p[i] == ' ')
			continue;
		break;
	}

//	p[i + 1] = '\0';
	memset(p + i + 1, 0, l - i - 1);

	return p;
}

/**********************************************************************************
 *  *  Function:  sTrimAll		去掉字符串中的所有空格
 *  *  Description:				改变输入字符串的值
 **********************************************************************************/
char*	sTrimAll(char *p)
{
	long	i, k = 0, l = 0;
	char	*q = p;

	if(!p || !strlen(p))
		return p;

	l = strlen(p);
	for(i = 0; i < l; i ++)
	{
		if(p[i] == ' ')
			continue;
		q[k ++] = p[i];
	}
	memset(q + k, 0, l - k);
	p = q;

	return q;
}

/**********************************************************************************
 *  *  Function:  lGetRecordNum		符串s出现总次数
 *  *  Description:					获取p 字符串中  s字符串出现总次数
 **********************************************************************************/
long	lGetRecordNum(char *p, char *s, long l)
{
	long	i, iLen, iCount = 0;
	char	*q = p;

	if(!p)		return 0;

	iLen = strlen(s);
	for(i = 0; i < l; i++, q++ )
	{
		if(0 == memcmp(q, s, iLen))
		{
			q = q + iLen - 1;
			iCount ++;
		}
	}

	return iCount;
}

/**********************************************************************************
 *  *  Function:  sGetValueByIdx	符串s出现总次数
 *  *  Description:					根据分隔符和域值取数据
 **********************************************************************************/
char*	sGetValueByIdx(char *p, char *s, long lIdx)
{
	long	lCnt = 0, lBeg = 0, lFlag = 0;
	long	i = 0, l = 0, lLen = 0;
	static	char szOut[1024];
	
	l = strlen(s);
	
	memset(szOut, 0, sizeof(szOut));
	while(1)
	{
		if((long)strlen(p) - l < i && lFlag == 0 )
		{
			if(lIdx == lCnt + 1)
				memcpy(szOut, p + lBeg, strlen(p) - lBeg);
			break;
	   }
	
	   else if(lIdx == lCnt && lFlag == 1)
	   {
			memcpy(szOut, p + lBeg, lLen);
			break;
	   }
	   else if(lFlag == 1)
		  lBeg = i;
	
	   if(!memcmp(p + i, s, l) )
	   {
		   lLen = i - lBeg;
		   lCnt += 1;
		   i += l;
		   lFlag = 1;
	   }
	   else
	   {
		  lFlag = 0;
		  i += 1;
	   }
	}

	return szOut;
}

/****************************************************************************************
* *  Function:  sEncryAcNo   加密卡号 采用前6后4方式，其他用*代替
* *  Description:			
****************************************************************************************/
char	*sEncryAcNo(char *pszActNo)
{
	long	l = 0, d = 4, i = 0; 
	static  char	szEncry[50];

	memset(szEncry, 0, sizeof(szEncry));
	if(!pszActNo || !strlen(pszActNo))
		return pszActNo;

	sTrimAll(pszActNo);
	l = strlen(pszActNo);
	if(10 > l)  return pszActNo;

	memcpy(szEncry, pszActNo, 6);
	for(i = 6; i < l - d; i ++)
		szEncry[i] = '*'; 
	memcpy(szEncry + i, pszActNo + (l - d), d);
	return szEncry;
}

