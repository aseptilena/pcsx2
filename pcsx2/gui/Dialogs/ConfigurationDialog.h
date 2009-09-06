/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2009  Pcsx2 Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/propdlg.h>

#include "wxHelpers.h"

class wxListbook;

namespace Dialogs
{
	class ConfigurationDialog : public wxDialogWithHelpers
	{
	protected:
		wxListbook&		m_listbook;
		wxArrayString	m_labels;

	public:
		virtual ~ConfigurationDialog();
		ConfigurationDialog(wxWindow* parent, int id=wxID_ANY);

	protected:
		template< typename T >
		void AddPage( const char* label, int iconid );

		void OnOk_Click( wxCommandEvent& evt );
		void OnCancel_Click( wxCommandEvent& evt );
		void OnApply_Click( wxCommandEvent& evt );

		virtual void OnSomethingChanged( wxCommandEvent& evt )
		{
			evt.Skip();
			if( (evt.GetId() != wxID_OK) && (evt.GetId() != wxID_CANCEL) && (evt.GetId() != wxID_APPLY) )
			{
				FindWindow( wxID_APPLY )->Enable();
			}
		}
	};
	
	
	class BiosSelectorDialog : public wxDialogWithHelpers
	{
	protected:

	public:
		virtual ~BiosSelectorDialog() {}
		BiosSelectorDialog(wxWindow* parent );

	protected:
		void OnOk_Click( wxCommandEvent& evt );
		void OnDoubleClicked( wxCommandEvent& evt );
	};
}
