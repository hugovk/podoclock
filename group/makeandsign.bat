
@echo ============== SELF-SIGNED ==========================

@set sisfile=PodOClock
@set certfile=%sisfile%.cer
@set keyfile=%sisfile%.key

@if exist %sisfile%.sis  move %sisfile%.sis  %sisfile%.bak.sis
@if exist %sisfile%.sisx move %sisfile%.sisx %sisfile%.bak.sisx
makesis %sisfile%.pkg  %sisfile%.sis
@if exist %sisfile%.sis signsis %sisfile%.sis  %sisfile%.sisx %certfile% %keyfile% password

@if exist %sisfile%.sisx copy  %sisfile%.sisx "%HOMEDRIVE%%HOMEPATH%\My Documents\Magic Briefcase"
@if exist %sisfile%.sisx start %sisfile%.sisx
@if exist %sisfile%.sis  del   %sisfile%.sis

@echo ============== OVI SIGNED ==========================

@set sisfile=PodOClock_signed
@set certfile=ovi.crt
@set keyfile=ovi.key

@if exist %sisfile%.sis  move %sisfile%.sis  %sisfile%.bak.sis
@if exist %sisfile%.sisx move %sisfile%.sisx %sisfile%.bak.sisx
makesis %sisfile%.pkg  %sisfile%.sis
@if exist %sisfile%.sis signsis %sisfile%.sis %sisfile%.sisx %certfile% %keyfile% 

@if exist %sisfile%.sisx copy  %sisfile%.sisx "%HOMEDRIVE%%HOMEPATH%\My Documents\Magic Briefcase"
@if exist %sisfile%.sisx start %sisfile%.sisx
REM @if exist %sisfile%.sis  del   %sisfile%.sis

@dir /b *.sisx