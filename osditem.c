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
#if APIVERSNUM >= 20301
myOsdItem::myOsdItem(const cEvent *Event, const cChannel *Channel, bool Next)
#else
myOsdItem::myOsdItem(const cEvent *Event, cChannel *Channel, bool Next)
#endif
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
	
    // time reference for timer matching
    time_t t;
    if (event)
        t = event->StartTime();
    else
        t = time(NULL);


    // look for timers
#if APIVERSNUM >= 20301
    LOCK_TIMERS_READ;
    for(const cTimer *ti = Timers->First(); ti; ti = Timers->Next(ti))
#else
    for(cTimer *ti = Timers.First(); ti; ti = Timers.Next(ti))
#endif
    {
        if(ti->Matches(t) && (ti->Channel() == channel))
        {
            timer = ti;
            if( progressbar == 1 )
                m = timer->Recording() ? "*" : "#";
            else
                m = timer->Recording() ? Icons::Recording() : Icons::AlarmClock();
        }
    }

    if(event)
    {
        const char *title = event->Title();
        if (!title)
            title = tr("<no title>");

        int duration = event->Duration();
        if (duration <= 0)
            duration = 1;

        if( progressbar == 2 )
        {
            // calculate progress bar
            float progress = (int)roundf( (float)(time(NULL) - event->StartTime()) / (float) duration * 100.0);
            if(progress < 0)
                progress = 0.;
            else if(progress > 100)
                progress = 100;
                
            asprintf(&strProgress, "%3.0f%%", progress);

            if(showchannelnumbers)
                asprintf(&buffer,"%s\t%d\t%-10s\t%s\t %s", m,
                        channel ? channel->Number() : 0,
                        channel ? channel->ShortName(true) : tr("<no channel>"),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : strProgress, title);
            else
                asprintf(&buffer,"%s\t%-10s\t%s\t %s", m,
                        channel ? channel->ShortName(true) : tr("<no channel>"),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : strProgress, title);
        } else if( progressbar == 0 )
        {
            // calculate progress bar
            float progress = (int)roundf( (float)(time(NULL) - event->StartTime()) / (float) duration * 10.0);
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
                asprintf(&buffer,"%s\t%d\t%-10s\t%s\t %s", m,
                        channel ? channel->Number() : 0,
                        channel ? channel->ShortName(true) : tr("<no channel>"),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : ProgressBar.c_str(), title);
            else
                asprintf(&buffer,"%s\t%-10s\t%s\t %s", m,
                        channel ? channel->ShortName(true) : tr("<no channel>"),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : ProgressBar.c_str(), title);
        } else {
            char szProgress[20] = "";
            char szProgressPart[25] = "";
            int progress = (int)roundf( (float)(time(NULL) - event->StartTime()) / (float) duration * 8.0);
            progress = std::min(8, std::max(0, progress));

            for(int i = 0; i < 8; i++) {
                if( i < progress )
                    szProgress[i] = '|';
                else
                    szProgress[i] = ' ';
            }
            szProgress[progress] = '\0';
            sprintf(szProgressPart, "%c%-8s%c", '[', szProgress, ']');
            if(showchannelnumbers)
                asprintf(&buffer,"%s\t%d\t%-10s\t%s\t %s", m,
                        channel ? channel->Number() : 0,
                        channel ? channel->ShortName(true) : tr("<no channel>"),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : szProgressPart, title);
            else
                asprintf(&buffer,"%s\t%-10s\t%s\t %s", m,
                        channel ? channel->ShortName(true) : tr("<no channel>"),
                        (!(event->RunningStatus()==4)&&next) ? *event->GetTimeString() : szProgressPart, title);
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
