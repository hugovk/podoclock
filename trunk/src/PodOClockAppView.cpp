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
#include <AknsBasicBackgroundControlContext.h>
#include <AknsDrawUtils.h>
#include <AknNaviDe.h>
#include <AknNaviLabel.h>
#include <AknNoteWrappers.h>
//#include <AknUtils.h>
#include <e32math.h>
//#include <EikSPane.h>
#include <s32file.h>
//#include <StringLoader.h>

#include <PodOClock.rsg>
#include "PodOClockAppView.h"
#include "PodOClockTracer.h"

// CONSTANTS
const TInt KMargin(10);
const TRgb KRgbNow(KRgbYellow);
const TInt KVolume(5);
const TInt KMaxVolume(10);

_LIT(KSettingsFile, "c:settings.dat");
const TInt KSettingsFileVersion(1);
_LIT(KDefaultAlarmTime, "070000."); // "HHMMSS."

CPodOClockAppView* CPodOClockAppView::NewL(const TRect& aRect)
	{
    TRACER_AUTO;
	CPodOClockAppView* self(CPodOClockAppView::NewLC(aRect));
	CleanupStack::Pop(self);
	return self;
	}


CPodOClockAppView* CPodOClockAppView::NewLC(const TRect& aRect)
	{
    TRACER_AUTO;
	CPodOClockAppView* self(new (ELeave) CPodOClockAppView);
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	return self;
	}


void CPodOClockAppView::ConstructL(const TRect& aRect)
	{
    TRACER_AUTO;
	LoadSettingsL();
//	LoadResourceFileTextL();
	iMetaDataFetched = EFalse;
	
	// Create a window for this application view
	CreateWindowL();

    // Create sound player for playing audio files
    iSoundPlayer = CPodOClockSoundPlayer::NewL(*this, iVolume);

	// Grab a seed for the random number generation
	TTime time;
	time.HomeTime();
	iSeed = time.Int64();

	// Set the font
	iFont = iEikonEnv->AnnotationFont();

	// Set the windows size
	SetRect(aRect);

	iBackground = CAknsBasicBackgroundControlContext::NewL(
					KAknsIIDQsnBgAreaMain, Rect(), EFalse);// new a background

	iNaviContainer = static_cast<CAknNavigationControlContainer*>(iEikonEnv
							->AppUiFactory()->StatusPane()
							->ControlL(TUid::Uid(EEikStatusPaneUidNavi)));
	iNaviLabelDecorator = iNaviContainer->CreateNavigationLabelL();
	iNaviContainer->PushL(*iNaviLabelDecorator);
	
	DoChangePaneTextL();
	
	// Activate the window, which makes it ready to be drawn
	ActivateL();
	iRectWidth = Size().iWidth - (2 * KMargin);
	}


CPodOClockAppView::CPodOClockAppView()
	{
    TRACER_AUTO;
	// No implementation required
	}


CPodOClockAppView::~CPodOClockAppView()
	{
    TRACER_AUTO;
	delete iAlarmTimer;
	delete iBackground;
    delete iSoundPlayer;
//	delete iSomeText;
	
	iNaviContainer->Pop(iNaviLabelDecorator);
	delete iNaviLabelDecorator;
	}

/*void CPodOClockAppView::LoadResourceFileTextL()
	{
    TRACER_AUTO;
//	iSomeText = StringLoader::LoadL(R_PODOCLOCK_TEXT);
	}
*/

void CPodOClockAppView::DrawText(const TDesC& aText, const TInt& aY, 
								 const TRgb& aPenColor) const
	{
    TRACER_AUTO;
	CWindowGc& gc(SystemGc());
	gc.SetDrawMode(CGraphicsContext::EDrawModePEN);
	gc.SetPenColor(aPenColor);
	
	TBidiText* bidi(TBidiText::NewL(aText, 1));
	bidi->WrapText(iRectWidth, *iFont, NULL);
	bidi->DrawText(gc, TPoint(KMargin, aY));
	delete bidi;
	}


