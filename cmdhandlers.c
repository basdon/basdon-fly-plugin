static
int cmd_admin_streamdistance(CMDPARAMS)
{
	int newdistance;
	float *stream_distance;

	stream_distance = samphost_GetPtrStreamDistance();
	if (cmd_get_int_param(cmdtext, &parseidx, &newdistance)) {
		*stream_distance = (float) newdistance;
	}
	csprintf(buf144, "stream_distance=%.0f", *stream_distance);
	NC_SendClientMessage(playerid, -1, buf144a);
	return 1;
}

static
int cmd_admin_goto(CMDPARAMS)
{
	int oldparseidx;
	int targetplayerid;
	int x, y, z;
	struct vec4 pos;

	pos.r = 0.0f;

	oldparseidx = parseidx;
	if (cmd_get_int_param(cmdtext, &parseidx, &x) &&
		cmd_get_int_param(cmdtext, &parseidx, &y) &&
		cmd_get_int_param(cmdtext, &parseidx, &z))
	{
		/*TODO: tp with vehicle?*/
		/*TODO: accept interior id as param*/
		/*TODO: common_tp_player seems to bug out?*/
		pos.coords.x = (float) x;
		pos.coords.y = (float) y;
		pos.coords.z = (float) z;
		common_tp_player(playerid, pos);
	} else {
		parseidx = oldparseidx;

		if (cmd_get_player_param(cmdtext, &parseidx, &targetplayerid)) {
			if (targetplayerid == INVALID_PLAYER_ID) {
				B144(WARN"Target player is offline");
				NC_SendClientMessage(playerid, COL_WARN, buf144a);
			} else {
				common_GetPlayerPos(targetplayerid, &pos.coords);
				/*TODO: tp with vehicle?*/
				/*TODO: same interior*/
				pos.coords.x += 0.3f;
				pos.coords.y += 0.3f;
				common_tp_player(playerid, pos);
			}
		} else {
			B144(WARN"Syntax: //goto (<x> <y> <z>|<playerid/name>)");
			NC_SendClientMessage(playerid, COL_WARN, buf144a);
		}
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
		natives_PutPlayerInVehicle(playerid, found_vehicle, 0);
	}

	return 1;
}

static const char
	*MSG_VEH_PARK_Y = SUCC"Vehicle parked!",
	*MSG_VEH_PARK_N = WARN"You are not allowed to park this vehicle",
	*MSG_VEH_SPRAY_N = WARN"You are not allowed to respray this vehicle";

/**
The /camera command gives the player a camera.
*/
static
int cmd_camera(CMDPARAMS)
{
	NC_GivePlayerWeapon(playerid, WEAPON_CAMERA, 3036);
	return 1;
}

/**
The /chute command gives the player a parachute.
*/
static
int cmd_chute(CMDPARAMS)
{
	NC_GivePlayerWeapon(playerid, WEAPON_PARACHUTE, 1);
	return 1;
}

/**
The /getspray command gets the colors of the vehicle the player is in.
*/
static
int cmd_getspray(CMDPARAMS)
{
	struct dbvehicle *veh;
	int vehicleid;

	vehicleid = NC_GetPlayerVehicleID(playerid);
	if (vehicleid && (veh = gamevehicles[vehicleid].dbvehicle)) {
		csprintf(buf144, "colors: %d, %d", veh->col1, veh->col2);
		NC_SendClientMessage(playerid, -1, buf144a);
	}
	return 1;
}

/**
The /helpkeys command shows gametext with important keys.
*/
static
int cmd_helpkeys(CMDPARAMS)
{
	B144("~w~start/stop engine: ~b~~k~~CONVERSATION_NO~~n~"
		"~w~landing gear: ~b~~k~~TOGGLE_SUBMISSIONS~");
	NC_GameTextForPlayer(playerid, buf144a, 5000, 3);
	return 1;
}

static
int cmd_me(CMDPARAMS)
{
	char *from;
	cell *to;
	int hascontent = 0;

	csprintf(buf4096, "* %s", pdata[playerid]->name);
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
		B144((char*) WARN"Syntax: /me <action>");
		NC_SendClientMessage(playerid, COL_WARN, buf144a);
	}
	return 1;
}

