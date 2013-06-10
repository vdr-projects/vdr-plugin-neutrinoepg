#include "osdmenu.h"
#include "osditem.h"
#include "menuevent.h"

time_t t;

int *GroupIndex;
int *CurrentGroupChannel;
int *FirstGroupChannel;
int *LastGroupChannel;

int LastMaxGroup;

int ChannelsShown;
int ChannelsBefore;
int ChannelsAfter;

myOsdMenu::myOsdMenu() : cOsdMenu("")
{
    jumpto = false;

    if( Setup.UseSmallFont == 2 )
    {
        if(showchannelnumbers)
            cOsdMenu::SetCols(1, CHNUMWIDTH + 1, ChannelNameWidth, 5);
        else
            cOsdMenu::SetCols(1, ChannelNameWidth, 5);
    }
    else
    {
        if(showchannelnumbers)
            cOsdMenu::SetCols(1, CHNUMWIDTH + 1, ChannelNameWidth, 6);
        else
            cOsdMenu::SetCols(1, ChannelNameWidth, 6);
    }

    t = time(NULL);
    next = false;

    // test how many OsdItems shown on display
    // add 50 items, make a PageDown and look what item is the current
    ChannelsShown = 0;
    char buf[20]="";
    for(int i = 0; i < 50; i++)
    {
        strcpy(buf, (const char *)itoa(i));
        if( i == 0 )
            Add(new cOsdItem(buf),  true);
        else
            Add(new cOsdItem(buf),  false);
    }
    Display();

    PageDown();
    cOsdItem *tItem = Get(Current());
    ChannelsShown = atoi((const char *)tItem->Text());
    Clear();
    //ChannelsShown = Skins.Current()->DisplayMenu()->MaxItems();

    // how many items are before and after the middle item
    if( (ChannelsShown % 2) != 0 )
    {
        ChannelsBefore = (ChannelsShown / 2);
        ChannelsAfter = (ChannelsShown / 2);
    }
    else
    {
        ChannelsBefore = (ChannelsShown / 2);
        ChannelsAfter = (ChannelsShown / 2)-1;
    }

    // Count the groups and channels
    int GroupCount = 0;
    for(cChannel *Channel = Channels.First(); Channel; Channel = Channels.Next(Channel))
    {
        if( Channel->GroupSep() )
            GroupCount++;
    }
    MaxGroup = GroupCount;

    if( !Channels.First()->GroupSep() )
        MaxGroup++;
    //syslog(LOG_ERR, "neutrinoepg: MaxGroup %d", MaxGroup);
    // Hide Groups?
    if( HideGroupsAt > MaxGroup )
        HideGroupsAt = MaxGroup;
    if( HideGroupsAt < 1 )
        HideGroupsAt = 0;
    if( HideGroupsAt > 0 )
        MaxGroup -= MaxGroup - HideGroupsAt;
    
    // test for different group count
    if( LastMaxGroup != MaxGroup )
    {
    //:(LOG_ERR, "neutrinoepg: HideGroup %d MaxGroup %d", HideGroupsAt, MaxGroup);
        
    if( GroupIndex != NULL )
        delete[] GroupIndex;
    if( CurrentGroupChannel != NULL )
        delete[] CurrentGroupChannel;
    if( FirstGroupChannel != NULL )
        delete[] FirstGroupChannel;
    if( LastGroupChannel != NULL )
        delete[] LastGroupChannel;

    // store max group count and add a little reserve
    GroupIndex = new int[MaxGroup+1];
    CurrentGroupChannel = new int[MaxGroup+1];
    FirstGroupChannel = new int[MaxGroup+1];
    LastGroupChannel = new int[MaxGroup+1];

    for( int i = 0; i < MaxGroup; i++)
    {
        CurrentGroupChannel[i] = -1;
        FirstGroupChannel[i] = -1;
        LastGroupChannel[i] = -1;
    }

    int index = 0;
    if( FirstChannelsHasGroup() == false )
    {
        GroupIndex[0] = -1;
        index = 1;
    }
    for( cChannel *Channel = Channels.First(); Channel && index < MaxGroup; Channel = Channels.Next(Channel) )
    {
        if( Channel->GroupSep() )
        {
            GroupIndex[index] = Channel->Index();
            index++;
        }
    }

    for( int Group = 0; Group < MaxGroup; Group++)
    {
        if( CurrentGroupChannel[Group] == -1 )
            CurrentGroupChannel[Group] = GetFirstChannelOfGroup(Group);
        if( FirstGroupChannel[Group] == -1 )
            FirstGroupChannel[Group] = GetFirstChannelOfGroup(Group);
        if( LastGroupChannel[Group] == -1 )
            LastGroupChannel[Group] = GetLastChannelOfGroup(Group);
    }

    LastMaxGroup = MaxGroup;
    
    }
    // what is the current watching channel?
    int CurrentChannelNr = cDevice::CurrentChannel();
    cChannel *CurrentChannel = Channels.GetByNumber(CurrentChannelNr);
    // is Current Channel is filtered?
    bool isRadio = ( (!CurrentChannel->Vpid()) && (CurrentChannel->Apid(0)) ) ? true : false;
    if( (isRadio && hideradiochannels) || (CurrentChannel->Ca() && hideencryptedchannels) )
    {
        CurrentChannel = Channels.Get( GetNextChannel( CurrentChannel->Index() ) );
    }
    // what is the current channel & group?
    CurrentGroup = GetGroupFromChannel( CurrentChannel->Index() );
    CurrentGroupChannel[CurrentGroup] = CurrentChannel->Index();

    LoadSchedules(0);
}

