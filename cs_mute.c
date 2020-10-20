#include "module.h"

#define AUTHOR "David Schultz (Examknow) based on version from Swampy"
#define VERSION "v0.0"

void myHelp(User * u);

int checkircdversion(void);
int myFullHelpMute(User * u);
int myFullHelpUnmute(User * u);
int do_csmute(User * u);
int do_csunmute(User * u);
void addBan(Channel * c, time_t timeout, char *banmask);
void delBan(Channel * c, time_t timeout, char *banmask);

int canBanUser(Channel * c, User * u, User * u2);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    int status = 0;
	
	if (!checkircdversion()) {
		alog("[cs_mute] ERROR: Only UnrealIRCd 3.2 and InspIRCd 1.2/2.0 are supported by this module, Unloading.");
		return MOD_STOP;
	} else {
		alog("[cs_mute] Your IRCd is supported (%s), Enabling Commands.", IRCDModule);
	}


    moduleSetChanHelp(myHelp);
    c = createCommand("MUTE", do_csmute, NULL, -1, -1, -1, -1, -1);
    moduleAddHelp(c, myFullHelpMute);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	
    c = createCommand("UNMUTE", do_csunmute, NULL, -1, -1, -1, -1, -1);
    moduleAddHelp(c, myFullHelpUnmute);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    if (status != MOD_ERR_OK) {
        return MOD_STOP;
    }
	alog("[cs_mute] Commands Enabled, Type /msg %s (UN)MUTE HELP for Usage", s_ChanServ);
    return MOD_CONT;
}

void AnopeFini(void)
{
    alog("[cs_mute] Unloading cs_mute..");
}

void myHelp(User * u)
{
    notice(s_ChanServ, u->nick, "    MUTE       Mute specified nickname on channel.");
	notice(s_ChanServ, u->nick, "    UNMUTE     Unmute specified Nickname on Channel.");
}

int myFullHelpMute(User * u)
{
    notice(s_ChanServ, u->nick, "Syntax: MUTE [channel] [nickname]");
	  notice(s_ChanServ, u->nick, "Mute specified nick on channel.");
    return MOD_CONT;
}
int myFullHelpUnmute(User * u)
{
    notice(s_ChanServ, u->nick, "Syntax: UNMUTE [channel] [nickname]");
    notice(s_ChanServ, u->nick, "Unmute specified nick on channel.");
    return MOD_CONT;
}

int do_csmute(User * u)
{
    char mask[BUFSIZE];
    Channel *c;
    User *u2 = NULL;

    char *buffer = moduleGetLastBuffer();
    char *chan;
    char *nick;
    char *time;

    chan = myStrGetToken(buffer, ' ', 0);
    nick = myStrGetToken(buffer, ' ', 1);
    time = myStrGetToken(buffer, ' ', 2);

    if (chan && nick) {

        if (!(c = findchan(chan))) {
            notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
        } else if (!(u2 = finduser(nick))) {
            notice_lang(s_ChanServ, u, NICK_X_NOT_IN_USE, nick);
        } else {
            if (canBanUser(c, u, u2)) {
				get_idealban(c->ci, u2, mask, sizeof(mask));
				alog("%s: Mute: %s!%s@%s on %s (by %s!%s@%s)", s_ChanServ, u2->nick, u2->vident, u2->host, chan, u->nick, u->vident, u->host);
                addBan(c, dotime(time), mask);
				
				if (!stricmp(IRCDModule, "unreal32"))
					anope_cmd_mode(whosends(c->ci), c->name, "-v %s", u2->nick);
			}
        }
    } else {
        notice(s_ChanServ, u->nick, "Syntax: MUTE [channel] [nickname]");
    }
    if (time)
        free(time);
    if (nick)
        free(nick);
    if (chan)
        free(chan);

    return MOD_CONT;
}

int do_csunmute(User * u)
{
    char mask[BUFSIZE];
    Channel *c;
    User *u2 = NULL;

    char *buffer = moduleGetLastBuffer();
    char *chan;
    char *nick;
    char *time;

    chan = myStrGetToken(buffer, ' ', 0);
    nick = myStrGetToken(buffer, ' ', 1);
    time = myStrGetToken(buffer, ' ', 2);

    if (chan && nick) {

        if (!(c = findchan(chan))) {
            notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
        } else if (!(u2 = finduser(nick))) {
            notice_lang(s_ChanServ, u, NICK_X_NOT_IN_USE, nick);
        } else {
            if (canBanUser(c, u, u2)) {
				get_idealban(c->ci, u2, mask, sizeof(mask));
				alog("%s: Unmute: %s!%s@%s on %s (by %s!%s@%s)", s_ChanServ, u2->nick, u2->vident, u2->host, chan, u->nick, u->vident, u->host);
                delBan(c, dotime(time), mask);
            }
        }
    } else {
        notice(s_ChanServ, u->nick, "Syntax: UNMUTE [channel] [nickname]");
    }
    if (time)
        free(time);
    if (nick)
        free(nick);
    if (chan)
        free(chan);

    return MOD_CONT;
}

void addBan(Channel * c, time_t timeout, char *banmask)
{
    char *av[3];
    char *cb[2];

    cb[0] = c->name;
    cb[1] = banmask;

    av[0] = sstrdup("+b");
    av[1] = banmask;

	if (!stricmp(IRCDModule, "unreal32"))
		anope_cmd_mode(whosends(c->ci), c->name, "+b ~q:%s", av[1]);
	
	if (!stricmp(IRCDModule, "inspircd12") || !stricmp(IRCDModule, "inspircd20") )
		anope_cmd_mode(whosends(c->ci), c->name, "+b m:%s", av[1]);
	
    chan_set_modes(s_ChanServ, c, 2, av, 1);

    free(av[0]);
}

void delBan(Channel * c, time_t timeout, char *banmask)
{
    char *av[3];
    char *cb[2];

    cb[0] = c->name;
    cb[1] = banmask;

    av[0] = sstrdup("-b");
    av[1] = banmask;

	if (!stricmp(IRCDModule, "unreal32"))
		anope_cmd_mode(whosends(c->ci), c->name, "-b ~q:%s", av[1]);

	if (!stricmp(IRCDModule, "inspircd12") ||  !stricmp(IRCDModule, "inspircd20") )
		anope_cmd_mode(whosends(c->ci), c->name, "-b m:%s", av[1]);

	chan_set_modes(s_ChanServ, c, 2, av, 1);

    free(av[0]);
}

int canBanUser(Channel * c, User * u, User * u2)
{
    ChannelInfo *ci;
    int ok = 0;
    if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, c->name);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, c->name);
    } else if (!check_access(u, ci, CA_BAN)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if (ircd->except && is_excepted(ci, u2)) {
        notice_lang(s_ChanServ, u, CHAN_EXCEPTED, u2->nick, ci->name);
    } else if (ircd->protectedumode && is_protected(u2)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
    } else {
        ok = 1;
    }

    return ok;
}

int checkircdversion(void) {
	if (!stricmp(IRCDModule, "unreal32"))
		return 1;

	if (!stricmp(IRCDModule, "inspircd12") || !stricmp(IRCDModule, "inspircd20"))
		return 1;

	return 0;
}


/* EOF */
