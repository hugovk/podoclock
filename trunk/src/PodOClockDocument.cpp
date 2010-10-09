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
#include "PodOClockAppUi.h"
#include "PodOClockDocument.h"
//#include "PodOClockTracer.h"

CPodOClockDocument* CPodOClockDocument::NewL(CEikApplication& aApp)
	{
//	TRACER_AUTO;
	CPodOClockDocument* self(NewLC(aApp));
	CleanupStack::Pop(self);
	return self;
	}

CPodOClockDocument* CPodOClockDocument::NewLC(CEikApplication& aApp)
	{
//	TRACER_AUTO;
	CPodOClockDocument* self(new (ELeave) CPodOClockDocument(aApp));
	
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CPodOClockDocument::ConstructL()
	{
//	TRACER_AUTO;
	// No implementation required
	}

CPodOClockDocument::CPodOClockDocument(CEikApplication& aApp)
	: CAknDocument(aApp)
	{
//	TRACER_AUTO;
	// No implementation required
	}

CPodOClockDocument::~CPodOClockDocument()
	{
//	TRACER_AUTO;
	// No implementation required
	}

CEikAppUi* CPodOClockDocument::CreateAppUiL()
	{
//	TRACER_AUTO;
	return (static_cast <CEikAppUi*> (new (ELeave) CPodOClockAppUi));
	}

// End of file
