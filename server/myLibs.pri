# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

    ERASE_COMMAND = rm {myfunctions.cpp, myfunctions.h, myproto.cpp, myproto.h, controlserver.cpp, controlserver.h, http.cpp, http.h}
win32|win64{
    ERASE_COMMAND = del myfunctions.cpp, myfunctions.h, myproto.cpp, myproto.h, controlserver.cpp, controlserver.h, http.cpp, http.h
}

system($$ERASE_COMMAND 2> $$NULL_DEVICE)

system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/myfunctions.cpp > myfunctions.cpp)
system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/myfunctions.h > myfunctions.h)
system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/myproto.h > myproto.h)
system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/myproto.cpp > myproto.cpp)
system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/controlserver.h > controlserver.h)
system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/controlserver.cpp > controlserver.cpp)
system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/http.h > http.h)
system(curl https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/http.cpp > http.cpp)