myOsdMenu::~myOsdMenu()
{
}

int myOsdMenu::GetGroupIndex(int Group)
{
    if( Group < 0 || Group > MaxGroup)
        return -1;

    return GroupIndex[Group];
}
int myOsdMenu::GetGroupByGroupIndex(int groupIndex)
{
    if( ChannelsHasGroup() == false || groupIndex == -1 )
        return 0;

    for(int index = 0; index < MaxGroup; index++)
    {
        if( GroupIndex[index] == groupIndex )
            return index;
    }
    return 0;
}
int myOsdMenu::GetGroupFromChannel(int ChanIndex)
{
    int GroupIndex = Channels.GetPrevGroup(ChanIndex);
    if( GroupIndex == -1 )
        return 0;
    return GetGroupByGroupIndex( GroupIndex );
}

int myOsdMenu::GetLastGroupIndex(void)
{
    cChannel *Channel = Channels.Last();
    return Channels.GetPrevGroup( Channel->Index() );
}

int myOsdMenu::GetFirstGroupIndex(void)
{
    cChannel *Channel = Channels.First();
    if( Channel->GroupSep() )
        return Channel->Index();
    return Channels.GetNextGroup( Channel->Index() );
}

int myOsdMenu::GetFirstChannelOfGroup(int Group)
{
    if( ChannelsHasGroup() == false ) // no groups -> get First Channel
    {
        cChannel *Channel;
        Channel = Channels.First();
        if( Channel == NULL )
            return -1;
        bool isRadio = ( (!Channel->Vpid()) && (Channel->Apid(0)) ) ? true : false;
        if( !(isRadio && hideradiochannels) && !(Channel->Ca() && hideencryptedchannels) )
            return Channel->Index();

        return GetNextChannelOfGroup( Channel->Index(), Group );
    }

    return GetNextChannelOfGroup( GetGroupIndex(Group), Group );
}

