#include	"load.h"

/****************************************************************************************
	���̻������˺��������
 ****************************************************************************************/
char*	sGetComIdx(char *pszAppId, long lAppSize, long  lAppType, long lAcctDef)
{
	long			n = sizeof(long);
	static	char	szIdx[MAX_INDEX_LEN];

	if(n > 4 || (lAppSize + 2 * n) > 30 )
	{
		vSetErrMsg("����������ȳ��ޣ�������MAX_INDEX_LEN����");
		return NULL;
	}

	memset(szIdx, 0, sizeof(szIdx));
	memcpy(szIdx, pszAppId, lAppSize); 
	memcpy(szIdx + lAppSize, (char *)&lAppType, n);
	memcpy(szIdx + lAppSize + n, (char *)&lAcctDef, n);

	return szIdx;
}

/****************************************************************************************
	��T+0ע���̻��������
 ****************************************************************************************/
char*	sGetRegistIdx(char *pszObjId, long lObjSize, long  lSrvId, char chObjType)
{
	long			n = sizeof(long);
	static	char	szIdx[MAX_INDEX_LEN];

	if(n > 4 || (lObjSize + n + 1) > 30 )
	{
		vSetErrMsg("����������ȳ��ޣ�������MAX_INDEX_LEN����");
		return NULL;
	}

	memset(szIdx, 0, sizeof(szIdx));
	memcpy(szIdx, pszObjId, lObjSize); 
	memcpy(szIdx + lObjSize, (char *)&lSrvId, n);
	szIdx[lObjSize + n] = chObjType;

	return szIdx;
}

/****************************************************************************************
	����AVL��
 ****************************************************************************************/
void	vDestroyTree(Tree *pstTree)
{
	if(NULL == pstTree)
		return ;

	vDestroyTree(pstTree->m_pstLeft);
	vDestroyTree(pstTree->m_pstRight);
	IBPFree(pstTree);
}

/****************************************************************************************
 *  *  ��ȡ���������ֵ
 ****************************************************************************************/
SHTree* pGetMaxSHTree(void *pstRoot)
{
	SHTree  *pstTree = NULL;

	pstTree = (SHTree *)pstRoot;
	if(NULL == pstTree)	 return NULL;

	while(0 != pstTree->m_lRight)
		pstTree = (SHTree *)((char *)pstRoot + pstTree->m_lRight);

	return pstTree;
}

/****************************************************************************************
 *  *  ��ȡ���������ֵ
 ****************************************************************************************/
SHTree* pGetMinSHTree(void *pstRoot)
{
	SHTree  *pstTree = NULL;

	pstTree = (SHTree *)pstRoot;
	if(NULL == pstTree)	 return NULL;

	while(0 != pstTree->m_lLeft)
		pstTree = (SHTree *)((char *)pstRoot + pstTree->m_lLeft);

	return pstTree;
}

/****************************************************************************************
	��AVL��������ƥ������
  ****************************************************************************************/
SHTree* pSearchSHTree(void *pstRoot, long lOffset, void *pstVoid)
{
	static  SHTree  stTree;
	SHTree  *pstTree = NULL;

	pstTree = (SHTree *)((char *)pstRoot + lOffset);
	if(NULL == pstTree)	 return NULL;

	if(!memcmp(pstTree->m_szIdx, pstVoid, pstTree->m_lIdxLen))
	{
		memcpy(&stTree, pstTree, sizeof(SHTree));
		return &stTree;
	}

	if(0 < memcmp(pstTree->m_szIdx, pstVoid, pstTree->m_lIdxLen))
	{
		if(0 == pstTree->m_lLeft)
			return NULL;
		return pSearchSHTree(pstRoot, pstTree->m_lLeft, pstVoid);
	}
	else
	{
		if(0 == pstTree->m_lRight)
			return NULL;
		return pSearchSHTree(pstRoot, pstTree->m_lRight, pstVoid);
	}
}

/****************************************************************************************
 *	 ����ڴ����ṹ  
 ****************************************************************************************/
SHTree* pInitSHTree(Tree *pstTree)
{
	static  SHTree  stTree;

	memset(&stTree, 0, sizeof(stTree));
	if(!pstTree) return &stTree;

	memset(&stTree, 0, sizeof(SHTree));
	if(MAX_INDEX_LEN < pstTree->m_lKeyLen)
		return &stTree;

	stTree.m_lKey = pstTree->m_lKey;
	memcpy(stTree.m_szIdx, pstTree->m_pstKey, pstTree->m_lKeyLen);
	stTree.m_lIdxLen = pstTree->m_lKeyLen;
	stTree.m_lIdxPos = pstTree->m_lKeyPos;
	stTree.m_lHeight = pstTree->m_lHeight;
	stTree.m_lData = pstTree->m_lData;

	if(!pstTree->m_pstLeft)
		stTree.m_lLeft = 0;
	else
		stTree.m_lLeft = pstTree->m_pstLeft->m_lKey * sizeof(SHTree);

	if(!pstTree->m_pstRight)
		stTree.m_lRight = 0;
	else
		stTree.m_lRight = pstTree->m_pstRight->m_lKey * sizeof(SHTree);

	return &stTree;
}

