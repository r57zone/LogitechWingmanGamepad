// Logitech WingMan Wheel with Arduino pedals

#include <windows.h>
#include <mutex>
#include <iostream>
#include "ViGEm\Client.h"
#include "IniReader\IniReader.h"
#include <thread>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

bool ExternalPedalsConnected = false;
HANDLE hSerial;
std::thread *pArduinoReadThread = NULL;
float PedalsValues[2];

#define XINPUT_GAMEPAD_DPAD_UP          0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
#define XINPUT_GAMEPAD_GUIDE            0x0400
#define XINPUT_GAMEPAD_START            0x0010
#define XINPUT_GAMEPAD_BACK             0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
#define XINPUT_GAMEPAD_A                0x1000
#define XINPUT_GAMEPAD_B                0x2000
#define XINPUT_GAMEPAD_X                0x4000
#define XINPUT_GAMEPAD_Y				0x8000

void ArduinoRead()
{
	DWORD bytesRead;

	while (ExternalPedalsConnected) {
		ReadFile(hSerial, &PedalsValues, sizeof(PedalsValues), &bytesRead, 0);

		if (PedalsValues[0] > 1.0 || PedalsValues[0] < 0 || PedalsValues[1] > 1.0 || PedalsValues[1] < 0)
		{
			PedalsValues[0] = 0;
			PedalsValues[1] = 0;

			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
		}

		if (bytesRead == 0) Sleep(1);
	}
}

static std::mutex m;
VOID CALLBACK notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
)
{
	m.lock();
	m.unlock();
}

SHORT ToLeftStick(double Value)
{
	if (Value < -32767)
		Value = -32767;
	else if (Value > 32767)
		Value = 32767;
	return trunc(Value);
}

