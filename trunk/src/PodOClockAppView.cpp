/*
Pod O'Clock for S60 phones.
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
#include <AknsBasicBackgroundControlContext.h>
#include <AknsDrawUtils.h>
#include <AknMessageQueryDialog.h>
#include <AknNaviDe.h>
#include <AknNaviLabel.h>
#include <AknNoteWrappers.h>
#include <apmrec.h>
#include <DocumentHandler.h>
#include <e32math.h>
#include <s32file.h>
#include <StringLoader.h>

#ifdef __S60_50__
#include <PodOClock/PodOClockTouchFeedbackInterface.h>
#include <touchfeedback.h>
#endif

#include <PodOClock.mbg>
#include <PodOClock.rsg>
#include "PodOClock.hrh"
#include "PodOClockAppView.h"
#include "PodOClockTracer.h"

// CONSTANTS
const TInt KMargin(10);
const TRgb KRgbNow(KRgbYellow);
const TInt KMaxChars(256);
const TUid KTouchFeedbackImplUid = {0xA89FD1D9};

_LIT(KSettingsFile, "c:settings.dat");
const TInt KSettingsFileVersion(1);
_LIT(KDefaultAlarmTime, "070000."); // "HHMMSS."
_LIT(KWildcard, "*.mp3");
_LIT(KSlash, "/");
_LIT(KQuestionMarkSlash, "?/");
_LIT(KAudioMpeg, "audio/mpeg");

#ifdef __WINS__
	_LIT(KPodcastPath, "C:\\Podcasts\\");
#else
	_LIT(KPodcastPath, "E:\\Podcasts\\");
#endif


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
#ifdef __S60_50__
	TRAP_IGNORE(iFeedback = static_cast<CPodOClockTouchFeedbackInterface*>(REComSession::CreateImplementationL(KTouchFeedbackImplUid, iDtorIdKey)));
#endif

	LoadSettingsL();
	LoadResourceFileTextL();
	
	// Create a window for this application view
	CreateWindowL();

	// Grab a seed for the random number generation
	TTime time;
	time.HomeTime();
	iSeed = time.Int64();

	// Set the font
	iFont = iEikonEnv->TitleFont();

	// Set the windows size
	SetRect(aRect);

	iBackground = CAknsBasicBackgroundControlContext::NewL(
					KAknsIIDQsnBgAreaMain, Rect(), EFalse);

	iNaviContainer = static_cast<CAknNavigationControlContainer*>(iEikonEnv
							->AppUiFactory()->StatusPane()
							->ControlL(TUid::Uid(EEikStatusPaneUidNavi)));
	iNaviLabelDecorator = iNaviContainer->CreateNavigationLabelL();
	iNaviContainer->PushL(*iNaviLabelDecorator);
	
	DoChangePaneTextL();
	
	// Activate the window, which makes it ready to be drawn
	ActivateL();
	SetPositions();
	
	LoadIconL(EMbmPodoclockPlay, iPlayIcon, iPlayMask, iPlayIconSize);
	if (!IsThirdEdition())
		{
		LoadIconL(EMbmPodoclockDelete, iDeleteIcon, iDeleteMask, iDeleteIconSize);
		}
	}


CPodOClockAppView::CPodOClockAppView()
	: iNumberOfFiles(KErrUnknown),
	  iAskRepeat(EFalse)
	{
	TRACER_AUTO;
	// No implementation required
	}


CPodOClockAppView::~CPodOClockAppView()
	{
	TRACER_AUTO;
	delete iPlayIcon;
	delete iPlayMask;
	delete iDeleteIcon;
	delete iDeleteMask;
	delete iAlarmTimer;
	delete iBackground;
	delete iAlarmSetText;
	delete iNoAlarmSetText;
	delete iDeleteFileText;
	
	iNaviContainer->Pop(iNaviLabelDecorator);
	delete iNaviLabelDecorator;

#ifdef __S60_50__
	if (iFeedback)
		{
		delete iFeedback;
		REComSession::DestroyedImplementation(iDtorIdKey);
		}
#endif
	}

void CPodOClockAppView::LoadResourceFileTextL()
	{
	TRACER_AUTO;
	iAlarmSetText = StringLoader::LoadL(R_PODOCLOCK_ALARM_SET);
	iNoAlarmSetText = StringLoader::LoadL(R_PODOCLOCK_NO_ALARM_SET);
	iDeleteFileText = StringLoader::LoadL(R_PODOCLOCK_DELETE_FILE);
	}


void CPodOClockAppView::DrawText(const TDesC& aText, const TInt& aY, 
								 const TRgb& aPenColor) const
	{
//	TRACER_AUTO;
	CWindowGc& gc(SystemGc());
	gc.SetDrawMode(CGraphicsContext::EDrawModePEN);
	gc.SetPenColor(aPenColor);
	
	TInt y(aY + iHalfTextHeightInPixels);
	TBidiText* bidi(TBidiText::NewL(aText, 1));
	bidi->WrapText(iRectWidth, *iFont, NULL);
	bidi->DrawText(gc, TPoint(KMargin, y), 0, CGraphicsContext::ECenter);
	delete bidi;
	}


void CPodOClockAppView::DrawText(const TDesC& aText, const TRect& aRect, 
								 const TRgb& aPenColor) const
	{
//	TRACER_AUTO;
	CWindowGc& gc(SystemGc());
	gc.SetDrawMode(CGraphicsContext::EDrawModePEN);
	gc.SetPenColor(aPenColor);
	
	TInt y(aRect.Center().iY + iHalfTextHeightInPixels);
	TBidiText* bidi(TBidiText::NewL(aText, 1));
	bidi->WrapText(aRect.Width(), *iFont, NULL);
	bidi->DrawText(gc, TPoint(aRect.iTl.iX, y), 0, CGraphicsContext::ECenter);
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

/*	gc.DrawRect(iAlarmTextRect);
	gc.DrawRect(iDeleteAlarmButtonRect);
	gc.DrawRect(iPlayButtonRect);
	gc.DrawRect(iFileNameRect);
	gc.DrawRect(iDeleteFileButtonRect);*/
	
	if (iPlayIcon)
		{
		TRect rect(TPoint(0, 0), iPlayIconSize);
		gc.BitBltMasked(iPlayButtonRect.iTl, iPlayIcon, rect, iPlayMask, EFalse);
		}
	
	if (AlarmActive())
		{
		TBuf<KMaxChars> alarmActive(*iAlarmSetText);

		TBuf<50> timeString;
		HBufC* timeFormatString(CEikonEnv::Static()->
			AllocReadResourceLC(R_QTN_TIME_USUAL));
		iAlarmTime.FormatL(timeString, *timeFormatString);
		CleanupStack::PopAndDestroy(timeFormatString); 
		alarmActive.Append(timeString);
		DrawText(alarmActive, iAlarmTextRect, KRgbGreen);
		
		if (iDeleteIcon)
			{
			TRect rect(TPoint(0, 0), iDeleteIconSize);
			gc.BitBltMasked(iDeleteAlarmButtonRect.iTl, iDeleteIcon, rect, iDeleteMask, EFalse);
			}
		}
	else
		{
		DrawText(*iNoAlarmSetText, iAlarmTextRect, rgbTheme);
		}

	TBuf<KMaxChars> countOfTotal;
	if (FileNameKnown())
		{
		countOfTotal.AppendNum(iCurrentFileNumber);
		countOfTotal.Append(KSlash);
		countOfTotal.AppendNum(iNumberOfFiles);
		DrawText(countOfTotal, iPlayButtonRect.iBr.iY, rgbTheme);
		
		DrawText(iChoppedFileName, iFileNameRect, rgbTheme);

		if (iDeleteIcon)
			{
			TRect rect(TPoint(0, 0), iDeleteIconSize);
			gc.BitBltMasked(iDeleteFileButtonRect.iTl, iDeleteIcon, rect, iDeleteMask, EFalse);
			}
		}
	// No current file, but a total is known
	else if (iNumberOfFiles != KErrUnknown)
		{
		countOfTotal.Copy(KQuestionMarkSlash);
		countOfTotal.AppendNum(iNumberOfFiles);
		DrawText(countOfTotal, iPlayButtonRect.iBr.iY, rgbTheme);
		}

	gc.DiscardFont();
	}