int myOsdMenu::GetLastChannelOfGroup(int Group)
{
    int NextGroup = Channels.GetNextGroup( GetGroupIndex(Group) );
    // next group -> get prev channel of next group
    if( NextGroup != -1 )
        return GetPrevChannelOfGroup(NextGroup, Group);
    else // no next group -> get last channel
    {
        cChannel *Channel;
        Channel = Channels.Last();
        if( Channel == NULL )
            return -1;
        if( isChannelInGroup( Channel->Index(), Group ) == false )
            return -1;

        bool isRadio = ( (!Channel->Vpid()) && (Channel->Apid(0)) ) ? true : false;
        if( !(isRadio && hideradiochannels) && !(Channel->Ca() && hideencryptedchannels) )
            return Channel->Index();
    
        return GetPrevChannelOfGroup( Channel->Index(), Group );
    }
    
}

int myOsdMenu::GetNextChannel(int ChanIndex)
{
    cChannel *Channel;
    Channel = Channels.Get( Channels.GetNextNormal(ChanIndex) );
    if( Channel == NULL )
        return -1;
    do
    {
        bool isRadio = ( (!Channel->Vpid()) && (Channel->Apid(0)) ) ? true : false;
        if( !(isRadio && hideradiochannels) && !(Channel->Ca() && hideencryptedchannels) )
            break;
    } while( (Channel = Channels.Get( Channels.GetNextNormal( Channel->Index() ))) != NULL );

    if( Channel == NULL )
        return -1;

    return Channel->Index();
}

int myOsdMenu::GetPrevChannel(int ChanIndex)
{
    cChannel *Channel;
    Channel = Channels.Get( Channels.GetPrevNormal(ChanIndex) );
    if( Channel == NULL )
        return -1;
    do
    {
        bool isRadio = ( (!Channel->Vpid()) && (Channel->Apid(0)) ) ? true : false;
        if( !(isRadio && hideradiochannels) && !(Channel->Ca() && hideencryptedchannels) )
            break;
    } while( (Channel = Channels.Get( Channels.GetPrevNormal( Channel->Index() ))) != NULL );

    if( Channel == NULL )
        return -1;

    return Channel->Index();
}

int myOsdMenu::GetNextChannelOfGroup(int ChanIndex, int Group)
{
    int NextIndex = GetNextChannel( ChanIndex );
    if( isChannelInGroup( NextIndex, Group ) == false )
        return -1;
    return NextIndex;
}
int myOsdMenu::GetPrevChannelOfGroup(int ChanIndex, int Group)
{
    int PrevIndex = GetPrevChannel( ChanIndex );
    if( isChannelInGroup( PrevIndex, Group ) == false )
        return -1;
    return PrevIndex;
}

bool myOsdMenu::isChannelInGroup(int ChanIndex, int Group)
{
    if( ChannelsHasGroup() == false )
        return true;

    int ChanGroup = GetGroupFromChannel(ChanIndex);
    return Group == ChanGroup ? true : false;
}

bool myOsdMenu::ChannelsHasGroup(void)
{
    cChannel *Channel = Channels.First();
    if( Channel->GroupSep() )
        return true;
    int Group = Channels.GetNextGroup(Channel->Index());
    return Group != -1 ? true : false;
}

bool myOsdMenu::FirstChannelsHasGroup(void)
{
    cChannel *Channel = Channels.First();
    if( Channel->GroupSep() )
        return true;
    return false;
}

