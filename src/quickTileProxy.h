/*
 * File:   quickTileProxy.h
 * Author: Fabrizio Lungo <fab@lungo.co.uk>
 *
 * Created on 21 December 2014, 11:05
 */

#define QT_DBUS_NAME "com.ssokolow.QuickTile"
#define QT_OBJECT_PATH "/com/ssokolow/QuickTile"
#define QT_INTERFACE "com.ssokolow.QuickTile"

#ifndef QUICKTILEPROXY_H
#define	QUICKTILEPROXY_H

class quickTileProxy {
    DBus::ObjectProxy::pointer object;
public:
    DBus::MethodProxy<bool, std::string>& doCommand;
    quickTileProxy(DBus::Connection::pointer connection);
private:

};

#endif	/* QUICKTILEPROXY_H */