/****************************************************************************************
	���ڵ�����ת���ṹ����ָ����ַ��
 ****************************************************************************************/
void	vInorderTree(void *pstVoid, Tree *pstTree)
{
	long	lOffset = 0;
	SHTree  *pstSHTree = NULL;

	if(!pstTree)		return ;
	if(!(pstSHTree = pInitSHTree(pstTree)))		return ;

	memcpy((char *)pstVoid + pstTree->m_lKey * sizeof(SHTree), pstSHTree, sizeof(SHTree));
	vInorderTree(pstVoid, pstTree->m_pstLeft);
	vInorderTree(pstVoid, pstTree->m_pstRight);
}

/****************************************************************************************
	�ݹ�ʵ�ֲ���"AVL��"�м�ֵ
 ****************************************************************************************/
Tree*	pSearchTree(Tree *pstTree, void *pstVoid, long lLen)
{
	if(NULL == pstTree)
		return NULL;

	if(!memcmp(pstTree->m_pstKey, pstVoid, lLen))
		return pstTree;
	if(0 < memcmp(pstTree->m_pstKey, pstVoid, lLen))
		return pSearchTree(pstTree->m_pstLeft, pstVoid, lLen);
	else
		return pSearchTree(pstTree->m_pstRight, pstVoid, lLen);
}

/****************************************************************************************
	������С��㣺����treeΪ������AVL������С��㡣
 ****************************************************************************************/
Tree*	pGetTreeMin(Tree *pstTree)
{
	if(NULL == pstTree)
		return NULL;

	while(pstTree->m_pstLeft != NULL)
		pstTree = pstTree->m_pstLeft;
	return pstTree;
}

/****************************************************************************************
	��������㣺����treeΪ������AVL��������㡣
 ****************************************************************************************/
Tree*	pGetTreeMax(Tree *pstTree)
{
	if(NULL == pstTree)
		return NULL;

	while(pstTree->m_pstRight != NULL)
		pstTree = pstTree->m_pstRight;
	return pstTree;
}

/****************************************************************************************
	Debug
 ****************************************************************************************/
void	vDebug(Tree *pstRoot, char *pszKey, long lType)
{
	if(!pstRoot)	return ;

	if(0 == lType)		// tree�Ǹ��ڵ�
		printf("%08X:%s is (%s)root:(%d)(%d)\n", pstRoot, pstRoot->m_pstKey, pszKey, pstRoot->m_lKey, pstRoot->m_lHeight);
	else				// tree�Ƿ�֧�ڵ�
		printf("%08X:%s is %s's %6s child(%d)(%d)\n", pstRoot, pstRoot->m_pstKey, pszKey, lType == 1? "right" : "left", 
			pstRoot->m_lKey, pstRoot->m_lHeight);

	vDebug(pstRoot->m_pstLeft, pstRoot->m_pstKey, -1);
	vDebug(pstRoot->m_pstRight,pstRoot->m_pstKey,  1);
}

/****************************************************************************************
	��ȡ�ڵ�߶�ֵ
 ****************************************************************************************/
long	lGetHeight(Tree *pstTree)
{
	if(!pstTree)
		return -1;
	else
		return pstTree->m_lHeight;
}

/****************************************************************************************
	����LL�����Ӧ����� �ҵ���ת����
 ****************************************************************************************/
Tree*	_pRightRotation(Tree *pstTree)
{
	Tree	*pstTmp = NULL;

	pstTmp = pstTree->m_pstLeft;
	pstTree->m_pstLeft = pstTmp->m_pstRight;
	pstTmp->m_pstRight = pstTree;

	pstTree->m_lHeight = MAX(lGetHeight(pstTree->m_pstLeft), lGetHeight(pstTree->m_pstRight)) + 1;
	pstTmp->m_lHeight = MAX(lGetHeight(pstTmp->m_pstLeft), pstTree->m_lHeight) + 1;

	return pstTmp;
}

/****************************************************************************************
	����RR���Ҷ�Ӧ����� ����ת����
 ****************************************************************************************/
Tree*	_pLeftRotation(Tree *pstTree)
{
	Tree	*pstTmp = NULL;

	pstTmp = pstTree->m_pstRight;
	pstTree->m_pstRight = pstTmp->m_pstLeft;
	pstTmp->m_pstLeft = pstTree;

	pstTree->m_lHeight = MAX(lGetHeight(pstTree->m_pstLeft), lGetHeight(pstTree->m_pstRight)) + 1;
	pstTmp->m_lHeight = MAX(lGetHeight(pstTmp->m_pstLeft), pstTree->m_lHeight) + 1;

	return pstTmp;
}