void myOsdMenu::LoadSchedules(int shift)
{
    t += shift * Step * 60;

    // clar all items
    Clear();

    schedules = cSchedules::Schedules(schedulesLock);
    if( middlemenuentry )
    {
        int NumAdded = 0;
        int AddChanIndex = -1, ChanIndex = -1;
        AddChanIndex = CurrentGroupChannel[CurrentGroup];
        ChanIndex = AddChanIndex;
        
        // count if we have more channels add to the front because we are at the end
        int ExtraAdded = ChannelsAfter;
        while( ExtraAdded-- )
        {
            ChanIndex = GetNextChannelOfGroup( ChanIndex, CurrentGroup );
            if( ChanIndex == -1 )
                break;
        }

        ChanIndex = AddChanIndex;

        // count Channels
        NumAdded -= ExtraAdded;
        for( ; NumAdded <= ChannelsBefore; NumAdded++ )
        {
            AddChanIndex = GetPrevChannelOfGroup( AddChanIndex, CurrentGroup );
            if( AddChanIndex == -1 )
                break;
            ChanIndex = AddChanIndex;
        }
        for( NumAdded = 0; NumAdded < ChannelsShown && ChanIndex != -1; NumAdded++ )
        {
            cChannel *Channel = Channels.Get( ChanIndex );
            const cSchedule *Schedule = schedules->GetSchedule( Channel->GetChannelID() );
            if(Schedule)
            {
                // event from now or any other date (next)
                const cEvent *Event = next ? Schedule->GetEventAround(t) : Schedule->GetPresentEvent();
                Add(new myOsdItem(Event, Channel, next), Channel->Index() == CurrentGroupChannel[CurrentGroup] );
            }
            else
                Add(new myOsdItem(NULL, Channel, next), Channel->Index() == CurrentGroupChannel[CurrentGroup] );
            ChanIndex = GetNextChannelOfGroup( ChanIndex, CurrentGroup );
        }
        
    }
    else
    {
        for( cChannel *Channel = Channels.First(); Channel; Channel = Channels.Next(Channel) )
        {
            // is filtered by config?
            bool isRadio = ( (!Channel->Vpid()) && (Channel->Apid(0)) ) ? true : false;
            if( (isRadio && hideradiochannels) || (Channel->Ca() && hideencryptedchannels) )
                continue;

            // only channels!
            if( Channel->GroupSep() )
                continue;

            // only channels of the current Group
            if( !isChannelInGroup( Channel->Index(), CurrentGroup ) )
                continue;

            const cSchedule *Schedule = schedules->GetSchedule( Channel->GetChannelID() );
            if(Schedule)
            {
                // event from now or any other date (next)
                const cEvent *Event = next ? Schedule->GetEventAround(t) : Schedule->GetPresentEvent();
                Add(new myOsdItem(Event, Channel, next), Channel->Index() == CurrentGroupChannel[CurrentGroup] );
            }
            else
                Add(new myOsdItem(NULL, Channel, next), Channel->Index() == CurrentGroupChannel[CurrentGroup] );
        }
    }
    sprintf(tmp, "%d:%d", bookmark / 100, bookmark % 100);

    SetMyTitle();

//    if( switchwithok )
//        SetHelp(next ? trVDR("Button$Now") : NULL, "<<", ">>", trVDR("Button$Info") );
//    else
    
    if( next )
        SetHelp(trVDR("Button$Now"), "<<", ">>", switchwithok ? trVDR("Button$Info") : trVDR("Button$Switch"));
    else
        SetHelp(bookmark ? tmp : NULL, "<<", ">>", switchwithok ? trVDR("Button$Info") : trVDR("Button$Switch"));

    Display();
}

void myOsdMenu::SetMyTitle(void)
{
    char *buffer = NULL;

    if( CurrentGroup == 0 && FirstChannelsHasGroup() == false )
    {
        if( next )
            asprintf(&buffer, "%d/%d %s - %s", 
                CurrentGroup + 1, MaxGroup, tr("without group"), *DayDateTime(t));
        else
            asprintf(&buffer, "%d/%d %s - %s", 
                CurrentGroup + 1, MaxGroup, tr("without group"), trVDR("What's on now?"));
    } else
    {
        int groupindex = GetGroupIndex(CurrentGroup);
        if( next )
        {
            if( groupindex == -1 )
                asprintf(&buffer, "%d/%d %s - %s", 
                    CurrentGroup + 1, MaxGroup, "no group", *DayDateTime(t));
            else
                asprintf(&buffer, "%d/%d %s - %s", 
                    CurrentGroup + 1, MaxGroup, Channels.Get( groupindex )->Name(), *DayDateTime(t));
        }
        else
        {
            if( groupindex == -1 )
                asprintf(&buffer, "%d/%d %s - %s", 
                    CurrentGroup + 1, MaxGroup, "no group", trVDR("What's on now?"));
            else
                asprintf(&buffer, "%d/%d %s - %s", 
                    CurrentGroup + 1, MaxGroup, Channels.Get( groupindex )->Name(), trVDR("What's on now?"));
        }
    }
    SetTitle(buffer);
    free(buffer);
}
eOSState myOsdMenu::Switch()
{
    myOsdItem *item = (myOsdItem *)Get(Current());
    if(item)
    {
        const cChannel *channel = item->channel;
        if(channel && cDevice::PrimaryDevice()->SwitchChannel(channel, true) )
            return keeposd ? osContinue : osEnd;
    }
    Skins.Message(mtError, trVDR("Can't switch channel!"));
    return osContinue;
}