void CPodOClockAppView::LoadIconL(TInt aIndex, CFbsBitmap*& aBitmap, 
									CFbsBitmap*& aMask, TSize& aSize)
    {
    _LIT(KIconsFile, "\\resource\\apps\\podoclock_aif.mif");
    // Create icon from SVG
    AknIconUtils::CreateIconL(aBitmap, aMask, KIconsFile, aIndex, aIndex + 1);
    // Give size
    AknIconUtils::SetSize(aBitmap, aSize);
    }


void CPodOClockAppView::SetPositions()
	{
	TRACER_AUTO;

	TBool portrait(ETrue);
	TRect rect(Rect());
	TInt width(rect.Width());
	TInt height(rect.Height());
	if (width > height)
		{
		portrait = EFalse;
		}

	iRectWidth = Size().iWidth - (2 * KMargin);
	iHalfTextHeightInPixels = iFont->HeightInPixels() / 2;
	
	// The layout is a simple 3x3 grid of unequal sized rectangles.
	// These define the horizontal dividing lines:
	TInt y1(height / 4);
	TInt y2(3 * height / 4);

	// And the vertical:
	TInt x1(y1 / 2);
	TInt x2(width - x1);
	
	// Exception: No touchscreen in third edition so no delete icons.
	// Therefore the layout is a simpler 3 rows.
	if (IsThirdEdition())
		{
		x1 = 0;
		x2 = width;
		}

	iDeleteIconSize = TSize(x1, x1);
	TInt halfDeleteIconWidth(x1 / 2);

	// Top line
	iAlarmTextRect = TRect(TPoint(x1, 0), TPoint(x2, y1));
	iDeleteAlarmButtonRect = TRect(TPoint(iAlarmTextRect.iBr.iX, 
										  iAlarmTextRect.Center().iY - halfDeleteIconWidth),
										  iDeleteIconSize);
	
	// Centre the play icon in the centre line
	TInt shortestEdge(Min(width, y2 - y1));
	iPlayIconSize = TSize(shortestEdge, shortestEdge);
	TInt playX((width - shortestEdge) / 2);
	iPlayButtonRect = TRect(TPoint(playX, y1), iPlayIconSize);
	
	// Lower line
	iFileNameRect = TRect(TPoint(0, y2), TPoint(x2, height));
	iDeleteFileButtonRect = TRect(TPoint(iFileNameRect.iBr.iX, 
										 iFileNameRect.Center().iY - halfDeleteIconWidth),
										 iDeleteIconSize);
	
	AknIconUtils::SetSize(iPlayIcon, iPlayIconSize);
	AknIconUtils::SetSize(iDeleteIcon, iDeleteIconSize);
	}


