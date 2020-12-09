cmake_minimum_required(VERSION 3.5)
project(mc2tcpd)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
link_libraries(pthread)

add_executable(mc2tcpd
        chunckedstream.h
        circbuffer.h
        commandlineparameters.h
        configuration.h
        ipaddress.h
        ipaddress.cpp
        jobqueue.h
        main.cpp
        objectpool.h
        onscopeexit.h
        PoolAllocator.h
        rjthread.h
        socket.h
        socketenums.h
        tcpmcoutput.cpp
        tcpmcoutput.h
        tcpsocket.h
        udppacket.h
        udppacket.cpp
        udprecv.cpp
        udprecv.h
        udprecvpool.cpp
        udprecvpool.h
        udpsocket.h
        udptcpstreamer.cpp
        udptcpstreamer.h
        unixdaemon.h
        zsocket.cpp
        zsocket.h
        ztcpsocket.cpp
        ztcpsocket.h
        zudpsocket.cpp
        zudpsocket.h)
