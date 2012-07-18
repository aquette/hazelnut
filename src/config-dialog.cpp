/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * hazelnut
 * Copyright (C) Emilien Kia 2012 <emilien.kia@gmail.com>
 * 
hazelnut is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * hazelnut is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <wx/wx.h>

#include "config-dialog.hpp"

#include "app.hpp"
#include "colored-gauge.hpp"

#include "../data/icons/32x32/apps/nut.xpm"

// ----------------------------------------------------------------------------
// HazelnutConfigDialog implementation
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(HazelnutConfigDialog, wxDialog)
	EVT_TIMER(wxID_ANY, HazelnutConfigDialog::OnTimerUpdate)
	EVT_BUTTON(wxID_REVERT, HazelnutConfigDialog::OnRefreshUPSList)
    EVT_BUTTON(wxID_ABOUT, HazelnutConfigDialog::OnAbout)
    EVT_BUTTON(wxID_OK, HazelnutConfigDialog::OnOK)
    EVT_BUTTON(wxID_EXIT, HazelnutConfigDialog::OnExit)
    EVT_CLOSE(HazelnutConfigDialog::OnCloseWindow)
	EVT_CHOICE(wxID_ANY, HazelnutConfigDialog::OnChoicePowerSource)
END_EVENT_TABLE()


HazelnutConfigDialog::HazelnutConfigDialog(const wxString& title)
        : wxDialog(NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	// Create the timer for updatting status
	timer = new wxTimer(this);
	
    wxSizer* gsz = new wxBoxSizer(wxVERTICAL);

	{
		wxSizer* sz = new wxBoxSizer(wxHORIZONTAL);
		sz->Add(new wxStaticText(this, wxID_ANY, wxT("Power source:")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
		sz->Add(choice = new wxChoice(this, wxID_ANY), 1, wxALL, 4);
		sz->Add(new wxButton(this, wxID_REVERT, wxT("Reload")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
		gsz->Add(sz, 0, wxEXPAND);
	}

	gsz->Add(gauge = new wxColoredGauge(this, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN), 0, wxEXPAND|wxALL, 4);
	gauge->SetColorStep(20, wxColour(255, 0, 0), wxColour(92, 0, 0));
	gauge->SetColorStep(30, wxColour(255, 128, 0), wxColour(92, 46, 0));
	gauge->SetColorStep(40, wxColour(255, 255, 0), wxColour(92, 92, 0));
	gauge->SetColorStep(100, wxColour(0, 255, 0), wxColour(0, 92, 0));
	gauge->SetMark(10, 0.33);
	gauge->SetMark(20, 0.33);
	gauge->SetMark(30, 0.33);
	gauge->SetMark(40, 0.33);
	gauge->SetMark(50, 0.5);
	gauge->SetMark(60, 0.33);
	gauge->SetMark(70, 0.33);
	gauge->SetMark(80, 0.33);
	gauge->SetMark(90, 0.33);
	
	gsz->Add(timeToEmpty = new wxStaticText(this, wxID_ANY, wxT("")), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 4);
	
	gsz->AddStretchSpacer(1);
	{
		wxSizer*  sz = new wxBoxSizer(wxHORIZONTAL);
		sz->Add(new wxButton(this, wxID_ABOUT, wxT("&About")), 0, wxALL, 10);
		sz->Add(new wxButton(this, wxID_OK, wxT("&Hide")), 0, wxALL, 10);
		sz->Add(new wxButton(this, wxID_EXIT, wxT("E&xit")), 0, wxALL, 10);
		gsz->Add(sz, 0, wxALIGN_CENTER_HORIZONTAL);
	}
    SetSizerAndFit(gsz);
	Layout();
    Centre();

	SetIcon(wxIcon(nut_32));
	
	RefreshList();
}

HazelnutConfigDialog::~HazelnutConfigDialog()
{
}

bool HazelnutConfigDialog::HazelnutConfigDialog::Show(bool show)
{
	if(!wxDialog::Show(show))
		return false;

	if(show)
	{
		timer->Start(5*1000);
	}
	else
	{
		if(timer->IsRunning())
			timer->Stop();
	}
	
	return true;
}

void HazelnutConfigDialog::RefreshList()
{
	devices = wxGetApp().getUps();
	Device current = wxGetApp().GetDevice();
	
	choice->Clear();
	choice->Append(wxT("None"));
	if(!current)
		choice->SetSelection(0);

	for(std::list<Device>::iterator it=devices.begin(); it!=devices.end(); it++)
	{
		Device& dev = *it;
		if(dev.getDeviceType()==wxT("ups"))
		{
			wxString label = dev.getManufacturer() + wxT(" - ") + dev.getModel() + wxT(" - ") + dev.getSerial();
			int idx = choice->Append(label, &dev);
			if(dev==current)
				choice->SetSelection(idx);
		}
	}

	RefreshInfos();
}

void HazelnutConfigDialog::RefreshInfos()
{
	Device current = wxGetApp().GetDevice();
	long longVal;

	if(current)
	{
		if(current.getVar("battery.charge").ToLong(&longVal))
		{
			gauge->SetValue(longVal);
			gauge->SetToolTip(wxString::Format(wxT("Battery charge: %ld %%"), longVal));
		}
		else
		{
			gauge->SetValue(0);
			gauge->SetToolTip(wxT("Battery charge: not available"));
		}

		if(current.getVar("battery.runtime").ToLong(&longVal))
		{
			timeToEmpty->SetLabel(wxString::Format(wxT("Remaining: %ld s"), longVal));
		}
		else
		{
			timeToEmpty->SetLabel(wxT("Remaining: N/A"));
		}
	}
	else
	{
		gauge->SetValue(0);
		gauge->SetToolTip(wxT("Battery charge: not available"));
		timeToEmpty->SetLabel(wxT("Remaining: N/A"));		
	}
	
	Layout();
}

void HazelnutConfigDialog::OnTimerUpdate(wxTimerEvent& event)
{
	RefreshInfos();
}

void HazelnutConfigDialog::OnChoicePowerSource(wxCommandEvent& WXUNUSED(event))
{
	int sel = choice->GetSelection();
	if(sel==-1 || sel==0)
		wxGetApp().SetDevice(Device());
	else
		wxGetApp().SetDevice(*(Device*)choice->GetClientData(sel));

	RefreshInfos();
}

void HazelnutConfigDialog::OnRefreshUPSList(wxCommandEvent& WXUNUSED(event))
{
	RefreshList();
}

void HazelnutConfigDialog::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxGetApp().About();
}

void HazelnutConfigDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    Hide();
}

void HazelnutConfigDialog::OnExit(wxCommandEvent& WXUNUSED(event))
{
    wxGetApp().Exit();
}

void HazelnutConfigDialog::OnCloseWindow(wxCloseEvent& event)
{
    event.Veto();
    Hide();
}


