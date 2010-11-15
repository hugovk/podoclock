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

#ifndef __PODOCLOCKAPPUI_H__
#define __PODOCLOCKAPPUI_H__

// INCLUDES
#include <aknappui.h>
#include <remconcoreapitargetobserver.h>    // link against RemConCoreApi.lib
#include <remconcoreapitarget.h>            // and
#include <remconinterfaceselector.h>        // RemConInterfaceBase.lib

// FORWARD DECLARATIONS
class CPodOClockAppView;

// CLASS DECLARATION
class CPodOClockAppUi : public CAknAppUi,
						public MRemConCoreApiTargetObserver
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

	private: // from MRemConCoreApiTargetObserver
	void MrccatoCommand(TRemConCoreApiOperationId aOperationId,
						TRemConCoreApiButtonAction aButtonAct);

	private: // Data
		CPodOClockAppView* iAppView;

		CRemConInterfaceSelector* iInterfaceSelector;
		CRemConCoreApiTarget*     iCoreTarget;
	};

#endif // __PODOCLOCKAPPUI_H__

// End of file