void myOsdMenu::GoToDay(int day)
{
    struct tm tm1, tm2;
    time_t tmp_t = time(NULL);

    localtime_r(&t, &tm1);
    localtime_r(&tmp_t, &tm2);
    tm2.tm_hour  = tm1.tm_hour;
    tm2.tm_min   = tm1.tm_min;
    tm2.tm_mday += day;

    t = mktime(&tm2);

    next = true;
    LoadSchedules(0);
}

void myOsdMenu::JumpTo(int hour, int min)
{
    struct tm tmp_tm;
    localtime_r(&t, &tmp_tm);

    if(tmp_tm.tm_hour > hour || tmp_tm.tm_min > min && tmp_tm.tm_hour >= hour)
        tmp_tm.tm_mday++;

    tmp_tm.tm_hour = hour;
    tmp_tm.tm_min  = min;

    t = mktime(&tmp_tm);

    next = true;
    LoadSchedules(0);
}

eOSState myOsdMenu::ProcessKey(eKeys Key)
{
    eOSState state;

    if(jumpto)
    {
        switch(Key)
        {
            case k0...k9:
                {
                    int number = Key-k0;
                    switch(index)
                    {
                        // first digit
                        case 0: 
                            if(number <= 2)
                            {
                                hh = number * 10;
                                sprintf(tmp,"%s: %d-:--",tr("Jump to"),number);
                                index++;
                            }
                            break;
                            // second digit
                        case 1:
                            if(hh <= 10 || hh == 20 && number <= 3)
                            {
                                hh += number;
                                sprintf(tmp,"%s: %02d:--",tr("Jump to"),hh);
                                index++;
                            }
                            break;
                            // third digit
                        case 2: 
                            if(number <= 5)
                            {
                                mm = number * 10;
                                sprintf(tmp,"%s: %02d:%d-",tr("Jump to"),hh,number);
                                index++;
                            }
                            break;
                            // fourth digit
                        case 3: 
                            mm += number;
                            sprintf(tmp,"%s: %02d:%02d",tr("Jump to"),hh,mm);
                            index++;
                            break;
                        default:
                            break;
                    }
                    if(index == 4)
                    {
                        JumpTo(hh, mm);
                        jumpto = 0;
                    }
                    else
                    {
                        SetTitle(tmp);
                        Display();
                    }
                    break;
                }
            case kOk:
                JumpTo(hh, mm);
                jumpto = 0;
                break;
            case kBack:
                jumpto = 0;
                SetMyTitle();
                Display();
                break;
            default:
                break;
        }
        state = osContinue;
    }
    else
    {
        state = cOsdMenu::ProcessKey(Key);

        if(state == osUnknown)
        {
            switch(Key)
            {
                case k1...k7:
                    GoToDay(Key-k0);
                    break;
                    // toggle asking for a time to jump to
                case k0:
                    hh = mm = index =0;
                    sprintf(tmp, "%s: --:--",tr("Jump to"));
                    SetTitle(tmp);
                    Display();
                    jumpto = 1;
                    break;
                case kRed:
                    if( next )
                    {
                        next = false;
                        t = time(NULL);
                        LoadSchedules(0);
                    } else
                    {
                        if( bookmark )
                           JumpTo(bookmark/100,bookmark%100);
                    }
                    break;
                case kGreen:
                    next = true;
                    LoadSchedules(-1);
                    break;
                case kYellow:
                    next = true;
                    LoadSchedules(1);
                    break;
                case kBlue:
                    if(switchwithok)
                    {
                        if( Count() )
                           return AddSubMenu(new myMenuEvent((myOsdItem*) Get(Current())));
                    } else
                    {
                        if( next )
                        {
                            next = false;
                            t = time(NULL);
                            LoadSchedules(0);
                        }
                        else
                            return Switch();
                    }
                    break;
                case kOk:
                    if(switchwithok)
                    {
                        return Switch();
                    } else
                    {
                        if(Count())
                            return AddSubMenu(new myMenuEvent((myOsdItem*)Get(Current())));
                    }
                case kNext:
                    if( isMenuEvent )
                        return osContinue;
                    // if we switch channel groups with next/prev key do it
                    if( switchgroupkey == 1 )
                    {
                        CurrentGroup++;
                        if( CurrentGroup >= MaxGroup )
                            CurrentGroup = 0;
                        LoadSchedules(0);
                    } else // we do a page down
                    {
                        if( CurrentGroupChannel[CurrentGroup] == LastGroupChannel[CurrentGroup] )
                        {
                            CurrentGroupChannel[CurrentGroup] = FirstGroupChannel[CurrentGroup];
                        } else
                        {
                            for(int i = 0; i < ChannelsShown; i++)
                            {
                                CurrentGroupChannel[CurrentGroup] = GetNextChannel( CurrentGroupChannel[CurrentGroup] );
                                if( CurrentGroupChannel[CurrentGroup] == -1 )
                                    break;
                            }
                            if( CurrentGroupChannel[CurrentGroup] > LastGroupChannel[CurrentGroup] || CurrentGroupChannel[CurrentGroup] == -1 )
                            {
                                CurrentGroupChannel[CurrentGroup] = LastGroupChannel[CurrentGroup];
                            }
                        }
                        
                        if( !middlemenuentry ) // if middlemenuentry hack is not set we can use vdr function for page down
                            PageDown();
                        else // otherwise we must do a page down by our own
                        {
                            LoadSchedules(0);
                        }
                    }
                    return osContinue;
                case kPrev:
                    if( isMenuEvent )
                        return osContinue;
                    // if we switch channel groups with next/prev key do it
                    if( switchgroupkey == 1 )
                    {
                        CurrentGroup--;
                        if( CurrentGroup < 0 )
                            CurrentGroup = MaxGroup-1;
                        LoadSchedules(0);
                    } else // we do a page up
                    {
                        if( CurrentGroupChannel[CurrentGroup] == FirstGroupChannel[CurrentGroup] )
                        {
                            CurrentGroupChannel[CurrentGroup] = LastGroupChannel[CurrentGroup];
                        } else
                        {
                            for(int i = 0; i < ChannelsShown; i++)
                            {
                                CurrentGroupChannel[CurrentGroup] = GetPrevChannel( CurrentGroupChannel[CurrentGroup] );
                            }
                            if( CurrentGroupChannel[CurrentGroup] < FirstGroupChannel[CurrentGroup] )
                            {
                                CurrentGroupChannel[CurrentGroup] = FirstGroupChannel[CurrentGroup];
                            }
                        }

                        if( !middlemenuentry ) // if middlemenuentry hack is not set we can use vdr function for page up
                            PageUp();
                        else // otherwise we must do a page down by our own
                        {
                            LoadSchedules(0);
                        }
                    }
                    return osContinue;
                default:
                    break;
            }
        } else
        {
            switch((int)Key)
            {
                case kUp|k_Repeat:
                case kUp:
                {
                    if( isMenuEvent )
                        return osContinue;
                   
                    CurrentGroupChannel[CurrentGroup] = GetPrevChannel( CurrentGroupChannel[CurrentGroup] );
                    if( CurrentGroupChannel[CurrentGroup] < FirstGroupChannel[CurrentGroup] )
                        CurrentGroupChannel[CurrentGroup] = LastGroupChannel[CurrentGroup];

                    if( !middlemenuentry ) // if middlemenuentry hack is not set we can let vdr kUp
                        return osContinue;

                    LoadSchedules(0);
                    return osContinue;
                }
                case kDown|k_Repeat:
                case kDown:
                {
                    if( isMenuEvent )
                        return osContinue;

                    CurrentGroupChannel[CurrentGroup] = GetNextChannel( CurrentGroupChannel[CurrentGroup] );
                    if( CurrentGroupChannel[CurrentGroup] > LastGroupChannel[CurrentGroup] || CurrentGroupChannel[CurrentGroup] == -1 )
                        CurrentGroupChannel[CurrentGroup] = FirstGroupChannel[CurrentGroup];

                    if( !middlemenuentry ) // if middlemenuentry hack is not set we can let vdr kDown
                        return osContinue;

                    LoadSchedules(0);
                    return osContinue;
                }
                case kLeft|k_Repeat:
                case kLeft:
                    if( isMenuEvent )
                        return osContinue;
                    if( switchgroupkey == 1 ) // if we not switch groups by left/right we let do vdr the rest (page up)
                    {
                        if( CurrentGroupChannel[CurrentGroup] == FirstGroupChannel[CurrentGroup] )
                        {
                            CurrentGroupChannel[CurrentGroup] = LastGroupChannel[CurrentGroup];
                        } else
                        {
                            for(int i = 0; i < ChannelsShown; i++)
                            {
                                CurrentGroupChannel[CurrentGroup] = GetPrevChannel( CurrentGroupChannel[CurrentGroup] );
                            }
                            if( CurrentGroupChannel[CurrentGroup] < FirstGroupChannel[CurrentGroup] )
                            {
                                CurrentGroupChannel[CurrentGroup] = FirstGroupChannel[CurrentGroup];
                            }
                        }
                        
                        if( middlemenuentry ) // if middlemenuentry hack is set do own page up
                        {
                            LoadSchedules(0);
                            break;
                        }
                        
                        return osContinue;
                    }
                    // otherwise we switch the group to the left
                    CurrentGroup--;
                    if( CurrentGroup < 0 )
                        CurrentGroup = MaxGroup-1;
                    LoadSchedules(0);
                    break;
                case kRight|k_Repeat:
                case kRight:
                    if( isMenuEvent )
                        return osContinue;
                    if( switchgroupkey == 1 ) // if we not switch groups by left/right we let do vdr the rest (page down)
                    {
                        if( CurrentGroupChannel[CurrentGroup] == LastGroupChannel[CurrentGroup] )
                        {
                            CurrentGroupChannel[CurrentGroup] = FirstGroupChannel[CurrentGroup];
                        } else
                        {
                            for(int i = 0; i < ChannelsShown; i++)
                            {
                                CurrentGroupChannel[CurrentGroup] = GetNextChannel( CurrentGroupChannel[CurrentGroup] );
                                if( CurrentGroupChannel[CurrentGroup] == -1 )
                                    break;
                            }
                            if( CurrentGroupChannel[CurrentGroup] > LastGroupChannel[CurrentGroup] || CurrentGroupChannel[CurrentGroup] == -1 )
                            {
                                CurrentGroupChannel[CurrentGroup] = LastGroupChannel[CurrentGroup];
                            }
                        }

                        if( middlemenuentry ) // if middlemenuentry hack is set do manuel page up
                        {
                            LoadSchedules(0);
                            break;
                        }
                        
                        return osContinue;
                    }

                    // otherwise we switch the group to the right
                    CurrentGroup++;
                    if( CurrentGroup >= MaxGroup )
                        CurrentGroup = 0;
                    LoadSchedules(0);
                    break;
                default:
                    break;
            }

        }
    }
    return state;
}
