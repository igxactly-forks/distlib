// Copyright (c) 2016 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 10/9/16.

#pragma once

#include <memory>
#include <netdb.h>
#include <sstream>


/**
 * Given a Linux {@link struct sockaddr} structure, it returns a string that
 * represents the {@literal host:port} for the network address.
 *
 * @param addr the address information struct
 * @param socklen the size of the struct pointed at by `addr`
 * @return a string of the form "localhost:8080"
 */
std::string inetAddress(const struct sockaddr *addr, socklen_t socklen);


/**
 * Given a port number will return a suitable socket address in string
 * format for the server to be listening on (e.g., "tcp://0.0.0.0:5000").
 *
 * @param port the host port the server would like to listen to
 * @return a string suitable for use with ZMQ {@link socket_t socket()} call.
 *      Or an empty string, if no suitable socket is available.
 */
std::string sockAddr(unsigned int port);
