
/* vim: set filetype=c ts=8 noexpandtab: */

/*This file is directly included in cmd.c,
but also added as a source file in VC2005. Check
for the IN_CMD macro to only compile this
when we're inside cmd.c*/
#ifdef IN_CMD

/**
Respawns the vehicle the player is in.
*/
static
int cmd_admin_respawn(CMDPARAMS)
{
	int vehicleid;

	vehicleid = NC_GetPlayerVehicleID(playerid);
	if (vehicleid) {
		NC_SetVehicleToRespawn(vehicleid);
	}
	return 1;
}

static
int cmd_at400(CMDPARAMS)
{
	struct vec3 playerpos, vehiclepos;
	struct dbvehicle *veh;
	int vehicleid, found_vehicle;
	float dx, dy, dz, shortest_distance, tmpdistance;

	vehicleid = NC_GetPlayerVehicleID(playerid);
	if (vehicleid) {
		return 1;
	}

	found_vehicle = 0;
	shortest_distance = FLOAT_PINF;
	common_GetPlayerPos(playerid, &playerpos);
	for (vehicleid = 0; vehicleid < MAX_VEHICLES; vehicleid++) {
		veh = gamevehicles[vehicleid].dbvehicle;
		if (veh != NULL &&
			veh->model == MODEL_AT400 &&
			veh_is_player_allowed_in_vehicle(playerid, veh))
		{
			common_GetVehiclePos(vehicleid, &vehiclepos);
			dx = vehiclepos.x - playerpos.x;
			dy = vehiclepos.y - playerpos.y;
			dz = vehiclepos.z - playerpos.z;
			tmpdistance = dx * dx + dy * dy + dz * dz;
			if (tmpdistance < shortest_distance &&
				common_find_vehicle_driver(vehicleid)
					== INVALID_PLAYER_ID)
			{
				shortest_distance = tmpdistance;
				found_vehicle = vehicleid;
			}
		}
	}
	if (found_vehicle && shortest_distance < 25.0f * 25.0f) {
		natives_NC_PutPlayerInVehicle(playerid, found_vehicle, 0);
	}

	return 1;
}

static const char
	*MSG_VEH_PARK_Y = SUCC"Vehicle parked!",
	*MSG_VEH_PARK_N = WARN"You are not allowed to park this vehicle",
	*MSG_VEH_SPRAY_N = WARN"You are not allowed to respray this vehicle";

static
int cmd_camera(CMDPARAMS)
{
	NC_GivePlayerWeapon(playerid, WEAPON_CAMERA, 3036);
	return 1;
}

static
int cmd_me(CMDPARAMS)
{
	char *from;
	cell *to;
	int hascontent = 0;

	sprintf(cbuf144, "* %s", pdata[playerid]->name);
	amx_SetUString(buf4096, cbuf144, 144);
	from = (char*) cmdtext + 3;
	to = buf4096 + 2 + pdata[playerid]->namelen;
	while ((*to = *from)) {
		if (*from != ' ') {
			hascontent = 1;
		}
		from++;
		to++;
	}
	if (hascontent) {
		buf4096[144] = 0;
		NC_SendClientMessageToAll(-1, buf4096a);
		echo_on_game_chat_or_action(1, playerid, (char*) cmdtext + 4);
	} else {
		amx_SetUString(buf144, WARN"Syntax: /me <action>", 144);
		NC_SendClientMessage(playerid, COL_WARN, buf144a);
	}
	return 1;
}

