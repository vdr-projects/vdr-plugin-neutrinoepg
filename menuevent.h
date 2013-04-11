#ifndef MENUEVENT_H_
#define MENUEVENT_H_
/*
 * See the README file for copyright information and how to reach the author.
 */
#include <vdr/plugin.h>
#include <vdr/menu.h>
#include <vdr/status.h>
#include <vdr/interface.h>

#include "neutrinoepg.h"
#include "osditem.h"

extern bool isMenuEvent;

class myMenuEvent : public cOsdMenu
{
    private:
        eOSState Record(void);
        eOSState Switch(void);
        eOSState Delete(void);
        eOSState SearchRepeats(void);
        const cEvent *event;
        const cChannel *channel;
        cTimer *timer;
        myOsdItem *item;
        cPlugin *pEpgSearch;
    public:
        myMenuEvent(myOsdItem *Item);
        ~myMenuEvent(void);
        virtual void Display(void);
        virtual eOSState ProcessKey(eKeys Key);
};

#endif
