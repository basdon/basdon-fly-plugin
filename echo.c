
/* vim: set filetype=c ts=8 noexpandtab: */

#define _CRT_SECURE_NO_DEPRECATE
#include "common.h"
#include "playerdata.h"
#include <string.h>

#define ECHO_PORT_OUT 7767
#define ECHO_PORT_IN 7768

#define PACK_HELLO 2
#define PACK_IMTHERE 3
#define PACK_BYE 4
#define PACK_PING 5
#define PACK_PONG 6
#define PACK_CHAT 10
#define PACK_PLAYER_CONNECTION 30

#define CONN_REASON_GAME_TIMEOUT 0
#define CONN_REASON_GAME_QUIT 1
#define CONN_REASON_GAME_KICK 2
#define CONN_REASON_GAME_CONNECTED 3
#define CONN_REASON_IRC_QUIT 6
#define CONN_REASON_IRC_PART 7
#define CONN_REASON_IRC_KICK 8
#define CONN_REASON_IRC_JOIN 9

#define COL_IRC COL_INFO_GENERIC

static cell socket_in = SOCKET_INVALID_SOCKET;
static cell socket_out = SOCKET_INVALID_SOCKET;

/**
Creates the echo sockets.
Call from the _end_ of OnGameModeInit, because for a yet unknown reason the
server crashes if it's called somewhere in the middle.
*/
cell AMX_NATIVE_CALL Echo_Init(AMX *amx, cell *params)
{
	const char *buflo = "127.0.0.1";

	NC_socket_create(SOCKET_UDP, &socket_in);
	if (socket_in == SOCKET_INVALID_SOCKET) {
		logprintf("failed to create echo game socket");
	} else {
		NC_socket_listen(socket_in, ECHO_PORT_IN);
	}
	NC_socket_create(SOCKET_UDP, &socket_out);
	if (socket_out == SOCKET_INVALID_SOCKET) {
		logprintf("failed to create echo irc socket");
	} else {
		amx_SetUString(buf32, buflo, 32);
		NC_socket_connect(socket_out, buf32a, ECHO_PORT_OUT);
		buf144[0] = 0x02594C46;
		buf144[1] = 0x07030301;
		NC_socket_send_array(socket_out, buf144a, 8);
	}
	return 1;
}

/**
Destroys the sockets used by the echo service.
Call from OnGameModeExit
*/
void echo_dispose(AMX *amx)
{
	if (socket_out != SOCKET_INVALID_SOCKET) {
		buf144[0] = 0x04594C46;
		NC_socket_send_array(socket_out, buf144a, 4);
		NC_socket_destroy(socket_out);
		socket_out = SOCKET_INVALID_SOCKET;
	}
	if (socket_in != SOCKET_INVALID_SOCKET) {
		NC_socket_stop_listen(socket_in);
		socket_in = SOCKET_INVALID_SOCKET;
	}
}

/**
Send player connection packet to IRC echo.

@param amx abstract machine
@param playerid playerid that (dis)connected
@param reason 3 when OnPlayerConnection, reason when OnPlayerDisconnect
*/
void echo_on_player_connection(AMX *amx, int playerid, int reason)
{
	int nicklen;
	struct playerdata *pd = pdata[playerid];

	if (socket_out != SOCKET_INVALID_SOCKET) {
		nicklen = pd->namelen;
		buf144[0] = 0x1E594C46;
		buf144[1] =
			(playerid & 0xFFFF) |
			((reason & 0xFF) << 16) |
			((nicklen & 0xFF) << 24);
		memcpy(((char*) buf144) + 8, pd->name, nicklen + 1);
		NC_socket_send_array(socket_out, buf144a, 8 + nicklen);
	}
}

/**
Send game chat to IRC echo.
Call from OnPlayerText
*/
void echo_on_game_chat(AMX *amx, int playerid, char *text)
{
	int nicklen, msglen;
	struct playerdata *pd = pdata[playerid];

	if (socket_out != SOCKET_INVALID_SOCKET) {
		nicklen = pd->namelen;
		msglen = strlen(text);
		if (msglen > 144) {
			msglen = 144;
		}
		buf144[0] = 0x0A594C46;
		buf144[1] =
			(playerid & 0xFFFF) |
			((pd->namelen & 0xFF) << 16) |
			((msglen & 0xFF) << 24);
		memcpy(((char*) buf144) + 8, pd->name, nicklen + 1);
		memcpy(((char*) buf144) + 9 + nicklen, text, msglen);
		*(((char*) buf144) + 9 + nicklen + msglen) = 0;
		NC_socket_send_array(socket_out, buf144a, 10+nicklen+msglen);
	}
}

