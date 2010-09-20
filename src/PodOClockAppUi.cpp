/*
Pod O'Clock for S60 phones.
http://code.google.com/p/podoclock/
Copyright (C) 2010  Hugo van Kemenade

This file is part of Pod O'Clock.

Pod O'Clock is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Pod O'Clock is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Pod O'Clock.  If not, see <http://www.gnu.org/licenses/>.
*/

// INCLUDE FILES
#include <AknMessageQueryDialog.h>
#include <PodOClock.rsg>

#include "PodOClock.hrh"
#include "PodOClockAppUi.h"
#include "PodOClockAppView.h"
#include "PodOClockTracer.h"

void CPodOClockAppUi::ConstructL()
	{
    TRACER_AUTO;
	// Initialise app UI with standard value
	BaseConstructL(CAknAppUi::EAknEnableSkin);
	
	// Create view object
	iAppView = CPodOClockAppView::NewL(ClientRect());
	AddToStackL(iAppView);

	// Change the Exit softkey to Hide
	HBufC* hideText(CCoeEnv::Static()->AllocReadResourceLC(R_PODOCLOCK_HIDE));
	TInt pos(Cba()->PositionById(EAknSoftkeyExit));
	Cba()->RemoveCommandFromStack(pos, EAknSoftkeyExit);
	Cba()->SetCommandL(pos, EPodOClockHide, *hideText);
	CleanupStack::PopAndDestroy(hideText);
	}


CPodOClockAppUi::CPodOClockAppUi()
	{
    TRACER_AUTO;
	// No implementation required
	}


CPodOClockAppUi::~CPodOClockAppUi()
	{
    TRACER_AUTO;
	if (iAppView)
		{
		RemoveFromStack(iAppView);
		delete iAppView;
		iAppView = NULL;
		}
	}


void CPodOClockAppUi::HandleCommandL(TInt aCommand)
	{
    TRACER_AUTO;
	switch(aCommand)
		{
		case EEikCmdExit:
		case EAknSoftkeyExit:
			Exit();
			break;

		case EPodOClockHide:
			{
			TApaTask task(iEikonEnv->WsSession());
			task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
			task.SendToBackground();
			}
			break;

		case EPodOClockSetAlarm:
			{
			CAknQueryDialog* dlg(CAknQueryDialog::NewL());
			if (dlg->ExecuteLD(R_PODOCLOCK_OK_CANCEL_QUERY_DIALOG, 
_L("Alarms are set at owner's risk. The makers of Pod O'Clock cannot be held responsible for tardiness.")))
				{
				TTime time(iAppView->AlarmTime());
				CAknTimeQueryDialog* dlg(CAknTimeQueryDialog::NewL(time));
				dlg->PrepareLC(R_AVKON_DIALOG_QUERY_VALUE_TIME);
				if (dlg->RunLD())
					{
					iAppView->SetAlarmL(time);
					}
				}
			}
			break;

		case EPodOClockRemoveAlarm:
			iAppView->RemoveAlarm();
			break;

		case EPodOClockAbout:
			{
			// Create the header text
			HBufC* title1(iEikonEnv->AllocReadResourceLC(R_PODOCLOCK_ABOUT_TEXT));
			HBufC* title2(KVersion().AllocLC());
			
			HBufC* title(HBufC::NewLC(title1->Length() + title2->Length()));
			title->Des().Append(*title1);
			title->Des().Append(*title2);
			
			CAknMessageQueryDialog* dlg(new(ELeave) CAknMessageQueryDialog());
			
			// Initialise the dialog
			dlg->PrepareLC(R_PODOCLOCK_ABOUT_BOX);
			dlg->QueryHeading()->SetTextL(*title);
			_LIT(KMessage, "(c) 2010 Hugo van Kemenade\ncode.google.com/p/podoclock\ntwitter.com/PodOClock");
			dlg->SetMessageTextL(KMessage);
			
			dlg->RunLD();
			
			CleanupStack::PopAndDestroy(3); // title, title1, title2
			}
			break;

		default:
			break;
		}
	}


void CPodOClockAppUi::HandleResourceChangeL(TInt aType)
	{
    TRACER_AUTO;
	// Also call the base class
	CAknAppUi::HandleResourceChangeL(aType);
	if (aType == KEikDynamicLayoutVariantSwitch)
		{
		if (iAppView)
			{
			iAppView->SetRect(ClientRect());
			}
		}
	}


void CPodOClockAppUi::HandleForegroundEventL(TBool aForeground)
	{
    TRACER_AUTO;
	CAknAppUi::HandleForegroundEventL(aForeground);
	if (aForeground)
		{
		//iAppView->UpdateValuesL();
		iAppView->DrawNow();
		}
	}


void CPodOClockAppUi::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
    TRACER_AUTO;
	if (aResourceId == R_PODOCLOCK_MENU_PANE)
		{
		aMenuPane->SetItemDimmed(EPodOClockRemoveAlarm, !iAppView->AlarmActive());
		}
	}

// End of file
