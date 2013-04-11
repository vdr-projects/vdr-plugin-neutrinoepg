#ifndef OSDITEM_H_
#define OSDITEM_H_
/*
 * See the README file for copyright information and how to reach the author.
 */

extern int showchannelnumbers;
extern time_t t;

class myOsdItem : public cOsdItem
{
    private:
        bool next;
    public:
        const cEvent *event;
        const cChannel *channel;
        cTimer *timer;
        virtual void Set();
        myOsdItem(const cEvent *Event,cChannel *Channel,bool Next);
};

class Icons
{
    private:
        static bool IsUTF8;
    public:
        static void InitCharSet();
        static const char* Continue(){return IsUTF8?"\ue000":"\x80";}
        static const char* DVD(){return IsUTF8?"\ue001":"\x81";}
        static const char* Directory(){return IsUTF8?"\ue002":"\x82";}
        static const char* FixedBlank(){return IsUTF8?"\ue003":"\x83";}
        static const char* Scissor(){return IsUTF8?"\ue004":"\x84";}
        static const char* MovingRecording(){return IsUTF8?"\ue005":"\x85";}
        static const char* MovingDirectory(){return IsUTF8?"\ue006":"\x86";}
        static const char* ProgressStart(){return IsUTF8?"\ue007":"\x87";}
        static const char* ProgressFilled(){return IsUTF8?"\ue008":"\x88";}
        static const char* ProgressEmpty(){return IsUTF8?"\ue009":"\x89";}
        static const char* ProgressEnd(){return IsUTF8?"\ue00a":"\x8a";}
        static const char* Recording(){return IsUTF8?"\ue00b":"\x8b";}
        static const char* AlarmClock(){return IsUTF8?"\ue00c":"\x8c";}
        static const char* TVScrambled(){return IsUTF8?"\ue00d":"\x8d";}
        static const char* Radio(){return IsUTF8?"\ue00e":"\x8e";}
        static const char* TV(){return IsUTF8?"\ue00f":"\x8f";}
        static const char* New(){return IsUTF8?"\ue010":"\x90";}
};

#endif
