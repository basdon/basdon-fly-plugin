
/* vim: set filetype=c ts=8 noexpandtab: */

/*This file is directly included in basdonfly.c,
but also added as a source file in VC2005. Check
for the IN_BASDONFLY macro to only compile this
when we're inside basdonfly.c*/
#ifdef IN_BASDONFLY

char playeronlineflag[MAX_PLAYERS];
short players[MAX_PLAYERS];
int playercount;

/* native B_Validate(maxplayers, buf4096[], buf144[], buf64[], buf32[],
                     buf32_1[], emptystring) */
cell AMX_NATIVE_CALL B_Validate(AMX *amx, cell *params)
{
	int i;
	for (i = 0; i < MAX_PLAYERS; i++) {
		playeronlineflag[i] = 0;
	}
	playercount = 0;

	amx_GetAddr(amx, buf4096a = params[2], &buf4096);
	amx_GetAddr(amx, buf144a = params[3], &buf144);
	amx_GetAddr(amx, buf64a = params[4], &buf64);
	amx_GetAddr(amx, buf32a = params[5], &buf32);
	amx_GetAddr(amx, buf32_1a = params[6], &buf32_1);

	if (!natives_find(amx)) {
		return 0;
	}

	if (MAX_PLAYERS != params[1]) {
		logprintf(
			"ERR: MAX_PLAYERS mismatch: %d (plugin) vs %d (gm)",
			MAX_PLAYERS,
			params[1]);
		return 0;
	}
	return MAX_PLAYERS;
}

/* native B_Loop25() */
cell AMX_NATIVE_CALL B_Loop25(AMX *amx, cell *params)
{
	void maps_process_tick(AMX *amx);

	static int loop25invoccount = 0;

	if (loop25invoccount > 3) {
		loop25invoccount = 1;
		/*loop100*/
		maps_process_tick(amx);
	} else {
		loop25invoccount++;
	}
	return 1;
}

/* native B_OnGameModeInit() */
cell AMX_NATIVE_CALL B_OnGameModeInit(AMX *amx, cell *params)
{
	void maps_load_from_db(AMX *amx);

	maps_load_from_db(amx);
	return 1;
}

/* native B_OnPlayerConnect(playerid) */
cell AMX_NATIVE_CALL B_OnPlayerConnect(AMX *amx, cell *params)
{
	const int playerid = params[1];
	int i;

	playeronlineflag[playerid] = 1;

	for (i = 0; i < playercount; ){
		if (players[i] == playerid) {
			return 1;
		}
	}
	players[playercount++] = playerid;
	return 1;
}

/* native B_OnPlayerDisconnect(playerid, reason)*/
cell AMX_NATIVE_CALL B_OnPlayerDisconnect(AMX *amx, cell *params)
{
	const int playerid = params[1];
	int i;

	playeronlineflag[playerid] = 0;

	for (i = 0; i < playercount; i++) {
		if (players[i] == playerid) {
			players[i] = players[playercount - 1];
			playercount--;
			return 1;
		}
	}
	return 1;
}

#endif /*IN_BASDONFLY*/
