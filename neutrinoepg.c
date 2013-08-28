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
int progressbar = false;
int middlemenuentry = false;
int switchgroupkey = 0;
int ChannelNameWidth = 15;
int HideGroupsAt = 0;

// --- myMenuSetup ------------------------------------------------------------
class myMenuSetup : public cMenuSetupPage
{
    private:
        int menuGroupCount;
        cStringList SwitchGroupKeyTexts;
        cStringList SwitchWithOKTexts;
        cStringList HideGroupsAtTexts;
        cStringList ProgressBarTexts;

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
            SetupStore("progressbar", progressbar);
            SetupStore("middlemenuentry", middlemenuentry);
            SetupStore("switchgroupkey", switchgroupkey);
            SetupStore("ChannelNameWidth", ChannelNameWidth);
            SetupStore("HideGroupsAt", HideGroupsAt);
        }
    public:
        ~myMenuSetup()
        {
        }
        myMenuSetup()
        {
            int index = 0;
            bool HideFirst = true;

            menuGroupCount = 0;
            
            SwitchGroupKeyTexts.Append( strdup( tr("left/right")) );
            SwitchGroupKeyTexts.Append( strdup( tr("prev/next")) );

            SwitchWithOKTexts.Append( strdup( tr("Blue")) );
            SwitchWithOKTexts.Append( strdup( tr("Ok")) );
            
            ProgressBarTexts.Append( strdup( tr("VDRSymbols")) );
            ProgressBarTexts.Append( strdup( tr("Text")) );
            ProgressBarTexts.Append( strdup( tr("Percent")) );

            for(cChannel *Channel = Channels.First(); Channel; Channel = Channels.Next(Channel))
            {
                if( Channel->GroupSep() )
                    menuGroupCount++;
            }

            if( HideGroupsAt < 0 || HideGroupsAt >= menuGroupCount )
                HideGroupsAt = 0;

            // add one group if the first is not one
            if( !Channels.First()->GroupSep() )
            {
                menuGroupCount++;
                HideFirst = false;
                HideGroupsAtTexts.Append( strdup( tr("no filter")) );
                index++;
            }
            
            for(cChannel *Channel = Channels.First(); Channel; Channel = Channels.Next(Channel))
            {
                if( Channel->GroupSep() )
                {
                    if( HideFirst )
                    {
                        HideFirst = false;
                        HideGroupsAtTexts.Append( strdup( tr("no filter")) );
                        index++;
                        continue;
                    }
                    HideGroupsAtTexts.Append( strdup(Channel->Name()) );
                    index++;
                }
            }


            Add(new cOsdItem(tr("Behavior"), osUnknown, false));
            Add(new cMenuEditIntItem(tr("Step width (min)"), &Step));
            Add(new cMenuEditTimeItem(tr("Favorite time"), &bookmark));
            Add(new cMenuEditStraItem(tr("Key to switch channel"), &switchwithok, SwitchWithOKTexts.Size(), &SwitchWithOKTexts[0]));
            Add(new cMenuEditBoolItem(tr("Selected item centered"), &middlemenuentry));
            Add(new cMenuEditStraItem(tr("Keys to switch channel group"), &switchgroupkey, SwitchGroupKeyTexts.Size(), &SwitchGroupKeyTexts[0]));

            Add(new cOsdItem(tr("Appearance"), osUnknown, false));
            Add(new cMenuEditBoolItem(tr("Hide main menu entry"), &hidemainmenu));
            Add(new cMenuEditIntItem(tr("Channel name width"), &ChannelNameWidth));
            Add(new cMenuEditBoolItem(tr("Keep display after switching"), &keeposd));
            Add(new cMenuEditBoolItem(tr("Show channel numbers"), &showchannelnumbers));
            Add(new cMenuEditStraItem(tr("Hide Groups at"), &HideGroupsAt, HideGroupsAtTexts.Size(), &HideGroupsAtTexts[0]));
            Add(new cMenuEditBoolItem(tr("Hide encrypted channels"), &hideencryptedchannels));
            Add(new cMenuEditBoolItem(tr("Hide radio channels"), &hideradiochannels));
            Add(new cMenuEditStraItem(tr("Progressbar modus"), &progressbar, ProgressBarTexts.Size(), &ProgressBarTexts[0]));
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
    return new myOsdMenu;
}

cMenuSetupPage *cPluginNeutrinoEpg::SetupMenu(void)
{
    return new myMenuSetup();
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
    else if(!strcmp("progressbar", Name))
        progressbar = atoi(Value);
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
