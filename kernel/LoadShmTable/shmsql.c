#include	"Ibp.h"
#include	"load.h"

void convert(char *src, char *dst)
{
	int flag = 1;

	for ( ; *src; src++, dst++)
	{
	   if ('\'' == *src)
	   {
		  flag = !flag;
	   }

	   if (1 == flag)
	   {
		  *dst = toupper(*src);
	   }
	   else
	   {
		  *dst = *src;
	   }
	}

	return;
}

char*	sDropChar(char *s, char c)
{
	long	i = 0, l = 0, k = 0;

	if(!s || !strlen(s))	return s;

	l = strlen(s);

	for(i = 0; i < l; i ++)
	{
		if(c != s[i])
			s[k++] = s[i];	
		else
			s[i] = 0x00;
	}
	s[k] = 0x00;
	
	return s;
}

long	lGetTableFiled(TABLE t,  BFieldDef **ppstField, long *plOut)
{
	long	   i = 0, lCount = 0;
	BFieldDef   stField, *pstField = NULL;
	RunTime  *pstRun = NULL;

	if(!(pstRun = (RunTime *)pInitRunTime(SYS_FILED_DEF)))
	{
	   fprintf(stderr, "初始化表(%s)内存块失败, err:(%s)\n",
		  sGetTableName(SYS_FILED_DEF), sGetError());
	   return RC_FAIL;
	}

	memset(&stField, 0, sizeof(stField));
	stField.m_table = lSetEXLong(t);

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = SYS_FILED_DEF;
	pstRun->lSize = sizeof(stField);
	pstRun->pstVoid = &stField;

	if(RC_SUCC != lRunSelect(pstRun, (void **)&pstField, &lCount))
	{
	   fprintf(stdout, "查询错误, err:(%s)\n", sGetError());
	   return RC_FAIL;
	}

	for(i = 0; i < lCount; i ++)
	{
	   lGetEXLong((long *)&pstField[i].m_table);
	   lGetEXLong(&pstField[i].m_lKeyAttr);
	   lGetEXLong(&pstField[i].m_lKeyOffSet);
	   lGetEXLong(&pstField[i].m_lKeyLen);
	   lGetEXLong(&pstField[i].m_lIsPk);
	   lGetEXLong(&pstField[i].m_lSeq);
		pstField[i].m_lSeq = -1;
	}

	*ppstField = pstField;
	*plOut = lCount;

	return RC_SUCC;
}

char	*sUpperSQL(char *s, long l)
{
	long	i;

	for(i=0; i < l; i++)
	{
	   if(s[i] >= 'a' && s[i] <= 'z')
		  s[i] = s[i]- 32;
	}
	return s;
}

char*	sUpperCommand(char *s, long l)
{
	char	*p = NULL, *q = NULL;
	long	i = 0;

	if(!s)  return NULL;

	for(i = 0; i < l; i ++)
	{
	   if('\t' == s[i])
		  s[i] = ' ';
	}

	convert(s, s);
	return s;
}

