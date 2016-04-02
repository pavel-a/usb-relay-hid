// GUIapp.cpp : main project file

#include "Form1.h"

#include "../../lib/usb_relay_device.h"
#include <cstring>

using namespace GUIapp;

static intptr_t g_dev = 0;
static int g_chanNum = 0;
static pusb_relay_device_info_t g_enumlist = nullptr;

#define GREEN Drawing::Color::LimeGreen
#define RED   Drawing::Color::Red
#define GREY  Drawing::Color::Gray

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
    g_dev = 0;
    g_enumlist = nullptr;

    // Enabling Windows XP visual effects before any controls are created
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // Create the main window and run it
    Application::Run(gcnew Form1());
    return 0;
}

void Form1::buttonFind_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (g_enumlist) {
        usb_relay_device_free_enumerate(g_enumlist);
        g_enumlist = nullptr;
    }

    comboDevices->Items->Clear();

    g_enumlist = usb_relay_device_enumerate();
    pusb_relay_device_info_t q = g_enumlist;
    while(q) {
        System::String^ s = "";
        for (char const *p = q->serial_number; *p; p++)
          s += (wchar_t)(*p);
        comboDevices->Items->Add(s);
        q = q->next;
    }

    if ( comboDevices->Items->Count != 0 ) {
        comboDevices->SelectedIndex = 0;
        setMsg( "Found:" + (comboDevices->Items->Count).ToString() + " device(s)", false );
    } else {
        setMsg( "No devices found", true );
    }
}

// Open device
void Form1::buttonOpen_Click(System::Object^  sender, System::EventArgs^  e)
{
    System::String^ s = comboDevices->Text;
    if ( !s ) return;
    char id[16];
    int i;
    for (i = 0; i < s->Length && i < (sizeof(id) - 1); i++ ) {
        id[i] = (char)s[i];
    }
    id[i] = 0;
    // Only one device can be open. TODO: allow multiple devices open, save&restore state?
    if (g_dev) {
        setMsg( "Closing the opened device", false );
        buttonClose_Click(sender, e);
    }

    intptr_t dh = usb_relay_device_open_with_serial_number(id, (int)strlen(id));
    if ( dh ) {
        g_dev = dh;
      IndOpen->BackColor = GREEN;

      // Get and show state:
      //BUGBUG g_dev is not usb_relay_device_info* in the orig library! use my new f()  ?? use only info in the list?
      struct usb_relay_device_info* di = (struct usb_relay_device_info*)g_dev;
      int numChannels = (unsigned char)di->type;
      if ( numChannels > MaxRelaysNum ||  numChannels == 0) {
           // oops?
           numChannels = MaxRelaysNum;
      }
      g_chanNum = numChannels;
      unsigned st = 0;
      int r = usb_relay_device_get_status( g_dev, &st);
      if ( r != 0 ) {
         setMsg("Error reading device state", true);
      } else {
          for (i = 0; i < MaxRelaysNum; i++ ) {
            System::Windows::Forms::TextBox^ t;
            t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));
            Drawing::Color c = st & (1 << i) ? GREEN : RED;
            if ( ((i + 1) > numChannels) && (c == RED) )
                    c = GREY;
            t->BackColor = c;
          }
      }

    } else {
        setMsg("Error opening the device", true);
        IndOpen->BackColor = RED;
    }
}

// Close device
void Form1::buttonClose_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (g_dev) {
        usb_relay_device_close(g_dev);
        IndOpen->BackColor = RED;

        // All indicators -> gray, but leave relays as is.
        System::Windows::Forms::TextBox^ t;
        usb_relay_device_close(g_dev);
        {
            for (int i = 0; i < MaxRelaysNum; i++ ) {
                t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));
                t->BackColor = GREY;
            }
        }

        g_dev = 0;
    }
}

// Open all channels
void Form1::RopenAll_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (!g_dev) {
        setMsg("No relay device is open!", true);
        return;
    }

    System::Windows::Forms::TextBox^ t;
    int r = usb_relay_device_open_all_relay_channel(g_dev);
    if ( r == 0 ) {
        for (int i=0; i < MaxRelaysNum; i++ ) {
            t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));
            t->BackColor = (i <= g_chanNum - 1) ? GREEN : GREY;
        }
    }
}

// Close all channels
void Form1::RcloseAll_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (!g_dev) {
        setMsg("No relay device is open!", true);
        return;
    }

    System::Windows::Forms::TextBox^ t;
    int r = usb_relay_device_close_all_relay_channel(g_dev);
    if (r == 0) {
        for (int i=0; i < MaxRelaysNum; i++ ) {
            t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));
            t->BackColor = (i <= g_chanNum - 1) ? RED : GREY;
        }
    }
}

// Open one channel
void Form1::Ropen1_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (!g_dev) {
        setMsg("No relay device is open!", true);
        return;
    }

    System::Windows::Forms::Control^ b = safe_cast<System::Windows::Forms::Control^>(sender);
    int i = *(safe_cast<System::Int16^>(b->Tag));
    System::Windows::Forms::TextBox^ t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));

    int r = usb_relay_device_open_one_relay_channel(g_dev, i+1);
    t->BackColor = (r == 0) ? GREEN : GREY;
}

// Close one channel
void Form1::Rclose1_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (!g_dev) {
        setMsg("No relay device is open!", true);
        return;
    }

    System::Windows::Forms::Control^ b = safe_cast<System::Windows::Forms::Control^>(sender);
    int i = *(safe_cast<System::Int16^>(b->Tag));
    System::Windows::Forms::TextBox^ t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));

    int r = usb_relay_device_close_one_relay_channel(g_dev, i+1);
    t->BackColor = (r == 0) ? RED : GREY;
}

