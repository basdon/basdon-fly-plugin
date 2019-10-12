
/* vim: set filetype=c ts=8 noexpandtab: */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "common.h"
#include <string.h>
#include "playerdata.h"

struct playerdata *pdata[MAX_PLAYERS];

void pdata_init()
{
	int i;
	for (i = 0; i < MAX_PLAYERS; i++) {
		pdata[i] = NULL;
	}
}

/**
Refetches the player's name and stores it in the plugin.
Will crash the server if pdata memory hasn't been allocated.

@param amx abstract machine
@param playerid player of which to refetch the name
*/
void pdata_update_name(AMX *amx, int playerid)
{
	struct playerdata *pd = pdata[playerid];
	char t, *nin = pd->name, *nout = pd->normname;

	NC_GetPlayerName_(playerid, buf144a, MAX_PLAYER_NAME + 1, &pd->namelen);
	amx_GetUString(pd->name, buf144, MAX_PLAYER_NAME + 1);
	/*update normname*/
	do {
		t = *nin++;
		if ('A' <= t && t <= 'Z') {
			t |= 0x20;
		}
		*nout++ = t;
	} while (t);
}

/**
Initializes player data for a player.

@param amx abstract machine
@param playerid playerid for which to initialize their data
*/
void pdata_init_player(AMX *amx, int playerid)
{
	struct playerdata *pd;

	if ((pd = pdata[playerid]) == NULL) {
		pd = pdata[playerid] = malloc(sizeof(struct playerdata));
	}
	NC_GetPlayerIp(playerid, buf144a, sizeof(pd->ip));
	amx_GetUString(pd->ip, buf144, sizeof(pd->ip));
	pdata_update_name(amx, playerid);
	pd->userid = -1;
	pd->groups = GROUP_GUEST;
}

void useridornull(int playerid, char *storage)
{
	if (pdata[playerid] == NULL || pdata[playerid]->userid < 1) {
		strcpy(storage, "NULL");
		return;
	}
	sprintf(storage, "%d", pdata[playerid]->userid);
}

/* native PlayerData_Clear(playerid) */
cell AMX_NATIVE_CALL PlayerData_Clear(AMX *amx, cell *params)
{
	int pid = params[1];
	if (pdata[pid] != NULL) {
		free(pdata[pid]);
		pdata[pid] = NULL;
	}
	return 1;
}

/* native PlayerData_FormatUpdateQuery(userid, score, money, Float:dis, flighttime, prefs, buf[]) */
cell AMX_NATIVE_CALL PlayerData_FormatUpdateQuery(AMX *amx, cell *params)
{
	const int userid = params[1], score = params[2], money = params[3];
	const int dis = (int) amx_ctof(params[4]), flighttime = params[5], prefs = params[6];
	char buf[144];
	cell *addr;
	sprintf(buf,
	        "UPDATE usr SET score=%d,cash=%d,distance=%d,flighttime=%d,prefs=%d WHERE i=%d",
	        score,
	        money,
	        dis,
	        flighttime,
	        prefs,
	        userid);
	amx_GetAddr(amx, params[7], &addr);
	amx_SetUString(addr, buf, sizeof(buf));
	return 1;
}

/* native PlayerData_SetUserId(playerid, id) */
cell AMX_NATIVE_CALL PlayerData_SetUserId(AMX *amx, cell *params)
{
	int pid = params[1];
	if (pdata[pid] == NULL) {
		return 0;
	}
	pdata[pid]->userid = params[2];
	return 1;
}

/* native PlayerData_UpdateGroup(playerid, groups) */
cell AMX_NATIVE_CALL PlayerData_UpdateGroup(AMX *amx, cell *params)
{
	const int pid = params[1], groups = params[2];
	if (pdata[pid] == NULL) {
		return 0;
	}
	pdata[pid]->groups = groups;
	return 1;
}

/* native PlayerData_UpdateName(playerid, name[], namelen) */
cell AMX_NATIVE_CALL PlayerData_UpdateName(AMX *amx, cell *params)
{
	pdata_update_name(amx, params[1]);
	return 1;
}