
/* vim: set filetype=c ts=8 noexpandtab: */

#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include "common.h"

#define MAP_MAX_FILENAME (24) /*see db col*/

struct OBJECT {
	int model;
	float x, y, z, rx, ry, rz;
};

struct MAP {
	int id;
	float x, y;
	float radius_in_sq, radius_out_sq;
	char name[64];
	int numobjects;
	struct OBJECT *objects;
	/* 1 streamed in, 0 streamed out */
	char streamstatus[MAX_PLAYERS];
};

static int nummaps;
static struct MAP *maps;

int maps_load_from_file(struct MAP *map)
{
	FILE *filehandle;
	char filename[18 + MAP_MAX_FILENAME];
	int specversion;
	struct OBJECT *obj;

	sprintf_s(filename, sizeof(filename), "scriptfiles/maps/%s", map->name);
	if (!(filehandle = fopen(filename, "rb"))) {
		logprintf("failed to open map %s", filename);
		return 0;
	}

	if (fread(&specversion, sizeof(int), 1, filehandle) != 1) {
		goto corrupted;
	}

	if (specversion != 1) {
		logprintf("unknown map spec v%d for %s", specversion, filename);
		goto exitzero;
	}
	obj = map->objects = malloc(sizeof(struct OBJECT) * 900);
	do {
		if (fread(obj, sizeof(struct OBJECT), 1, filehandle) != 1) {
			if (feof(filehandle)) {
				goto allread;
			} else {
				free(map->objects);
				goto corrupted;
			}
		}
		obj++;
	} while (obj - map->objects < 900);
	logprintf("warn: map %s truncated to 900 objects", filename);
allread:
	map->numobjects = obj - map->objects;

	fclose(filehandle);
	return 1;
corrupted:
	logprintf("corrupted map %s", filename);
exitzero:
	fclose(filehandle);
	return 0;
}

void maps_load_from_db(AMX *amx)
{
	char q[] =
		"SELECT id,filename,midx,midy,radi,rado FROM map "
		"LEFT JOIN apt ON map.ap=apt.i "
		"WHERE apt.e=1 OR apt.e IS NULL";
	cell cacheid, rowcount;
	cell f1, f2, f3, f4;
	struct MAP *map, *m;
	const struct MAP *end;

	amx_SetUString(buf4096, q, sizeof(q));
	NC_mysql_query_(buf4096a, &cacheid);
	NC_cache_get_row_count_(&rowcount);
	nummaps = rowcount;
	if (rowcount) {
		maps = malloc(nummaps * sizeof(struct MAP));
		while (rowcount--) {
			map = maps + rowcount;
			NC_cache_get_row_int(rowcount, 0, (cell*) &map->id);
			NC_cache_get_row_str(rowcount, 1, buf32a);
			NC_cache_get_row_flt(rowcount, 2, &f1);
			NC_cache_get_row_flt(rowcount, 3, &f2);
			NC_cache_get_row_flt(rowcount, 4, &f3);
			NC_cache_get_row_flt(rowcount, 5, &f4);
			amx_GetUString(map->name, buf32, sizeof(map->name));
			if (!maps_load_from_file(map)) {
				map->id = -1;
				continue;
			}
			map->x = amx_ctof(f1);
			map->y = amx_ctof(f2);
			map->radius_in_sq = amx_ctof(f3);
			map->radius_out_sq = amx_ctof(f4);
			map->radius_in_sq *= map->radius_in_sq;
			map->radius_out_sq *= map->radius_out_sq;
		}
	}
	NC_cache_delete(cacheid);

	/*remove gaps caused by maps that failed to load*/
	end = maps + nummaps;
	map = m = maps;
	while (map < end) {
		if (map->id == -1) {
			nummaps--;
		} else {
			if (map != m) {
				memcpy(m, map, sizeof(struct MAP));
			}
			m++;
		}
		map++;
	}
}

void maps_stream_for_player(AMX *amx, int playerid)
{
	float dx, dy, dist;
	struct MAP *map = maps + nummaps;

	NC_GetPlayerPos(playerid, buf32a, buf64a, buf144a);
	while (map-- != maps) {
		dx = map->x - amx_ctof(*buf32);
		dy = map->y - amx_ctof(*buf64);
		dist = dx * dx + dy * dy;
		if (map->streamstatus[playerid]) {
			if (dist > map->radius_out_sq) {
				/*stream out*/
				map->streamstatus[playerid] = 0;
				logprintf("map %d streamed out for %d",
					map->id,
					playerid);
			}
		} else  {
			if (dist < map->radius_in_sq) {
				/*stream in*/
				map->streamstatus[playerid] = 1;
				logprintf("map %d streamed in for %d",
					map->id,
					playerid);
			}
		}
	}
}

/*
Go over players to see if maps need to be streamed for them
This function should be called from loop100
It checks 1+playercount/10 amount of players per call, so:

 playercount | checked players | ms between ticks
     1-9              1              100-900
    10-19             2              500-950
    20-29             3              700-966
   100-109           11	            1000-1000
 (not sure if this is correct; I'm too tired)
*/
void maps_process_tick(AMX *amx)
{
	static int currentplayeridx = 0;
	int increment = 1 + playercount / 10;

	if (playercount > 0) {
		while (increment--) {
			if (++currentplayeridx >= playercount) {
				currentplayeridx = 0;
			}
			maps_stream_for_player(amx, players[currentplayeridx]);
		}
	}
}

void maps_OnPlayerDisconnect(int playerid)
{
	struct MAP *map = maps + nummaps;
	while (map-- != maps) {
		map->streamstatus[playerid] = 0;
	}
}
