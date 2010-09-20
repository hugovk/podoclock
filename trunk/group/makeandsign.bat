move    PodOClock.sis  PodOClock.bak.sis
move    PodOClock.sisx PodOClock.bak.sisx
makesis PodOClock.pkg  PodOClock.sis
signsis PodOClock.sis  PodOClock.sisx PodOClock.cer PodOClock.key password
call PodOClock.sisx
