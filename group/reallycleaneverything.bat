call bldmake bldfiles
call abld reallyclean gcce
call abld reallyclean winscw
call abld reallyclean
call abld clean gcce
call abld clean winscw
call abld clean

call del /s %EPOCROOT%epoc32\*podoclock*.*
