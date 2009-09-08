/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
 * 
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "ModalPopups.h"

#include <wx/stdpaths.h>

using namespace wxHelpers;
using namespace Panels;

Dialogs::PickUserModeDialog::PickUserModeDialog( wxWindow* parent, int id ) :
	wxDialogWithHelpers( parent, id, _("PCSX2 First Time configuration"), false )
,	m_panel_usersel( new UsermodeSelectionPanel( *this, 620, false ) )
,	m_panel_langsel( new LanguageSelectionPanel( *this, 620 ) )
{
	wxBoxSizer& s_main = *new wxBoxSizer( wxVERTICAL );

	AddStaticText( s_main, _("PCSX2 is starting from a new or unknown folder and needs to be configured.") );

	s_main.Add( m_panel_langsel, SizerFlags::StdCenter() );
	s_main.Add( m_panel_usersel, wxSizerFlags().Expand().Border( wxALL, 8 ) );

	AddOkCancel( s_main );
	SetSizerAndFit( &s_main );
	CenterOnScreen();

	Connect( wxID_OK,		wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PickUserModeDialog::OnOk_Click ) );
	// TODO : Add a command event handler for language changes, that dynamically re-update contents of this window.
}

void Dialogs::PickUserModeDialog::OnOk_Click( wxCommandEvent& evt )
{
	if( g_ApplyState.ApplyAll() )
	{
		Close();
		evt.Skip();
	}
}
