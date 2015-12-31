/**********************************************************************************
 *  功能: 批量主控进程,基础函数
 *
 *  Edit History:
 *
 *********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Ibp.h"
#include "pos_stlm.h"

#define LCTrue 1

int hCEstablishListen(char *sIp, int iPortNum) 
{
	int				lhListenSocket;
	unsigned short	 lnLocPort;
	struct sockaddr_in lsaddrinBind;
	int				llOpt;
	struct linger	  lslngrOpt;


	lnLocPort = iPortNum;

	if ((lhListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		close(lhListenSocket);
		return -1;
	} /* end of if */

#if 0
	llOpt = LCTrue;
	if (setsockopt(lhListenSocket, SOL_SOCKET, SO_KEEPALIVE, &llOpt,sizeof(llOpt))) {
		close(lhListenSocket);
		return -1;
	} /* end of if */

	lslngrOpt.l_onoff = LCTrue;
	lslngrOpt.l_linger = 0; 
	if (setsockopt(lhListenSocket, SOL_SOCKET, SO_LINGER, &lslngrOpt, sizeof(lslngrOpt))) {
		close(lhListenSocket);
		return -1;
	} /* end of if */

	llOpt = LCTrue;
	if (setsockopt( lhListenSocket, SOL_SOCKET, SO_REUSEADDR, &llOpt, sizeof(llOpt))) {
		close(lhListenSocket);
		return -1;
	} /* end of if */
#endif

	lslngrOpt.l_onoff = LCTrue;
	lslngrOpt.l_linger = 10; 
	if (setsockopt(lhListenSocket, SOL_SOCKET, SO_LINGER, &lslngrOpt, sizeof(lslngrOpt))) {
		close(lhListenSocket);
		return -1;
	} /* end of if */

	lsaddrinBind.sin_family = AF_INET;
	lsaddrinBind.sin_port = htons(lnLocPort);
	lsaddrinBind.sin_addr.s_addr = htonl(INADDR_ANY);
	//lsaddrinBind.sin_addr.s_addr = inet_addr(sIp);
	if (bind(lhListenSocket, (struct sockaddr*)(&lsaddrinBind), sizeof(lsaddrinBind))) {
		close(lhListenSocket);
		return -1;
	} /* end of if */

	if (setsockopt(lhListenSocket, SOL_SOCKET, SO_LINGER, &lslngrOpt, sizeof(lslngrOpt))) {
		close(lhListenSocket);
		return -1;
	} /* end of if */

	if (listen(lhListenSocket, 5) == -1) {
		close(lhListenSocket);
		return -1;
	} /* end of if */

	return lhListenSocket;
} /* end of hCEstablishListen */

int nCSocketRcv(int	  vhDataSocket,
				void*	vvpTcpData,
				int*	 vnpExpDataL)
{

	int			 lnDataL;
	int			 lnTotalDataL;
	char*		   lspTcpData;

	lspTcpData = vvpTcpData;
	lnTotalDataL = 0;
	while (*vnpExpDataL > lnTotalDataL) {
		if ((lnDataL = recv(vhDataSocket, lspTcpData,
						   *vnpExpDataL - lnTotalDataL,
						   0)) <= 0) {
			*vnpExpDataL = lnTotalDataL;
			return -1;
		} /* end of if */

		lnTotalDataL += lnDataL;
		lspTcpData += lnDataL;
	} /* end of while */
	return 0;
} /* end of nCSocketRcv */

int nCSocketSnd(int	vhDataSocket,
				void*  vvpTcpData,
				int*   vnpExpDataL) 
{
	int				  lnDataL;
	int				  lnTotalDataL;
	char*				lspTcpData;
	char*				lspTmp;
	int				  llTmp;

	lspTcpData = vvpTcpData;
	lnTotalDataL = 0;
	while (*vnpExpDataL > lnTotalDataL) {
		if ((lnDataL = send(vhDataSocket, lspTcpData,
							*vnpExpDataL - lnTotalDataL,
							0)) <= 0) {
			*vnpExpDataL = lnTotalDataL;
			return -1;
		} /* end of if */
		lnTotalDataL += lnDataL;
		lspTcpData += lnDataL;
	} /* end of while */
	return 0;
} /* end of nCSocketSnd */


int nCConnectSocket(int Port, char *Ip_addr)
{
	struct sockaddr_in   Sin;
	int   Socket_id, RetryTimeSap = 2, nRetryFlag = 0;
	unsigned short	 lnLocPort;

	HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, 
		   "Ip_addr:%s, port:%d.", Ip_addr, Port);
	lnLocPort = Port;
	memset(&Sin, 0, sizeof(Sin));
	Sin.sin_port = htons(lnLocPort);
	Sin.sin_family = AF_INET;
	Sin.sin_addr.s_addr = inet_addr(Ip_addr);

	while(1)
	{
		while ((Socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "socket, %d", errno);
			if (nRetryFlag++ == 5)
			{
				if ( RetryTimeSap<100) RetryTimeSap += 2;
				nRetryFlag = 0;
			}
			sleep(RetryTimeSap);
		}

		if (connect(Socket_id, (struct sockaddr *)&Sin, sizeof(Sin)) < 0)
		{
			HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "connect error[%s][%d], %d",Ip_addr , Port, errno);
			close(Socket_id);
			if (errno == ECONNREFUSED)
			{
				sleep(5);
				continue;
			}
			sleep(30);
			continue;
		}

		break;
	}

	return(Socket_id);
}

/*****************************************************************************/
/* FUNC:   short nCEstablishConnect(unsigned int  vnPortNum,				 */
/*								  void* vvpLocAddr,						*/
/*								  int* vhpSocketId)						*/
/* INPUT:  vnPortNum:6K?Z:E												  */
/*		 vvpLocAddr: A,=S5DT66KIP5XV7									  */
/* OUTPUT: vhpSocketId: socket id											*/
/* RETURN: 0: 3I9&, FdK|: J'0\											   */
/*****************************************************************************/
int nCEstablishConnect(unsigned int vnPortNum,
					   void* vvpRemAddr,
					   int* vhpSocketId)
{
	int					rc;
	int					lhConnectSocket;
	struct sockaddr_in	 lsaddrinRemote;

	memset(&lsaddrinRemote, 0, sizeof(lsaddrinRemote));
	lsaddrinRemote.sin_port = htons(vnPortNum);
	lsaddrinRemote.sin_family = AF_INET;
	lsaddrinRemote.sin_addr.s_addr = inet_addr(vvpRemAddr);

	if((lhConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		HtLog (gsLogFile, HT_LOG_MODE_ERROR, __FILE__,__LINE__, "socket, %d", errno);
		return -1;
	}

	if (connect(lhConnectSocket, (struct sockaddr*)(&lsaddrinRemote), sizeof(lsaddrinRemote))) {
		rc = errno;
		close(lhConnectSocket);
		return rc;
	} /* end of if */

	*vhpSocketId = lhConnectSocket;
	return 0;
}
