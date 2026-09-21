#pragma once

#ifndef _WIN32_WCE

//
// Modeless sensor panel for the Win32 simulator.
//
// On a real terminal the SIU reports the door / ear-jack sensors and
// CDEV_DIO::WatchSensor() posts WM_DIO_NOTIFY whenever the byte changes. There
// is no hardware on the desktop, so this panel stands in for it: each check box
// owns one bit of that sensor byte and can be toggled while the ATM is running.
//
// The window is modeless and lives on its own UI thread, so toggling a sensor
// never blocks the ATM, and the ATM never blocks the panel.
//
// Bit layout (see CSVC_Manager::OnDioNotify):
//    0x01  front door open
//    0x02  safe door open
//    0x04  upper frame open   (accepted, but the ATM never reads it)
//    0x10  ear jack inserted
//
// Those are the *panel's* bits: set means the sensor has tripped. The front
// door is active low on the wire (FRONT_DOOR_REVERSE), so GetSensorByte()
// inverts that bit on the way out - an all-clear panel therefore reports a
// closed front door instead of tripping Z20001.
//
// The initial value comes from DeviceSim.ini, [DIO] SensorStatus, and uses the
// panel's bits: 0 means every sensor idle.
//

class CDevSimSensorDlg
{
public:
	// Shows the panel (first call creates it). Safe to call more than once.
	static void	Show();

	// Tears the panel down; called from CDEV_DIO::Close().
	static void	Destroy();

	// Current sensor byte. Safe to call from any thread - this is what
	// CDEV_DIO::GetSensor() and the watch thread read.
	static BYTE	GetSensorByte();

private:
	static void			ApplyToggle(int nBit, BOOL bOn);
	static UINT __cdecl	PanelThread(LPVOID pParam);
	static LRESULT CALLBACK	PanelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif	// !_WIN32_WCE
