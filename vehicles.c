
/* vim: set filetype=c ts=8 noexpandtab: */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "common.h"
#include <string.h>
#include <math.h>
#include "game_sa.h"
#include "vehicles.h"

static struct dbvehicle *dbvehicles;
int dbvehiclenextid, dbvehiclealloc;
static struct vehicle gamevehicles[MAX_VEHICLES];
short labelids[MAX_PLAYERS][MAX_VEHICLES]; /* 200KB+ of mapping errrr */

static void freeDbVehicleTable()
{
	struct dbvehicle *veh = dbvehicles;
	while (dbvehiclealloc) {
		dbvehiclealloc--;
		if (veh->ownerstring != NULL) {
			free(veh->ownerstring);
			veh->ownerstring = NULL;
		}
		free(veh);
		veh++;
	}
        dbvehicles = NULL;
}

static void resizeDbVehicleTable()
{
	struct dbvehicle *nw;
	int i = 100;
	dbvehiclealloc += 100;
	dbvehicles = realloc(dbvehicles, sizeof(struct dbvehicle) * dbvehiclealloc);
	nw = dbvehicles + dbvehiclealloc - 100;
	while (i-- > 0) {
		nw->ownerstring = NULL;
		nw++;
	}
}

/* native Veh_Add(id, model, owneruserid, Float:x, Float:y, Float:z, Float:r, col1, col2, ownername[]) */
cell AMX_NATIVE_CALL Veh_Add(AMX *amx, cell *params)
{
	cell *ownernameaddr;
	char ownername[MAX_PLAYER_NAME + 1];
	int model;
	struct dbvehicle *veh;
	if (dbvehiclenextid == dbvehiclealloc) {
		resizeDbVehicleTable();
	}
	veh = dbvehicles + dbvehiclenextid;
	dbvehiclenextid++;
	veh->id = params[1];
	veh->model = model = params[2];
	veh->owneruserid = params[3];
	veh->x = amx_ctof(params[4]);
	veh->y = amx_ctof(params[5]);
	veh->z = amx_ctof(params[6]);
	veh->r = amx_ctof(params[7]);
	veh->col1 = params[8];
	veh->col2 = params[9];
	if (veh->owneruserid != 0 && 400 <= model && model <= 611) {
		amx_GetAddr(amx, params[10], &ownernameaddr);
		amx_GetUString(ownername, ownernameaddr, sizeof(ownername));
		veh->ownerstring = malloc((9 + strlen(vehnames[model - 400]) + strlen(ownername)) * sizeof(char));
		sprintf(veh->ownerstring, "%s Owner\n%s", vehnames[model - 400], ownername);
	} else {
		veh->ownerstring = NULL;
	}
	return dbvehiclenextid - 1;
}

/* native Veh_Destroy() */
cell AMX_NATIVE_CALL Veh_Destroy(AMX *amx, cell *params)
{
	if (dbvehicles != NULL) {
		freeDbVehicleTable();
	}
	return 1;
}

/* native Veh_Init(dbvehiclecount) */
cell AMX_NATIVE_CALL Veh_Init(AMX *amx, cell *params)
{
	int i = dbvehiclealloc = params[1];
	int pid;
	if (dbvehicles != NULL) {
		logprintf("Veh_Init: warning: dbvehicles table was not empty");
		freeDbVehicleTable();
	}
	dbvehiclenextid = 0;
	if (dbvehiclealloc <= 0) {
		dbvehiclealloc = 100;
	}
	dbvehicles = malloc(sizeof(struct dbvehicle) * dbvehiclealloc);
	i = dbvehiclealloc;
	while (i--) {
		dbvehicles[i].ownerstring = NULL;
	}
	i = 0;
	while (i < MAX_VEHICLES) {
		gamevehicles[i].id = i;
		gamevehicles[i].dbvehicle = NULL;
		i++;
	}
	pid = MAX_PLAYERS;
	while (pid--) {
		for (i = 0; i < MAX_VEHICLES; i++) {
			labelids[pid][i] = -1;
		}
	}
	return 1;
}

/* native Veh_GetLabelToDelete(vehicleid, playerid, &PlayerText3D:labelid) */
cell AMX_NATIVE_CALL Veh_GetLabelToDelete(AMX *amx, cell *params)
{
	int vehicleid = params[1], playerid = params[2];
	int labelid = labelids[playerid][vehicleid];
	cell *addr;
	if (labelid == -1) {
		return 0;
	}
	labelids[playerid][vehicleid] = -1;
	amx_GetAddr(amx, params[3], &addr);
	*addr = labelid;
	return 1;
}

/* native Veh_OnPlayerDisconnect(playerid) */
cell AMX_NATIVE_CALL Veh_OnPlayerDisconnect(AMX *amx, cell *params)
{
	short *labelid = &labelids[params[1]][0];
	int i = MAX_VEHICLES;
	while (i--) {
		*labelid = -1;
		labelid++;
	}
	return 1;
}

/* native Veh_RegisterLabel(vehicleid, playerid, PlayerText3D:labelid) */
cell AMX_NATIVE_CALL Veh_RegisterLabel(AMX *amx, cell *params)
{
	int vehicleid = params[1], playerid = params[2], labelid = params[3];
	labelids[playerid][vehicleid] = labelid;
	return 1;
}

/* native Veh_ShouldCreateLabel(vehicleid, playerid, buf[]) */
cell AMX_NATIVE_CALL Veh_ShouldCreateLabel(AMX *amx, cell *params)
{
	int vehicleid = params[1], playerid = params[2];
	cell *addr;
	struct vehicle veh = gamevehicles[vehicleid];
	if (veh.dbvehicle == NULL) {
		logprintf("unknown vehicle streamed in for player");
		return 0;
	}
	if (labelids[playerid][vehicleid] != -1) {
		return 0;
	}
	if (veh.dbvehicle->owneruserid == 0) {
		return 0;
	}
	amx_GetAddr(amx, params[3], &addr);
	amx_SetUString(addr, veh.dbvehicle->ownerstring, 144);
	return 1;
}

/* native Veh_UpdateSlot(vehicleid, dbid) */
cell AMX_NATIVE_CALL Veh_UpdateSlot(AMX *amx, cell *params)
{
	int vehicleid = params[1], dbid = params[2];
	gamevehicles[vehicleid].dbvehicle = dbid == -1 ? NULL : &dbvehicles[dbid];
	return 1;
}
