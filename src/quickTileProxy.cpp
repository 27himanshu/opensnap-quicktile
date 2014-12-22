/*
 * File:   quickTileProxy.cpp
 * Author: Fabrizio Lungo <fab@lungo.co.uk>
 *
 * Created on 21 December 2014, 11:05
 */

#include <dbus-cxx.h>
#include "quickTileProxy.h"

quickTileProxy::quickTileProxy(DBus::Connection::pointer connection) :
    object(connection->create_object_proxy(QT_DBUS_NAME, QT_OBJECT_PATH)),
        doCommand(*(object->create_method<bool,std::string>(QT_INTERFACE,"doCommand")))
{

}

