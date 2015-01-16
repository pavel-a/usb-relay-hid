// GUIapp.cpp : main project file

//#include "stdafx.h"

namespace GUIapp {
  const int MaxRelaysNum = 8;
}

#include "Form1.h"

#include "../../lib/usb_relay_device.h"
#include <cstring>

using namespace GUIapp;

static intptr_t g_dev = 0;
static int g_chanNum = 0;
static pusb_relay_device_info_t g_enumlist = nullptr;

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


System::Void Form1::buttonFind_Click(System::Object^  sender, System::EventArgs^  e)
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
System::Void Form1::buttonOpen_Click(System::Object^  sender, System::EventArgs^  e)
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
	  IndOpen->BackColor = Drawing::Color::LimeGreen;

	  // Get and show state:
      //BUGBUG g_dev is not usb_relay_device_info* in the orig library! use my new f()  ?? use only info in the list?
	  struct usb_relay_device_info* di = (struct usb_relay_device_info*)g_dev;
	  int numChannels = (int)di->type;
	  g_chanNum = numChannels;
	  unsigned st = 0;
	  int r = usb_relay_device_get_status( g_dev, &st);
	  if ( r != 0 ) {
		 setMsg("Error reading device state", true);
	  } else {
		  for (int i = 0; i < MaxRelaysNum; i++ ) {
			System::Windows::Forms::TextBox^ t;
			t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));
			Drawing::Color c = st & (1 << i) ? Drawing::Color::LimeGreen : Drawing::Color::Red;
			if ( ((i + 1) > numChannels) && (c == Drawing::Color::Red) )
					c = Drawing::Color::Gray;
			t->BackColor = c;
		  }
	  }

	} else {
		setMsg("Error opening the device", true);
	}
}

// Close device
System::Void Form1::buttonClose_Click(System::Object^  sender, System::EventArgs^  e)
{
	if (g_dev) {
		usb_relay_device_close(g_dev);
		IndOpen->BackColor = Drawing::Color::Red;

		// All indicators -> gray, but leave relays as is.
		System::Windows::Forms::TextBox^ t;
		usb_relay_device_close(g_dev);
		{
			for (int i = 0; i < MaxRelaysNum; i++ ) {
				t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));
				t->BackColor = Drawing::Color::Gray;
			}
		}

		g_dev = 0;
	}
}

// Open all channels
System::Void Form1::RopenAll_Click(System::Object^  sender, System::EventArgs^  e)
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
			t->BackColor = (i <= g_chanNum - 1) ? Drawing::Color::LimeGreen : Drawing::Color::Gray;
		}
	}
}

// Close all channels
System::Void Form1::RcloseAll_Click(System::Object^  sender, System::EventArgs^  e)
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
			t->BackColor = (i <= g_chanNum - 1) ? Drawing::Color::Red : Drawing::Color::Gray;
		}
	}
}

// Open one channel
System::Void Form1::Ropen1_Click(System::Object^  sender, System::EventArgs^  e)
{
	if (!g_dev) {
		setMsg("No relay device is open!", true);
		return;
	}

	int i;
	System::Windows::Forms::Button^ b = safe_cast<System::Windows::Forms::Button^>(sender);
	System::Int16^ n = safe_cast<System::Int16^>(b->Tag);
	i = *n;
	System::Windows::Forms::TextBox^ t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));

	int r = usb_relay_device_open_one_relay_channel(g_dev, i+1);
	if (r == 0) {
		t->BackColor = Drawing::Color::LimeGreen;
	}
}

// Close one channel
System::Void Form1::Rclose1_Click(System::Object^  sender, System::EventArgs^  e)
{
	if (!g_dev) {
		setMsg("No relay device is open!", true);
		return;
	}

	int i;
	System::Windows::Forms::Button^ b = safe_cast<System::Windows::Forms::Button^>(sender);
	System::Int16^ n = safe_cast<System::Int16^>(b->Tag);
	i = *n;
	System::Windows::Forms::TextBox^ t = static_cast<System::Windows::Forms::TextBox^>(aIndicators->GetValue(i));

	int r = usb_relay_device_close_one_relay_channel(g_dev, i+1);
	if (r == 0) {
		t->BackColor = Drawing::Color::Red;
	}
}

