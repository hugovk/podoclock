/*
Pod O'Clock for Symbian phones.
http://code.google.com/p/podoclock/
Copyright (C) 2010, 2011  Hugo van Kemenade

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
#include <apgcli.h>
#include <BrowserLauncher.h>
#ifdef __OVI_SIGNED__
#include <SWInstApi.h>
#include <SWInstDefs.h>
#endif

#include "PodOClock.hrh"
#include "PodOClock.rsg.h"
#include "PodOClockAppUi.h"
#include "PodOClockAppView.h"
#include "PodOClockTracer.h"
#include "PodOClockUids.h"

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

#ifndef __OVI_SIGNED__
	LaunchOviSignedVersionL();
#endif
	}


CPodOClockAppUi::CPodOClockAppUi()
#ifdef __OVI_SIGNED__
	: iUninstallAttempted(EFalse)
#endif
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

		case EPodOClockCmdPlayRandomTrack:
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
			
			OpenWebBrowserL(url);
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
			_LIT(KMessage, "(C) 2010-2011 Hugo van Kemenade\ncode.google.com/p/podoclock\ntwitter.com/PodOClock");
			dlg->SetMessageTextL(KMessage);
			
			dlg->RunLD();
			
			CleanupStack::PopAndDestroy(3, title1); // title1, version, title
			}
			break;

		case EEikCmdExit:
		case EAknSoftkeyExit:
			HandleCommandL(EPodOClockCmdHide);
			Exit();
			break;

		case EPodOClockCmdHide:
			{
			TApaTask task(iEikonEnv->WsSession());
			task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
			task.SendToBackground();
#ifdef __OVI_SIGNED__
			UninstallSelfSignedVersionL();
#endif
			}
			break;

		default:
			break;
		}
	iAppView->DrawDeferred();
	}


void CPodOClockAppUi::OpenWebBrowserL(const TDesC& aUrl)
	{
	(void)aUrl;
#ifdef __OVI_SIGNED__
	// Find the default browser, on S^1/S^3 it may be a 3rd party browser
	RApaLsSession lsSession;
	User::LeaveIfError(lsSession.Connect());
	CleanupClosePushL(lsSession); 
	_LIT8(KMimeDataType, "application/x-web-browse");
	TDataType mimeDataType(KMimeDataType);
	TUid handlerUid;
	// Get the default application UID for "application/x-web-browse"
	lsSession.AppForDataType(mimeDataType, handlerUid);
	
	if (handlerUid.iUid == 0)
		{
		// For S60 3.x
		const TUid KBrowserUid = {0x10008D39};
		handlerUid = KBrowserUid;
		}
	
	TApaTaskList taskList(CEikonEnv::Static()->WsSession());
	TApaTask task(taskList.FindApp(handlerUid));
	if(task.Exists())
		{
		task.BringToForeground();
		HBufC8* param8(HBufC8::NewLC(aUrl.Length()));
		param8->Des().Append(aUrl);
		task.SendMessage(TUid::Uid(0), *param8); // UID not used
		CleanupStack::PopAndDestroy(param8);
		}
	else
		{
		TThreadId thread;
		User::LeaveIfError(lsSession.StartDocument(aUrl, handlerUid, thread));
		}
	CleanupStack::PopAndDestroy(&lsSession);
#else // !__OVI_SIGNED__
#ifndef __WINS__
	if (!iBrowserLauncher)
		{
		iBrowserLauncher = CBrowserLauncher::NewL();
		}
	iBrowserLauncher->LaunchBrowserEmbeddedL(aUrl);
#endif
#endif // __OVI_SIGNED__
	}


#ifdef __OVI_SIGNED__
void CPodOClockAppUi::UninstallSelfSignedVersionL()
	{
	TRACER_AUTO;
	if (iUninstallAttempted)
		{
		return;
		}
	else
		{
		iUninstallAttempted = ETrue;
		}
	
	SwiUI::RSWInstLauncher iLauncher; 
	TInt error(iLauncher.Connect());
	LOGINT(error);
	 if (KErrNone == error)
		{
		SwiUI::TInstallOptions options;
		SwiUI::TInstallOptionsPckg optionsPckg;  
		options.iKillApp = SwiUI::EPolicyAllowed;
		optionsPckg = options;  
		error = iLauncher.SilentUninstall(
			TUid::Uid(KUidSelfSigned), 
			optionsPckg,
			SwiUI::KSisxMimeType);
		LOGINT(error);
		}
	iLauncher.Close();
	}
#endif // __OVI_SIGNED__


#ifndef __OVI_SIGNED__
void CPodOClockAppUi::LaunchOviSignedVersionL()
	{
	TRACER_AUTO;
	RApaLsSession lsSession;
	User::LeaveIfError(lsSession.Connect());
	CleanupClosePushL(lsSession);

	TApaAppInfo appInfo;
	TInt error(lsSession.GetAppInfo(
		appInfo, 
		TUid::Uid(KUidOviSigned)));

	if (KErrNone == error)
		{
		CApaCommandLine* cmdLine(CApaCommandLine::NewLC());
		cmdLine->SetExecutableNameL(appInfo.iFullName);
		cmdLine->SetCommandL(EApaCommandRun);
		User::LeaveIfError(lsSession.StartApp(*cmdLine));
		CleanupStack::PopAndDestroy(cmdLine);
		}
	
	CleanupStack::PopAndDestroy(&lsSession);
	
	if (KErrNone == error)
		{
		HandleCommandL(EEikCmdExit);
		}
	}
#endif // !__OVI_SIGNED__


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