void CPodOClockAppView::SizeChanged()
	{
	TRACER_AUTO;
	SetPositions();
	//DoChangePaneTextL();
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
		// Play
		case EKeyDevice3: // OK key	// intentional fall through
		case EKeyRightArrow:		// intentional fall through
			PlayRandomFileL();
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
			
			TBool alarmRemoved(AskRemoveAlarmL());
			
			if (!alarmRemoved)
				{
				AskDeleteFileL();
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


void CPodOClockAppView::HandlePointerEventL(const TPointerEvent& aPointerEvent)
	{
	TRACER_AUTO;
	// Check if they have touched any of the buttons.
	// If so, issue a command.
	
	/* TKeyEvent event;
	event.iCode = EKeyNull; */
	
	TInt command(KErrNotFound);

	if (aPointerEvent.iType == TPointerEvent::EButton1Down ||
		aPointerEvent.iType == TPointerEvent::EButton1Up)
		{
		if (aPointerEvent.iType == TPointerEvent::EButton1Down)
			{
			iLastTouchPosition = aPointerEvent.iPosition;
			}
		
		if (iAlarmTextRect.Contains(aPointerEvent.iPosition) &&
			iAlarmTextRect.Contains(iLastTouchPosition))
			{
			command = EPodOClockCmdSetAlarm;
			}
		else if (AlarmActive() &&
				 iDeleteAlarmButtonRect.Contains(aPointerEvent.iPosition) &&
				 iDeleteAlarmButtonRect.Contains(iLastTouchPosition))
			{
			command = EPodOClockCmdAskRemoveAlarm;
			}
		else if (iPlayButtonRect.Contains(aPointerEvent.iPosition) &&
				 iPlayButtonRect.Contains(iLastTouchPosition))
			{
			command = EPodOClockCmdPlayRandomTrack;
			}
		else if (FileNameKnown() &&
				 iFileNameRect.Contains(aPointerEvent.iPosition) &&
				 iFileNameRect.Contains(iLastTouchPosition))
			{
			command = EPodOClockCmdPlayLastTrack;
			}
		else if (FileNameKnown() &&
				 iDeleteFileButtonRect.Contains(aPointerEvent.iPosition) &&
				 iDeleteFileButtonRect.Contains(iLastTouchPosition))
			{
			command = EPodOClockCmdAskDeleteFile;
			}

		if (command != KErrNotFound && iFeedback)
			{
#ifdef __S60_50__
			iFeedback->InstantFeedback(ETouchFeedbackBasic);
#endif
			}
		}

	if (aPointerEvent.iType == TPointerEvent::EButton1Up)
		{
		switch (command)
			{
			case EPodOClockCmdSetAlarm:
				iAvkonAppUi->HandleCommandL(EPodOClockCmdSetAlarm);
				break;
			case EPodOClockCmdAskRemoveAlarm:
				AskRemoveAlarmL();
				break;
			case EPodOClockCmdPlayRandomTrack:
				PlayRandomFileL();
				break;
			case EPodOClockCmdPlayLastTrack:
				LaunchFileEmbeddedL(iCurrentFileName);
				break;
			case EPodOClockCmdAskDeleteFile:
				AskDeleteFileL();
				break;
			default:
				break;
			}
		}
	
	CCoeControl::HandlePointerEventL(aPointerEvent);
	DrawDeferred();
	}


void CPodOClockAppView::SetAlarmL(const TTime aTime, const TBool aShowConfirmation)
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
	
	if (aShowConfirmation)
		{
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
		}
	
	DrawDeferred();
	SaveSettingsL();
	}


void CPodOClockAppView::AskRepeatAlarmL()
	{
	if (iAskRepeat)
		{
		iAskRepeat = EFalse;
		CAknQueryDialog* dlg(CAknQueryDialog::NewL());
		HBufC* text(StringLoader::LoadLC(R_PODOCLOCK_SAME_TIME_TOMORROW));
		if (dlg->ExecuteLD(R_PODOCLOCK_YES_NO_QUERY_DIALOG, *text))
			{
			SetAlarmL(iAlarmTime);
			}
		CleanupStack::PopAndDestroy(text);
		}
	}


TBool CPodOClockAppView::AskRemoveAlarmL()
	{
	TRACER_AUTO;
	TBool removed(EFalse);
	if (AlarmActive())
		{
		CAknQueryDialog* dlg(CAknQueryDialog::NewL());
		HBufC* text(StringLoader::LoadLC(R_PODOCLOCK_REMOVE_ALARM_QUERY));
		if (dlg->ExecuteLD(R_PODOCLOCK_YES_NO_QUERY_DIALOG, *text))
			{
			RemoveAlarm();
			removed = ETrue;
			}
		CleanupStack::PopAndDestroy(text);
		}
	return removed;
	}


void CPodOClockAppView::RemoveAlarm()
	{
	TRACER_AUTO;
	if (AlarmActive())
		{
		iAlarmTimer->Cancel();
		HBufC* text(StringLoader::LoadLC(R_PODOCLOCK_ALARM_REMOVED));
		CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));
		note->ExecuteLD(*text);
		CleanupStack::PopAndDestroy(text);
		SaveSettingsL();
		}
	}


