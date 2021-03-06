
/**
Map of airports corresponding to nearest airports from /nearest cmd.
*/
static struct AIRPORT **nearest_airports_map[MAX_PLAYERS];
/**
Amount of airports in the nearest_airports_map array.
*/
static short nearest_airports_map_size[MAX_PLAYERS];

/**
Clears all data and frees all allocated memory.
*/
static
void airports_destroy()
{
	struct AIRPORT *ap = airports;

	while (numairports) {
		numairports--;
		free(ap->runways);
		ap++;
	}
        free(airports);
	airports = NULL;
}

#ifdef AIRPORT_PRINT_STATS
static
void airports_print_stats()
{
	struct AIRPORT *ap;
	struct RUNWAY *rnw;
	struct MISSIONPOINT *msp;
	int i, j;

	ap = airports;
	for (i = 0; i < numairports; i++) {
		printf("airport %d: %s code %s\n", ap->id, ap->name, ap->code);
		printf(" runwaycount: %d\n", ap->runwaysend - ap->runways);
		rnw = ap->runways;
		while (rnw != ap->runwaysend) {
			printf("  runway %s type %d nav %d\n", rnw->id, rnw->type, rnw->nav);
			rnw++;
		}
		printf(" missionpoints: %d types: %d\n", ap->num_missionpts, ap->missiontypes);
		msp = ap->missionpoints;
		for (j = 0; j < ap->num_missionpts; j++) {
			printf("  missionpoint %d: %s type %d point_type %d\n", msp->id, msp->name, msp->type, msp->point_type);
			msp++;
		}
		ap++;
	}
}
#endif

