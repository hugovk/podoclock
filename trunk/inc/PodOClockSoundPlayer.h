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

#ifndef __PODOCLOCKSOUNDPLAYER_H__
#define __PODOCLOCKSOUNDPLAYER_H__

//  INCLUDES
#include <e32base.h>
#include <MdaAudioSamplePlayer.h>

#include "PodOClock.hrh"

// CONSTANTS
const TInt KMaxChars(256);

// ENUMERATIONS
enum TPodOClockPlayerState // State of the audio player
	{
	EPodOClockNotReady = 0,
	EPodOClockReadyToPlay,
	EPodOClockPlaying,
	EPodOClockPaused
	};


// CLASS DECLARATION

class MPodOClockSoundPlayerNotify
	{
public:
	virtual void PlayerStartedL(TInt aError, TInt aVolume) = 0;
	virtual void PlayerEndedL() = 0;
	};

/**
*  Sound player plays audio files in the application.
*/
class CPodOClockSoundPlayer 
	: public CBase,
	  public MMdaAudioPlayerCallback
	{
	public:  // Constructors and destructor

		/**
		 * Creates and returns a new instance of this class.
		 * @return Pointer to the CPodOClockSoundPlayer object
		 */
		static CPodOClockSoundPlayer* NewL(MPodOClockSoundPlayerNotify& aNotify,
											TInt aVolume);

		/**
		 * Creates and returns a new instance of this class. The returned
		 * object is left on the cleanup stack.
		 * @return Pointer to the CPodOClockSoundPlayer object
		 */
		static CPodOClockSoundPlayer* NewLC(MPodOClockSoundPlayerNotify& aNotify,
											TInt aVolume);

		/**
		* Destructor.
		*/
		virtual ~CPodOClockSoundPlayer();

	private:	// Constructors
		/**
		* C++ default constructor.
		*/
		CPodOClockSoundPlayer(MPodOClockSoundPlayerNotify& aNotify, TInt aVolume);

		/**
		* Symbian 2nd phase constructor.
		*/
		void ConstructL();

	protected:  // Functions from base classes

		/**
		* From MMdaAudioPlayerCallback. Defines client behaviour 
		* when opening and initialization of an audio sample has 
		* completed.
		*/
		void MapcInitComplete(TInt aError, 
							   const TTimeIntervalMicroSeconds& aDuration);

		/**
		* From MMdaAudioPlayerCallback. Defines client behaviour 
		* when playback of an audio sample has completed.
		*/
		void MapcPlayComplete(TInt aError);

	public: // New functions

		/**
		* Open music file and start music playback.
		* @param aFileName Path and file name of the audio file
		*/
		void StartPlaybackL(const TDesC& aFileName);
		
		/**
		* Stop music playback.
		*/
		void StopPlayback();
		void PausePlayback();
		void ResumePlayback();
		
		/**
		* Get the state of the audio player.
		* @return State of the audio player
		*/
		TPodOClockPlayerState PlayerState();
		
		TInt Volume();
		TInt MaxVolume() const;
		TInt ChangeVolume(TInt aDifference);
		TInt GetPosition(TTimeIntervalMicroSeconds& aPosition);
		void SetPosition(TUint aPosition);
		TInt DurationInSeconds();
		
		void GetMetaDataL(HBufC*& aTitle, 
						  HBufC*& aAlbum, 
						  HBufC*& aArtist, 
						  HBufC*& aYear,
						  HBufC*& aComment);
		
	private:	// Data
		
		// Audio player utility (owned)
		CMdaAudioPlayerUtility* iMdaAudioPlayerUtility;
		
		// State of the audio player
		TPodOClockPlayerState iPlayerState;
		
		MPodOClockSoundPlayerNotify& iNotify;

		TInt iVolume;
	};

#endif // __PODOCLOCKSOUNDPLAYER_H__

// End of file
