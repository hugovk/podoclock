/*
Pod O'Clock for S60 phones.
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

#ifndef __PODOCLOCKAPPUI_H__
#define __PODOCLOCKAPPUI_H__

// INCLUDES
#include <aknappui.h>

// FORWARD DECLARATIONS
class CBrowserLauncher;
class CPodOClockAppView;

// CLASS DECLARATION
class CPodOClockAppUi : public CAknAppUi
	{
	public: // Constructors and destructor
		void ConstructL();
		CPodOClockAppUi();
		virtual ~CPodOClockAppUi();

	private:  // Functions from base classes
		void HandleCommandL(TInt aCommand);
		void HandleForegroundEventL(TBool aForeground);
		void HandleResourceChangeL(TInt aType);

	private: // from CAknView
		void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

	private: // new methods
		void OpenWebBrowserL(const TDesC& aUrl, 
							 const TBool aForceNative = EFalse);
#ifdef __OVI_SIGNED__
		void UninstallSelfSignedVersionL();
#else
		void LaunchOviSignedVersionL();
#endif

	private: // Data
		CBrowserLauncher* iBrowserLauncher;
		CPodOClockAppView* iAppView;
		TBool iHideSugarSync;
#ifdef __OVI_SIGNED__
		TBool iUninstallAttempted;
#endif

	};

#endif // __PODOCLOCKAPPUI_H__

// End of file
