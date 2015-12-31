#include	"load.h"

/****************************************************************************************
	������������
 ****************************************************************************************/
CMList*	pInsertList(CMList *pstRoot, void *pszData, long lSize)
{
	static	long	lId = 0;
	static	CMList	*pstCurt = NULL;
	CMList	*pstNode = NULL;

	pstNode = (CMList *)malloc(sizeof(CMList));
	if(!pstNode)
	{
		vSetErrMsg("ϵͳ�����ڴ��С(%d)ʧ��", pstNode->lSize);
		return pstRoot;
	}

	pstNode->lSize = lSize;
	pstNode->pstData = (char *)malloc(pstNode->lSize);
	if(!pstNode->pstData)
	{
		vSetErrMsg("ϵͳ�����ڴ��С(%d)ʧ��", pstNode->lSize);
		return pstRoot;
	}

	pstNode->lID = ++ lId;
	pstNode->pstNext = NULL;
	pstNode->pstLast = NULL;
	memcpy(pstNode->pstData, pszData, pstNode->lSize);

	if(!pstRoot)
		pstRoot = pstNode;
	else
	{
		pstNode->pstLast = pstCurt;
		pstCurt->pstNext = pstNode;			
	}

	pstCurt = pstNode;
	return pstRoot;
}

/****************************************************************************************
	��������	
 ****************************************************************************************/
CMList*	pSearchNode(CMList *pstRoot, void *psvData, long lSize)
{
	CMList	*pstFind = NULL, *pstNode = pstRoot;

	while(pstNode)
	{
		if(!memcmp(pstNode->pstData, psvData, lSize))
		{
			pstFind = pstNode;
			return pstFind;
		}
		pstNode = pstNode->pstNext;
	}

	return pstFind;
}

/****************************************************************************************
	��������	
 ****************************************************************************************/
CMList*	pSortList(CMList *pstRoot, long lType)
{
//	������





	return pstRoot;
}

/****************************************************************************************
	ɾ������
 ****************************************************************************************/
void	vDestroyList(CMList *pstRoot)
{
	CMList	*pstNode = pstRoot, *pstList = NULL;

	while(pstNode)
	{
		pstList = pstNode;
		IBPFree(pstList->pstData);
		IBPFree(pstList);

		pstNode = pstNode->pstNext;
	}

	pstRoot = NULL;
}

/****************************************************************************************
	ɾ��ĳһ�ڵ�
 ****************************************************************************************/
CMList*	pDeleteNode(CMList *pstRoot, void *psvData, long lSize)
{
	CMList	*pstNode = pstRoot;

	if(!pstNode)	return pstRoot;

	while(pstNode)
	{
		if(!memcmp(pstNode->pstData, psvData, lSize))
			break;
		pstNode = pstNode->pstNext;
	}
	if(!pstNode)	//	û�ҵ�
		return pstRoot;

	if(pstNode->pstNext)
		pstNode->pstNext->pstLast = pstNode->pstLast;

	if(!pstNode->pstLast)
		pstRoot = pstNode->pstNext;
	else
	{
		pstNode->pstLast->pstNext = pstNode->pstNext;
		IBPFree(pstNode->pstData);
		IBPFree(pstNode);
	}	

	return pstRoot;
}

/****************************************************************************************
	����
 ****************************************************************************************/
void	vDebugList(CMList *pstRoot)
{
	CMList	*pstNode = pstRoot;

	IBPrace("��ʼ���Խڵ�");
	while(pstNode)
	{
		IBPrace("��%d�ڵ�, ���ڵ�[%08X], �½ڵ�[%08X], �Ͻڵ�[%08X]", pstNode->lID, 
			pstNode, pstNode->pstNext, pstNode->pstLast);

		pstNode = pstNode->pstNext;
	}
}

/****************************************************************************************
	code end
 ****************************************************************************************/
