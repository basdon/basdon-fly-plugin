
/* vim: set filetype=c ts=8 noexpandtab: */

/**
Dialog response handler for base //vehinfo dialog
*/
void admin_dlg_vehinfo_response(int pid, int res, int listitem);
/**
The //respawn cmd respawns the vehicle the player is in.
*/
int cmd_admin_respawn(CMDPARAMS);
/**
The //vehinfo cmd shows vehicle info

Also allows editing enabled state and linked airport.
*/
int cmd_admin_vehinfo(CMDPARAMS);