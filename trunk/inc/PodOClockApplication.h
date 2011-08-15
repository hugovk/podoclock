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

#ifndef __PODOCLOCKAPPLICATION_H__
#define __PODOCLOCKAPPLICATION_H__

// INCLUDES
#include <aknapp.h>

// CLASS DECLARATION

class CPodOClockApplication : public CAknApplication
	{
	public: // Functions from base class
		TUid AppDllUid() const;

	protected: // Functions from base class
		CApaDocument* CreateDocumentL();
	};

#endif // __PODOCLOCKAPPLICATION_H__

// End of file