void CPodOClockAppView::AskDeleteFileL()
	{
	TRACER_AUTO;
	if (FileNameKnown())
		{
		HBufC* format(
			CEikonEnv::Static()->AllocReadResourceLC(
				R_PODOCLOCK_DELETE_FILE_QUERY));
		HBufC* question(HBufC::NewLC(iCurrentFileName.Length() + format->Length()));
		question->Des().Format(*format, &iCurrentFileName);

		CAknQueryDialog* dlg(CAknQueryDialog::NewL());
		if (dlg->ExecuteLD(R_PODOCLOCK_YES_NO_QUERY_DIALOG, 
										 *question))
			{
			DeleteFileL();
			}
		CleanupStack::PopAndDestroy(question);
		CleanupStack::PopAndDestroy(format);
		}
	}


void CPodOClockAppView::DeleteFileL()
	{
	TRACER_AUTO;
	TInt error(CCoeEnv::Static()->FsSession().Delete(iCurrentFileName));

	CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));

	HBufC* format(NULL);
	HBufC* confirmation;
	if (error == KErrNone || error == KErrNotFound)
		{
		format = CEikonEnv::Static()->AllocReadResourceLC(R_PODOCLOCK_FILE_DELETED);
		confirmation = HBufC::NewLC(format->Length() + iCurrentFileName.Length());
		confirmation->Des().Format(*format, &iCurrentFileName);
		iCurrentFileName.Zero();
		--iNumberOfFiles;
		}
	else
		{
		format = CEikonEnv::Static()->AllocReadResourceLC(R_PODOCLOCK_FILE_NOT_DELETED);
		
		confirmation = HBufC::NewLC(format->Length() + 6); // lowest error value ~= -XXXXX
		confirmation->Des().Format(*format, error);
		}
	
	note->ExecuteLD(*confirmation);

	CleanupStack::PopAndDestroy(confirmation);
	CleanupStack::PopAndDestroy(format);
	}


