	
@echo ============== SELF-SIGNED ==========================

@if exist PodOClock.sis  move PodOClock.sis  PodOClock.bak.sis
@if exist PodOClock.sisx move PodOClock.sisx PodOClock.bak.sisx
makesis PodOClock.pkg  PodOClock.sis
@if exist PodOClock.sis signsis PodOClock.sis  PodOClock.sisx PodOClock.cer PodOClock.key password

@if exist PodOClock.sisx copy  PodOClock.sisx "%HOMEDRIVE%%HOMEPATH%\My Documents\Magic Briefcase"
@if exist PodOClock.sisx start PodOClock.sisx
@if exist PodOClock.sis  del   PodOClock.sis

@echo ============== OVI SIGNED ==========================

@if exist PodOClock_signed.sis  move PodOClock_signed.sis  PodOClock_signed.bak.sis
@if exist PodOClock_signed.sisx move PodOClock_signed.sisx PodOClock_signed.bak.sisx
makesis PodOClock_signed.pkg  PodOClock_signed.sis
@if exist PodOClock_signed.sis signsis PodOClock_signed.sis PodOClock_signed.sisx ovi_2011-05-30.crt ovi_2011-05-30.key

@if exist PodOClock_signed.sisx copy  PodOClock_signed.sisx "%HOMEDRIVE%%HOMEPATH%\My Documents\Magic Briefcase"
@if exist PodOClock_signed.sisx start PodOClock_signed.sisx
@if exist PodOClock_signed.sis  del   PodOClock_signed.sis
