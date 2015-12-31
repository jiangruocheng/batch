#include	"load.h"

/****************************************************************************************
	插入链表数据
 ****************************************************************************************/
CMList*	pInsertList(CMList *pstRoot, void *pszData, long lSize)
{
	static	long	lId = 0;
	static	CMList	*pstCurt = NULL;
	CMList	*pstNode = NULL;

	pstNode = (CMList *)malloc(sizeof(CMList));
	if(!pstNode)
	{
		vSetErrMsg("系统分配内存大小(%d)失败", pstNode->lSize);
		return pstRoot;
	}

	pstNode->lSize = lSize;
	pstNode->pstData = (char *)malloc(pstNode->lSize);
	if(!pstNode->pstData)
	{
		vSetErrMsg("系统分配内存大小(%d)失败", pstNode->lSize);
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
	查找数据	
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
	查找数据	
 ****************************************************************************************/
CMList*	pSortList(CMList *pstRoot, long lType)
{
//	排序功能





	return pstRoot;
}

/****************************************************************************************
	删除链表
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
	删除某一节点
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
	if(!pstNode)	//	没找到
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
	调试
 ****************************************************************************************/
void	vDebugList(CMList *pstRoot)
{
	CMList	*pstNode = pstRoot;

	IBPrace("开始测试节点");
	while(pstNode)
	{
		IBPrace("第%d节点, 本节点[%08X], 下节点[%08X], 上节点[%08X]", pstNode->lID, 
			pstNode, pstNode->pstNext, pstNode->pstLast);

		pstNode = pstNode->pstNext;
	}
}

/****************************************************************************************
	code end
 ****************************************************************************************/
