#include "module.h"


#define operchan "#opers"

#define AUTHOR "Examknow"
#define VERSION "0.1"

int do_staffcall(int argc, char **argv);


int AnopeInit(int argc, char **argv)
{
    EvtMessage *msg = NULL;
    EvtHook    *hook = NULL;
    int status;
    hook = createEventHook(EVENT_BOT_FANTASY, do_staffcall);
    status = moduleAddEventHook(hook);
    alog("bs_staffcall: Module loaded.");
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void)
{
	alog("bs_staffcall: Module unloaded.");
}



int do_staffcall(int argc, char **argv) {
    ChannelInfo *ci;
    if(argc>=3) { 
		if(stricmp(argv[0],"staff")==0) { 
            		if((ci = cs_findchan(argv[2]))) {

				alog("!staff activated and opers notified.");
                		anope_cmd_privmsg(ci->bi->nick, ci->name, "Staff have been notified of your request for help. Thank you for choosing TechNet.",argv[1]);
				anope_cmd_privmsg(s_OperServ, operchan, "%s wants staff attention in %s", argv[1], ci->name)
				return MOD_STOP; 

            		}
		} 
    }
    return MOD_CONT; 
}