void CPodOClockAppView::Draw(const TRect& /*aRect*/) const
	{
    TRACER_AUTO;
	// Note that the whole screen is drawn every time, 
	// so aRect parameter is ignored

	// Get the standard graphics context
	CWindowGc& gc(SystemGc());

	 // Draw background
	MAknsSkinInstance* skin(AknsUtils::SkinInstance());
	MAknsControlContext* cc(AknsDrawUtils::ControlContext(this));
	AknsDrawUtils::Background(skin, cc, this, gc, Rect());

	TRgb rgbTheme;
	AknsUtils::GetCachedColor(skin, rgbTheme, KAknsIIDQsnTextColors, 
											EAknsCIQsnTextColorsCG6);

	gc.UseFont(iFont);
	gc.SetPenColor(KRgbNow);
	gc.SetPenStyle(CGraphicsContext::ESolidPen);

	TInt y(20);
	DrawText(_L("Play a random podcast"), y, rgbTheme);
	y += 20;
	DrawText(_L("at a given time"), y, rgbTheme);
	y += 20;
	y += 20;
	
	TBuf<KMaxChars> alarmActive;
	if (AlarmActive())
		{
		alarmActive.Append(_L("Alarm set: "));

		TBuf<50> timeString;
		HBufC* timeFormatString(CEikonEnv::Static()->
            AllocReadResourceLC(R_QTN_TIME_USUAL));
		iAlarmTime.FormatL(timeString, *timeFormatString);
		CleanupStack::PopAndDestroy(timeFormatString); 
		alarmActive.Append(timeString);
		DrawText(alarmActive, y, KRgbGreen);
		}
	else
		{
		alarmActive.Append(_L("No alarm set"));
		DrawText(alarmActive, y, rgbTheme);
		}
	y += 20;
	y += 20;

	if (iSoundPlayer->PlayerState() == ECEPlaying)
		{
		if (!iMetaDataFetched)
			{
			iSoundPlayer->GetMetaDataL(iTitle, iAlbum, iArtist, 
									   iYear, iGenre);
			iMetaDataFetched = ETrue;
			}
		DrawText(iTitle, y, KRgbGreen);
		y += 20;
		DrawText(iAlbum, y, KRgbGreen);
		y += 20;
		DrawText(iArtist, y, KRgbGreen);
		y += 20;
		TBuf<KMaxChars> yearAndGenre(iYear);
		if (iYear.Length() > 0 && iGenre.Length() > 0)
			{
			yearAndGenre.Append(_L(", "));
			}
		yearAndGenre.Append(iGenre);
		
		DrawText(yearAndGenre, y, KRgbGreen);
		y += 20;
		y += 20;

		TBuf<KMaxChars> volume(_L("Volume: "));
		volume.AppendNum(iVolume);
		DrawText(volume, y, rgbTheme);
		y += 20;
		}

	gc.DiscardFont();
	}


void CPodOClockAppView::SizeChanged()
	{
    TRACER_AUTO;
	DrawDeferred();
	}


TTypeUid::Ptr CPodOClockAppView::MopSupplyObject(TTypeUid aId)
	{
    TRACER_AUTO;
	if(aId.iUid == MAknsControlContext::ETypeId && iBackground)
		{
		return MAknsControlContext::SupplyMopObject(aId, iBackground);
		}
	return CCoeControl::MopSupplyObject(aId);
	}


TKeyResponse CPodOClockAppView::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
											   TEventCode /*aType*/)
	{
    TRACER_AUTO;
	TKeyResponse response(EKeyWasNotConsumed);
	
	switch (aKeyEvent.iCode)
		{
		 // Play or stop
		case EKeyDevice3: // OK key
			SelectL();
			response = EKeyWasConsumed;
			break;

		// Volume down
		case '*':
		case EKeyDownArrow:
		case EKeyLeftArrow:
			iVolume = iSoundPlayer->ChangeVolume(-1);
			SaveSettingsL();
			response = EKeyWasConsumed;
			break;

		// Volume up
		case '#':
		case EKeyUpArrow:
			iVolume = iSoundPlayer->ChangeVolume(+1);
			SaveSettingsL();
			response = EKeyWasConsumed;
			break;

		// Skip
		case EKeyRightArrow:
			PlayRandomFileL();
			response = EKeyWasConsumed;
			break;

		// Remove alarm, or delete file
		case EKeyBackspace: // C key
			{
			// Is alarm active?
				// Yes. Remove alarm?
					// Yes. Remove alarm. End.
					// No. Ask about file.
				// No. Ask about file.

			// Ask about file.
			// Delete file?
				// Yes. Delete it. End.
				// No. End.
			
			TBool removeAlarm(EFalse);
			if (AlarmActive())
				{
				CAknQueryDialog* dlg(CAknQueryDialog::NewL());
				if (dlg->ExecuteLD(R_PODOCLOCK_YES_NO_QUERY_DIALOG, 
												 _L("Remove alarm?")))
					{
					RemoveAlarm();
					}
				}

				if (!removeAlarm && iCurrentFileName.Length() > 0)
					{

					TBuf<KMaxChars> question(_L("Delete "));
					question.Append(iTitle);
					question.Append(_L("?"));
					CAknQueryDialog* dlg(CAknQueryDialog::NewL());
					if (dlg->ExecuteLD(R_PODOCLOCK_YES_NO_QUERY_DIALOG, 
													 question))
						{
						DeleteFileL();
						}
					}
			}
			
			break;

			default:
			break;
		}
	
	if (response == EKeyWasConsumed)
		{
		DrawDeferred();
		}
	
	return response;
	}


