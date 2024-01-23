add_library(
    servers
    tcp/interface/server.h
    tcp/server.c
    websock/interface/server.h
    websock/server.c
    commons/interface/adaptor.h
    commons/adaptor.c
    commons/interface/globals.h
    commons/globals.c
    auth/interface/auth.h
    auth/auth.c
)

