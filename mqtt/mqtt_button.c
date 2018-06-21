/*
 * stdin publisher
 * 
 * compulsory parameters:
 * 
 * --topic topic to publish on
 * 
 * defaulted parameters:
 * 
 *  --host :: localhost
 *  --port :: 1883
 *  --qos :: 0
 *  --delimiters :: \n
 *  --clientid :: stdin-publisher-async
 *  --maxdatalen :: 100
 *  --keepalive :: 10
 * 
 *  --userid :: none
 *  --password :: none
 *  
 * gcc -I/usr/local/include -L/usr/local/lib/ -o select_mqtt_async_pub select_mqtt_async_pub.c -lpaho-mqtt3a
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/select.h>

/* struct input_event */
#include <linux/input.h>

/* Logmask for Informational, Debug and Error Messages */
#include <syslog.h>

#include <MQTTAsync.h>

#define DEVICE_PATH "/dev/input/event2"
#define LOG_LEVEL LOG_DEBUG

MQTTAsync client;

struct {
	char *clientid;
	char *delimiter;
	int maxdatalen;
	int qos;
	int retained;
	char *username;
	char *password;
	char *host;
	char *port;
	int verbose;
	int keepalive;
} opts = {
"stdin-publisher-async", "\n", 100, 0, 0, NULL, NULL, "localhost",
	    "1883", 1, 10};

char *events[EV_MAX + 1] = {
	[0 ... EV_MAX] = NULL,
	[EV_SYN]        = "Sync",			[EV_KEY] = "Key",
	[EV_REL]        = "Relative",			[EV_ABS] = "Absolute",
	[EV_MSC]        = "Misc",			[EV_LED] = "LED",
	[EV_SND]        = "Sound",			[EV_REP] = "Repeat",
	[EV_FF]         = "ForceFeedback",		[EV_PWR] = "Power",
	[EV_FF_STATUS]  = "ForceFeedbackStatus",
};

char *repeats[REP_MAX + 1] = {
	[0 ... REP_MAX] = NULL,
	[REP_DELAY] = "Delay",		[REP_PERIOD] = "Period"
};

char *sounds[SND_MAX + 1] = {
	[0 ... SND_MAX] = NULL,
	[SND_CLICK] = "Click",		[SND_BELL] = "Bell",
	[SND_TONE] = "Tone"
};

char *leds[LED_MAX + 1] = {
	[0 ... LED_MAX] = NULL,
	[LED_NUML] = "NumLock",		[LED_CAPSL] = "CapsLock", 
	[LED_SCROLLL] = "ScrollLock",	[LED_COMPOSE] = "Compose",
	[LED_KANA] = "Kana",		[LED_SLEEP] = "Sleep", 
	[LED_SUSPEND] = "Suspend",	[LED_MUTE] = "Mute",
	[LED_MISC] = "Misc",
};

char *absolutes[ABS_MAX + 1] = {
	[0 ... ABS_MAX] = NULL,
	[ABS_X] = "X",			[ABS_Y] = "Y",
	[ABS_Z] = "Z",			[ABS_RX] = "Rx",
	[ABS_RY] = "Ry",		[ABS_RZ] = "Rz",
	[ABS_THROTTLE] = "Throttle",	[ABS_RUDDER] = "Rudder",
	[ABS_WHEEL] = "Wheel",		[ABS_GAS] = "Gas",
	[ABS_BRAKE] = "Brake",		[ABS_HAT0X] = "Hat0X",
	[ABS_HAT0Y] = "Hat0Y",		[ABS_HAT1X] = "Hat1X",
	[ABS_HAT1Y] = "Hat1Y",		[ABS_HAT2X] = "Hat2X",
	[ABS_HAT2Y] = "Hat2Y",		[ABS_HAT3X] = "Hat3X",
	[ABS_HAT3Y] = "Hat 3Y",		[ABS_PRESSURE] = "Pressure",
	[ABS_DISTANCE] = "Distance",	[ABS_TILT_X] = "XTilt",
	[ABS_TILT_Y] = "YTilt",		[ABS_TOOL_WIDTH] = "Tool Width",
	[ABS_VOLUME] = "Volume",	[ABS_MISC] = "Misc",
};