static
int cmd_park(CMDPARAMS)
{
	char q[144];
	struct dbvehicle *veh;
	struct vec3 vpos;
	int vehicleid;
	float r;

	vehicleid = NC_GetPlayerVehicleID(playerid);
	if (vehicleid) {
		veh = gamevehicles[vehicleid].dbvehicle;
		if (!veh_can_player_modify_veh(playerid, veh))
		{
			amx_SetUString(buf144, MSG_VEH_PARK_N, 144);
			NC_SendClientMessage(playerid, COL_WARN, buf144a);
			return 1;
		}
		common_GetVehiclePos(vehicleid, &vpos);
		NC_GetVehicleZAngle(vehicleid, buf32_1a);
		r = amx_ctof(*buf32_1);
		if (356.0f < r || r < 4.0f) {
			r = 0.0f;
		} else if (86.0f < r && r < 94.0f) {
			r = 90.0f;
		} else if (176.0f < r && r < 184.0f) {
			r = 180.0f;
		} else if (266.0f < r && r < 274.0f) {
			r = 270.0f;
		}
		veh->x = vpos.x;
		veh->y = vpos.y;
		veh->z = vpos.z;
		veh->r = r;
		gamevehicles[vehicleid].need_recreation = 1;
		sprintf(q, "UPDATE veh SET x=%f,y=%f,z=%f,r=%f WHERE i=%d",
			vpos.x, vpos.y, vpos.z, r, veh->id);
		amx_SetUString(buf144, q, sizeof(q));
		NC_mysql_tquery_nocb(buf144a);
		amx_SetUString(buf144, MSG_VEH_PARK_Y, 144);
		NC_SendClientMessage(playerid, COL_SUCC, buf144a);
	}
	return 1;
}

static const char* NO_RECLASSSPAWN =
	WARN"You cannot reclass/respawn while on a mission. "
	"Use /s to cancel your current mission for a fee "
	"($"EQ(MISSION_CANCEL_FINE_)").";

static
int cmd_reclass(CMDPARAMS)
{
	if (missions_is_player_on_mission(playerid)) {
		amx_SetUString(buf144, NO_RECLASSSPAWN, 144);
		NC_SendClientMessage(playerid, COL_WARN, buf144a);
	} else {
		spawned[playerid] = 0;
		NC_ForceClassSelection(playerid);
		NC_TogglePlayerSpectating(playerid, 1);
		nc_params[2] = 0;
		NC(n_TogglePlayerSpectating);
	}
	return 1;
}

static
int cmd_respawn(CMDPARAMS)
{
	if (missions_is_player_on_mission(playerid)) {
		amx_SetUString(buf144, NO_RECLASSSPAWN, 144);
		NC_SendClientMessage(playerid, COL_WARN, buf144a);
	} else {
		natives_NC_SpawnPlayer(playerid);
	}
	return 1;
}

static
int cmd_spray(CMDPARAMS)
{
	char q[144];
	struct dbvehicle *veh;
	int vehicleid, *col1, *col2;

	vehicleid = NC_GetPlayerVehicleID(playerid);
	if (vehicleid) {
		veh = gamevehicles[vehicleid].dbvehicle;
		if (!veh_can_player_modify_veh(playerid, veh)) {
			amx_SetUString(buf144, MSG_VEH_SPRAY_N, 144);
			NC_SendClientMessage(playerid, COL_WARN, buf144a);
			return 1;
		}
		col1 = nc_params + 1;
		col2 = nc_params + 2;
		if (cmd_get_int_param(cmdtext, &parseidx, col1)) {
			if (!cmd_get_int_param(cmdtext, &parseidx, col2)) {
				goto rand2nd;
			}
		} else {
			*col1 = NC_random(256);
rand2nd:
			*col2 = NC_random(256);
		}
		NC_PARS(2);
		NC(n_ChangeVehicleColor);
		if (veh != NULL) {
			veh->col1 = *col1;
			veh->col2 = *col2;
			gamevehicles[vehicleid].need_recreation = 1;
			sprintf(q,
				"UPDATE veh SET col1=%d,col2=%d WHERE i=%d",
				col1,
				col2,
				veh->id);
			amx_SetUString(buf144, q, sizeof(q));
			NC_mysql_tquery_nocb(buf144a);
		}
	}
	return 1;
}

#endif /*IN_CMD*/
