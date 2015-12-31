#include	"Ibp.h"
#include	"load.h"

/**********************************************************************************
 *  *  Function:  sUpper		��Ӣ���ַ����ĳɴ�д
 *  *  Description:				�ı������ַ�����ֵ
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
 *  *  Function:  sLower		��Ӣ���ַ����ĳ�Сд
 *  *  Description:				�ı������ַ�����ֵ
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
 *  *  Function:  sTrimCRLF		ȥ���ַ����еĻس�����
 *  *  Description:				�ı������ַ�����ֵ
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
 *  *  Function:  sTrimLeft		ȥ���ַ����еĻس�����
 *  *  Description:				�ı������ַ�����ֵ
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
 *  *  Function:  sTrimRight		ȥ���ַ����еĻس�����
 *  *  Description:				�ı������ַ�����ֵ
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
 *  *  Function:  sTrimAll		ȥ���ַ����е����пո�
 *  *  Description:				�ı������ַ�����ֵ
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
 *  *  Function:  lGetRecordNum		����s�����ܴ���
 *  *  Description:					��ȡp �ַ�����  s�ַ��������ܴ���
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
 *  *  Function:  sGetValueByIdx	����s�����ܴ���
 *  *  Description:					���ݷָ�������ֵȡ����
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
* *  Function:  sEncryAcNo   ���ܿ��� ����ǰ6��4��ʽ��������*����
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

