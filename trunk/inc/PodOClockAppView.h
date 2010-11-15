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

#ifndef __PODOCLOCKAPPVIEW_H__
#define __PODOCLOCKAPPVIEW_H__

#include "PodOClockSoundPlayer.h"
#include "PodOClockTimer.h"

// CONSTANTS
_LIT(KVersion, "1.20");

// FORWARD DECLARATIONS
class CAknNavigationControlContainer;
class CAknNavigationDecorator;

// CLASS DECLARATION
class CPodOClockAppView : public CCoeControl,
						public MPodOClockSoundPlayerNotify,
						public MPodOClockTimerNotify
	{

	public: // New methods
		static CPodOClockAppView* NewL(const TRect& aRect);
		static CPodOClockAppView* NewLC(const TRect& aRect);
		virtual ~CPodOClockAppView();

	public:  // Functions from base classes
		void Draw(const TRect& aRect) const;

	public: // New methods
		void SetAlarmL(const TTime aTime, const TBool aShowConfirmation = ETrue);
		void RemoveAlarm();
		void AskRepeatAlarmL();
		void BackFiveSeconds();
		void AskDeleteFileL();
		void DeleteFileL();
		void SelectL();
		void PlayRandomFileL();
		TBool AlarmActive() const { return iAlarmTimer ? iAlarmTimer->IsActive() : EFalse; }
		TTime AlarmTime() const { return iAlarmTime; }
		void ChangeVolumeL(TInt aDifference);
//		void SetVolume(TInt aVolume);
		TBool TrackInfoAvailable();
		void ShowTrackInfoL();
		TBool Playing() const { return iSoundPlayer->PlayerState() == EPodOClockPlaying; }
		TBool Paused() const { return iSoundPlayer->PlayerState() == EPodOClockPaused; }
		TBool FileNameKnown() const { return iCurrentFileName.Length() > 0; }
		void Stop();
		void Pause();
		void Resume();

	private: // from CCoeControl
		virtual void SizeChanged();
		TTypeUid::Ptr MopSupplyObject(TTypeUid aId);
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, 
									TEventCode aType);

	private: // from MPodOClockSleepTimerNotify
		void TimerExpiredL(TAny* aTimer, TInt aError);

	private: // from MPodOClockSoundPlayerNotify
		void PlayerStartedL(TInt aError, TInt aVolume);
		void PlayerEndedL();

		private: // Constructors
		void ConstructL(const TRect& aRect);
		CPodOClockAppView();

		// Drawing helper methods
		void DrawText(const TDesC& aText, 
					  const TInt& aY, 
					  const TRgb& aPenColor) const;

		void LoadResourceFileTextL();
		void LoadSettingsL();
		void SaveSettingsL();

		void DoChangePaneTextL() const;

		void PlayL(const TDesC& aFileName);
		
		void FindFiles(TFindFile& aFinder, const TDesC& aDir);
		
	private:
		// Text from resource files
		HBufC* iAlarmSetText;
		HBufC* iNoAlarmSetText;
		HBufC* iVolumeText;
		HBufC* iYearFormat;
		HBufC* iXOfYFormat;
		HBufC* iKeysText1;
		HBufC* iKeysText2;
		HBufC* iKeysText3;
		HBufC* iKeysText4;
		HBufC* iKeysText5;
		HBufC* iKeysText6;
		HBufC* iKeysText7;
		HBufC* iKeysText8;

		const CFont* iFont;

		MAknsControlContext* iBackground; // for skins support 

		TInt iRectWidth;

		CAknNavigationControlContainer *iNaviContainer;
		CAknNavigationDecorator* iNaviLabelDecorator;

		TTime iAlarmTime;
		CPodOClockTimer* iAlarmTimer;

		TInt64 iSeed;
		
		// Sound player for playing audio files (owned) 
		CPodOClockSoundPlayer* iSoundPlayer;
		TInt iVolume;
		
		// For finding files
		CDir* iFoundFiles;
		TFileName iFoundFile;
		RArray<TFileName> iFileArray;
		
		// For keeping track of the playing track
		TFileName iCurrentFileName;
		TInt iCurrentFileNumber;
		TInt iNumberOfFiles;
		
		HBufC* iTitle;
		HBufC* iAlbum;
		HBufC* iArtist;
		HBufC* iYear;
		HBufC* iComment;
		TUint iHours;
		TUint iMinutes;
		TUint iSeconds;

		TBool iAskRepeat;
	};

#endif // __PODOCLOCKAPPVIEW_H__

// End of file