char *misc[MSC_MAX + 1] = {
	[ 0 ... MSC_MAX] = NULL,
	[MSC_SERIAL] = "Serial",	[MSC_PULSELED] = "Pulseled",
	[MSC_GESTURE] = "Gesture",	[MSC_RAW] = "RawData",
	[MSC_SCAN] = "ScanCode",
};
char *keys[KEY_MAX + 1] = {
	[0 ... KEY_MAX] = NULL,
	[KEY_RESERVED] = "Reserved",		[KEY_ESC] = "Esc",
	[KEY_1] = "1",				[KEY_2] = "2",
	[KEY_3] = "3",				[KEY_4] = "4",
	[KEY_5] = "5",				[KEY_6] = "6",
	[KEY_7] = "7",				[KEY_8] = "8",
	[KEY_9] = "9",				[KEY_0] = "0",
	[KEY_MINUS] = "Minus",			[KEY_EQUAL] = "Equal",
	[KEY_BACKSPACE] = "Backspace",		[KEY_TAB] = "Tab",
	[KEY_Q] = "Q",				[KEY_W] = "W",
	[KEY_E] = "E",				[KEY_R] = "R",
	[KEY_T] = "T",				[KEY_Y] = "Y",
	[KEY_U] = "U",				[KEY_I] = "I",
	[KEY_O] = "O",				[KEY_P] = "P",
	[KEY_LEFTBRACE] = "LeftBrace",		[KEY_RIGHTBRACE] = "RightBrace",
	[KEY_ENTER] = "Enter",			[KEY_LEFTCTRL] = "LeftControl",
	[KEY_A] = "A",				[KEY_S] = "S",
	[KEY_D] = "D",				[KEY_F] = "F",
	[KEY_G] = "G",				[KEY_H] = "H",
	[KEY_J] = "J",				[KEY_K] = "K",
	[KEY_L] = "L",				[KEY_SEMICOLON] = "Semicolon",
	[KEY_APOSTROPHE] = "Apostrophe",	[KEY_GRAVE] = "Grave",
	[KEY_LEFTSHIFT] = "LeftShift",		[KEY_BACKSLASH] = "BackSlash",
	[KEY_Z] = "Z",				[KEY_X] = "X",
	[KEY_C] = "C",				[KEY_V] = "V",
	[KEY_B] = "B",				[KEY_N] = "N",
	[KEY_M] = "M",				[KEY_COMMA] = "Comma",
	[KEY_DOT] = "Dot",			[KEY_SLASH] = "Slash",
	[KEY_RIGHTSHIFT] = "RightShift",	[KEY_KPASTERISK] = "KPAsterisk",
	[KEY_LEFTALT] = "LeftAlt",		[KEY_SPACE] = "Space",
	[KEY_CAPSLOCK] = "CapsLock",		[KEY_F1] = "F1",
	[KEY_F2] = "F2",			[KEY_F3] = "F3",
	[KEY_F4] = "F4",			[KEY_F5] = "F5",
	[KEY_F6] = "F6",			[KEY_F7] = "F7",
	[KEY_F8] = "F8",			[KEY_F9] = "F9",
	[KEY_F10] = "F10",			[KEY_NUMLOCK] = "NumLock",
	[KEY_SCROLLLOCK] = "ScrollLock",	[KEY_KP7] = "KP7",
	[KEY_KP8] = "KP8",			[KEY_KP9] = "KP9",
	[KEY_KPMINUS] = "KPMinus",		[KEY_KP4] = "KP4",
	[KEY_KP5] = "KP5",			[KEY_KP6] = "KP6",
	[KEY_KPPLUS] = "KPPlus",		[KEY_KP1] = "KP1",
	[KEY_KP2] = "KP2",			[KEY_KP3] = "KP3",
	[KEY_KP0] = "KP0",			[KEY_KPDOT] = "KPDot",
	[KEY_ZENKAKUHANKAKU] = "Zenkaku/Hankaku", [KEY_102ND] = "102nd",
	[KEY_F11] = "F11",			[KEY_F12] = "F12",
	[KEY_RO] = "RO",			[KEY_KATAKANA] = "Katakana",
	[KEY_HIRAGANA] = "HIRAGANA",		[KEY_HENKAN] = "Henkan",
	[KEY_KATAKANAHIRAGANA] = "Katakana/Hiragana", [KEY_MUHENKAN] = "Muhenkan",
	[KEY_KPJPCOMMA] = "KPJpComma",		[KEY_KPENTER] = "KPEnter",
	[KEY_RIGHTCTRL] = "RightCtrl",		[KEY_KPSLASH] = "KPSlash",
	[KEY_SYSRQ] = "SysRq",			[KEY_RIGHTALT] = "RightAlt",
	[KEY_LINEFEED] = "LineFeed",		[KEY_HOME] = "Home",
	[KEY_UP] = "Up",			[KEY_PAGEUP] = "PageUp",
	[KEY_LEFT] = "Left",			[KEY_RIGHT] = "Right",
	[KEY_END] = "End",			[KEY_DOWN] = "Down",
	[KEY_PAGEDOWN] = "PageDown",		[KEY_INSERT] = "Insert",
	[KEY_DELETE] = "Delete",		[KEY_MACRO] = "Macro",
	[KEY_MUTE] = "Mute",			[KEY_VOLUMEDOWN] = "VolumeDown",
	[KEY_VOLUMEUP] = "VolumeUp",		[KEY_POWER] = "Power",
	[KEY_KPEQUAL] = "KPEqual",		[KEY_KPPLUSMINUS] = "KPPlusMinus",
	[KEY_PAUSE] = "Pause",			[KEY_KPCOMMA] = "KPComma",
	[KEY_HANGUEL] = "Hanguel",		[KEY_HANJA] = "Hanja",
	[KEY_YEN] = "Yen",			[KEY_LEFTMETA] = "LeftMeta",
	[KEY_RIGHTMETA] = "RightMeta",		[KEY_COMPOSE] = "Compose",
	[KEY_STOP] = "Stop",			[KEY_AGAIN] = "Again",
	[KEY_PROPS] = "Props",			[KEY_UNDO] = "Undo",
	[KEY_FRONT] = "Front",			[KEY_COPY] = "Copy",
	[KEY_OPEN] = "Open",			[KEY_PASTE] = "Paste",
	[KEY_FIND] = "Find",			[KEY_CUT] = "Cut",
	[KEY_HELP] = "Help",			[KEY_MENU] = "Menu",
	[KEY_CALC] = "Calc",			[KEY_SETUP] = "Setup",
	[KEY_SLEEP] = "Sleep",			[KEY_WAKEUP] = "WakeUp",
	[KEY_FILE] = "File",			[KEY_SENDFILE] = "SendFile",
	[KEY_DELETEFILE] = "DeleteFile",	[KEY_XFER] = "X-fer",
	[KEY_PROG1] = "Prog1",			[KEY_PROG2] = "Prog2",
	[KEY_WWW] = "WWW",			[KEY_MSDOS] = "MSDOS",
	[KEY_COFFEE] = "Coffee",		[KEY_DIRECTION] = "Direction",
	[KEY_CYCLEWINDOWS] = "CycleWindows",	[KEY_MAIL] = "Mail",
	[KEY_BOOKMARKS] = "Bookmarks",		[KEY_COMPUTER] = "Computer",
	[KEY_BACK] = "Back",			[KEY_FORWARD] = "Forward",
	[KEY_CLOSECD] = "CloseCD",		[KEY_EJECTCD] = "EjectCD",
	[KEY_EJECTCLOSECD] = "EjectCloseCD",	[KEY_NEXTSONG] = "NextSong",
	[KEY_PLAYPAUSE] = "PlayPause",		[KEY_PREVIOUSSONG] = "PreviousSong",
	[KEY_STOPCD] = "StopCD",		[KEY_RECORD] = "Record",
	[KEY_REWIND] = "Rewind",		[KEY_PHONE] = "Phone",
	[KEY_ISO] = "ISOKey",			[KEY_CONFIG] = "Config",
	[KEY_HOMEPAGE] = "HomePage",		[KEY_REFRESH] = "Refresh",
	[KEY_EXIT] = "Exit",			[KEY_MOVE] = "Move",
	[KEY_EDIT] = "Edit",			[KEY_SCROLLUP] = "ScrollUp",
	[KEY_SCROLLDOWN] = "ScrollDown",	[KEY_KPLEFTPAREN] = "KPLeftParenthesis",
	[KEY_KPRIGHTPAREN] = "KPRightParenthesis", [KEY_F13] = "F13",
	[KEY_F14] = "F14",			[KEY_F15] = "F15",
	[KEY_F16] = "F16",			[KEY_F17] = "F17",
	[KEY_F18] = "F18",			[KEY_F19] = "F19",
	[KEY_F20] = "F20",			[KEY_F21] = "F21",
	[KEY_F22] = "F22",			[KEY_F23] = "F23",
	[KEY_F24] = "F24",			[KEY_PLAYCD] = "PlayCD",
	[KEY_PAUSECD] = "PauseCD",		[KEY_PROG3] = "Prog3",
	[KEY_PROG4] = "Prog4",			[KEY_SUSPEND] = "Suspend",
	[KEY_CLOSE] = "Close",			[KEY_PLAY] = "Play",
	[KEY_FASTFORWARD] = "Fast Forward",	[KEY_BASSBOOST] = "Bass Boost",
	[KEY_PRINT] = "Print",			[KEY_HP] = "HP",
	[KEY_CAMERA] = "Camera",		[KEY_SOUND] = "Sound",
	[KEY_QUESTION] = "Question",		[KEY_EMAIL] = "Email",
	[KEY_CHAT] = "Chat",			[KEY_SEARCH] = "Search",
	[KEY_CONNECT] = "Connect",		[KEY_FINANCE] = "Finance",
	[KEY_SPORT] = "Sport",			[KEY_SHOP] = "Shop",
	[KEY_ALTERASE] = "Alternate Erase",	[KEY_CANCEL] = "Cancel",
	[KEY_BRIGHTNESSDOWN] = "Brightness down", [KEY_BRIGHTNESSUP] = "Brightness up",
	[KEY_MEDIA] = "Media",			[KEY_UNKNOWN] = "Unknown",
	[BTN_0] = "Btn0",			[BTN_1] = "Btn1",
	[BTN_2] = "Btn2",			[BTN_3] = "Btn3",
	[BTN_4] = "Btn4",			[BTN_5] = "Btn5",
	[BTN_6] = "Btn6",			[BTN_7] = "Btn7",
	[BTN_8] = "Btn8",			[BTN_9] = "Btn9",
	[BTN_LEFT] = "LeftBtn",			[BTN_RIGHT] = "RightBtn",
	[BTN_MIDDLE] = "MiddleBtn",		[BTN_SIDE] = "SideBtn",
	[BTN_EXTRA] = "ExtraBtn",		[BTN_FORWARD] = "ForwardBtn",
	[BTN_BACK] = "BackBtn",			[BTN_TASK] = "TaskBtn",
	[BTN_TRIGGER] = "Trigger",		[BTN_THUMB] = "ThumbBtn",
	[BTN_THUMB2] = "ThumbBtn2",		[BTN_TOP] = "TopBtn",
	[BTN_TOP2] = "TopBtn2",			[BTN_PINKIE] = "PinkieBtn",
	[BTN_BASE] = "BaseBtn",			[BTN_BASE2] = "BaseBtn2",
	[BTN_BASE3] = "BaseBtn3",		[BTN_BASE4] = "BaseBtn4",
	[BTN_BASE5] = "BaseBtn5",		[BTN_BASE6] = "BaseBtn6",
	[BTN_DEAD] = "BtnDead",			[BTN_A] = "BtnA",
	[BTN_B] = "BtnB",			[BTN_C] = "BtnC",
	[BTN_X] = "BtnX",			[BTN_Y] = "BtnY",
	[BTN_Z] = "BtnZ",			[BTN_TL] = "BtnTL",
	[BTN_TR] = "BtnTR",			[BTN_TL2] = "BtnTL2",
	[BTN_TR2] = "BtnTR2",			[BTN_SELECT] = "BtnSelect",
	[BTN_START] = "BtnStart",		[BTN_MODE] = "BtnMode",
	[BTN_THUMBL] = "BtnThumbL",		[BTN_THUMBR] = "BtnThumbR",
	[BTN_TOOL_PEN] = "ToolPen",		[BTN_TOOL_RUBBER] = "ToolRubber",
	[BTN_TOOL_BRUSH] = "ToolBrush",		[BTN_TOOL_PENCIL] = "ToolPencil",
	[BTN_TOOL_AIRBRUSH] = "ToolAirbrush",	[BTN_TOOL_FINGER] = "ToolFinger",
	[BTN_TOOL_MOUSE] = "ToolMouse",		[BTN_TOOL_LENS] = "ToolLens",
	[BTN_TOUCH] = "Touch",			[BTN_STYLUS] = "Stylus",
	[BTN_STYLUS2] = "Stylus2",		[BTN_TOOL_DOUBLETAP] = "Tool Doubletap",
	[BTN_TOOL_TRIPLETAP] = "Tool Tripletap", [BTN_GEAR_DOWN] = "WheelBtn",
	[BTN_GEAR_UP] = "Gear up",		[KEY_OK] = "Ok",
	[KEY_SELECT] = "Select",		[KEY_GOTO] = "Goto",
	[KEY_CLEAR] = "Clear",			[KEY_POWER2] = "Power2",
	[KEY_OPTION] = "Option",		[KEY_INFO] = "Info",
	[KEY_TIME] = "Time",			[KEY_VENDOR] = "Vendor",
	[KEY_ARCHIVE] = "Archive",		[KEY_PROGRAM] = "Program",
	[KEY_CHANNEL] = "Channel",		[KEY_FAVORITES] = "Favorites",
	[KEY_EPG] = "EPG",			[KEY_PVR] = "PVR",
	[KEY_MHP] = "MHP",			[KEY_LANGUAGE] = "Language",
	[KEY_TITLE] = "Title",			[KEY_SUBTITLE] = "Subtitle",
	[KEY_ANGLE] = "Angle",			[KEY_ZOOM] = "Zoom",
	[KEY_MODE] = "Mode",			[KEY_KEYBOARD] = "Keyboard",
	[KEY_SCREEN] = "Screen",		[KEY_PC] = "PC",
	[KEY_TV] = "TV",			[KEY_TV2] = "TV2",
	[KEY_VCR] = "VCR",			[KEY_VCR2] = "VCR2",
	[KEY_SAT] = "Sat",			[KEY_SAT2] = "Sat2",
	[KEY_CD] = "CD",			[KEY_TAPE] = "Tape",
	[KEY_RADIO] = "Radio",			[KEY_TUNER] = "Tuner",
	[KEY_PLAYER] = "Player",		[KEY_TEXT] = "Text",
	[KEY_DVD] = "DVD",			[KEY_AUX] = "Aux",
	[KEY_MP3] = "MP3",			[KEY_AUDIO] = "Audio",
	[KEY_VIDEO] = "Video",			[KEY_DIRECTORY] = "Directory",
	[KEY_LIST] = "List",			[KEY_MEMO] = "Memo",
	[KEY_CALENDAR] = "Calendar",		[KEY_RED] = "Red",
	[KEY_GREEN] = "Green",			[KEY_YELLOW] = "Yellow",
	[KEY_BLUE] = "Blue",			[KEY_CHANNELUP] = "ChannelUp",
	[KEY_CHANNELDOWN] = "ChannelDown",	[KEY_FIRST] = "First",
	[KEY_LAST] = "Last",			[KEY_AB] = "AB",
	[KEY_NEXT] = "Next",			[KEY_RESTART] = "Restart",
	[KEY_SLOW] = "Slow",			[KEY_SHUFFLE] = "Shuffle",
	[KEY_BREAK] = "Break",			[KEY_PREVIOUS] = "Previous",
	[KEY_DIGITS] = "Digits",		[KEY_TEEN] = "TEEN",
	[KEY_TWEN] = "TWEN",			[KEY_DEL_EOL] = "Delete EOL",
	[KEY_DEL_EOS] = "Delete EOS",		[KEY_INS_LINE] = "Insert line",
	[KEY_DEL_LINE] = "Delete line",
};

