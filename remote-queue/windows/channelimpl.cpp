/*
 * networkobj.cpp
 *
 *  Created on: 2014-8-20
 *      Author: qianqians
 */
#ifdef _WINDOWS

#include "../../pool/objpool.h"

#include "../channel.h"

#include "overlapped.h"
#include "endpointimpl.h"
#include "channelimpl.h"


#include <Ws2tcpip.h>

namespace Fossilizid{
namespace remote_queue {

CHANNEL connect(ENDPOINT ep) {
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", (void*)&addr.sin_addr);
	addr.sin_port = ::htons(0);
	if (::bind(s, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) != 0){
		int error = WSAGetLastError();
		printf("error %d\n", error);
		closesocket(s);
		return 0;
	}

	if (::connect(s, (SOCKADDR*)&(((endpointimpl*)((handle*)ep))->addr), sizeof(SOCKADDR_IN)) != 0){
		int error = WSAGetLastError();
		printf("error %d\n", error);
		closesocket(s);
		return 0;
	}

	unsigned long ul = 1;
	if (ioctlsocket(s, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR){
	}

	channelimpl * ch = pool::objpool<channelimpl>::allocator(1);
	new (ch) channelimpl(0, s);

	if (ch->que != 0){
		WSABUF * wsabuf = pool::objpool<WSABUF>::allocator(1);
		wsabuf->buf = ch->buf;
		wsabuf->len = 0;
		DWORD bytes = 0;
		DWORD flags = 0;
		overlappedex * ovp = pool::objpool<overlappedex>::allocator(1);
		new (ovp)overlappedex();
		ovp->h = (handle*)ch;
		ovp->type = iocp_type_recv;
		OVERLAPPED * ovp_ = static_cast<OVERLAPPED *>(ovp);
		memset(ovp_, 0, sizeof(OVERLAPPED));
		WSARecv(ch->s, wsabuf, 1, &bytes, &flags, ovp_, 0);
	}

	return (CHANNEL)((handle*)ch);
}

} /* namespace remote_queue */
} /* namespace Fossilizid */

#endif