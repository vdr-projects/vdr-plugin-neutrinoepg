/*
 * See the README file for copyright information and how to reach the author.
 */

#include "menuevent.h"

extern int keeposd;
extern time_t t;
char Title[32];

bool isMenuEvent;

struct Epgsearch_search_v1_0
{
// in
    char* query;               // search term
    int mode;                  // search mode (0=phrase, 1=and, 2=or, 3=regular expression)
    int channelNr;             // channel number to search in (0=any)
    bool useTitle;             // search in title
    bool useSubTitle;          // search in subtitle
    bool useDescription;       // search in description
// out
    cOsdMenu* pResultMenu;   // pointer to the menu of results
};

myMenuEvent::myMenuEvent(myOsdItem *Item) : cOsdMenu(trVDR("Event"))
{
    item    = Item;
    event   = item->event;
    timer   = item->timer;
    channel = item->channel;

    isMenuEvent = true;
    
    SetTitle( channel->Name() );
    pEpgSearch = cPluginManager::GetPlugin("epgsearch");

    if( timer )
    {
        if( pEpgSearch )
            SetHelp(tr("Button$Edit T."), tr("Button$Delete T."), tr("Button$Search for repeats"), trVDR("Button$Switch"));
        else
            SetHelp(tr("Button$Edit T."), tr("Button$Delete T."), NULL, trVDR("Button$Switch"));
    }
    else
    {
        if( pEpgSearch && event )
            SetHelp(trVDR("Button$Record"), NULL, tr("Button$Search for repeats"), trVDR("Button$Switch"));
        else
            SetHelp(trVDR("Button$Record"), NULL, NULL, trVDR("Button$Switch"));
    }
}
myMenuEvent::~myMenuEvent(void)
{
    isMenuEvent = false;
}
void myMenuEvent::Display()
{
    cOsdMenu::Display();

    if(event)
    {
        DisplayMenu()->SetEvent(event);
        cStatus::MsgOsdTextItem(event->Description());
    }
    else
        DisplayMenu()->SetText(tr("no program info"), 0);
}

eOSState myMenuEvent::SearchRepeats(void)
{
    if( !pEpgSearch )
        return osContinue;

    Epgsearch_search_v1_0* serviceData = new Epgsearch_search_v1_0;
    serviceData->query = strdup( event->Title() );
    serviceData->mode = 0;
    serviceData->channelNr = 0;
    serviceData->useTitle = true;
    serviceData->useSubTitle = serviceData->useDescription = false;

    cOsdMenu* pMenu = NULL;
    if (pEpgSearch->Service("Epgsearch-search-v1.0", serviceData))
        pMenu = serviceData->pResultMenu;
    else
        Skins.Message(mtError, "EPGSearch does not support this service!");

    delete serviceData;

    if (pMenu)
        return AddSubMenu(pMenu);
    else
        return osContinue;

}

eOSState myMenuEvent::Switch()
{
    const cChannel *channel = item->channel;
    if(channel && cDevice::PrimaryDevice()->SwitchChannel(channel, true))
        return keeposd ? osContinue : osEnd;
    Skins.Message(mtError, tr("Can't switch channel!"));
    return osContinue;
}

