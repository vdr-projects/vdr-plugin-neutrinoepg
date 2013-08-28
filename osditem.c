/*
 * See the README file for copyright information and how to reach the author.
 */

#include <string>
#include <locale.h>
#include <langinfo.h>
#include <math.h>
#include <vdr/menu.h>
#include <vdr/status.h>
#include "osditem.h"
#include "neutrinoepg.h"

// --- Icons ------------------------------------------------------------------
bool Icons::IsUTF8 = false;

void Icons::InitCharSet()
{
    // Taken from VDR's vdr.c
    char *CodeSet = NULL;
    if(setlocale(LC_CTYPE, ""))
        CodeSet = nl_langinfo(CODESET);
    else
    {
        char *LangEnv = getenv("LANG"); // last resort in case locale stuff isn't installed
        if(LangEnv)
        {
            CodeSet = strchr(LangEnv,'.');
            if( CodeSet )
                CodeSet++; // skip the dot
        }
    }

    if( CodeSet && strcasestr(CodeSet,"UTF-8") != 0 )
        IsUTF8 = true;
}

// --- myWhatsOnItem ----------------------------------------------------------
myOsdItem::myOsdItem(const cEvent *Event, cChannel *Channel, bool Next)
{
    event = Event;
    channel = Channel;
    next = Next;
    timer = NULL;

    Set();
}

void myOsdItem::Set()
{
    int i;
    char *buffer = NULL, *strProgress = NULL;
    const char *m = " ";

    // look for timers
    for(cTimer *ti = Timers.First(); ti; ti = Timers.Next(ti))
    {
        if(ti->Matches(t) && (ti->Channel() == channel))
        {
            timer = ti;
            m = timer->Recording() ? Icons::Recording() : Icons::AlarmClock();
        }
    }

    if(event)
    {
        if( progressbar == 2 )
        {
            // calculate progress bar
            float progress = (int)roundf( (float)(time(NULL) - event->StartTime()) / (float) (event->Duration()) * 100.0);
            if(progress < 0)
                progress = 0.;
            else if(progress > 100)
                progress = 100;
                
            asprintf(&strProgress, "%3.0f%%", progress);

            if(showchannelnumbers)
                asprintf(&buffer,"%s\t%d\t%-10s\t%s\t %s", m, channel->Number(), channel->ShortName(true),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : strProgress, event->Title());
            else
                asprintf(&buffer,"%s\t%-10s\t%s\t %s", m, channel->ShortName(true),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : strProgress,event->Title());
        } else if( progressbar == 0 )
        {
            // calculate progress bar
            float progress = (int)roundf( (float)(time(NULL) - event->StartTime()) / (float) (event->Duration()) * 10.0);
            if(progress < 0)
                progress = 0.;
            else if(progress > 9)
                progress = 9;

            std::string ProgressBar;
            ProgressBar += Icons::ProgressStart();
            for(i=0;i < 10;i++)
            {
                if(i < progress)
                    ProgressBar += Icons::ProgressFilled();
                else
                    ProgressBar += Icons::ProgressEmpty();
            }
            ProgressBar += Icons::ProgressEnd();

            if(showchannelnumbers)
                asprintf(&buffer,"%s\t%d\t%-10s\t%s\t %s", m, channel->Number(), channel->ShortName(true),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : ProgressBar.c_str(), event->Title());
            else
                asprintf(&buffer,"%s\t%-10s\t%s\t %s", m, channel->ShortName(true),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : ProgressBar.c_str(), event->Title());
        } else {
            char szProgress[20] = "";
            char szProgressPart[25] = "";
            int progress = (int)roundf( (float)(time(NULL) - event->StartTime()) / (float) (event->Duration()) * 8.0);
            progress = min(8, max(0, progress));

            for(int i = 0; i < 8; i++) {
                if( i < progress )
                    szProgress[i] = '|';
                else
                    szProgress[i] = ' ';
            }
            szProgress[progress] = '\0';
            sprintf(szProgressPart, "%c%-8s%c", '[', szProgress, ']');
            if(showchannelnumbers)
                asprintf(&buffer,"%s\t%d\t%-10s\t%s\t %s", m, channel->Number(), channel->ShortName(true),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : szProgressPart, event->Title());
            else
                asprintf(&buffer,"%s\t%-10s\t%s\t %s", m, channel->ShortName(true),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : szProgressPart, event->Title());
        }
    }
    else
    {
        if(showchannelnumbers)
            asprintf(&buffer,"%s\t%d\t%-10s\t \t(%s)", m, channel->Number(), channel->ShortName(true), tr("no program info"));
        else
            asprintf(&buffer,"%s\t%-10s\t \t(%s)", m, channel->ShortName(true), tr("no program info"));
    }

    SetText(buffer, false);
}
