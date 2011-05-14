#include "trayicon.h"

// Forward declaration
void qt_mac_set_dock_menu(QMenu *menu);

void TrayIcon::sysInstall()
{
	qt_mac_set_dock_menu(pop);
}

void TrayIcon::sysRemove()
{
	// Nothing to do on MacOS X
	// as we are only using the dock menu
}

void TrayIcon::sysUpdateIcon()
{
	// Nothing to do on MacOS X
	// as we are only using the dock menu
}

void TrayIcon::sysUpdateToolTip()
{
	// Nothing to do on MacOS X
	// as we are only using the dock menu
}