/****************************************************************************************
	����LL�����Ӧ����� �ҵ���ת����
 ****************************************************************************************/
Tree*	pLLTreeSect(Tree *pstTree)
{
	return _pRightRotation(pstTree);
}

/****************************************************************************************
	����RR���Ҷ�Ӧ����� ����ת����
 ****************************************************************************************/
Tree*	pRRTreeSect(Tree *pstTree)
{
	return _pLeftRotation(pstTree);
}

/****************************************************************************************
	LR�����Ҷ�Ӧ�����, ��������������, ��ת��ĸ��ڵ�
 ****************************************************************************************/
Tree*	pLRTreeSect(Tree *pstTree)
{
	pstTree->m_pstLeft = _pLeftRotation(pstTree->m_pstLeft);
	return _pRightRotation(pstTree);
}

/****************************************************************************************
	RL�������Ӧ�����, ������������������ת��ĸ��ڵ�
 ****************************************************************************************/
Tree*	pRLTreeSect(Tree *pstTree)
{
	pstTree->m_pstRight = _pRightRotation(pstTree->m_pstRight);
	return _pLeftRotation(pstTree);
}

/******************************************************************************
	����AVL����㡣
	����˵����
		pstLeft		--��ڵ�
		pstRight	--�ҽڵ�
 ******************************************************************************/
Tree*	pCreatTreeNode(Tree *pstLeft, Tree *pstRight, long lKey)
{
	Tree	*pstTree;

	if((pstTree = (Tree *)malloc(sizeof(Tree))) == NULL)
		return NULL;
	pstTree->m_lKey = lKey;
	pstTree->m_lHeight = 0;
	pstTree->m_pstLeft = pstLeft;
	pstTree->m_pstRight = pstRight;

	return pstTree;
}

/******************************************************************************
	�������뵽AVL���У������ظ��ڵ�
	����˵����
		pstTree				--Root�ڵ�
		pstVoid				--����ֵ
		lLen				--��������
	����ֵ��
		Tree				--���ڵ�
 ******************************************************************************/
Tree*	pInsertTree(Tree *pstTree, void *pstVoid, long lKey, long lPos, long lLen, long lOffset, long *plRes)
{
	if(!pstTree)
	{
		pstTree = pCreatTreeNode(NULL, NULL, lKey);
		if(NULL == pstTree)
		{
			vSetErrMsg("���������ڵ����ڴ�ռ�ʧ��, err:(%s)", strerror(errno));
			return NULL;
		}
		
		if((pstTree->m_pstKey = (char *)malloc(lLen)) == NULL)
		{
			vSetErrMsg("���������ڴ�ռ�ʧ��, err:(%s)", strerror(errno));
			return NULL;
		}

		memcpy(pstTree->m_pstKey, pstVoid, lLen);
		pstTree->m_lKeyLen = lLen;
		pstTree->m_lKeyPos = lPos;
		pstTree->m_lData = lOffset;
	}
	else if(0 < memcmp((char *)pstTree->m_pstKey, (char *)pstVoid, lLen))	// ���뵽"tree��������"�����
	{
		pstTree->m_pstLeft = pInsertTree(pstTree->m_pstLeft, pstVoid, lKey, lPos, lLen, lOffset, plRes);
		if(lGetHeight(pstTree->m_pstLeft) - lGetHeight(pstTree->m_pstRight) == 2)
		{
			if(0 < memcmp((char *)pstTree->m_pstLeft->m_pstKey, (char *)pstVoid, lLen))
				pstTree = pLLTreeSect(pstTree);
			else
				pstTree = pLRTreeSect(pstTree);
		}
	}
	else if(0 > memcmp((char *)pstTree->m_pstKey, (char *)pstVoid, lLen))	//	���뵽"tree��������"�����
	{
		pstTree->m_pstRight = pInsertTree(pstTree->m_pstRight, pstVoid, lKey, lPos, lLen, lOffset, plRes);
		if(lGetHeight(pstTree->m_pstRight) - lGetHeight(pstTree->m_pstLeft) == 2)
		{
			if(0 < memcmp((char *)pstTree->m_pstRight->m_pstKey, (char *)pstVoid, lLen))
				pstTree = pRLTreeSect(pstTree);
		   	else 
				pstTree = pRRTreeSect(pstTree);
		}
	}
	else
	{
		*plRes = RC_FAIL;
		vSetErrMsg("���ʧ�ܣ������������ͬ�Ľڵ㣡");
		return NULL;
	}

	pstTree->m_lHeight = MAX(lGetHeight(pstTree->m_pstLeft), lGetHeight(pstTree->m_pstRight)) + 1;
	return pstTree;
}
