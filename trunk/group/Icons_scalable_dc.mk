# 
# Pod O'Clock for S60 phones.
# http://code.google.com/p/podoclock/
# Copyright (C) 2010, 2011  Hugo van Kemenade
# 
# This file is part of Pod O'Clock.
# 
# Pod O'Clock is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# Pod O'Clock is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Pod O'Clock.  If not, see <http://www.gnu.org/licenses/>.
# 

ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=$(EPOCROOT)epoc32\data\z
endif

TARGETDIR=$(ZDIR)\resource\apps

ICONTARGETFILENAME=$(TARGETDIR)\podoclock.mif
OSICONTARGETFILENAME=$(TARGETDIR)\podoclock_0x200427FA.mif

ICONDIR=..\gfx

HEADERDIR=$(EPOCROOT)epoc32\include

HEADERFILENAME=$(HEADERDIR)\podoclock.mbg
OSHEADERFILENAME=$(HEADERDIR)\podoclock_0x200427FA.mbg

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE :
	mifconv $(ICONTARGETFILENAME) /X /h$(HEADERFILENAME) \
		/c32 $(ICONDIR)\podoclock.svg \
		/c32,8 $(ICONDIR)\play.svg \
		/c32,8 $(ICONDIR)\delete.svg

	mifconv $(OSICONTARGETFILENAME) /X /h$(OSHEADERFILENAME) \
		/c32 $(ICONDIR)\podoclock_bg_black.svg \
		/c32,8 $(ICONDIR)\play.svg \
		/c32,8 $(ICONDIR)\delete.svg

FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(ICONTARGETFILENAME)
	@echo $(HEADERFILENAME)
	@echo $(OSICONTARGETFILENAME)
	@echo $(OSHEADERFILENAME)
	
FINAL : do_nothing

# End of file
