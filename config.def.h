/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* My Smart View */
#define SMART_TAGVIEW 1
#define REMEMBER_FOCUS 1
#define SHOW_WINDOW_TITLE 0

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 0;    /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int focusonwheel       = 0;
static const float toggle_float_ratio = 0.5;   /* weight and height ratio of toggle floating client */
static const char font[]            = "Hack Nerd Font Mono 14";
static const char dmenufont[]       = "Hack Nerd Font Mono:size=14";
static const char col_gray1[]       = "#2e3440"; // UnFocus bg
static const char col_gray2[]       = "#2e3440"; // UnFocus border
static const char col_gray3[]       = "#eceff4"; // UnFocus fg
static const char col_gray4[]       = "#eceff4"; // Focus fg
static const char col_cyan[]        = "#5e81ac"; // Focus bg, border

static const char col_norm_fg[]      = "#eceff4";
static const char col_norm_bg[]      = "#2e3440";
static const char col_norm_bd[]      = "#2e3440";
static const char col_sel_fg[]       = "#eceff4";
static const char col_sel_bg[]       = "#5e81ac";
static const char col_sel_bd[]       = "#eceff4";
static const char col_occ_fg[]       = "#eceff4";
static const char col_occ_bg[]       = "#4c566a";
static const char col_occ_bd[]       = "#eceff4";
static const char col_title_fg[]     = "#eceff4";
static const char col_title_bg[]     = "#2e3440";
static const char col_title_bd[]     = "#eceff4";
static const char col_systray_fg[]   = "#eceff4";
static const char col_systray_bg[]   = "#5e81ac";
static const char col_systray_bd[]   = "#eceff4";
static const char col_status_fg[]    = "#eceff4";
static const char col_status_bg[]    = "#2e3440";
static const char col_status_bd[]    = "#eceff4";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_norm_fg, col_norm_bg, col_norm_bd },
	[SchemeSel]  = { col_sel_fg, col_sel_bg, col_sel_bd },
  [SchemeTitle]    = { col_title_fg, col_title_bg, col_title_bd }, // client title
  [SchemeSystray]  = { col_systray_fg, col_systray_bg, col_systray_bd }, // systray
  [SchemeStatus]   = { col_status_fg, col_status_bg, col_status_bd }, // status
  [SchemeOcc]  = { col_occ_fg, col_occ_bg, col_occ_bd },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "wezterm", NULL };
static const char *menucmd[] = { "rofi", "-i", "-show", "drun", NULL };
static const char *runmenucmd[] = { "rofi", "-show", "run", NULL };
static const char *browsercmd[] = { "zen-browser", NULL };
static const char *lockcmd[] = { "x-lock", NULL };
static const char *clipboardcmd[] = { "greenclip-rofi", NULL };
static const char *powermenucmd[] = { "rofi-power-menu", NULL };
static const char *screenshotcmd[] = { "maim-screenshot", "full", NULL };
static const char *screenshotselectcmd[] = { "maim-screenshot", "select", NULL };
static const char *volumemutecmd[] = { "my-volume", "mute", NULL };
static const char *volumedowncmd[] = { "my-volume", "down", NULL };
static const char *volumeupcmd[] = { "my-volume", "up", NULL };
static const char *brightnessupcmd[] = { "brightnessctl", "set", "10%+", NULL };
static const char *brightnessdowncmd[] = { "brightnessctl", "set", "10%-", NULL };
static const char *playerctlplaypausecmd[] = { "playerctl", "play-pause", NULL };
static const char *playerctlnextcmd[] = { "playerctl", "next", NULL };
static const char *playerctlprevcmd[] = { "playerctl", "previous", NULL };

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_d,      spawn,          {.v = menucmd } },
	{ MODKEY,                       XK_r,      spawn,          {.v = runmenucmd} },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_f,      spawn,          {.v = browsercmd} },
	{ MODKEY|ShiftMask,             XK_l,      spawn,          {.v = lockcmd} },
	{ MODKEY,                       XK_c,      spawn,          {.v = clipboardcmd} },
	{ MODKEY|ShiftMask,             XK_p,      spawn,          {.v = powermenucmd} },
  { 0,                            XK_Print,  spawn,          {.v = screenshotcmd} },
  { MODKEY,                       XK_Print,  spawn,          {.v = screenshotselectcmd} },
	{ 0,                            XF86XK_AudioRaiseVolume, spawn, {.v = volumeupcmd} },
	{ 0,                            XF86XK_AudioLowerVolume, spawn, {.v = volumedowncmd} },
	{ 0,                            XF86XK_AudioMute, spawn,       {.v = volumemutecmd} },
	{ 0,                            XF86XK_MonBrightnessUp, spawn, {.v = brightnessupcmd} },
	{ 0,                            XF86XK_MonBrightnessDown, spawn, {.v = brightnessdowncmd} },
	{ 0,                            XF86XK_AudioPlay,       spawn, {.v = playerctlplaypausecmd} },
	{ 0,                            XF86XK_AudioNext,       spawn, {.v = playerctlnextcmd} },
	{ 0,                            XF86XK_AudioPrev,       spawn, {.v = playerctlprevcmd} },
	// { MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	// { MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	// { MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	// { MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_e,      setlayout,      {.v = &layouts[0]} },
	// { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_f,      togglefullscr,  {0} },
	// { MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY,                       XK_Right,  viewnext,       {0} },
	{ MODKEY,                       XK_Left,   viewprev,       {0} },
	// { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	// { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	// { MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	// { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	// { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_e,      quit,           {0} },
	{ MODKEY|ShiftMask,             XK_s,      togglesticky,   {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	// { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	// { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	// { ClkWinTitle,          0,              Button2,        zoom,           {0} },
	// { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY|ShiftMask, Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY|ShiftMask, Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY|ShiftMask, Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	// { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	// { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

static const char *ipcsockpath = "/tmp/dwm.sock";
static IPCCommand ipccommands[] = {
  IPCCOMMAND(  view,                1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggleview,          1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tag,                 1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggletag,           1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tagmon,              1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  focusmon,            1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  focusstack,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  zoom,                1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  incnmaster,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  killclient,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  togglefloating,      1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  setmfact,            1,      {ARG_TYPE_FLOAT}  ),
  IPCCOMMAND(  setlayoutsafe,       1,      {ARG_TYPE_PTR}    ),
  IPCCOMMAND(  quit,                1,      {ARG_TYPE_NONE}   )
};