char *absval[5] = { "Value", "Min  ", "Max  ", "Fuzz ", "Flat " };

char *relatives[REL_MAX + 1] = {
	[0 ... REL_MAX] = NULL,
	[REL_X] = "X",			[REL_Y] = "Y",
	[REL_Z] = "Z",			[REL_HWHEEL] = "HWheel",
	[REL_DIAL] = "Dial",		[REL_WHEEL] = "Wheel", 
	[REL_MISC] = "Misc",	
};

char **names[EV_MAX + 1] = {
	[0 ... EV_MAX] = NULL,
	[EV_SYN] = events,			[EV_KEY] = keys,
	[EV_REL] = relatives,			[EV_ABS] = absolutes,
	[EV_MSC] = misc,			[EV_LED] = leds,
	[EV_SND] = sounds,			[EV_REP] = repeats,
};

void getopts(int argc, char **argv);
void myconnect(MQTTAsync * client);

void usage(void)
{
	printf("MQTT stdin publisher\n");
	printf("Usage: stdinpub topicname <options>, where options are:\n");
	printf("    --host <hostname> (default is %s)\n", opts.host);
	printf("    --port <port> (default is %s)\n", opts.port);
	printf("    --qos <qos> (default is %d)\n", opts.qos);
	printf("    --retained (default is %s)\n",
	       opts.retained ? "on" : "off");
	printf("    --delimiter (default is \\n)\n");
	printf("    --clientid <clientid> (defaults is %s)\n", opts.clientid);
	printf("    --maxdatalen <bytes> (default is %d)\n", opts.maxdatalen);
	printf("    --username none\n");
	printf("    --password none\n");
	printf("    --keepalive <seconds> (default is 10 seconds)\n");

	exit(EXIT_FAILURE);
}

