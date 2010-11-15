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

	// To handle media keys
	iInterfaceSelector = CRemConInterfaceSelector::NewL();
	iCoreTarget = CRemConCoreApiTarget::NewL(*iInterfaceSelector, *this);
	iInterfaceSelector->OpenTargetL();

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
	delete iInterfaceSelector;
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

		case EPodOClockCmdHide:
			{
			TApaTask task(iEikonEnv->WsSession());
			task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
			task.SendToBackground();
			}
			break;

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
			if (iAppView->Paused())
				{
				iAppView->Resume();
				}
			else
				{
				iAppView->PlayRandomFileL();
				}
			break;

		case EPodOClockCmdPauseTrack:
			iAppView->Pause();
			break;

		case EPodOClockCmdBackTrack:
			iAppView->BackFiveSeconds();
			break;

		case EPodOClockCmdSkipTrack:
			iAppView->PlayRandomFileL();
			break;

		case EPodOClockCmdStopTrack:
			iAppView->Stop();
			break;

		case EPodOClockCmdTrackInfo:
			iAppView->ShowTrackInfoL();
			break;

		case EPodOClockCmdDeleteFile:
			iAppView->AskDeleteFileL();
			break;

/*		case EPodOClockCmdHelp:
			{
			// Create the header text
			HBufC* title(iEikonEnv->AllocReadResourceLC(R_PODOCLOCK_HELP));
			HBufC* message(iEikonEnv->AllocReadResourceLC(R_PODOCLOCK_HELP_TEXT));
			
			CAknMessageQueryDialog* dlg(new(ELeave) CAknMessageQueryDialog());
			
			// Initialise the dialog
			dlg->PrepareLC(R_PODOCLOCK_ABOUT_BOX);
			dlg->QueryHeading()->SetTextL(*title);
			dlg->SetMessageTextL(*message);
			
			dlg->RunLD();
			
			CleanupStack::PopAndDestroy(2); // title, message
			}
			break;*/

		case EPodOClockCmdAbout:
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
	TBool playing(iAppView->Playing());
	TBool paused(iAppView->Paused());
	if (aResourceId == R_PODOCLOCK_MENU_PANE)
		{
		// Shown when alarm not active
		aMenuPane->SetItemDimmed(EPodOClockCmdSetAlarm, alarmActive);

		// Shown when alarm active
		aMenuPane->SetItemDimmed(EPodOClockCmdResetAlarm,  !alarmActive);
		aMenuPane->SetItemDimmed(EPodOClockCmdRemoveAlarm, !alarmActive);

		// Show when track info available
		aMenuPane->SetItemDimmed(EPodOClockCmdTrackInfo, !iAppView->TrackInfoAvailable());

		// Show when track playing
		aMenuPane->SetItemDimmed(EPodOClockCmdPauseTrack, !playing);

		// Show when track playing or paused
		aMenuPane->SetItemDimmed(EPodOClockCmdBackTrack, !playing && !paused);
		aMenuPane->SetItemDimmed(EPodOClockCmdSkipTrack, !playing && !paused);
		aMenuPane->SetItemDimmed(EPodOClockCmdStopTrack, !playing && !paused);

		// Show when track not playing
		aMenuPane->SetItemDimmed(EPodOClockCmdPlayTrack, playing);

		// Show when current/last file name known
		aMenuPane->SetItemDimmed(EPodOClockCmdDeleteFile, !iAppView->FileNameKnown());
		}
	}

// ----------------------------------------------------------------------------
// MrccatoCommand()
// Receives events (press/click/release) from the following buttons:
// ’Play/Pause’, ’Volume Up’, ’Volume Down’, ’Stop’, ’Rewind’, ’Forward’
// ----------------------------------------------------------------------------
void CPodOClockAppUi::MrccatoCommand(TRemConCoreApiOperationId aOperationId,
									 TRemConCoreApiButtonAction aButtonAct)
	{
	TRequestStatus status;
	switch (aOperationId)
		{
		case ERemConCoreApiVolumeUp:
			{
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
					break;
				case ERemConCoreApiButtonRelease:
					break;
				case ERemConCoreApiButtonClick:
					iAppView->ChangeVolumeL(+1);
					break;
				default:
					break;
				}
			iCoreTarget->VolumeUpResponse(status, KErrNone);
			User::WaitForRequest(status);
			break;
			}
		case ERemConCoreApiVolumeDown:
			{
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
					break;
				case ERemConCoreApiButtonRelease:
					break;
				case ERemConCoreApiButtonClick:
					iAppView->ChangeVolumeL(-1);
					break;
				default:
					break;
				}
			iCoreTarget->VolumeDownResponse(status, KErrNone);
			User::WaitForRequest(status);
			break;
			}
 		case ERemConCoreApiPausePlayFunction:
			{
			LOGTEXT("ERemConCoreApiPausePlayFunction");
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
					break;
				case ERemConCoreApiButtonRelease:
					break;
				case ERemConCoreApiButtonClick:
					iAppView->SelectL();
					break;
				default:
					break;
				}
			iCoreTarget->PausePlayFunctionResponse(status, KErrNone);
			User::WaitForRequest(status);
			break;
			}
		case ERemConCoreApiStop:
			{
			LOGTEXT("ERemConCoreApiStop");
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
					break;
				case ERemConCoreApiButtonRelease:
					break;
				case ERemConCoreApiButtonClick:
					iAppView->Stop();
					break;
				default:
					break;
				}
			iCoreTarget->StopResponse(status, KErrNone);
			User::WaitForRequest(status);
			break;
			}
  		case ERemConCoreApiBackward:
			{
			LOGTEXT("ERemConCoreApiBackward");
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
					break;
				case ERemConCoreApiButtonRelease:
					break;
				case ERemConCoreApiButtonClick:
					iAppView->BackFiveSeconds();
					break;
				default:
					break;
				}
			iCoreTarget->BackwardResponse(status, KErrNone);
			User::WaitForRequest(status);
			break;
			}
 		case ERemConCoreApiForward:
			{
			LOGTEXT("ERemConCoreApiForward");
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
					break;
				case ERemConCoreApiButtonRelease:
					break;
				case ERemConCoreApiButtonClick:
					iAppView->PlayRandomFileL();
					break;
				default:
					break;
				}
			iCoreTarget->ForwardResponse(status, KErrNone);
			User::WaitForRequest(status);
			break;
			}
		case ERemConCoreApiRewind:
			{
			LOGTEXT("ERemConCoreApiRewind");
/*			switch (aButtonAct)
				{
				}
			iCoreTarget->RewindResponse(status, KErrNone);
			User::WaitForRequest(status);*/
			break;
			}
 		case ERemConCoreApiFastForward:
			{
			LOGTEXT("ERemConCoreApiFastForward");
/*			switch (aButtonAct)
				{
				// see above for possible actions
				}
			iCoreTarget->FastForwardResponse(status, KErrNone);
			User::WaitForRequest(status);*/
			break;
			}

		default:
			break;
		}
	}

// End of file
