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

This file incorporates work covered by the following copyright and  
permission notice:  

	Mobbler, a Last.fm mobile scrobbler for Symbian smartphones.
	Copyright (C) 2008, 2009  Michael Coffey
	Copyright (C) 2009  Hugo van Kemenade

	http://code.google.com/p/mobbler

	This file is part of Mobbler.

	Mobbler is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	Mobbler is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Mobbler.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ecom/implementationproxy.h>
#include "podoclocktouchfeedback.h"

const TInt KImplementationUid = {0xA89FD1D9};


const TImplementationProxy ImplementationTable[] =
	{
	{KImplementationUid, TProxyNewLPtr(CPodOClockTouchFeedback::NewL)}
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
	return ImplementationTable;
	}

CPodOClockTouchFeedback* CPodOClockTouchFeedback::NewL()
	{
	CPodOClockTouchFeedback* self(new(ELeave) CPodOClockTouchFeedback());
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CPodOClockTouchFeedback::CPodOClockTouchFeedback()
	{
	}

void CPodOClockTouchFeedback::ConstructL()
	{
#ifdef  __S60_50__
	iTouchFeedback = MTouchFeedback::Instance();
	iTouchFeedback->SetFeedbackEnabledForThisApp(ETrue);
#else
	User::Leave(KErrNotSupported);
#endif
	}

CPodOClockTouchFeedback::~CPodOClockTouchFeedback()
	{
	}

void CPodOClockTouchFeedback::InstantFeedback(TInt aType)
	{
#ifdef  __S60_50__
	iTouchFeedback->InstantFeedback(static_cast<TTouchLogicalFeedback>(aType));
#endif
	}

// End of file
