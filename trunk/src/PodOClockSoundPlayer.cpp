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
#include <MmfMeta.h>
#include "PodOClockSoundPlayer.h"
#include "PodOClockTracer.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::CPodOClockSoundPlayer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CPodOClockSoundPlayer::CPodOClockSoundPlayer(
										MPodOClockSoundPlayerNotify& aNotify,
										TInt aVolume)
:iNotify(aNotify),
 iVolume(aVolume)
    {
    TRACER_AUTO;
	// No implementation required
    }

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CPodOClockSoundPlayer::ConstructL()
    {
    TRACER_AUTO;
    // Create a player utility
    iMdaAudioPlayerUtility = CMdaAudioPlayerUtility::NewL(*this);
    }

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CPodOClockSoundPlayer* CPodOClockSoundPlayer::NewL(
										MPodOClockSoundPlayerNotify& aNotify,
										TInt aVolume)
    {
    TRACER_AUTO;
    CPodOClockSoundPlayer* self(CPodOClockSoundPlayer::NewLC(aNotify, aVolume));
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::NewLC
// Two-phased constructor, pointer is left to the CleanupStack.
// -----------------------------------------------------------------------------
//
CPodOClockSoundPlayer* CPodOClockSoundPlayer::NewLC(
										MPodOClockSoundPlayerNotify& aNotify,
										TInt aVolume)
    {
    TRACER_AUTO;
    CPodOClockSoundPlayer* self(new (ELeave) CPodOClockSoundPlayer(aNotify, 
																	aVolume));
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// Destructor
CPodOClockSoundPlayer::~CPodOClockSoundPlayer()
    {
    TRACER_AUTO;
    if (iPlayerState == ECEPlaying)
        {
        StopPlayback();
        }
        
    delete iMdaAudioPlayerUtility;
    }

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::MapcInitComplete
// Defines client behaviour when opening and initialisation of an audio sample 
// has completed.
// -----------------------------------------------------------------------------
//
void CPodOClockSoundPlayer::MapcInitComplete(
                            TInt aError, 
                            const TTimeIntervalMicroSeconds& /*aDuration*/)
	{
    TRACER_AUTO;
	iPlayerState = aError ? ECENotReady : ECEReadyToPlay;

    // Start the playback, if audio file initialisation was successful
    if (iPlayerState == ECEReadyToPlay)
	    {
		iMdaAudioPlayerUtility->SetVolume(iVolume);
	    iMdaAudioPlayerUtility->Play();
		iPlayerState = ECEPlaying;
		iNotify.PlayerStartedL();
	    }
	}

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::MapcPlayComplete
// Defines client behaviour when playback of an audio sample has completed.
// -----------------------------------------------------------------------------
//
void CPodOClockSoundPlayer::MapcPlayComplete(TInt aError)
	{
    TRACER_AUTO;
	iPlayerState = aError ? ECENotReady : ECEReadyToPlay;
	}

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::StartPlayback
// Open music file and start music playback.
// -----------------------------------------------------------------------------
//
void CPodOClockSoundPlayer::StartPlaybackL(const TDesC& aFileName)
	{
    TRACER_AUTO;
	// Open the sound file
	iMdaAudioPlayerUtility->OpenFileL(aFileName);
	}

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::StopPlayback
// Stop music playback.
// -----------------------------------------------------------------------------
//
void CPodOClockSoundPlayer::StopPlayback()
	{
    TRACER_AUTO;
	// Stop audio playback and close the audio file
	if (iMdaAudioPlayerUtility && iPlayerState == ECEPlaying)
	    {
	    iMdaAudioPlayerUtility->Stop();
	    iMdaAudioPlayerUtility->Close();
	    iPlayerState = ECENotReady;
	    }
	}

// -----------------------------------------------------------------------------
// CPodOClockSoundPlayer::PlayerState
// Get the state of the audio player.
// -----------------------------------------------------------------------------
//
TPodOClockPlayerState CPodOClockSoundPlayer::PlayerState()
    {
    TRACER_AUTO;
    return iPlayerState;
    }

TInt CPodOClockSoundPlayer::ChangeVolume(TInt aDifference)
    {
    TRACER_AUTO;
	TInt volume;
	TInt error(iMdaAudioPlayerUtility->GetVolume(volume));
	if (error == KErrNone)
		{
		volume += aDifference;
		
		if ((volume >= 0) && (volume <= iMdaAudioPlayerUtility->MaxVolume()))
			{
			error = iMdaAudioPlayerUtility->SetVolume(volume);
			if (error == KErrNone)
				{
				iVolume = volume;
				}
			}
		}

	return iVolume;
    }

void CPodOClockSoundPlayer::GetMetaDataL(TDes& aTitle, 
										 TDes& aAlbum, 
										 TDes& aArtist, 
										 TDes& aYear,
										 TDes& aGenre)
	{
    TRACER_AUTO;
	TInt numEntries(0);
	User::LeaveIfError(iMdaAudioPlayerUtility->GetNumberOfMetaDataEntries(numEntries));
 
	for (TInt i(0); i < numEntries; ++i)
		{
		CMMFMetaDataEntry* entry(NULL);
		TRAPD(error, entry = iMdaAudioPlayerUtility->GetMetaDataEntryL(i));
		CleanupStack::PushL(entry);
 
		if (error == KErrNone)
			{
			if (entry->Name().CompareF(KMMFMetaEntrySongTitle) == KErrNone)
				{
				aTitle.Copy(entry->Value());
				}
			else if (entry->Name().CompareF(KMMFMetaEntryAlbum) == KErrNone)
				{
				aAlbum.Copy(entry->Value());
				}
			else if (entry->Name().CompareF(KMMFMetaEntryArtist) == KErrNone)
				{
				aArtist.Copy(entry->Value());
				}
			else if (entry->Name().CompareF(KMMFMetaEntryYear) == KErrNone)
				{
				aYear.Copy(entry->Value());
				}
			else if (entry->Name().CompareF(KMMFMetaEntryGenre) == KErrNone)
				{
				aGenre.Copy(entry->Value());
				}
			}
 
		CleanupStack::PopAndDestroy(entry);
		}
	}

//  End of file