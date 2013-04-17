/*
 * See the README file for copyright information and how to reach the author.
 */

#include "neutrinoepg.h"
#include "menuevent.h"

// setup values
int bookmark = 2015;
int Step = 30;
int keeposd = 0;
int showchannelnumbers = 1;
int hideencryptedchannels = 0;
int hideradiochannels = 0;
int switchwithok = 1;
int hidemainmenu = false;
int percentprogress = false;
int middlemenuentry = false;
int switchgroupkey = 0;
int ChannelNameWidth = 15;
int HideGroupsAt = 0;

bool ReloadFilters = false;
// --- myMenuSetup ------------------------------------------------------------
class myMenuSetup : public cMenuSetupPage
{
    private:
        const char *SwitchGroupKeyTexts[2];
        const char *SitchWithOKTexts[2];
    protected:
        virtual void Store()
        {
            SetupStore("hidemainmenu", hidemainmenu);
            SetupStore("bookmark", bookmark);
            SetupStore("step", Step);
            SetupStore("keeposd", keeposd);
            SetupStore("hideencryptedchannels", hideencryptedchannels);
            SetupStore("hideradiochannels", hideradiochannels);
            SetupStore("showchannelnumbers", showchannelnumbers);
            SetupStore("switchwithok", switchwithok);
            SetupStore("percentprogress", percentprogress);
            SetupStore("middlemenuentry", middlemenuentry);
            SetupStore("switchgroupkey", switchgroupkey);
            SetupStore("ChannelNameWidth", ChannelNameWidth);
            SetupStore("HideGroupsAt", HideGroupsAt);

            ReloadFilters = true;
        }
    public:
        myMenuSetup()
        {
            SwitchGroupKeyTexts[0] = tr("left/right");
            SwitchGroupKeyTexts[1] = tr("prev/next");

            SitchWithOKTexts[0] = tr("Blue");
            SitchWithOKTexts[1] = tr("Ok");

            Add(new cOsdItem(tr("Behavior"), osUnknown, false));
            Add(new cMenuEditIntItem(tr("Step width (min)"), &Step));
            Add(new cMenuEditTimeItem(tr("Favorite time"), &bookmark));
            Add(new cMenuEditStraItem(tr("Key to switch channel"), &switchwithok, 2, SitchWithOKTexts));
            Add(new cMenuEditBoolItem(tr("Selected item centered"), &middlemenuentry));
            Add(new cMenuEditStraItem(tr("Keys to switch channel group"), &switchgroupkey, 2, SwitchGroupKeyTexts));

            Add(new cOsdItem(tr("Appearance"), osUnknown, false));
            Add(new cMenuEditBoolItem(tr("Hide main menu entry"), &hidemainmenu));
            Add(new cMenuEditIntItem(tr("Channel name width"), &ChannelNameWidth));
            Add(new cMenuEditBoolItem(tr("Keep display after switching"), &keeposd));
            Add(new cMenuEditBoolItem(tr("Show channel numbers"), &showchannelnumbers));
            Add(new cMenuEditIntItem(tr("Hide Groups at Groupnr"), &HideGroupsAt));
            Add(new cMenuEditBoolItem(tr("Hide encrypted channels"), &hideencryptedchannels));
            Add(new cMenuEditBoolItem(tr("Hide radio channels"), &hideradiochannels));
            Add(new cMenuEditBoolItem(tr("Progress as percent"), &percentprogress));
        }
};

cPluginNeutrinoEpg::cPluginNeutrinoEpg(void)
{
}

cPluginNeutrinoEpg::~cPluginNeutrinoEpg()
{
}

const char *cPluginNeutrinoEpg::CommandLineHelp(void)
{
    return NULL;
}

bool cPluginNeutrinoEpg::ProcessArgs(int argc, char *argv[])
{
    return true;
}

bool cPluginNeutrinoEpg::Initialize(void)
{
    return true;
}

bool cPluginNeutrinoEpg::Start(void)
{
    Icons::InitCharSet();

    GroupIndex = NULL;
    CurrentGroupChannel = NULL;
    FirstGroupChannel = NULL;
    LastGroupChannel = NULL;

    ReloadFilters = true;
    
    return true;
}

void cPluginNeutrinoEpg::Stop(void)
{
    if( GroupIndex != NULL )
        delete[] GroupIndex;
    if( CurrentGroupChannel != NULL )
        delete[] CurrentGroupChannel;
    if( FirstGroupChannel != NULL )
        delete[] FirstGroupChannel;
    if( LastGroupChannel != NULL )
        delete[] LastGroupChannel;
}

void cPluginNeutrinoEpg::Housekeeping(void)
{
}

cOsdObject *cPluginNeutrinoEpg::MainMenuAction(void)
{
    syslog(LOG_ERR, "neutrinoepg MainMenuAction");
    
    return new myOsdMenu;
}

cMenuSetupPage *cPluginNeutrinoEpg::SetupMenu(void)
{
    return new myMenuSetup;
}

bool cPluginNeutrinoEpg::SetupParse(const char *Name, const char *Value)
{
    if(!strcmp("hidemainmenu", Name))
        hidemainmenu = atoi(Value);
    else if(!strcmp("bookmark", Name))
        bookmark = atoi(Value);
    else if(!strcmp("step", Name))
        Step = atoi(Value);
    else if(!strcmp("keeposd", Name))
        keeposd = atoi(Value);
    else if(!strcmp("hideencryptedchannels", Name))
        hideencryptedchannels = atoi(Value);
    else if(!strcmp("showchannelnumbers", Name))
        showchannelnumbers = atoi(Value);
    else if(!strcmp("hideradiochannels", Name))
        hideradiochannels = atoi(Value);
    else if(!strcmp("switchwithok", Name))
        switchwithok = atoi(Value);
    else if(!strcmp("percentprogress", Name))
        percentprogress = atoi(Value);
    else if(!strcmp("middlemenuentry", Name))
        middlemenuentry = atoi(Value);
    else if(!strcmp("ChannelNameWidth", Name))
        ChannelNameWidth = atoi(Value);
    else if(!strcmp("switchgroupkey", Name))
        switchgroupkey = atoi(Value);
    else if(!strcmp("HideGroupsAt", Name))
        HideGroupsAt = atoi(Value);
    else
        return false;
    return true;
}

VDRPLUGINCREATOR(cPluginNeutrinoEpg); // Don't touch this!
