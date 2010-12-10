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
	Copyright (C) 2008  Michael Coffey
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

#ifndef __PODOCLOCKTOUCHFEEDBACK_H__
#define __PODOCLOCKTOUCHFEEDBACK_H__

#include <e32base.h>
#include <podoclock\PodOClockTouchFeedbackInterface.h>

#ifdef  __S60_50__
#include <touchfeedback.h>
#endif

class CPodOClockTouchFeedback : public CPodOClockTouchFeedbackInterface
	{
public:
	static CPodOClockTouchFeedback* NewL();
	~CPodOClockTouchFeedback();
	
private:
	CPodOClockTouchFeedback();
	void ConstructL();

private: // from CPodOClockTouchFeedbackInterface
	void InstantFeedback(TInt aType);

private:
#ifdef  __S60_50__
	MTouchFeedback* iTouchFeedback;
#endif
	};

#endif // __PODOCLOCKTOUCHFEEDBACK_H__

// End of file
