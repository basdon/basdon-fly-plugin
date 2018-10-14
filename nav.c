
/* vim: set filetype=c ts=8 noexpandtab: */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "common.h"
#include "airport.h"
#include <string.h>
#include <math.h>

static struct navdata {
	struct airport *beacon;
	struct runway *vor;
	struct runway *ils;
	int dist;
	int alt;
	int crs;
} *nav[MAX_VEHICLES];

static struct playercache {
	int dist;
	int alt;
	int crs;
} pcache[MAX_PLAYERS];

#define INVALID_CACHE 500000

void nav_init()
{
	int i = 0;
	while (i < MAX_VEHICLES) {
		nav[i++] = NULL;
	}
}

void nav_resetcache(int playerid)
{
	pcache[playerid].dist = INVALID_CACHE;
	pcache[playerid].alt = INVALID_CACHE;
	pcache[playerid].crs = INVALID_CACHE;
}

/* native Nav_Reset(vehicleid) */
cell AMX_NATIVE_CALL Nav_Reset(AMX *amx, cell *params)
{
	int vid = params[1];
	if (nav[vid] != NULL) {
		free(nav[vid]);
		nav[vid] = NULL;
		return 1;
	}
	return 0;
}

/* native Nav_EnableADF(vehicleid, beacon[]) */
cell AMX_NATIVE_CALL Nav_EnableADF(AMX *amx, cell *params)
{
	int vid = params[1];
	cell *addr;
	char beacon[5], *bp = beacon, len = 0;
	struct airport *ap = airports;

	amx_GetAddr(amx, params[2], &addr);
	amx_GetUString(beacon, addr, 5);

	while (1) {
		if (*bp == 0) {
			break;
		}
		if (++len >= 5) {
			return 0;
		}
		*bp &= ~0x20;
		if (*bp < 'A' || 'Z' < *bp) {
			return 0;
		}
		bp++;
	}

	len = airportscount;
	while (len--) {
		if (strcmp(beacon, ap->beacon) == 0) {
			if (nav[vid] == NULL) {
				nav[vid] = malloc(sizeof(struct navdata));
				nav[vid]->alt = nav[vid]->crs = nav[vid]->dist = 0.0f;
			}
			nav[vid]->beacon = ap;
			nav[vid]->vor = nav[vid]->ils = NULL;
			return 1;
		}
		ap++;
	}

	return 0;
}

/* native Nav_Update(vehicleid, Float:x, Float:y, Float:z, Float:heading) */
cell AMX_NATIVE_CALL Nav_Update(AMX *amx, cell *params)
{
	struct navdata *n = nav[params[1]];
	float x = amx_ctof(params[2]);
	float y = amx_ctof(params[3]);
	float z = amx_ctof(params[4]);
	float heading = amx_ctof(params[5]);
	float dx, dy;
	float crs;
	struct vec3 *pos;

	if (n == NULL) {
		return 0;
	}

	if (n->beacon != NULL) {
		pos = &n->beacon->pos;
	} else if (n->vor != NULL) {
		pos = &n->vor->pos;
	} else if (n->ils != NULL) {
		pos = &n->ils->pos;
	} else {
		return 0;
	}

	dx = x - pos->x;
	dy = pos->y - y;
	n->dist = (int) sqrt(dx * dx + dy * dy);
	if (n->dist > 1000) {
		n->dist = (n->dist / 100) * 100;
	}
	n->alt = (int) (z - pos->z);
	crs = -(atan2(dx, dy) * 180.0f / M_PI);
	crs = 360.0f - heading - crs;
	n->crs = (int) (crs - floor((crs + 180.0f) / 360.0f) * 360.0f);

	return 1;
}

/* native Nav_Format(playerid, vehicleid, bufdist[], bufalt[], bufcrs[]) */
cell AMX_NATIVE_CALL Nav_Format(AMX *amx, cell *params)
{
	int pid = params[1];
	struct navdata *n = nav[params[2]];
	cell *addr;
	char dist[16], alt[16], crs[16];

	if (n == NULL) {
		return 0;
	}

	dist[0] = alt[0] = crs[0] = 0;

	if (n->dist != pcache[pid].dist) {
		if (n->dist >= 1000) {
			sprintf(dist, "%.1fK", n->dist / 1000.0f);
		} else {
			sprintf(dist, "%d", n->dist);
		}
		pcache[pid].dist = n->dist;
	}
	if (n->alt != pcache[pid].alt) {
		sprintf(alt, "%d", n->alt);
		pcache[pid].alt = n->alt;
	}
	if (n->crs != pcache[pid].crs) {
		sprintf(crs, "%d", n->crs);
		pcache[pid].crs = n->crs;
	}

	amx_GetAddr(amx, params[3], &addr);
	amx_SetUString(addr, dist, 16);
	amx_GetAddr(amx, params[4], &addr);
	amx_SetUString(addr, alt, 16);
	amx_GetAddr(amx, params[5], &addr);
	amx_SetUString(addr, crs, 16);

	return dist[0] != 0 || alt[0] != 0 || crs[0] != 0;
}