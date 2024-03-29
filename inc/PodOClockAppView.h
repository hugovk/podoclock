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

#ifndef __PODOCLOCKAPPVIEW_H__
#define __PODOCLOCKAPPVIEW_H__

#include <aknserverapp.h>  // MAknServerAppExitObserver
#include <apgcli.h>

#include "PodOClockTimer.h"

// CONSTANTS
#ifdef __OVI_SIGNED__
_LIT(KVersion, "3.07");
#else
_LIT(KVersion, "2.07");
#endif

// FORWARD DECLARATIONS
class CAknNavigationControlContainer;
class CAknNavigationDecorator;
class CDocumentHandler;
class CPodOClockTouchFeedbackInterface;

// CLASS DECLARATION
class CPodOClockAppView : public CCoeControl,
//						public CAknAppUi,
						public MAknServerAppExitObserver,
						public MPodOClockTimerNotify
	{

	public: // Constructors and destructors
		static CPodOClockAppView* NewL(const TRect& aRect);
		static CPodOClockAppView* NewLC(const TRect& aRect);
		virtual ~CPodOClockAppView();

	public:  // Functions from base classes
		void Draw(const TRect& aRect) const;

	public: // New methods
		void SetAlarmL(const TTime aTime, const TBool aShowConfirmation = ETrue);
		void AskRepeatAlarmL();
		TBool AskRemoveAlarmL();
		void RemoveAlarm();
		void AskDeleteFileL();
		void DeleteFileL();
		void PlayRandomFileL();
		TBool AlarmActive() const { return iAlarmTimer ? iAlarmTimer->IsActive() : EFalse; }
		TTime AlarmTime() const { return iAlarmTime; }
		TBool FileNameKnown() const { return iCurrentFileName.Length() > 0; }
		TBool IsThirdEdition() const { return (iFeedback == NULL); }
	
	private: // from CCoeControl
		virtual void SizeChanged();
		TTypeUid::Ptr MopSupplyObject(TTypeUid aId);
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, 
									TEventCode aType);
		void HandlePointerEventL(const TPointerEvent& aPointerEvent);

	private: // from MPodOClockSleepTimerNotify
		void TimerExpiredL(TAny* aTimer, TInt aError);

	private: // from MAknServerAppExitObserver
		void HandleServerAppExit(TInt aReason);

	private: // Constructors
		void ConstructL(const TRect& aRect);
		CPodOClockAppView();

	private: // New methods
		// Drawing methods
		void DrawText(const TDesC& aText, 
					  const TInt& aY, 
					  const TRgb& aPenColor) const;
		void DrawText(const TDesC& aText, 
					  const TRect& aRect, 
					  const TRgb& aPenColor) const;
		void DrawIcon(const CFbsBitmap& aIcon, 
					  const TRect& aRect, 
					  const TSize& aSize) const;
		void DoChangePaneTextL() const;
		void SetPositions();
		void LoadIconL(TInt aIndex, CFbsBitmap*& aBitmap, 
						CFbsBitmap*& aMask, TSize& aSize);

		void LoadResourceFileTextL();
		void LoadSettingsL();
		void SaveSettingsL();

		void FindFilesOnDriveL(const TInt& aDriveNumber);
		void FindFilesInDir(TFindFile& aFinder, const TDesC& aDir);
		TBool IsAudioFile(const TDesC& aFileName, const TInt aFileSize);
		TBool IsAudioFileL(const TDesC& aFileName);
		void LaunchFileEmbeddedL(const TDesC& aFileName);
		
	private:
		// Text from resource files
		HBufC* iAlarmSetText;
		HBufC* iNoAlarmSetText;
		HBufC* iDeleteFileText;

		TRect iAlarmTextRect;
		TRect iDeleteAlarmButtonRect;
		TRect iPlayButtonRect;
		TRect iFileNameRect;
		TRect iDeleteFileButtonRect;
		TInt iHalfTextHeightInPixels;
		
		CFbsBitmap* iPlayIcon;
		CFbsBitmap* iPlayMask;
		TSize iPlayIconSize;
		
		CFbsBitmap* iDeleteIcon;
		CFbsBitmap* iDeleteMask;
		TSize iDeleteIconSize;
		
		const CFont* iFont;
		CAknsBasicBackgroundControlContext* iBackground; // for skins support 
		TInt iRectWidth;

		CAknNavigationControlContainer *iNaviContainer;
		CAknNavigationDecorator* iNaviLabelDecorator;

		TTime iAlarmTime;
		CPodOClockTimer* iAlarmTimer;

		// For finding files
		RArray<TFileName> iFileArray;
		RApaLsSession iApaLsSession;
		TInt64 iSeed;
		
		// For keeping track of the playing track
		TFileName iCurrentFileName;
		TFileName iChoppedFileName;
		TInt iCurrentFileNumber;
		TInt iNumberOfFiles;
		
		TBool iAskRepeat;
		CDocumentHandler* iDocHandler;
		TPoint iLastTouchPosition;
		CPodOClockTouchFeedbackInterface* iFeedback;
		TUid iDtorIdKey;
	};

#endif // __PODOCLOCKAPPVIEW_H__

// End of file