void CPodOClockAppView::PlayRandomFileL()
	{
	TRACER_AUTO;
	// Get the file list
	LOGTEXT("Get the file list");

	iFileArray.Reset();
	CDirScan* scan(CDirScan::NewLC(CCoeEnv::Static()->FsSession()));
	scan->SetScanDataL(KPodcastPath, KEntryAttNormal, ESortNone);
	TInt error(KErrNone);
	CDir* dirList(NULL);
	TFindFile finder(CCoeEnv::Static()->FsSession());
	User::LeaveIfError(iApaLsSession.Connect());
	FOREVER
		{
		TRAP(error, scan->NextL(dirList));
		if (error | !dirList)
			{
			break;
			}
		FindFiles(finder, scan->FullPath());
		};
	iApaLsSession.Close();
	delete dirList;
	CleanupStack::PopAndDestroy(scan);
	
	// Select a random file
	iNumberOfFiles = iFileArray.Count();
	LOGTEXT("iNumberOfFiles");
	LOGINT(iNumberOfFiles);

	if (iNumberOfFiles < 1)
		{
		// Nothing to play

		// Maybe localise: "No [KWildcard] podcasts found under [KPodcastPath]"
		HBufC* text(StringLoader::LoadLC(R_PODOCLOCK_NO_PODCASTS_FOUND));
		CAknInformationNote* note(new (ELeave) CAknInformationNote(ETrue));
		note->ExecuteLD(*text); 
		CleanupStack::PopAndDestroy(text);
		}
	else
		{

////

		TInt random(Math::Rand(iSeed) % iNumberOfFiles);
		LOGTEXT("random");
		LOGINT(random);
		
		iCurrentFileNumber = random + 1;
		iCurrentFileName = iFileArray[random];
		iFileArray.Reset();
		
		TInt chopLength(iCurrentFileName.Length() - KPodcastPath().Length());
		iChoppedFileName = iCurrentFileName.Right(chopLength);
		
		LOGBUF(iCurrentFileName);

//		LOGINT(IsAudioFile(iCurrentFileName));
//		LOGINT(IsAudioFile(_L("E:\\podcasts\\A history of the world in 100 objects\\ahow_20100615-1000a-40960000.mp3")));
//		LOGINT(IsAudioFile(_L("E:\\podcasts\\A history of the world in 100 objects\\ahow_20100622-1000a-4175901727.mp3")));

		LaunchFileEmbeddedL(iCurrentFileName);

////

		AskRepeatAlarmL();
		}
	
	DrawDeferred();
	}


