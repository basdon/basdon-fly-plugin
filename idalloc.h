/*See MAX_MISSIONPOINTS_PER_AIRPORT. 15 is enough.
If an airport has more, they are probably split out enough anyways.*/
#define MAX_MISSION_INDICATORS (15)

/*ACTOR IDS*/

/*Octavia's actor id on Octavia Island*/
#define OCTA_ACTORID 999

/*DIALOG IDS*/

/*can be used when no response is needed*/
#define DIALOG_DUMMY 127
/*max dialog is 32767*/
#define DIALOG_SPAWN_SELECTION 1001
#define DIALOG_PREFERENCES 1002
#define DIALOG_AIRPORT_NEAREST 1003
#define DIALOG_REGISTER_FIRSTPASS 1004
#define DIALOG_LOGIN_LOGIN_OR_NAMECHANGE 1005
#define DIALOG_REGISTER_CONFIRMPASS 1006
#define DIALOG_LOGIN_NAMECHANGE 1007
#define DIALOG_CHANGEPASS_PREVPASS 1008
#define DIALOG_CHANGEPASS_NEWPASS 1009
#define DIALOG_CHANGEPASS_CONFIRMPASS 1010
#define DIALOG_GUESTREGISTER_CHANGENAME 1011
#define DIALOG_GUESTREGISTER_FIRSTPASS 1012
#define DIALOG_GUESTREGISTER_CONFIRMPASS 1013
#define DIALOG_VEHINFO_VEHINFO 1014
#define DIALOG_VEHINFO_ASSIGNAP 1015
/*Dialog transactions.*/
/*don't use an id that is used in a dialog id (unless they relate), use 32768+*/
/*max transaction id is int max*/
#define TRANSACTION_NONE 0
#define TRANSACTION_OVERRIDE 1
#define TRANSACTION_MISSION_OVERVIEW 100000
#define TRANSACTION_LOGIN 100001
#define TRANSACTION_CHANGEPASS 100002
#define TRANSACTION_CHANGEPASS_ABORTED 100003
#define TRANSACTION_GUESTREGISTER 100004

/*OBJECT IDS*/

/*Max amount of objects the mapsystem may use.*/
/*These are also the ids, so mapsystem uses ids 0-899
(unless the code changed to not use objectid 0).*/
#define MAX_MAPSYSTEM_OBJECTS (900)
#define OBJECT_MISSION_INDICATOR_BASE (984)
#define OBJECT_ROTATING_RADAR (999)
#if OBJECT_MISSION_INDICATOR_BASE + MAX_MISSION_INDICATORS != OBJECT_ROTATING_RADAR
#error "update object ids"
#endif

/*TEXTDRAW IDS*/

/*Ids 0-2047 are global TDs, ids 2048-2303 are player TDs.*/
/*Drawing order is .. questionable and TBD.*/

/*TODO lower these ids again when all textdraws are converted*/
#define TEXTDRAW_MISSIONHELP_BASE (1000)
/*Missionmap: using same base as missionhelp, since they can't show at the same time anyways.*/
#define TEXTDRAW_MISSIONMAP_BASE TEXTDRAW_MISSIONHELP_BASE
#define TEXTDRAW_JOBSATISFACTION (TEXTDRAW_MISSIONHELP_BASE - 1)
#define TEXTDRAW_GPS (TEXTDRAW_JOBSATISFACTION - 1)
#define NUM_KNEEBOARD_TEXTDRAWS (3)
#define TEXTDRAW_KNEEBOARD_BASE (TEXTDRAW_GPS - NUM_KNEEBOARD_TEXTDRAWS)
#define NUM_PANEL_TEXTDRAWS (28)
#define TEXTDRAW_PANEL_BASE (TEXTDRAW_KNEEBOARD_BASE - NUM_PANEL_TEXTDRAWS)
STATIC_ASSERT(TEXTDRAW_PANEL_BASE > 0);
