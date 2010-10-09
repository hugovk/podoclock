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

#include "PodOClockTimer.h"
#include "PodOClockTracer.h"

CPodOClockTimer::CPodOClockTimer(const TInt aPriority, 
									   MPodOClockTimerNotify& aNotify)
:CActive(aPriority), iNotify(aNotify)
	{
	TRACER_AUTO;
	}

CPodOClockTimer::~CPodOClockTimer()
	{
	TRACER_AUTO;
	Cancel();
	iTimer.Close();
	}

CPodOClockTimer* CPodOClockTimer::NewL(const TInt aPriority,
											 MPodOClockTimerNotify& aNotify)
	{
	TRACER_AUTO;
	CPodOClockTimer* timer(new (ELeave) CPodOClockTimer(aPriority, 
																aNotify));
	CleanupStack::PushL(timer);
	timer->ConstructL();
	CleanupStack::Pop();
	return timer;
	}

void CPodOClockTimer::ConstructL(void)
	{
	TRACER_AUTO;
	CActiveScheduler::Add(this);
	iTimer.CreateLocal();
	}

void CPodOClockTimer::At(const TTime& aTime)
	{
	TRACER_AUTO;
	Cancel();
	iTimer.At(iStatus, aTime);
	SetActive();
	}

void CPodOClockTimer::DoCancel()
	{
	TRACER_AUTO;
	iTimer.Cancel();
	}
 
void CPodOClockTimer::RunL()
	{
	TRACER_AUTO;
	iNotify.TimerExpiredL(this, iStatus.Int());
	}

// End of file
