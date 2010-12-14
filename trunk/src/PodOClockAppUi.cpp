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
#include <BrowserLauncher.h>
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
	//AknsUtils::InitSkinSupportL();
	
	// Create view object
	iAppView = CPodOClockAppView::NewL(ClientRect());
	AddToStackL(iAppView);
	
	// Change the Exit softkey to Hide
	HBufC* hideText(CCoeEnv::Static()->AllocReadResourceLC(R_PODOCLOCK_HIDE));
	TInt pos(Cba()->PositionById(EAknSoftkeyExit));
	Cba()->RemoveCommandFromStack(pos, EAknSoftkeyExit);
	Cba()->SetCommandL(pos, EPodOClockCmdHide, *hideText);
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
	delete iBrowserLauncher;
	}


void CPodOClockAppUi::HandleCommandL(TInt aCommand)
	{
	TRACER_AUTO;
	switch (aCommand)
		{
		case EPodOClockCmdSetAlarm:
		case EPodOClockCmdResetAlarm:
			{
			TBool showDisclaimer(ETrue);
			if (iAppView->AlarmActive())
				{
				showDisclaimer = EFalse;
				}
			
			TBool showAlarmDialog(ETrue);
			if (showDisclaimer)
				{
				CAknQueryDialog* dlg(CAknQueryDialog::NewL());
				HBufC* text(iEikonEnv->AllocReadResourceLC(R_PODOCLOCK_ALARM_DISCLAIMER));
				showAlarmDialog = dlg->ExecuteLD(R_PODOCLOCK_OK_CANCEL_QUERY_DIALOG, *text);
				CleanupStack::PopAndDestroy(text);
				}
			
			if (showAlarmDialog)
				{
				TTime time(iAppView->AlarmTime());
				CAknTimeQueryDialog* dlg(CAknTimeQueryDialog::NewL(time));
				dlg->PrepareLC(R_PODOCLOCK_TIME_QUERY_DIALOG);
				if (dlg->RunLD())
					{
					iAppView->SetAlarmL(time);
					}
				}
			}
			break;

		case EPodOClockCmdRemoveAlarm:
			iAppView->RemoveAlarm();
			break;

		case EPodOClockCmdPlayTrack:
			iAppView->PlayRandomFileL();
			break;

		case EPodOClockCmdDeleteFile:
			iAppView->AskDeleteFileL();
			break;

		case EPodOClockMoreAppsDataQuota:	// intentional fall-through
		case EPodOClockMoreAppsMobbler:		// intentional fall-through
		case EPodOClockMoreAppsSugarSync:	// intentional fall-through
			{
			TBuf<256> url;
			switch (aCommand)
				{
				case EPodOClockMoreAppsDataQuota:
					{
					_LIT(KUrl, "http://code.google.com/p/dataquota/");
					url.Copy(KUrl);
					}
					break;
				case EPodOClockMoreAppsMobbler:
					{
					_LIT(KUrl, "http://code.google.com/p/mobbler/");
					url.Copy(KUrl);
					}
					break;
				case EPodOClockMoreAppsSugarSync:
					{
					_LIT(KUrl, "https://www.sugarsync.com/referral?rf=eoovtb627jrd7");
					url.Copy(KUrl);
					}
					break;
				default:
					break;
				}
			
			if (!iBrowserLauncher)
				{
				iBrowserLauncher = CBrowserLauncher::NewL();
				}
			iBrowserLauncher->LaunchBrowserEmbeddedL(url);
			}
			break;

		case EPodOClockCmdHelp:
			{
			// Create the header text
			HBufC* title(iEikonEnv->AllocReadResourceLC(R_PODOCLOCK_HELP));
			HBufC* help(iEikonEnv->AllocReadResourceLC(R_PODOCLOCK_HELP_TEXT));
			
			CAknMessageQueryDialog* dlg(new(ELeave) CAknMessageQueryDialog());
			
			// Initialise the dialog
			dlg->PrepareLC(R_PODOCLOCK_ABOUT_BOX);
			dlg->QueryHeading()->SetTextL(*title);
			dlg->SetMessageTextL(*help);
			
			dlg->RunLD();
			
			CleanupStack::PopAndDestroy(2, title); // title, help
			}
			break;

		case EPodOClockCmdAbout:
			{
			// Create the header text
			HBufC* title1(iEikonEnv->AllocReadResourceLC(R_PODOCLOCK_ABOUT_TEXT));
			HBufC* version(KVersion().AllocLC());
			
			HBufC* title(HBufC::NewLC(title1->Length() + version->Length()));
			title->Des().Append(*title1);
			title->Des().Append(*version);
			
			CAknMessageQueryDialog* dlg(new(ELeave) CAknMessageQueryDialog());
			
			// Initialise the dialog
			dlg->PrepareLC(R_PODOCLOCK_ABOUT_BOX);
			dlg->QueryHeading()->SetTextL(*title);
			_LIT(KMessage, "2010 Hugo van Kemenade\ncode.google.com/p/podoclock\ntwitter.com/PodOClock");
			dlg->SetMessageTextL(KMessage);
			
			dlg->RunLD();
			
			CleanupStack::PopAndDestroy(3, title1); // title1, version, title
			}
			break;

		case EEikCmdExit:
		case EAknSoftkeyExit:
			Exit();
			break;

		case EPodOClockCmdHide:
			{
			TApaTask task(iEikonEnv->WsSession());
			task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
			task.SendToBackground();
			}
			break;

		default:
			break;
		}
	iAppView->DrawDeferred();
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
		iAppView->DrawNow();
		}
	}


void CPodOClockAppUi::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
	TRACER_AUTO;
	TBool alarmActive(iAppView->AlarmActive());
	if (aResourceId == R_PODOCLOCK_MENU_PANE)
		{
		// Shown when alarm not active
		aMenuPane->SetItemDimmed(EPodOClockCmdSetAlarm, alarmActive);

		// Shown when alarm active
		aMenuPane->SetItemDimmed(EPodOClockCmdResetAlarm,  !alarmActive);
		aMenuPane->SetItemDimmed(EPodOClockCmdRemoveAlarm, !alarmActive);

		// Show when current/last file name known
		aMenuPane->SetItemDimmed(EPodOClockCmdDeleteFile, !iAppView->FileNameKnown());
		}
	}

// End of file