static
void echo_sendclientmessage_filtered(AMX *amx, char *buf, int len)
{
	char *b = buf, *bufend = buf + sizeof(buf);

	/*escape stuff TODO: escape embedded colors?*/
	while (b != bufend) {
		if (*b == '%') {
			*b = '#';
		}
		b++;
	}
	amx_SetUString(buf144, buf, sizeof(buf));
	NC_SendClientMessageToAll(COL_IRC, buf144a);
}

static const char *msg_bridge_up = "IRC bridge is up";
static const char *msg_bridge_down = "IRC bridge is down";

/**
Handle received UDP packet.
Call from onUDPReceiveData
*/
void echo_on_receive(AMX *amx, cell socket_handle,
		     cell data_a, cell *data, int len)
{
	if (socket_handle == socket_in && len >= 4 &&
		data[0] == 'F' && data[1] == 'L' && data[2] == 'Y')
	{
		switch (data[3]) {
		case PACK_HELLO:
			logprintf((char*) msg_bridge_up);
			amx_SetUString(buf144, msg_bridge_up, 144);
			NC_SendClientMessageToAll(COL_IRC, buf144a);
			if (len == 8) {
				data[3] = PACK_IMTHERE;
				NC_socket_send(socket_out, data_a, 8);
			}
			break;
		case PACK_IMTHERE:
			logprintf((char*) msg_bridge_up);
			/*no point printing this to chat, because this only
			happens right after the server starts, thus nobody
			is connected yet*/
			break;
		case PACK_PING:
			if (len == 8) {
				data[3] = PACK_PONG;
				NC_socket_send(socket_out, data_a, 8);
			}
			break;
		/*game doesn't send PING packets, so not checking PONG*/
		case PACK_BYE:
			logprintf((char*) msg_bridge_down);
			if (len == 4) {
				amx_SetUString(buf144, msg_bridge_down, 144);
				NC_SendClientMessageToAll(COL_IRC, buf144a);
			}
			break;
		case PACK_CHAT:
		{
			int nicklen, msglen;
			char buf[144], *b = buf;

			if (len < 12 ||
				(nicklen = data[6]) < 1 || nicklen > 49 ||
				(msglen = data[7]) < 1 || msglen > 143 ||
				10 + nicklen + msglen != len)
			{
				break;
			}
			buf[0] = '<';
			amx_GetUString(buf + 1, data + 8, 50);
			buf[1 + nicklen] = '>';
			buf[2 + nicklen] = ' ';
			b = buf + 3 + nicklen;
			amx_GetUString(b, data + 9 + nicklen, 144);
			echo_sendclientmessage_filtered(amx, buf, sizeof(buf));
			break;
		}
		case PACK_PLAYER_CONNECTION:
		{
			int nicklen;
			char buf[144], *b = buf;

			if (len < 9 ||
				(nicklen = data[7]) < 1 || nicklen > 49 ||
				8 + nicklen != len)
			{
				break;
			}
			*((int*) b) = 0x3A435249;
			b += 4;
			*(b++) = ' ';
			switch (data[6]) {
			case CONN_REASON_IRC_QUIT:
				memcpy(b, "quits: ", 7);
				b += 7;
				break;
			case CONN_REASON_IRC_PART:
				memcpy(b, "parts: ", 7);
				b += 7;
				break;
			case CONN_REASON_IRC_KICK:
				memcpy(b, "kicked: ", 8);
				b += 8;
				break;
			case CONN_REASON_IRC_JOIN:
				memcpy(b, "joins: ", 7);
				b += 7;
				break;
			}
			amx_GetUString(b, data + 8, 50);
			echo_sendclientmessage_filtered(amx, buf, sizeof(buf));
			break;
		}
		}
	}
}

