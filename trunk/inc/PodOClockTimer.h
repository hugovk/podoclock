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

#ifndef __PODOCLOCKTIMER_H__
#define __PODOCLOCKTIMER_H__

#include <e32base.h>
 
class MPodOClockTimerNotify
	{
public:
	virtual void TimerExpiredL(TAny* aTimer, TInt aError) = 0;
	};

class CPodOClockTimer: public CActive
	{
public:
	static CPodOClockTimer* NewL(const TInt aPriority, 
									MPodOClockTimerNotify& aNotify);
	~CPodOClockTimer();
public:
	void At(const TTime& aTime);

protected:
	void RunL();
	void DoCancel();

private:
	CPodOClockTimer(const TInt aPriority, MPodOClockTimerNotify& aNotify);
	void ConstructL(void);

private:
	RTimer iTimer;
	MPodOClockTimerNotify& iNotify;
};

#endif // __PODOCLOCKTIMER_H__

// End of file
