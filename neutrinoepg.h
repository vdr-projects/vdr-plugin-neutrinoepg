/*
 * See the README file for copyright information and how to reach the author.
 */

#ifndef NEUTRINOEPG_H_
#define NEUTRINOEPG_H_

#include <vdr/plugin.h>
#include <vdr/menu.h>
#include <vdr/status.h>
#include <vdr/interface.h>

#include "osdmenu.h"

#define CHNUMWIDTH ( numdigits(Channels.MaxNumber()) )

extern int hidemainmenu;

extern int bookmark;
extern int Step;
extern int keeposd;
extern int hideencryptedchannels;
extern int showchannelnumbers;
extern int hideradiochannels;
extern int switchwithok;
extern int progressbar;
extern int middlemenuentry;
extern int switchgroupkey;
extern int ChannelNameWidth;
extern int HideGroupsAt;

static const char *VERSION        = "0.3.6";
static const char *DESCRIPTION    = "View the EPG like neutrino does";
static const char *MAINMENUENTRY  = "Neutrino EPG";

// --- cPluginNeutrinoEpg ---------------------------------------------------
class cPluginNeutrinoEpg:public cPlugin
{
    private:
    
    public:
        cPluginNeutrinoEpg(void);
        virtual ~cPluginNeutrinoEpg();
        virtual const char *Version(void) { return VERSION; }
        virtual const char *Description(void) { return tr(DESCRIPTION); }
        virtual const char *CommandLineHelp(void);
        virtual bool ProcessArgs(int argc, char *argv[]);
        virtual bool Initialize(void);
        virtual bool Start(void);
        virtual void Stop(void);
        virtual void Housekeeping(void);
        virtual const char *MainMenuEntry(void) {return hidemainmenu?NULL:tr(MAINMENUENTRY);}
        virtual cOsdObject *MainMenuAction(void);
        virtual cMenuSetupPage *SetupMenu(void);
        virtual bool SetupParse(const char *Name, const char *Value);
        virtual bool Service(const char *Id, void *Data = NULL){return false;};
        virtual const char **SVDRPHelpPages(void){return NULL;};
        virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode){return NULL;};
};

#endif