static
int cmd_park(CMDPARAMS)
{
	struct dbvehicle *veh;
	int vehicleid;
	float lastrot;

	vehicleid = NC_GetPlayerVehicleID(playerid);
	if (vehicleid) {
		veh = gamevehicles[vehicleid].dbvehicle;
		if (!veh_can_player_modify_veh(playerid, veh))
		{
			B144((char*) MSG_VEH_PARK_N);
			NC_SendClientMessage(playerid, COL_WARN, buf144a);
			return 1;
		}
		lastrot = veh->pos.r;
		common_GetVehiclePosRot(vehicleid, &veh->pos);
		if (356.0f < veh->pos.r || veh->pos.r < 4.0f) {
			veh->pos.r = 0.0f;
		} else if (86.0f < veh->pos.r && veh->pos.r < 94.0f) {
			veh->pos.r = 90.0f;
		} else if (176.0f < veh->pos.r && veh->pos.r < 184.0f) {
			veh->pos.r = 180.0f;
		} else if (266.0f < veh->pos.r && veh->pos.r < 274.0f) {
			veh->pos.r = 270.0f;
		}
		if (fabs(lastrot - veh->pos.r) > 3.0f) {
			gamevehicles[vehicleid].need_recreation = 1;
		}
		csprintf(buf144,
			"UPDATE veh SET x=%f,y=%f,z=%f,r=%f WHERE i=%d",
			veh->pos.coords.x,
			veh->pos.coords.y,
			veh->pos.coords.z,
			veh->pos.r,
			veh->id);
		NC_mysql_tquery_nocb(buf144a);
		B144((char*) MSG_VEH_PARK_Y);
		NC_SendClientMessage(playerid, COL_SUCC, buf144a);
	}
	return 1;
}

static const char* NO_RECLASSSPAWN =
	WARN"You cannot reclass/respawn while on a mission. "
	"Use /s to cancel your current mission for a fee "
	"($"EQ(MISSION_CANCEL_FINE)").";

static
int cmd_reclass(CMDPARAMS)
{
	if (missions_is_player_on_mission(playerid)) {
		B144((char*) NO_RECLASSSPAWN);
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
		B144((char*) NO_RECLASSSPAWN);
		NC_SendClientMessage(playerid, COL_WARN, buf144a);
	} else {
		natives_SpawnPlayer(playerid);
	}
	return 1;
}

/**
The /spray [col1] [col2] command allows a player to spray their vehicle.

One or both colors may be omitted to get a fully random color. A 'normal'
random color combination is chosen when both colors are set to -1. These colors
are the colors as defined by the game itself.
*/
static
int cmd_spray(CMDPARAMS)
{
	struct dbvehicle *veh;
	int vehicleid, *col1, *col2, *a, *b;

	vehicleid = NC_GetPlayerVehicleID(playerid);
	if (vehicleid) {
		veh = gamevehicles[vehicleid].dbvehicle;
		if (!veh_can_player_modify_veh(playerid, veh)) {
			B144((char*) MSG_VEH_SPRAY_N);
			NC_SendClientMessage(playerid, COL_WARN, buf144a);
			return 1;
		}
		a = b = NULL;
		col1 = nc_params + 2;
		col2 = nc_params + 3;
		if (cmd_get_int_param(cmdtext, &parseidx, col1)) {
			if (*col1 == -1) {
				a = col1;
			}
			if (!cmd_get_int_param(cmdtext, &parseidx, col2)) {
				goto rand2nd;
			}
			if (*col2 == -1) {
				b = col2;
			}
		} else {
			*col1 = NC_random(256);
rand2nd:
			*col2 = NC_random(256);
		}
		if (a != NULL || b != NULL) {
			game_random_carcol(NC_GetVehicleModel(vehicleid), a, b);
		}
		NC_PARS(3);
		nc_params[1] = vehicleid;
		NC(n_ChangeVehicleColor);
		if (veh != NULL) {
			veh->col1 = (unsigned char) *col1;
			veh->col2 = (unsigned char) *col2;
			csprintf(buf144,
				"UPDATE veh SET col1=%d,col2=%d WHERE i=%d",
				veh->col1,
				veh->col2,
				veh->id);
			NC_mysql_tquery_nocb(buf144a);
		}
	}
	return 1;
}

/**
The /tickrate command to print the server's current tick rate.
*/
static
int cmd_tickrate(CMDPARAMS)
{
	csprintf(buf32, "%d", (int) NC_GetServerTickRate());
	NC_SendClientMessage(playerid, -1, buf32a);
	return 1;
}