void CPodOClockAppView::FindFiles(TFindFile& aFinder, const TDesC& aDir)
	{
	CDir* foundFiles;
	TFileName foundFile;
	TInt error(aFinder.FindWildByDir(KWildcard, aDir, foundFiles));
	if (error == KErrNone)
		{
		for (TInt i(0); i < foundFiles->Count(); ++i)
			{
//			LOGINT((*foundFiles)[i].iSize);
//			LOGTEXT("bytes");
			// Don't bother with zero byte files
			if ((*foundFiles)[i].iSize > 0)
				{
				foundFile = aDir;
				foundFile.Append((*foundFiles)[i].iName);
//				LOGBUF(foundFile);
				if (IsAudioFile(foundFile, (*foundFiles)[i].iSize))
					{
					iFileArray.Append(foundFile);
					}
				}
			}
		}
	}


TBool CPodOClockAppView::IsAudioFile(const TDesC& aFileName, const TInt aFileSize)
	{
	TRACER_AUTO;
	LOGBUF(aFileName);

	iFileSize = aFileSize;
	LOGTEXT("fileSize");
	LOGINT(iFileSize);
	if (iFileSize > 255)
		{
		iFileSize = 255;
		}

	if (CCoeEnv::Static()->FsSession().ReadFileSection(aFileName,
		0, iFileBuffer, iFileSize) == KErrNone)
		{
		// LOGTEXT("ReadFileSection() OK");

		iApaLsSession.RecognizeData(aFileName, iFileBuffer, *&iMimeType);
		// LOGTEXT("iMimeType.iDataType");
		// LOGBUF(iMimeType.iDataType.Des());
		// LOGTEXT("iMimeType.iConfidence");
		// LOGINT(iMimeType.iConfidence);

		if (iMimeType.iConfidence >= CApaDataRecognizerType::EProbable)
			{
			if (iMimeType.iDataType.Des().Compare(KAudioMpeg) == KErrNone)
				{
				return ETrue;
				}
			}
		}
	return EFalse;
	}

void CPodOClockAppView::LaunchFileEmbeddedL(const TDesC& aFileName)
	{
	if (!iDocHandler)
		{
		iDocHandler = CDocumentHandler::NewL(CEikonEnv::Static()->Process());
		}
	
	// Set the exit observer so HandleServerAppExit will be called
	iDocHandler->SetExitObserver(this);
	
	TDataType emptyDataType = TDataType();
	// Open a file embedded
	iDocHandler->OpenFileEmbeddedL(aFileName, emptyDataType);
	}
 
 
void CPodOClockAppView::HandleServerAppExit(TInt aReason)
	{
	// Handle closing the handler application
	MAknServerAppExitObserver::HandleServerAppExit(aReason);
	}


void CPodOClockAppView::LoadSettingsL()
	{
	TRACER_AUTO;
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
		readStream.ReadInt32L(); // Ignore volume
		alarmOn = readStream.ReadInt8L();

		TUint32 high(readStream.ReadInt32L());
		TUint32 low(readStream.ReadInt32L());
		alarmTime = TTime(MAKE_TINT64(high, low));

		CleanupStack::PopAndDestroy(&readStream);
		}
	
	CleanupStack::PopAndDestroy(&file);
	
	iAlarmTime = alarmTime;

 	if (alarmOn)
		{
		SetAlarmL(iAlarmTime, EFalse);
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
		writeStream.WriteInt32L(1); // was iVolume
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
	CEikonEnv::Static()->InfoMsg(_L("Alarm!"));
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
		iAskRepeat = ETrue;
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


// End of file