eOSState myMenuEvent::Record()
{
    // if a timer exists, edit it
    if(timer)
        return AddSubMenu(new cMenuEditTimer(timer));
    else
    {
        // we have to create a new one
        if(event)
        {
            timer = new cTimer(event);
            Timers.Add(timer);
            timer->Matches();
            Timers.SetModified();
        }
        else
        {
            // this is a hack to create a timer for channels without events
            char *buffer;
            int starthh, startmm, stophh, stopmm;

            struct tm tm_t;
            localtime_r(&t, &tm_t);

            sscanf(*TimeString(t), "%2d:%2d", &starthh, &startmm);
            sscanf(*TimeString(t + Setup.InstantRecordTime*60), "%2d:%2d", &stophh, &stopmm);

            asprintf(&buffer, "1:%d:%d:%d%d:%d%d:%d:%d:%s",
                    item->channel->Number(), tm_t.tm_mday, starthh, startmm, stophh, stopmm,
                    Setup.DefaultPriority, Setup.DefaultLifetime, item->channel->Name());

            timer = new cTimer(false, false);
            timer->Parse(buffer);
            free(buffer);

            // we start a cMenuEditTimer because there are no reasonable values
            return AddSubMenu(new cMenuEditTimer(timer, true));
        }
    }
    SetHelp(tr("Button$Edit T."), tr("Button$Delete T."), NULL, trVDR("Button$Switch"));
    // update item text
    item->Set();
    return osContinue;
}

eOSState myMenuEvent::Delete()
{
    if(Interface->Confirm(trVDR("Delete timer?")))
    {
        if(timer->Recording())
        {
            if(Interface->Confirm(trVDR("Timer still recording - really delete?")))
            {
                timer->Skip();
                cRecordControls::Process(time(NULL));
            }
            else
                return osContinue;
        }
        Timers.Del(timer);
        Timers.SetModified();
        timer = NULL;

        SetHelp(trVDR("Button$Record"), NULL, NULL, trVDR("Button$Switch"));
    }
    // update item text
    item->Set();
    return osContinue;
}

eOSState myMenuEvent::ProcessKey(eKeys Key)
{
    if(!HasSubMenu())
    {
        switch((int)Key)
        {
            case kUp|k_Repeat:
            case kUp:
            case kDown|k_Repeat:
            case kDown:
                DisplayMenu()->Scroll(
                        NORMALKEY(Key) == kUp   || NORMALKEY(Key) == kLeft,
                        NORMALKEY(Key) == kLeft ||  NORMALKEY(Key) == kRight);
                cStatus::MsgOsdTextItem(NULL, NORMALKEY(Key) == kUp);
                return osContinue;
            case kLeft:
            {
                // previous event
                cSchedulesLock schedulesLock;
                const cSchedules *schedules;
                schedules = cSchedules::Schedules(schedulesLock);
                const cSchedule *Schedule = schedules->GetSchedule( channel->GetChannelID() );

                if(Schedule)
                {
                    // do not go before first schedule
                    if( event == Schedule->Events()->First() )
                        break;
                    
                    const cEvent *prev = NULL, *e = NULL;
                    for( e = Schedule->Events()->First(); e; e = Schedule->Events()->Next(e) )
                    {
                        if( e == event )
                            break;
                        prev = e;
                    }
                    if( prev != e )
                        event = prev;
                    else
                        event = NULL;

                    Display();
                }
                
                return osContinue;
            }
            case kRight:
            {
                // next event
                cSchedulesLock schedulesLock;
                const cSchedules *schedules;
                schedules = cSchedules::Schedules(schedulesLock);
                const cSchedule *Schedule = schedules->GetSchedule( channel->GetChannelID() );
                if(Schedule)
                {
                    const cEvent *next, *e = NULL;
                    for( e = Schedule->Events()->First(); e; e = Schedule->Events()->Next(e) )
                    {
                        if( e == event )
                            break;
                    }
                    if( e )
                    {
                        next = Schedule->Events()->Next(e);
                        if( next )
                            event = next;
                        else
                            event = NULL;
                    }
                    else
                        event = NULL;
               
                    Display();
                }
                return osContinue;
            }
            default:
                break;
        }
    }

    eOSState state = cOsdMenu::ProcessKey(Key);

    if(state == osUnknown)
    {
        switch(Key)
        {
            case kRecord:
            case kRed:
                return Record();
            case kGreen:
                if(timer)
                    return Delete();
            case kYellow:
                return SearchRepeats();
            case kBlue:
                return Switch();
            case kOk:
                return osBack;
            default:
                break;
        }
    }
    return state;
}
