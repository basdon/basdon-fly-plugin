
/* vim: set filetype=c ts=8 noexpandtab: */

#define DIALOG_DUMMY 127
/*used ids from 1000 and up; gm uses < 1000*/
#define DIALOG_SPAWN_SELECTION 1001

#define TRANSACTION_NONE 0
#define TRANSACTION_OVERRIDE 1

#define DIALOG_STYLE_MSGBOX 0
#define DIALOG_STYLE_INPUT 1
#define DIALOG_STYLE_LIST 2
#define DIALOG_STYLE_PASSWORD 3
#define DIALOG_STYLE_TABLIST 4
#define DIALOG_STYLE_TABLIST_HEADERS 5

/**
Ends dialog transaction for player if it matches given transactionid.

If the player has a different transaction going, it will not be
ended and a warning will be logged.
*/
void dialog_end_transaction(int playerid, int transactionid);

/**
Ensures the current transactionid for given player.

If the player already has a different transaction going, it will not be
overridden and a warning will be logged.
*/
void dialog_ensure_transaction(int playerid, int transactionid);

/**
Shows the player a synchronous (only one at a time) dialog box.

Uses buf4096 for info, buf64 for caption, buf32 and buf32_1 for buttons.
buf32_1 is not used if button2 is NULL.

Transactions are used to make sure open dialogs do not disappear when calling.
If a transaction is already running, the dialog to open gets added to a queue
which is being polled and will be shown once no transaction is active anymore.

The transaction is reset in OnDialogResponse, use dialog_ensure_transaction
in your dialog's response code to keep the transaction set (eg when doing a
db query and not wanting any other dialog shown while the query is executing).

@param style see DIALOG_STYLE_ constants
@param playerid The ID of the player to show the dialog to
@param dialogid An ID to assign this dialog to, so responses can be processed.
                Max dialogid is 32767. Using negative values will close any
		open dialog
@param style The style of the dialog
@param caption The title at the top of the dialog.
               The length of the caption can not exceed more than 64 characters
	       before it starts to cut off
@param info The text to display in the main dialog. Use \n to start a new line
            and \t to tabulate
@param button1 The text on the left button
@param button2 The text on the right button. Leave it blank ("") to hide it
@param transactionid Transaction id this dialog belongs to. Use -1 to use the
                     dialog id as transacton id. TRANSACTION_OVERRIDE can be
		     used to override any active transactions (cases will be
		     logged to console). TRANSACTION_NONE can be used to not
		     case about it being overridden.
@returns 0 on failure, player is not connected. NOTE: also returns 1 if the
         dialog was queued.
*/
int dialog_NC_ShowPlayerDialog(
	AMX *amx, int playerid, int dialogid, int style,
	char *caption, char *info,
	char *button1, char *button2, int transactionid);
