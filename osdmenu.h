#ifndef OSDMENU_H_
#define OSDMENU_H_

#include <vdr/menu.h>
#include "neutrinoepg.h"

extern time_t t;
extern int *GroupIndex;
extern int *CurrentGroupChannel;
extern int *FirstGroupChannel;
extern int *LastGroupChannel;
extern int LastMaxGroup;

// --- myMenuWhatsOn ----------------------------------------------------------
class myOsdMenu : public cOsdMenu
{
    private:
        bool next, jumpto;
        char tmp[64];
    
        int MaxGroup;

        int ChannelsShown;
        int ChannelsBefore;
        int ChannelsAfter;
        int CurrentGroup;

        int hh, mm, index;
        const cSchedules *schedules;
        cSchedulesLock schedulesLock;

        void LoadSchedules(int shift);
        void GoToDay(int day);
        void JumpTo(int hh,int mm);
        void SetMyTitle(void);
        eOSState Switch();
        int Tab(){ return Setup.UseSmallFont == 2 ? 5 : 6; }

        int GetGroupIndex(int Group);
        int GetGroupByGroupIndex(int GroupIndex);
        int GetGroupFromChannel(int ChanIndex);
        int GetLastGroupIndex(void);
        int GetFirstGroupIndex(void);
        int GetFirstChannelOfGroup(int Group);
        int GetLastChannelOfGroup(int Group);
        int GetNextChannel(int ChanIndex);
        int GetPrevChannel(int ChanIndex);
        int GetNextChannelOfGroup(int ChanIndex, int Group);
        int GetPrevChannelOfGroup(int ChanIndex, int Group);
        bool isChannelInGroup(int ChanIndex, int Group);
        bool ChannelsHasGroup(void);
        bool FirstChannelsHasGroup(void);

    public:
        myOsdMenu();
        ~myOsdMenu();
        virtual eOSState ProcessKey(eKeys Key);
};

#endif