void	vSelectSQL(char *pszField, char *pszCon, TABLE t)
{
	char		*pszCodt = NULL, szName[256], szValue[256];
	char		*p = NULL, szTemp[256], szFmt[1024], szHead[1024];
	BOOL		bFind = false;
	double		dRate = 0.00f;
	RunTime		*pstRun = NULL;
	BFieldDef	*pstField = NULL;
	long		lSize = 0, lCount = 0, i = 0, b = 0, lOfs = 0;
	long		lLine = strlen(pszCon), lFind = 0, lRet = 0;

	if(RC_SUCC != lGetTableFiled(t, &pstField,  &lCount))
	{
	   fprintf(stderr, "获取表(%s)字段失败, err:(%s)\n", sGetTableName(t),
		  sGetError());
		return ;
	}

	if(!(pstRun = (RunTime *)pInitRunTime(t)))
	{
	   fprintf(stderr, "初始化表(%s)失败, err:(%s)\n", sGetTableName(t),
		  sGetError());
		IBPFree(pstField);
	   return ;
	}

	lSize = ((IBPShm *)pGetShmRun())->m_lLSize[t];
	if(0 >= lSize)
	{
	   fprintf(stderr, "获取表表(%s)记录大小失败!\n", sGetTableName(t));
		IBPFree(pstField);
		return ;
	}

	pszCodt = (char *)malloc(lSize);
	if(!pszCodt)
	{
	   fprintf(stderr, "分配内存(%d)失败, err:(%s)\n", lSize, strerror(errno));
		IBPFree(pstField);
		return ;
	}
	memset(pszCodt, 0, lSize);

	while((p = strstr(pszCon + lOfs, "=")))
	{
		memset(szTemp, 0, sizeof(szTemp));
		strcpy(szTemp, sGetValueByIdx(pszCon, "AND", ++ b));
		lOfs += strlen(szTemp) + strlen("AND");

		memset(szName, 0, sizeof(szName));
		strcpy(szName, sGetValueByIdx(szTemp, "=", 1));
		sTrimAll(szName);
		sUpper(szName);

		memset(szValue, 0, sizeof(szValue));
		strcpy(szValue, sGetValueByIdx(szTemp, "=", 2));
		sTrimAll(szValue);

		if(!strcmp(szName, "1") && !strcmp(szValue, "1"))
			break;
		for(i = 0; i < lCount; i ++)
		{
			if(!strcmp(pstField[i].m_szField, szName))
			{
				if(FIELD_CHAR == pstField[i].m_lKeyAttr)
					memcpy(pszCodt + pstField[i].m_lKeyOffSet, sDropChar(szValue, '\''), 
						pstField[i].m_lKeyLen);
				else if(FIELD_LONG == pstField[i].m_lKeyAttr)
				{
					lRet = lSetEXLong(atol(szValue));
					memcpy(pszCodt + pstField[i].m_lKeyOffSet, &lRet, pstField[i].m_lKeyLen);
				}
				else
					;
			}
		}

		if(lLine <= lOfs)	break;
	}

	pstRun->lFind = CHK_SELECT;
	pstRun->tblName = t;
	pstRun->lSize = lSize;
  	pstRun->pstVoid = pszCodt;

	memset(szFmt, 0, sizeof(szFmt));
	memset(szHead, 0, sizeof(szHead));
	sTrimAll(pszField);

	if(!strcmp(pszField, "*"))
	{
		for(i = 0; i < lCount; i ++)
		{
//			sprintf(szFmt, "%%%ds ", pstField[i].m_lKeyLen);
			sprintf(szHead + strlen(szHead), "%s\t", pstField[i].m_szField);
			pstField[i].m_lSeq = 1;
		}
	}
	else
	{
		lOfs = b = 0;
		strcat(pszField, ",");
		while((p = strstr(pszField + lOfs, ",")))
		{
			bFind = false;
			memset(szName, 0, sizeof(szName));
			strcpy(szName, sGetValueByIdx(pszField, ",", ++ b));
			sTrimAll(szName);
			sUpper(szName);
			lOfs += strlen(szName) + strlen(",");
			
			for(i = 0; i < lCount; i ++)
			{
				if(!strcmp(pstField[i].m_szField, szName))
				{
					bFind = true;
					sprintf(szHead + strlen(szHead), "%s\t", pstField[i].m_szField);
					pstField[i].m_lSeq = 1;
					break;
				}		
			}

			if(!bFind)
			{
	   			fprintf(stderr, "错误, 表(%s)不存在字段(%s)\n\n", sGetTableName(t), szName);
				IBPFree(pstField);
				IBPFree(pszCodt);
				return ;
			}
		}
	}

	fprintf(stdout, "\n\n\n------------------------------------------------------SQL结果"
	   "-----------------------------------------------------\n");
	fprintf(stdout, "%s\n", szHead);

	lRet = lTableDeclare(pstRun);
	if(RC_SUCC != lRet)
	{   
		fprintf(stderr, "打开表%s游标失败, 原因:%s\n", sGetTableName(t), sGetError());
		IBPFree(pstField);
		IBPFree(pszCodt);
	   return ;
	}

	while(1) 
	{
		memset(pszCodt, 0, lSize);
	   lRet = lTableFetch(t, pszCodt, lSize);
	   if(RC_FAIL == lRet)
	   {
		  fprintf(stderr, "查询表%s错误, 原因:%s", sGetTableName(t), sGetError());
		  vTableClose(t, SHM_DISCONNECT);
			IBPFree(pstField);
			IBPFree(pszCodt);
		  return ;
	   }
	   else if(RC_NOTFOUND == lRet)
			break;

		++ lFind;

		for(i = 0; i < lCount; i ++)
		{
			if(pstField[i].m_lSeq < 0)
				continue;

			if(FIELD_CHAR == pstField[i].m_lKeyAttr)
//				fprintf(stdout, "%.*s|", pstField[i].m_lKeyLen, pszCodt + pstField[i].m_lKeyOffSet);
				fprintf(stdout, "%s|", pszCodt + pstField[i].m_lKeyOffSet);
			else if(FIELD_LONG == pstField[i].m_lKeyAttr)
			{
				if(sizeof(double) == pstField[i].m_lKeyLen)
				{
					memcpy(&dRate, pszCodt + pstField[i].m_lKeyOffSet, pstField[i].m_lKeyLen);
					fprintf(stdout, "%.2f\t|", dRate);
				}
				else
				{
					memcpy(&lRet, pszCodt + pstField[i].m_lKeyOffSet, pstField[i].m_lKeyLen);
					lRet = lGetEXLong(&lRet);
					fprintf(stdout, "%d\t|", lRet);
				}
			}
			else
				;
		}
		fprintf(stdout, "\n");
		
	}
  	vTableClose(t, SHM_DISCONNECT);
	IBPFree(pstField);
	IBPFree(pszCodt);

	fprintf(stdout, "\n(%d)记录被选中\n", lFind);
}