void CPodOClockAppView::SetAlarmL(const TTime aTime)
	{
    TRACER_AUTO;
	TDateTime alarmDateTime(aTime.DateTime());
	TTime now;
	now.HomeTime();
	
	// Set the date to today, keep the time
	alarmDateTime.SetYear(now.DateTime().Year());
	alarmDateTime.SetMonth(now.DateTime().Month());
	alarmDateTime.SetDay(now.DateTime().Day());

	// TTime from TDateTime
	TTime alarmTime(alarmDateTime);
	
	// If the time was earlier today, it must be for tomorrow
	if (alarmTime < now)
		{
		alarmTime += (TTimeIntervalDays)1;
		}
	
	iAlarmTime = alarmTime;
	if (!iAlarmTimer)
		{
		iAlarmTimer = CPodOClockTimer::NewL(EPriorityLow, *this);
		}
	
	iAlarmTimer->At(iAlarmTime);
#ifdef _DEBUG
	CEikonEnv::Static()->InfoMsg(_L("Alarm set"));
#endif
	
	TTimeIntervalMinutes minutesInterval;
	iAlarmTime.MinutesFrom(now, minutesInterval);
	TInt hours(minutesInterval.Int() / 60);
	TInt minutes(minutesInterval.Int() - (hours * 60));
	CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));
	
/*	TInt resourceId(R_PODOCLOCK_ALARM_HOURS_MINUTES);
	if (hours == 1 && minutes == 1)
		{
		resourceId = R_PODOCLOCK_ALARM_HOUR_MINUTE;
		}
	else if (hours == 1 && minutes != 1)
		{
		resourceId = R_PODOCLOCK_ALARM_HOUR_MINUTES;
		}
	else if (hours != 1 && minutes == 1)
		{
		resourceId = R_PODOCLOCK_ALARM_HOURS_MINUTE;
		}
	*/
	TBuf<KMaxChars> confirmationText;
	//confirmationText.Format(iResourceReader->ResourceL(resourceId), hours, minutes);
	confirmationText.Format(_L("Time left until alarm: %U hours and %U minutes"), hours, minutes);
	note->ExecuteLD(confirmationText);
	
	SaveSettingsL();
	}


void CPodOClockAppView::RemoveAlarm()
	{
    TRACER_AUTO;
	if (AlarmActive())
		{
		iAlarmTimer->Cancel();
		CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));
		note->ExecuteLD(_L("Alarm removed"));
		}
	}


void CPodOClockAppView::DeleteFileL()
	{
    TRACER_AUTO;
	StopL();
	TInt error(CCoeEnv::Static()->FsSession().Delete(iCurrentFileName));

	TBuf<KMaxChars> confirmation(iTitle);
	CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));
	if (error == KErrNone)
		{
		confirmation.Append(_L(" deleted"));
		iCurrentFileName = _L("");
		}
	else
		{
		confirmation.Append(_L(" not deleted"));
		}
	note->ExecuteLD(confirmation);
	}


void CPodOClockAppView::PlayRandomFileL()
	{
    TRACER_AUTO;
	StopL();
	
#ifdef __WINS__
	_LIT(KDirName, "C:\\podcasts\\sounds\\");
	_LIT(KFileSpec,"C:\\podcasts\\sounds\\*.mp3");
#else
	_LIT(KDirName, "E:\\podcasts\\sounds\\");
	_LIT(KFileSpec,"E:\\podcasts\\sounds\\*.mp3");
#endif

	// Connect to the file server
	RFs fileSession;
	fileSession.Connect();
	CleanupClosePushL(fileSession);

	// Get the file list, sorted by name
	// (Leave if an error occurs)
	CDir* dirList;
	User::LeaveIfError(
		fileSession.GetDir(KFileSpec,
						   KEntryAttMaskSupported,
						   ESortNone,
						   dirList));
	CleanupStack::PushL(dirList);
	
	TInt maxVal(dirList->Count());
	TInt random(Math::Rand(iSeed) % maxVal);

/*	TBuf<KMaxChars> thing;
	thing.AppendNum(random);
	thing.Append(_L("/"));
	thing.AppendNum(dirList->Count());
	DrawText(thing, 30, KRgbBlue);
*/

	TBuf<KMaxFileName> fileName((*dirList)[random].iName);
	TBuf<KMaxFileName> totalPath(KDirName);
	totalPath.Append(fileName);
	PlayL(totalPath);
	iCurrentFileName = totalPath;

	// Close the connection with the file server
	// and destroy dirList
	CleanupStack::PopAndDestroy(2); // fileSession, dirList
	DrawDeferred();
	}