/**
Loads airports/runways/missionpoints (and init other things).
*/
static
void airports_init()
{
	struct AIRPORT *ap;
	struct RUNWAY *rnw;
	struct MISSIONPOINT *msp;
	int cacheid, rowcount, lastap, *field = nc_params + 2;
	int i, j, airportid;

	/*init indexmap*/
	i = MAX_PLAYERS;
	while (i--) {
		nearest_airports_map[i] = NULL;
	}

	/*load airports*/
	atoc(buf144, "SELECT i,c,n,e,x,y,z,flags FROM apt ORDER BY i ASC", 144);
	cacheid = NC_mysql_query(buf144a);
	numairports = NC_cache_get_row_count();
	if (!numairports) {
		assert(((void) "no airports", 0));
	} else if (numairports > MAX_AIRPORTS) {
		assert(((void) "too many airports", 0));
	}
	airports = malloc(sizeof(struct AIRPORT) * numairports);
	nc_params[3] = buf144a;
	for (i = 0; i < numairports; i++) {
		ap = &airports[i];
		ap->runways = ap->runwaysend = NULL;
		ap->missionpoints = NULL;
		ap->num_missionpts = 0;
		ap->missiontypes = 0;
		NC_PARS(3);
		nc_params[1] = i;
		ap->id = (*field = 0, NC(n_cache_get_field_i));
		*field = 1; NC(n_cache_get_field_s);
		ctoa(ap->code, buf144, sizeof(ap->code));
		*field = 2; NC(n_cache_get_field_s);
		ctoa(ap->name, buf144, sizeof(ap->name));
		NC_PARS(2);
		ap->enabled = (char) (*field = 3, NC(n_cache_get_field_i));
		ap->pos.x = (*field = 4, NCF(n_cache_get_field_f));
		ap->pos.y = (*field = 5, NCF(n_cache_get_field_f));
		ap->pos.z = (*field = 6, NCF(n_cache_get_field_f));
		ap->flags = (*field = 7, NC(n_cache_get_field_i));
	}
	NC_cache_delete(cacheid);

	/*TODO make this into one big arraw aswell?*/
	/*load runways*/
	B144("SELECT a,x,y,z,n,type,h,s FROM rnw ORDER BY a ASC");
	cacheid = NC_mysql_query(buf144a);
	rowcount = NC_cache_get_row_count();
	if (!rowcount) {
		assert(((void) "no runways", 0));
	}
	rowcount--;
	while (rowcount >= 0) {
		lastap = NC_cache_get_field_int(rowcount, 0);
		ap = airports + lastap;

		/*look 'ahead' to see how many runways there are*/
		i = rowcount - 1;
		while (i > 0) {
			i--;
			nc_params[1] = i;
			if (NC(n_cache_get_field_i) != lastap) {
				break;
			}
		}

		/*gottem*/
		i = rowcount - i;
		ap->runways = rnw = malloc(sizeof(struct RUNWAY) * i);
		ap->runwaysend = ap->runways + i;
		nc_params[1] = rowcount;
		nc_params[3] = buf32a;
		while (i--) {
			NC_PARS(2);
			nc_params[1] = rowcount;
			rnw->pos.x = (*field = 1, NCF(n_cache_get_field_f));
			rnw->pos.y = (*field = 2, NCF(n_cache_get_field_f));
			rnw->pos.z = (*field = 3, NCF(n_cache_get_field_f));
			rnw->nav = (*field = 4, NC(n_cache_get_field_i));
			rnw->type = (*field = 5, NC(n_cache_get_field_i));
			rnw->heading = (*field = 6, NCF(n_cache_get_field_f));
			rnw->headingr = (360.0f - rnw->heading + 90.0f) * DEG_TO_RAD;
			NC_PARS(3);
			*field = 7; NC(n_cache_get_field_s);
			sprintf(rnw->id, "%02.0f", (float) ceil(rnw->heading / 10.0f));
			rnw->id[2] = (char) buf32[0];
			rnw->id[3] = 0;
			rnw++;
			rowcount--;
		}
	}
	NC_cache_delete(cacheid);

	/*load missionpoints*/
	/*They _HAVE_ to be ordered by airport.*/
	atoc(buf144, "SELECT a,i,x,y,z,t,name FROM msp ORDER BY a ASC,i ASC", 144);
	cacheid = NC_mysql_query(buf144a);
	nummissionpoints = NC_cache_get_row_count();
	if (!nummissionpoints) {
		assert(((void) "no missionpoints", 0));
	}
	missionpoints = malloc(sizeof(struct MISSIONPOINT) * nummissionpoints);
	ap = NULL;
	nc_params[3] = buf32a;
	for (i = 0; i < nummissionpoints; i++) {
		msp = &missionpoints[i];
		NC_PARS(2);
		nc_params[1] = i;
		airportid = (*field = 0, NC(n_cache_get_field_i));
		if (!ap || ap->id != airportid) {
			for (j = 0; j < numairports; j++) {
				if (airports[j].id == airportid) {
					ap = &airports[j];
					goto have_airport;
				}
			}
			/*This should never happen, as per database scheme.*/
			assert(((void) "failed to link msp to ap", 0));
have_airport:
			ap->missionpoints = msp;
		}
		ap->num_missionpts++;

		if (ap->num_missionpts > MAX_MISSIONPOINTS_PER_AIRPORT) {
			assert(((void) "too many missionpoints", 0));
		}

		/*missionlocations are filled in mission.c, they should still be zero initialized though.
		(They are filled based on if they are null.)*/
		memset(msp->missionlocations, 0, sizeof(msp->missionlocations));
		msp->ap = ap;
		msp->id = (unsigned short) (*field = 1, NC(n_cache_get_field_i));
		msp->pos.x = (*field = 2, NCF(n_cache_get_field_f));
		msp->pos.y = (*field = 3, NCF(n_cache_get_field_f));
		msp->pos.z = (*field = 4, NCF(n_cache_get_field_f));
		msp->type = (*field = 5, NC(n_cache_get_field_i));
		msp->has_player_browsing_missions = 0;
		ap->missiontypes |= msp->type;
		if (msp->type & PASSENGER_MISSIONTYPES) {
			msp->point_type = MISSION_POINT_PASSENGERS;
			if (msp->type & ~PASSENGER_MISSIONTYPES) {
mixed_missionpoints:
				logprintf("mixed missionpoint types msp id %d ap %s", msp->id, ap->name);
				assert(((void) "mixed missionpoint types", 0));
			}
		} else if (msp->type & CARGO_MISSIONTYPES) {
			msp->point_type = MISSION_POINT_CARGO;
			if (msp->type & ~CARGO_MISSIONTYPES) {
				goto mixed_missionpoints;
			}
		} else {
			msp->point_type = MISSION_POINT_SPECIAL;
		}
		NC_PARS(3);
		*field = 6; NC(n_cache_get_field_s);
		ctoa(msp->name, buf32, MAX_MSP_NAME + 1);
	}
	NC_cache_delete(cacheid);

#ifdef AIRPORT_PRINT_STATS
	airports_print_stats();
#endif
}

/**
Structure used for sorting airports by distance for the /nearest list dialog.
*/
struct APREF {
	float distance;
	int index;
};

/**
Sort function used to sort airports by distance for the /nearest list dialog.
*/
static
int sortaprefs(const void *_a, const void *_b)
{
	struct APREF *a = (struct APREF*) _a, *b = (struct APREF*) _b;
	return (int) (10.0f * (b->distance - a->distance));
}

