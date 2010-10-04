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
#include <AknMessageQueryDialog.h>
#include <AknNaviDe.h>
#include <AknNaviLabel.h>
#include <AknNoteWrappers.h>
//#include <AknUtils.h>
#include <e32math.h>
//#include <EikSPane.h>
#include <s32file.h>
#include <StringLoader.h>

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
	LoadResourceFileTextL();
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
	delete iAlarmSetText;
	delete iNoAlarmSetText;
	
	iNaviContainer->Pop(iNaviLabelDecorator);
	delete iNaviLabelDecorator;
	}

void CPodOClockAppView::LoadResourceFileTextL()
	{
	TRACER_AUTO;
	iAlarmSetText = StringLoader::LoadL(R_PODOCLOCK_ALARM_SET);
	iNoAlarmSetText = StringLoader::LoadL(R_PODOCLOCK_NO_ALARM_SET);
	}


void CPodOClockAppView::DrawText(const TDesC& aText, const TInt& aY, 
								 const TRgb& aPenColor) const
	{
//	TRACER_AUTO;
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
//	TRACER_AUTO;
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
	if (AlarmActive())
		{
		TBuf<KMaxChars> alarmActive(_L("Alarm set: "));

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
		DrawText(*iNoAlarmSetText, y, rgbTheme);
		}
	y += 20;
	y += 20;

	if (iSoundPlayer->PlayerState() > EPodOClockReadyToPlay)
		{
		TRgb metaDataColour(KRgbGreen);
		if (iSoundPlayer->PlayerState() == EPodOClockPaused)
			{
			metaDataColour = rgbTheme;
			}

		if (!iMetaDataFetched)
			{
			iTitle	= _L("");
			iAlbum	= _L("");
			iArtist   = _L("");
			iYear	 = _L("");
			iComment  = _L("");
			iSoundPlayer->GetMetaDataL(iTitle, iAlbum, iArtist, 
									   iYear, iComment);
			iMetaDataFetched = ETrue;
			}
		
		// Use filename if no title
		if (iTitle.Length() == 0)
			{
			TParse parse;
			parse.Set(iCurrentFileName, NULL, NULL);
			iTitle = parse.NameAndExt();
			}
		DrawText(iTitle, y, metaDataColour);
		y += 20;

		// Don't show album if same as title
		if (iTitle.Compare(iAlbum) != 0)
			{
			DrawText(iAlbum, y, metaDataColour);
			y += 20;
			}
		
		// Show "Artist (year)"
		TBuf<KMaxChars> artistAndYear(iArtist);
		if (iYear.Length() > 0)
			{
			artistAndYear.Append(_L(" ("));
			artistAndYear.Append(iYear);
			artistAndYear.Append(_L(")"));
			}
		DrawText(artistAndYear, y, metaDataColour);
		y += 20;
		
		DrawText(iComment, y, metaDataColour);
		y += 20;

		TBuf<KMaxChars> countOfTotal;
		countOfTotal.AppendNum(iCurrentFileNumber);
		countOfTotal.Append(_L("/"));
		countOfTotal.AppendNum(iNumberOfFiles);
		DrawText(countOfTotal, y, rgbTheme);
		y += 20;
		y += 20;

		TBuf<KMaxChars> volume(_L("Volume: "));
		volume.AppendNum(iVolume);
		DrawText(volume, y, rgbTheme);
		}
	else // not playing/paused
		{
					DrawText(_L("OK: Play or pause"), y, rgbTheme);
		y += 20;	DrawText(_L("Left: Back 5 seconds"), y, rgbTheme);
		y += 20;	DrawText(_L("Right: Skip"), y, rgbTheme);
		y += 20;	DrawText(_L("Up: Info"), y, rgbTheme);
		y += 20;	DrawText(_L("Down: Stop"), y, rgbTheme);
		y += 20;	DrawText(_L("*: Volume down"), y, rgbTheme);
		y += 20;	DrawText(_L("#: Volume up"), y, rgbTheme);
		y += 20;	DrawText(_L("C: Remove alarm, or delete file"), y, rgbTheme);
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
	TKeyResponse response(EKeyWasConsumed);
	
	switch (aKeyEvent.iCode)
		{
		 // Play or stop
		case EKeyDevice3: // OK key
			SelectL();
			break;

		// Volume down
		case '*':
			iVolume = iSoundPlayer->ChangeVolume(-1);
			SaveSettingsL();
			break;

		// Volume up
		case '#':
			iVolume = iSoundPlayer->ChangeVolume(+1);
			SaveSettingsL();
			break;

		// Back 5 seconds
		case EKeyLeftArrow:
			{
			TTimeIntervalMicroSeconds position(0);
			TInt error(iSoundPlayer->GetPosition(position));
			if (error == KErrNone)
				{
				TUint difference(5 * 1000000);
				TUint newPosition(0);
				if (difference < position.Int64())
					{
					newPosition = position.Int64() - difference;
					}
				
				iSoundPlayer->SetPosition(newPosition);
				}
			}
			break;

		// Skip
		case EKeyRightArrow:
			PlayRandomFileL();
			break;

		// Show info
		case EKeyUpArrow:
			if ((iSoundPlayer->PlayerState() > EPodOClockReadyToPlay) &&
				(iComment.Length() > 0))
				{
				// Create the header text
				CAknMessageQueryDialog* dlg(new(ELeave) CAknMessageQueryDialog());
				
				// Initialise the dialog
				dlg->PrepareLC(R_PODOCLOCK_ABOUT_BOX);
				dlg->QueryHeading()->SetTextL(iTitle);
				dlg->SetMessageTextL(iComment);
				
				dlg->RunLD();
				}
			break;

		// Stop
		case EKeyDownArrow:
			Stop();
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
				HBufC* text(StringLoader::LoadLC(R_PODOCLOCK_REMOVE_ALARM));
				if (dlg->ExecuteLD(R_PODOCLOCK_YES_NO_QUERY_DIALOG, *text))
					{
					RemoveAlarm();
					}
				CleanupStack::PopAndDestroy(text);
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
			response = EKeyWasNotConsumed;
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
	
	TInt resourceId(R_PODOCLOCK_ALARM_HOURS_MINUTES);
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
	

	CArrayFix<TInt>* integers(new (ELeave) CArrayFixFlat<TInt>(2));
	CleanupStack::PushL(integers);
	integers->AppendL(hours);
	integers->AppendL(minutes);
	HBufC* confirmationText(StringLoader::LoadLC(resourceId, *integers));

	note->ExecuteLD(*confirmationText);
	CleanupStack::PopAndDestroy(confirmationText); 
	CleanupStack::PopAndDestroy(integers);

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
	Stop();
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
	Stop();
	
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
	LOGTEXT("Get the file list, sorted by name");
	CDir* dirList;
	User::LeaveIfError(
		fileSession.GetDir(KFileSpec,
						   KEntryAttMaskSupported,
						   ESortNone,
						   dirList));
	CleanupStack::PushL(dirList);
	
	TInt bytes(0);
	TInt attemptsLeft(10);
	TInt maxVal(dirList->Count());
	LOGTEXT("maxVal");			LOGINT(maxVal);
	TEntry entry;
	LOGTEXT("bytes");			LOGINT(bytes);

	while ((bytes == 0) && (attemptsLeft > 0))
		{
		// Find a random file
		TInt random(Math::Rand(iSeed) % maxVal);
		LOGTEXT("random");			LOGINT(random);
		
		TBuf<KMaxFileName> fileName((*dirList)[random].iName);
		TBuf<KMaxFileName> totalPath(KDirName);
		totalPath.Append(fileName);
		LOGBUF(totalPath);
		
		// Check it's bigger than 0 bytes
		bytes = (*dirList)[random].iSize;
		LOGTEXT("bytes");			LOGINT(bytes);

		if (bytes > 0)
			{
			iCurrentFileNumber = random;
			iNumberOfFiles = dirList->Count();
			iCurrentFileName = totalPath;
			LOGTEXT("PlayL");
			PlayL(iCurrentFileName);
			}
		else
			{
			--attemptsLeft;
			LOGTEXT("attemptsLeft");	LOGINT(attemptsLeft);
			}
		}

	// Close the connection with the file server
	// and destroy dirList
	CleanupStack::PopAndDestroy(2); // fileSession, dirList
	DrawDeferred();
	}


void CPodOClockAppView::SelectL()
	{
	TRACER_AUTO;
	switch (iSoundPlayer->PlayerState())
		{
		case EPodOClockPaused:
			Resume();
			break;

		case EPodOClockPlaying:
			Pause();
			break;

		default:
			PlayRandomFileL();
			break;
		}
	}


void CPodOClockAppView::PlayL(const TDesC& aFileName)
	{
	TRACER_AUTO;
	// Opens the file and starts playback
	iSoundPlayer->StartPlaybackL(aFileName);
	}


void CPodOClockAppView::Stop()
	{
	TRACER_AUTO;
	// Stop playback
	if (iSoundPlayer->PlayerState() > EPodOClockReadyToPlay)
		{
		iSoundPlayer->StopPlayback();
		}
	iMetaDataFetched = EFalse;
	}


void CPodOClockAppView::Pause()
	{
	TRACER_AUTO;
	// Pause playback
	if (iSoundPlayer->PlayerState() == EPodOClockPlaying)
		{
		iSoundPlayer->PausePlayback();
		}
	}


void CPodOClockAppView::Resume()
	{
	TRACER_AUTO;
	// Reume playback
	if (iSoundPlayer->PlayerState() == EPodOClockPaused)
		{
		iSoundPlayer->ResumePlayback();
		}
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

/* 	if (alarmOn)
		{
		SetAlarmL(iAlarmTime); // TODO
		}
 */
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
	LOGINT(aError);
#ifdef _DEBUG
	CEikonEnv::Static()->InfoMsg(_L("Timer expired!"));
#endif

	if (aTimer == iAlarmTimer && aError == KErrNone)
		{
		// Trigger
		
		// Turn the screensaver off and backlight on
		User::ResetInactivityTime();

		// Bring to the foreground
		/*TApaTask task(iEikonEnv->WsSession());
		task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
		task.BringToForeground();
		*/
		CEikonEnv::Static()->RootWin().SetOrdinalPosition(0);
		
		// Show note and play file
		HBufC* text(StringLoader::LoadLC(R_PODOCLOCK_ALARM));
		CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));
		note->ExecuteLD(*text);
		CleanupStack::PopAndDestroy(text);
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

void CPodOClockAppView::PlayerStartedL(TInt aError)
	{
	TRACER_AUTO;
	LOGINT(aError);
#ifdef _DEBUG
	if (aError != KErrNone)
		{
		TBuf<KMaxChars> thing(iCurrentFileName);
		thing.Append(_L(" "));
		thing.AppendNum(aError);
		CEikonEnv::Static()->InfoMsg(thing);
		}
#endif

	DrawDeferred();
	}

void CPodOClockAppView::PlayerEndedL()
	{
	TRACER_AUTO;
#ifdef _DEBUG
	CEikonEnv::Static()->InfoMsg(_L("MapcPlayComplete"));
#endif
	DrawDeferred();
	}

// End of file