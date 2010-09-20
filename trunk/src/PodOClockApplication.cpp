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
#include "PodOClockApplication.h"
#include "PodOClockDocument.h"
//#include "PodOClockTracer.h"

// UID for the application should match the UID defined in the mmp file
const TUid KUidPodOClockApp = {0xA89FB98E};

CApaDocument* CPodOClockApplication::CreateDocumentL()
	{
//    TRACER_AUTO;
	// Create an PodOClock document, and return a pointer to it
	return (static_cast<CApaDocument*>
					(CPodOClockDocument::NewL(*this)));
	}


TUid CPodOClockApplication::AppDllUid() const
	{
//    TRACER_AUTO;
	// Return the UID for the PodOClock application
	return KUidPodOClockApp;
	}

// End of file