/**
The /nearest command, shows a list dialog with airports.

Airports are sorted by distance from player. Choosing an airport shows an
additional dialog with information about the selected airport.
*/
static
int airport_cmd_nearest(CMDPARAMS)
{
	int i = 0;
	int num = 0;
	struct vec3 playerpos;
	float dx, dy;
	char buf[4096], *b;
	struct AIRPORT *ap;
	struct AIRPORT **map;
	struct APREF *aps = malloc(sizeof(struct APREF) * numairports);

	common_GetPlayerPos(playerid, &playerpos);

	while (i < numairports) {
		if (airports[i].enabled) {
			dx = airports[i].pos.x - playerpos.x;
			dy = airports[i].pos.y - playerpos.y;
			aps[num].distance = sqrt(dx * dx + dy * dy);
			aps[num].index = i;
			num++;
		}
		i++;
	}
	if (num == 0) {
		SendClientMessage(playerid, COL_WARN, WARN"No airports!");
		return 1;
	}
	qsort(aps, num, sizeof(struct APREF), sortaprefs);
	map = nearest_airports_map[playerid];
	if (map == NULL) {
		map = malloc(sizeof(struct AIRPORT*) * num);
		nearest_airports_map[playerid] = map;
	}
	i = nearest_airports_map_size[playerid] = num;
	b = buf;
	while (i--) {
		*map = ap = airports + aps[i].index;
		map++;
		if (aps[i].distance < 1000.0f) {
			b += sprintf(b, "\n%.0f", aps[i].distance);
		} else {
			b += sprintf(b, "\n%.1fK", aps[i].distance / 1000.0f);
		}
		b += sprintf(b, "\t%s\t[%s]", ap->name, ap->code);
	}

	dialog_ShowPlayerDialog(
		playerid,
		DIALOG_AIRPORT_NEAREST,
		DIALOG_STYLE_TABLIST,
		"Nearest airports",
		buf + 1,
		"Info",
		"Close",
		-1);

	free(aps);
	return 1;
}

/**
Call when getting a response from DIALOG_AIRPORT_NEAREST, to show info dialog.
*/
static
void airport_nearest_dialog_response(int playerid, int response, int idx)
{
	struct AIRPORT *ap;
	struct RUNWAY *rnw;
	int helipads = 0;
	char title[64], info[4096], *b;
	char szRunways[] = "Runways:";

	if (nearest_airports_map[playerid] == NULL) {
		return;
	}
	if (!response ||
		idx < 0 ||
		nearest_airports_map_size[playerid] <= idx)
	{
		goto freereturn;
	}
	ap = nearest_airports_map[playerid][idx];

	sprintf(title, "%s - %s", ap->code, ap->name);

	b = info;
	b += sprintf(b, "\nElevation:\t%.0f FT", ap->pos.z);
	rnw = ap->runways;
	while (rnw != ap->runwaysend) {
		if (rnw->type == RUNWAY_TYPE_HELIPAD) {
			helipads++;
		} else {
			b += sprintf(b, "\n%s\t%s", szRunways, rnw->id);
			if (rnw->nav == (NAV_VOR | NAV_ILS)) {
				b += sprintf(b, " (VOR+ILS)");
			} else if (rnw->nav) {
				b += sprintf(b, " (VOR)");
			}
			szRunways[0] = '\t';
			szRunways[1] = 0;
		}
		rnw++;
	}
	b += sprintf(b, "\nHelipads:\t%d", helipads);

	dialog_ShowPlayerDialog(
		playerid,
		DIALOG_DUMMY,
		DIALOG_STYLE_MSGBOX,
		title,
		info + 1,
		"Close",
		NULL,
		-1);

freereturn:
	free(nearest_airports_map[playerid]);
	nearest_airports_map[playerid] = NULL;
}

/**
The /beacons command, shows a dialog with list of all airport beacons.
*/
static
int airport_cmd_beacons(CMDPARAMS)
{
	char buf[4096], *b = buf;
	struct AIRPORT *ap = airports;
	int count = numairports;

	while (count-- > 0) {
		if (ap->enabled) {
			b += sprintf(b, " %s", ap->code);
		}
		ap++;
	}
	if (b == buf) {
		strcpy(buf, " None!");
	}
	dialog_ShowPlayerDialog(
		playerid,
		DIALOG_DUMMY,
		DIALOG_STYLE_MSGBOX,
		"Beacons",
		buf + 1,
		"Close",
		NULL,
		-1);
	return 1;
}

/**
Cleanup stored stuff for player when they disconnect.
*/
static
void airport_on_player_disconnect(int playerid)
{
	if (nearest_airports_map[playerid] != NULL) {
		free(nearest_airports_map[playerid]);
		nearest_airports_map[playerid] = NULL;
	}
}
