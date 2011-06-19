
@echo ============== SELF-SIGNED ==========================

@move    PodOClock.sis  PodOClock.bak.sis
@move    PodOClock.sisx PodOClock.bak.sisx
makesis PodOClock.pkg  PodOClock.sis
signsis PodOClock.sis  PodOClock.sisx PodOClock.cer PodOClock.key password

copy PodOClock.sisx "%HOMEDRIVE%%HOMEPATH%\My Documents\Magic Briefcase"
if exist PodOClock.sisx start PodOClock.sisx


@echo ============== OVI SIGNED ==========================

@move    PodOClock_signed.sis  PodOClock_signed.bak.sis
@move    PodOClock_signed.sisx PodOClock_signed.bak.sisx
makesis PodOClock_signed.pkg  PodOClock_signed.sis
signsis PodOClock_signed.sis  PodOClock_signed.sisx ovi_2011-05-30.crt ovi_2011-05-30.key

copy PodOClock_signed.sisx "%HOMEDRIVE%%HOMEPATH%\My Documents\Magic Briefcase"
if exist PodOClock_signed.sisx start PodOClock_signed.sisx
