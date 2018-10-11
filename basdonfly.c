
/* vim: set filetype=c ts=8 noexpandtab: */

#include "common.h"
#include "playerdata.h"

logprintf_t logprintf;
extern void *pAMXFunctions;

#define FORWARD(X) cell AMX_NATIVE_CALL X(AMX *amx, cell *params)

/* airport.c */
FORWARD(APT_Init);
FORWARD(APT_Destroy);
FORWARD(APT_Add);
FORWARD(APT_AddRunway);
FORWARD(APT_FormatNearestList);
FORWARD(APT_MapIndexFromListDialog);
FORWARD(APT_FormatInfo);
FORWARD(APT_FormatCodeAndName);
/* commands.c */
FORWARD(CommandHash);
FORWARD(IsCommand);
/* dialog.c */
FORWARD(QueueDialog);
FORWARD(DropDialogQueue);
FORWARD(HasDialogsInQueue);
FORWARD(PopDialogQueue);
/* various.c */
FORWARD(Urlencode);
/* panel.c */
FORWARD(Panel_ResetCaches);
FORWARD(Panel_FormatAltitude);
FORWARD(Panel_FormatSpeed);
FORWARD(Panel_FormatHeading);
/* playerdata.c */
FORWARD(PlayerData_Init);
FORWARD(PlayerData_Clear);
FORWARD(PlayerData_UpdateName);
/* game_sa.c */
FORWARD(IsAirVehicle);
/* login.c */
FORWARD(ResetPasswordConfirmData);
FORWARD(SetPasswordConfirmData);
FORWARD(ValidatePasswordConfirmData);
FORWARD(FormatLoginApiRegister);
FORWARD(FormatLoginApiLogin);
FORWARD(FormatLoginApiGuestRegister);
FORWARD(FormatLoginApiCheckChangePass);
FORWARD(FormatLoginApiUserExistsGuest);
/* zones.c */
FORWARD(Zone_UpdateForPlayer);

cell AMX_NATIVE_CALL ValidateMaxPlayers(AMX *amx, cell *params)
{
	if (MAX_PLAYERS != params[1]) {
		logprintf("[FATAL] MAX_PLAYERS mistmatch: %d (plugin) vs %d (gm)", MAX_PLAYERS, params[1]);
		return 0;
	}
	return MAX_PLAYERS;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
	void game_sa_init(), login_init(), dialog_init(), zones_init();

	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t) ppData[PLUGIN_DATA_LOGPRINTF];

	game_sa_init();
	login_init();
	dialog_init();
	pdata_init();
	zones_init();

	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
}

#define REGISTERNATIVE(X) {#X, X}
AMX_NATIVE_INFO PluginNatives[] =
{
	REGISTERNATIVE(ValidateMaxPlayers),
	REGISTERNATIVE(Urlencode),
	REGISTERNATIVE(Panel_ResetCaches),
	REGISTERNATIVE(Panel_FormatAltitude),
	REGISTERNATIVE(Panel_FormatSpeed),
	REGISTERNATIVE(Panel_FormatHeading),
	REGISTERNATIVE(IsAirVehicle),
	REGISTERNATIVE(ResetPasswordConfirmData),
	REGISTERNATIVE(SetPasswordConfirmData),
	REGISTERNATIVE(ValidatePasswordConfirmData),
	REGISTERNATIVE(CommandHash),
	REGISTERNATIVE(IsCommand),
	REGISTERNATIVE(QueueDialog),
	REGISTERNATIVE(DropDialogQueue),
	REGISTERNATIVE(HasDialogsInQueue),
	REGISTERNATIVE(PopDialogQueue),
	REGISTERNATIVE(PlayerData_Init),
	REGISTERNATIVE(PlayerData_Clear),
	REGISTERNATIVE(PlayerData_UpdateName),
	REGISTERNATIVE(FormatLoginApiRegister),
	REGISTERNATIVE(FormatLoginApiLogin),
	REGISTERNATIVE(FormatLoginApiGuestRegister),
	REGISTERNATIVE(FormatLoginApiCheckChangePass),
	REGISTERNATIVE(FormatLoginApiUserExistsGuest),
	REGISTERNATIVE(APT_Init),
	REGISTERNATIVE(APT_Destroy),
	REGISTERNATIVE(APT_Add),
	REGISTERNATIVE(APT_AddRunway),
	REGISTERNATIVE(APT_FormatNearestList),
	REGISTERNATIVE(APT_MapIndexFromListDialog),
	REGISTERNATIVE(APT_FormatInfo),
	REGISTERNATIVE(APT_FormatCodeAndName),
	REGISTERNATIVE(Zone_UpdateForPlayer),
	{0, 0}
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	return amx_Register(amx, PluginNatives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	return AMX_ERR_NONE;
}