volatile int toStop = 0;	/* why is this volatile and not static ? */

/* this is a signal handler */
void cfinish(int sig)
{
	sig = sig;
	/* you want to send a signal 
	   from a signal handler 
	   which the handler is subscribed to
	   endless loop? */
	signal(SIGINT, NULL);	/* what is this? */

	toStop = 1;		/* this is not necessarily atomic */
}

static int connected = 0;	/* that's a shared resource */
static int disconnected = 0;	/* that's a shared resource */

void onConnectFailure(void *context, MQTTAsync_failureData * response)
{
	MQTTAsync client;
	printf("Connect failed, rc is %d\n", response ? response->code : -1);	/* don't use ? - unreadable code */
	connected = -1;		/* not necessarily atomic */

	client = (MQTTAsync) context;
	myconnect(client);
}

void onConnect(void *context, MQTTAsync_successData * response)
{
	context = context;
	response = response;
	printf("Connected");
	connected = 1;		/* not necessarily atomic */
}

void connectionLost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync) context;
	MQTTAsync_connectOptions conn_opts =
	    MQTTAsync_connectOptions_initializer;
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	int rc = 0;
	ssl_opts = ssl_opts;

	cause = cause;

	printf("Connecting\n");
	conn_opts.keepAliveInterval = 10;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;

	ssl_opts.enableServerCertAuth = 0;
	/*conn_opts.ssl_opts = &ssl_opts; */

	connected = 0;

	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start connect, return code is %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