void CPodOClockAppView::SelectL()
	{
    TRACER_AUTO;
	// Start playback if the player is not playing
	if (iSoundPlayer->PlayerState() != ECEPlaying)
		{
		PlayRandomFileL();
		}
	// Stop current playback if the player is playing
	else
		{
		StopL();
		}
	}


void CPodOClockAppView::PlayL(const TDesC& aFileName)
	{
    TRACER_AUTO;
	// Opens the file and starts playback
	iSoundPlayer->StartPlaybackL(aFileName);
	}

	
void CPodOClockAppView::StopL()
	{
    TRACER_AUTO;
	// Stop playback
	if (iSoundPlayer->PlayerState() == ECEPlaying)
		{
		iSoundPlayer->StopPlayback();
		}
	iMetaDataFetched = EFalse;
	}


void CPodOClockAppView::LoadSettingsL()
	{
    TRACER_AUTO;
	TInt volume(KVolume);
//	iVolume = KVolume;
	TBool alarmOn(EFalse);
	TTime alarmTime(KDefaultAlarmTime);
	
	RFile file;
	CleanupClosePushL(file);
	TInt openError(file.Open(CCoeEnv::Static()->FsSession(), 
								KSettingsFile, EFileRead));
	
	if (openError == KErrNone)
		{
		RFileReadStream readStream(file);
		CleanupClosePushL(readStream);
		
		readStream.ReadInt32L(); // Ignore settings version
		volume = readStream.ReadInt32L();
		alarmOn = readStream.ReadInt8L();

		TUint32 high(readStream.ReadInt32L());
		TUint32 low(readStream.ReadInt32L());
		alarmTime = TTime(MAKE_TINT64(high, low));

		CleanupStack::PopAndDestroy(&readStream);
		}
	
	CleanupStack::PopAndDestroy(&file);
	
	iVolume = volume;
	iAlarmTime = alarmTime;

	if (alarmOn)
		{
//		SetAlarmL(iAlarmTime); // TODO
		}
	}


void CPodOClockAppView::SaveSettingsL()
	{
    TRACER_AUTO;
	CCoeEnv::Static()->FsSession().MkDirAll(KSettingsFile);
	
	RFile file;
	CleanupClosePushL(file);
	TInt replaceError(file.Replace(CCoeEnv::Static()->FsSession(), 
								   KSettingsFile, EFileWrite));
	
	if (replaceError == KErrNone)
		{
		RFileWriteStream writeStream(file);
		CleanupClosePushL(writeStream);
		
		writeStream.WriteInt32L(KSettingsFileVersion);
		writeStream.WriteInt32L(iVolume);
		writeStream.WriteInt8L(AlarmActive());
		writeStream.WriteInt32L(I64HIGH(iAlarmTime.Int64()));
		writeStream.WriteInt32L(I64LOW(iAlarmTime.Int64()));
		
		CleanupStack::PopAndDestroy(&writeStream);
		}
	
	CleanupStack::PopAndDestroy(&file);
	}


void CPodOClockAppView::DoChangePaneTextL() const
	{
    TRACER_AUTO;
	TBuf<KMaxChars> dateText;
	
	TTime time;
	time.HomeTime(); // set time to home time
	_LIT(KDateFormat, "%/0%1%/1%2%/2%3%/3");
	time.FormatL(dateText, KDateFormat); 
	
	TBuf<KMaxChars> stateText(KVersion);
	_LIT(KSpace, " - ");
	stateText.Append(KSpace);
	stateText.Append(dateText);
	
	static_cast<CAknNaviLabel*>(iNaviLabelDecorator->DecoratedControl())->
														SetTextL(stateText);
	iNaviContainer->Pop();
	iNaviContainer->PushL(*iNaviLabelDecorator);
	}


void CPodOClockAppView::TimerExpiredL(TAny* aTimer, TInt aError)
	{
    TRACER_AUTO;
#ifdef _DEBUG
	CEikonEnv::Static()->InfoMsg(_L("Timer expired!"));
#endif

	if (aTimer == iAlarmTimer && aError == KErrNone)
		{
		// Trigger
		CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));
		note->ExecuteLD(_L("Alarm!"));
		PlayRandomFileL();
		SaveSettingsL();
		}
	
	// When the system time changes, At() timers will complete immediately with
	// KErrAbort. This can happen either if the user changes the time, or if 
	// the phone is set to auto-update with the network operator time.
	else if (aTimer == iAlarmTimer && aError == KErrAbort)
		{
		iAlarmTimer->At(iAlarmTime);
		}
	else if (aTimer == iAlarmTimer && aError == KErrUnderflow)
		{
		iAlarmTimer->At(iAlarmTime);
		}
	}

void CPodOClockAppView::PlayerStartedL()
	{
    TRACER_AUTO;
	DrawDeferred();
	}

// End of file