void	vAnalySQL(char *pszSQL)
{
	TABLE	  table;
	long	   lCount = 0, lOperate = 0;
	char	   szCondion[2048], szFiled[1024];
	char	szTable[1024], *p = NULL, *q = NULL;
	BFieldDef   *pstField = NULL;

	memset(szCondion, 0, sizeof(szCondion));
	memset(szFiled, 0, sizeof(szFiled));

	sTrimLeft(pszSQL);
	sTrimRight(pszSQL);
	if(!pszSQL || !strlen(pszSQL)) return ;
	strcpy(szCondion, pszSQL);

	sUpperCommand(pszSQL, strlen(pszSQL));
	if(((p = strstr(pszSQL, "SELECT ")) && (p = strstr(pszSQL, " FROM "))))
		lOperate = 1;
	else if(((p = strstr(pszSQL, "UPDATE ")) && (p = strstr(pszSQL, " SET "))))
		lOperate = 2;
	else if(((p = strstr(pszSQL, "DELETE ")) && (p = strstr(pszSQL, " FROM "))))
		lOperate = 0;
	else if(((p = strstr(pszSQL, "INSERT ")) && (p = strstr(pszSQL, " INTO "))))
		lOperate = 0;
	else if(((p = strstr(pszSQL, "ALTER ")) && (p = strstr(pszSQL, " TABLE "))))
		lOperate = 0;
	else if(((p = strstr(pszSQL, "CREATE ")) && (p = strstr(pszSQL, " TABLE "))))
		lOperate = 0;
	else
	{
	   fprintf(stderr, "*错误, 无效的SQL指令,%d\n", __LINE__);
	   return ;
	}

	if(0 == lOperate)
	{
	   fprintf(stderr, "*目前只支持SELECT、UPDATE指令\n");
		return ;
	}
	else if(1 == lOperate)
	{
		memset(szTable, 0, sizeof(szTable));
		p = strstr(pszSQL, " FROM ");
		q = strstr(pszSQL, " WHERE");
		if(!q)
		{
			lCount = p - pszSQL;
			memcpy(szTable, p + strlen(" FROM "), strlen(pszSQL) - lCount + strlen(" FROM "));
		}
		else
		{
			lCount = strlen(" FROM ");
			memcpy(szTable, p + lCount, (q - p) - lCount);
		}
		sTrimCRLF(szTable);

		table = tGetTableNumb(szTable);
		if((long )table <= RC_FAIL)
		{
			fprintf(stderr, "*表(%s)不存在\n\n", szTable);
			return ;
		}

		memset(szFiled, 0, sizeof(szFiled));
		memcpy(szFiled, pszSQL + strlen("SELECT "), (p - pszSQL) - strlen("SELECT "));
		sTrimAll(szFiled);
		if(!(q = strstr(pszSQL, " WHERE ")))
		{
			q = strstr(pszSQL, szTable);
			if(strlen(q) > strlen(szTable))
			{
	   			fprintf(stderr, "*错误, 无效的SQL指令,%d\n", __LINE__);
				return ;
			}

			memset(szCondion, 0, sizeof(szCondion));
			strcpy(szCondion, "1=1");
		}
		else	
		{
			strcpy(szCondion, q + strlen(" WHERE "));
		}

		fprintf(stdout, "查询表(%s)(%d), 查询字段[%s], 查询条件[%s]\n", szTable, table, 
			szFiled, szCondion);
		vSelectSQL(szFiled, szCondion, table);
	}
	else if(2 == lOperate)
	{
		fprintf(stdout, "暂未实现\n");
		return ;
	}

	return ;	
}

/*
int		main(int argc, char *argv[])
{

//	char	szSQL[]="select	id, tx_date	from TBL_PUB_DISC_ALGO WHERE tx_date = 21321 and 1= 1";
//	char	szSQL[]="SELECT	*, ID, TX_DATE, DISC_CYCLE	FROM TBL_PUB_DISC_ALGO WHERE ID = 'ABCDE' AND TX_DATE = '2015REAL'";
//	char	szSQL[]="SELECT	ACCT_ID,APP_TYPE,APP_ID FROM TBL_ACCT_INFO WHERE 1 = 1";
	char	szSQL[]="SELECT	USER_ID,MCHT_NO,USER_NAME,USER_EMAIL FROM TBL_MCHT_USER WHERE USER_ID = 10000112";
//	char	szSQL[]="SELECT	* FROM TBL_PUB_DISC_ALGO WHERE 1=1";
//	char	szSQL[]="select	*	from TBL_PUB_DISC_ALGO\n";


	vSetLogName("tt.log");




	vAnalySQL(szSQL);




	return RC_SUCC;
}
*/