int main(int argc, char **argv)
{
	SetConsoleTitle("Logitech WingMan 1.0");
	// Config parameters
	CIniReader IniFile("Config.ini");

	int SleepTimeOut = IniFile.ReadInteger("Gamepad", "SleepTimeOut", 1);

	int ExternalPedalsCOMPort = IniFile.ReadInteger("ExternalPedals", "COMPort", 3);
	if (ExternalPedalsCOMPort != 0) {
		char sPortName[32];
		sprintf_s(sPortName, "\\\\.\\COM%d", ExternalPedalsCOMPort);

		hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (hSerial != INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_NOT_FOUND) {

			DCB dcbSerialParams = { 0 };
			dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

			if (GetCommState(hSerial, &dcbSerialParams))
			{
				dcbSerialParams.BaudRate = CBR_115200;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;

				if (SetCommState(hSerial, &dcbSerialParams))
				{
					ExternalPedalsConnected = true;
					PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
					pArduinoReadThread = new std::thread(ArduinoRead);
				}
			}
		}
	}

	int SkipPollCount = 0;

	const auto client = vigem_alloc();
	auto ret = vigem_connect(client);

	const auto x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);

	XUSB_REPORT report;

	JOYINFOEX joyInfo;
	joyInfo.dwFlags = JOY_RETURNALL;
	joyInfo.dwSize = sizeof(joyInfo);

	int JoyIndex = JOYSTICKID1;

	if (joyGetPosEx(JOYSTICKID1, &joyInfo) == JOYERR_NOERROR)
		JoyIndex = JOYSTICKID1;
	else if (joyGetPosEx(JOYSTICKID2, &joyInfo) == JOYERR_NOERROR)
		JoyIndex = JOYSTICKID2;

	printf("\n");
	if (joyGetPosEx(JoyIndex, &joyInfo) == JOYERR_NOERROR)
		printf(" Wheel connected.\n");
	else
		printf(" Wheel not connected.\n");
	
	int CalibMin = IniFile.ReadInteger("Gamepad", "WheelMin", 2816);
	int CalibMax = IniFile.ReadInteger("Gamepad", "WheelMax", 41090);
	int WheelPercentCenterOffset = IniFile.ReadInteger("Gamepad", "WheelPercentCenterOffset", 0);
	float Sensitivity = IniFile.ReadInteger("Gamepad", "Sensitivity", 100) / 100.0f;

	
	if (ExternalPedalsConnected)
		printf(" External pedals connected\n");

	printf(" Press ALT + F8 for show wheel percent.\n");
	printf(" Press ALT + F9 for get calibration data.\n");

	bool Calibration = false;
	bool ShowPercentWheel = false;

	while (! ( GetAsyncKeyState(VK_LMENU) & 0x8000 && GetAsyncKeyState(VK_ESCAPE) & 0x8000 ) )
	{
		XUSB_REPORT_INIT(&report);

		joyGetPosEx(JoyIndex, &joyInfo);

		// Calibration
		if (((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) && (GetAsyncKeyState(VK_F9) & 0x8000) != 0  && SkipPollCount == 0)
		{
			SkipPollCount = 15;
			Calibration = !Calibration;
			if (Calibration) {
				CalibMax = 0;
				CalibMin = 65535;
			}
			system("cls");
			printf(" Turn the steering wheel to left and right, then the save the values to config and restart app.\n");
			Sleep(4000);
		}

		if (Calibration) {
			if (joyInfo.dwXpos < CalibMin)
				CalibMin = joyInfo.dwXpos;
			if (joyInfo.dwXpos > CalibMax)
				CalibMax = joyInfo.dwXpos;
			printf(" Current=%d Min=%d, Max=%d\n", joyInfo.dwXpos, CalibMin, CalibMax);
		}

		else
			report.sThumbLX = ToLeftStick(((joyInfo.dwXpos * 100 / (CalibMax - CalibMin) + WheelPercentCenterOffset) * 655.35 - 32767) * Sensitivity);

		if (((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) && (GetAsyncKeyState(VK_F8) & 0x8000) != 0 && SkipPollCount == 0) 
		{
			ShowPercentWheel = !ShowPercentWheel;
			SkipPollCount = 15;
			printf(" The percentage must be between 0 and 100, if it is not, then change the offset parameter in the config and restart app.\n");
			Sleep(4000);
		}
		if (ShowPercentWheel)
			printf(" Wheel percent=%d\n", joyInfo.dwXpos * 100 / (CalibMax - CalibMin) + WheelPercentCenterOffset);

		if (ExternalPedalsConnected) {
			report.bLeftTrigger = PedalsValues[0] * 255;
			report.bRightTrigger = PedalsValues[1] * 255;
		}

		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON3 ? XINPUT_GAMEPAD_BACK : 0; // Сверху слева руля
		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON4 ? XINPUT_GAMEPAD_START : 0; // Сверху справа руля

		if (ExternalPedalsConnected) {
			report.wButtons |= joyInfo.dwButtons & JOY_BUTTON5 ? XINPUT_GAMEPAD_B : 0; // Слева спереди на руле
			report.wButtons |= joyInfo.dwButtons & JOY_BUTTON6 ? XINPUT_GAMEPAD_A : 0; // Справа спереди на руле
			report.wButtons |= joyInfo.dwButtons & JOY_BUTTON1 ? XINPUT_GAMEPAD_X : 0; // Бампер слева за рулём
			report.wButtons |= joyInfo.dwButtons & JOY_BUTTON2 ? XINPUT_GAMEPAD_Y : 0; // Бампер справа за рулём
		} else {
			report.bLeftTrigger = joyInfo.dwButtons & JOY_BUTTON5 ? 255 : 0;
			report.bRightTrigger = joyInfo.dwButtons & JOY_BUTTON6 ? 255 : 0;
			report.wButtons |= joyInfo.dwButtons & JOY_BUTTON1 ? XINPUT_GAMEPAD_B : 0; // Бампер слева за рулём
			report.wButtons |= joyInfo.dwButtons & JOY_BUTTON2 ? XINPUT_GAMEPAD_A : 0; // Бампер справа за рулём
		}

		if (SkipPollCount > 0) SkipPollCount--;
		
		ret = vigem_target_x360_update(client, x360, report);

		Sleep(SleepTimeOut);
	}

	if (ExternalPedalsConnected) {
		ExternalPedalsConnected = false;
		pArduinoReadThread->join();
		delete pArduinoReadThread;
		pArduinoReadThread = nullptr;
		CloseHandle(hSerial);
	}

	vigem_target_x360_unregister_notification(x360);
	vigem_target_remove(client, x360);
	vigem_target_free(x360);
}
