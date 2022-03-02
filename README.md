# SimpleServerAndClient
A dead simple chat server and client written in C.

This was written while reading Beej's Guide to Network Programming (here: https://beej.us/guide/bgnet/).

# Compilation

This can be compiled with MSVC like this:

```cl pollserver.c /link Ws2_32.lib``` and ```cl pollclient.c /link Ws2_32.lib```

I haven't compiled this with anything else.