void myconnect(MQTTAsync * client)
{
	MQTTAsync_connectOptions conn_opts =
	    MQTTAsync_connectOptions_initializer;
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	int rc;
	ssl_opts = ssl_opts;
	rc = 0;

	printf("Connecting\n");
	conn_opts.keepAliveInterval = opts.keepalive;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;

	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;

	conn_opts.context = client;

	ssl_opts.enableServerCertAuth = 0;
	conn_opts.automaticReconnect = 1;

	connected = 0;

	if ((rc = MQTTAsync_connect(*client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

int messageArrived(void *context, char *topicName, int topicLen,
		   MQTTAsync_message * m)
{
	context = context;
	topicName = topicName;
	topicLen = topicLen;
	m = m;
	/* Not expecting any messages */
	return 1;
}

void onDisconnect(void *context, MQTTAsync_successData * response)
{
	context = context;
	response = response;
	disconnected = 1;
}

static int published = 0;

void onPublish(void *context, MQTTAsync_successData * response)
{
	context = context;
	response = response;
	published = 1;
}

void onPublishFailure(void *context, MQTTAsync_failureData * response)
{
	context = context;
	response = response;
	printf("Published failed, return code is %d\n",
	       response ? response->code : -1);
	published = -1;
}

/* 
 * 
 * 
 */
void 
tryToConnect( char *url ) {
	int rc = 0;

	MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;

	create_opts.sendWhileDisconnected = 1;
	
	rc = MQTTAsync_createWithOptions(&client, url, opts.clientid,
					 MQTTCLIENT_PERSISTENCE_NONE, NULL,
					 &create_opts);

	/* what should be happening with rc? 
	 * Error handling?
	 * since we are no in the loop here, maybe loop and retry? 
	 * (performance) Variable 'rc' is reassigned a value before the old one has been used. 
	 */

	/* I guess here you want to connect SIGINT 
	 * and SIGTERM with the cfinish signal handler
	 */

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	rc = MQTTAsync_setCallbacks(client, client, connectionLost,
				    messageArrived, NULL);
}

void 
tryToDisconnect() {
	int rc = 0;
	
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

	disc_opts.onSuccess = onDisconnect;


	if ((rc =
	     MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start disconnect, return code is %d\n", rc);
		exit(EXIT_FAILURE);
	}

	while (!disconnected) {
		/* usleep(10000L); *//* POSIX.1-2008 removes the specification of usleep(). */
		sleep(1);
	}

	MQTTAsync_destroy(&client);
}

void 
tryToSend( const char *topic, int data_len, const char *buffer) 
{
	int rc = 0;

	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	
	pub_opts.onSuccess = onPublish;
		pub_opts.onFailure = onPublishFailure;

		do {
			rc = MQTTAsync_send(client, topic, data_len, buffer,
					    opts.qos, opts.retained, &pub_opts);
		} while (rc != MQTTASYNC_SUCCESS);
}

/*
 * Main() 
 */
int main(int argc, char **argv)
{
	char url[100];
	char *topic = NULL;
	int rc = 0;
	char *buffer = NULL;

	char buffer2[] = "Button Pressed\n";
	int delim_len;

	/* The number of bytes read */
	ssize_t rd;

	/* identifier for keystrokes info */
	struct input_event ev[64];

	int i;

	/* select() system call configuration arguments 
	 * ******************************************** */
	struct timeval *pto;

	/* select() ... returns number of ready file descriptors
	 *  0    on timeout
	 * -1   on failure
	 */
	int ready;

	/* File descriptor sets
	 * readfsd      .. set of file descriptors / if INPUT is possible 
	 * writefds     .. set of file descriptors / if OUTPUT is possible
	 * exceptfds    .. set of file descriptors / if EXCEPTIONAL condition occurred
	 */
	fd_set readfds;

	/* nfds .. should be set to the highest-number file descriptor 
	 *         in any of the three-sets plus(+) 1
	 */
	int nfds;

	/* An abstract indicator used to access an input/output resource */
	int fd;

	/* ************************* SYSLOG Setup **************************  */
	/* *****************************************************************  */
	/* Open a connection to system logger for Button-Handler */ 
	setlogmask( LOG_UPTO( LOG_LEVEL) );
	/* Write to system console if error sending to system logger ... LOG_CONS
	 * Open connection immediately not when first message logged ... LOG_NDELAY
	 * Include caller's PID with each message ... LOG_PID
	 * 
	 * openlog may or may not open the /dev/log socket, depending on option. If it does, it tries to open it and connect it as a stream socket.
	 */  
	openlog ( "button-handler", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	//const char *format = "%syslogpriority%,%syslogfacility%,%timegenerated%,%HOSTNAME%,%syslogtag%,%msg%\n";
	syslog (LOG_DEBUG, "Button Handler is Launching");

	/* First Check ... Necessary Input Parameters Otherwise Handler Cannot Operate */
	if (argc < 2) {
		usage();
	}	

	/* Enough Command Promt Arguments ... Start Parsing Read & Assign to Application Variables */
	/* IMPORTANT MUST CHANGE ... Use GetOpt() instead */
	getopts(argc, argv);

	/* Local Broker URL */
	sprintf(url, "%s:%s", opts.host, opts.port);

	if (opts.verbose) {
		//printf("Broker URL is %s\n", url);
		syslog (LOG_DEBUG, "MQTT Broker URL : `%s`\n", url);
	}

	topic = argv[1];
	printf("Using Topic %s\n", topic);

	/* Is there a specific reason those are not global variables ? */
	/* as global vars we could move MQTT malas in specific functions */

	/* */
	tryToConnect( url );

	/* also this could go into some different function out of main */

	/* what should be happening with rc? 
	 * Error handling?
	 * since we are no in the loop here, maybe loop and retry? 
	 * (performance) Variable 'rc' is reassigned a value before the old one has been used. 
	 */

	myconnect(&client);

	/* after connect MQTT is active?
	 * I guess like this the malloc should be before connect 
	 */

	/* where does opts.maxdatalen come from?
	 * commandline ? 
	 */

	buffer = malloc(opts.maxdatalen);

	/* *** SELECT() system call configuration *** */
	/* ****************************************** */

	/* Timeout for select() */
	pto = NULL;		/* Infinite timeout */

	/* Highest numbered file descriptor */
	nfds = 0;
	FD_ZERO(&readfds);

	/* Open Device File */
	if ((fd = open(DEVICE_PATH, O_RDONLY)) < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	FD_SET(fd, &readfds);

	if (fd > nfds) {
		nfds = fd + 1;
	}

	/* Evtest Code Snippet .. */
	while (1) {
		/* Button Process Wait Until New Keystroke */
        rd = read( fd, ev, sizeof(struct input_event)*64 );

        if ( rd < (int) sizeof(struct input_event) ) {
            //perror("read");
			//printf("Read() Error Value is : %s\n", strerror(errorno));

            return 1;
        }

        for (i=0; i<rd/sizeof(struct input_event); i++) {
            if (ev[i].type == EV_SYN) {
                printf("Event: time %ld.%06ld, -------- %s --------\n",
                    ev[i].time.tv_sec, ev[i].time.tv_usec, 
                    ev[i].code ? "Config Sync" : "Report Sync");
            } else if (ev[i].type == EV_MSC && (ev[i].code == MSC_RAW || ev[i].code == MSC_SCAN)) {
                printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %02x\n",
                    ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                    events[ev[i].type] ? events[ev[i].type] : "?",
                    ev[i].code,
                    names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
                    ev[i].value);
            } else {
                printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
                    ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                    events[ev[i].type] ? events[ev[i].type] : "?",
                    ev[i].code,
                    names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
                    ev[i].value);                    
            }
        }
    }
	while (!toStop) {
		int data_len = 0;

		/* 
		 * I/O Multiplexing .. 
		 * Simultaneous monitor many file descriptors and see if 
		 * read/ write is possible on any of them. 
		 */

		/* ************ End of select() system call **********
		 * ******* to monitor multiple file descriptors ****** 
		 */

		/* here should be pselect instead of select */
		/* something like:
		   sigaddset(&ss, SIGWHATEVER);
		   ready = pselect(nfds, &readfds, NULL, NULL, pto, &ss);
		 */
		ready = select(nfds, &readfds, NULL, NULL, pto);

		if (ready == -1) {
			/* An error occured */
			perror("select");
			exit(EXIT_FAILURE);
		}
		/* else */
		if (ready == 0) {
			/* Call was timed out */
			;
		}

		/*else if (ready == 1) {
		   // The device file descriptor block released
		   } */

		/* Ready for Select() Wakeup ... Device File is changed */
		/* char buffer2[] = "Button Pressed\n"; - moved further up */

		/* Read message-to-send from terminal */
		/* int delim_len = 0; *//* why???? */

		delim_len = (int)strlen(opts.delimiter);

		do {
			buffer[data_len] = buffer2[data_len];
			data_len++;

			if (data_len > delim_len) {
				if (strncmp
				    (opts.delimiter,
				     &buffer[data_len - delim_len],
				     delim_len) == 0) {
					break;
				}
			}
		} while (data_len < opts.maxdatalen);

		if (opts.verbose) {
			printf("Publishing data of length %d\n", data_len);
		}

		tryToSend( topic, data_len, buffer );
	}	

	printf("Stopping\n");

	free(buffer);

	tryToDisconnect();

	return EXIT_SUCCESS;
}

/*
 * End of Main()
 */

void getopts(int argc, char **argv)
{
	int count = 2;

	while (count < argc) {
		if (strcmp(argv[count], "--retained") == 0) {
			opts.retained = 1;
		}

		if (strcmp(argv[count], "--verbose") == 0) {
			opts.verbose = 1;
		}

		else if (strcmp(argv[count], "--qos") == 0) {
			if (++count < argc) {
				if (strcmp(argv[count], "0") == 0) {
					opts.qos = 0;
				} else if (strcmp(argv[count], "1") == 0) {
					opts.qos = 1;
				} else if (strcmp(argv[count], "2") == 0) {
					opts.qos = 2;
				} else {
					usage();
				}
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--host") == 0) {
			if (++count < argc) {
				opts.host = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--port") == 0) {
			if (++count < argc) {
				opts.port = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--clientid") == 0) {
			if (++count < argc) {
				opts.clientid = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--username") == 0) {
			if (++count < argc) {
				opts.username = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--password") == 0) {
			if (++count < argc) {
				opts.password = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--maxdatalen") == 0) {
			if (++count < argc) {
				opts.maxdatalen = atoi(argv[count]);
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--delimiter") == 0) {
			if (++count < argc) {
				opts.delimiter = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--keepalive") == 0) {
			if (++count < argc) {
				opts.keepalive = atoi(argv[count]);
			} else {
				usage();
			}
		}

		count++;

	}
}
