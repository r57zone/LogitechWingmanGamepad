// Logitech WingMan Wheel with Arduino pedals

#include <windows.h>
#include <mutex>
#include <iostream>
#include "ViGEm\Client.h"
#include "IniReader\IniReader.h"
#include <thread>
#include <mmsystem.h>
#include <atlstr.h>

#pragma comment(lib, "winmm.lib")

std::vector <std::string> GamepadProfiles;
int ProfileIndex = 0;

bool ExternalPedalsDInputConnected = false;
JOYINFOEX ExternalPedalsJoyInfo;
JOYCAPS ExternalPedalsJoyCaps;
int ExternalPedalsJoyIndex = JOYSTICKID1;

bool ExternalPedalsArduinoConnected = false;
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

	while (ExternalPedalsArduinoConnected) {
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

int clamp(int value, int min_value, int max_value) {
	if (value < min_value)
		return min_value;
	else if (value > max_value)
		return max_value;
	else
		return value;
}

bool IsKeyPressed(int KeyCode) {
	return (GetAsyncKeyState(KeyCode) & 0x8000) != 0;
}

int GamepadKeyNameToKeyCode(std::string KeyName) {
	std::transform(KeyName.begin(), KeyName.end(), KeyName.begin(), ::toupper);

	if (KeyName == "NONE") return 0;

	else if (KeyName == "DPAD-UP") return XINPUT_GAMEPAD_DPAD_UP;
	else if (KeyName == "DPAD-DOWN") return XINPUT_GAMEPAD_DPAD_DOWN;
	else if (KeyName == "DPAD-LEFT") return XINPUT_GAMEPAD_DPAD_LEFT;
	else if (KeyName == "DPAD-RIGHT") return XINPUT_GAMEPAD_DPAD_RIGHT;
	else if (KeyName == "BACK") return XINPUT_GAMEPAD_BACK;
	else if (KeyName == "START") return XINPUT_GAMEPAD_START;
	else if (KeyName == "LEFT-STICK") return XINPUT_GAMEPAD_LEFT_THUMB;
	else if (KeyName == "RIGHT-STICK") return XINPUT_GAMEPAD_RIGHT_THUMB;
	else if (KeyName == "LEFT-SHOULDER") return XINPUT_GAMEPAD_LEFT_SHOULDER;
	else if (KeyName == "RIGHT-SHOULDER") return XINPUT_GAMEPAD_RIGHT_SHOULDER;
	else if (KeyName == "A") return XINPUT_GAMEPAD_A;
	else if (KeyName == "B") return XINPUT_GAMEPAD_B;
	else if (KeyName == "X") return XINPUT_GAMEPAD_X;
	else if (KeyName == "Y") return XINPUT_GAMEPAD_Y;
	else if (KeyName == "BUTTON1") return JOY_BUTTON1;
	else if (KeyName == "BUTTON2") return JOY_BUTTON2;
	else if (KeyName == "BUTTON3") return JOY_BUTTON3;
	else if (KeyName == "BUTTON4") return JOY_BUTTON4;
	else if (KeyName == "BUTTON5") return JOY_BUTTON5;
	else if (KeyName == "BUTTON6") return JOY_BUTTON6;

	else return 0;
}

struct _ProfileButtons {
	int Button1 = 0;
	int Button2 = 0;
	int Button3 = 0;
	int Button4 = 0;
	int Button5 = 0;
	int Button6 = 0;
	int LeftPedal = 0;
	int RightPedal = 0;
}; _ProfileButtons ProfileButtons;

void LoadProfile(std::string ProfileFile) {
	CIniReader IniFile("Profiles\\" + ProfileFile);
	ProfileButtons.Button1 = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "BUTTON1", "NONE"));
	ProfileButtons.Button2 = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "BUTTON2", "NONE"));
	ProfileButtons.Button3 = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "BUTTON3", "NONE"));
	ProfileButtons.Button4 = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "BUTTON4", "NONE"));
	ProfileButtons.Button5 = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "BUTTON5", "NONE"));
	ProfileButtons.Button6 = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "BUTTON6", "NONE"));
	ProfileButtons.LeftPedal = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "LEFT-PEDAL", "NONE"));
	ProfileButtons.RightPedal = GamepadKeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "RIGHT-PEDAL", "NONE"));
}

void ExternalPedalsDInputSearch() {
	ExternalPedalsDInputConnected = false;
	if (joyGetPosEx(JOYSTICKID1, &ExternalPedalsJoyInfo) == JOYERR_NOERROR &&
		joyGetDevCaps(JOYSTICKID1, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps)) == JOYERR_NOERROR &&
		ExternalPedalsJoyCaps.wNumButtons == 16) { // DualSense - 15, DigiJoy - 16
		ExternalPedalsJoyIndex = JOYSTICKID1;
		ExternalPedalsDInputConnected = true;

		joyGetDevCaps(JOYSTICKID1, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps));
	} 
	else if (joyGetPosEx(JOYSTICKID2, &ExternalPedalsJoyInfo) == JOYERR_NOERROR &&
		joyGetDevCaps(JOYSTICKID2, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps)) == JOYERR_NOERROR &&
		ExternalPedalsJoyCaps.wNumButtons == 16) {
		ExternalPedalsJoyIndex = JOYSTICKID2;
		ExternalPedalsDInputConnected = true;

		joyGetDevCaps(1, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps));
	}
	else if (joyGetPosEx(2, &ExternalPedalsJoyInfo) == JOYERR_NOERROR && // Оказывается может быть джойстик 3
		joyGetDevCaps(2, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps)) == JOYERR_NOERROR &&
		ExternalPedalsJoyCaps.wNumButtons == 16) {
		ExternalPedalsJoyIndex = 2;
		ExternalPedalsDInputConnected = true;

		joyGetDevCaps(2, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps));
	}
}

struct _AppStatus {
	bool WheelConneted = false;
	int WheelMode = 0;
	//bool ExternalPedalsDInputConnected = false;
	//bool ExternalPedalsArduinoConnected = false;
	bool ExternalPedalsDInputSearch = false;

	bool Calibration = false;

}; _AppStatus AppStatus;

void MainTextUpdate() {
	system("cls");
	printf("\n");
	if (AppStatus.WheelConneted)
		printf(" Wheel connected.\n");
	else
		printf(" Wheel not connected.\n");

	if (AppStatus.WheelMode == 0)
		printf(" Mode: wheel + default pedals.\n");
	else if (AppStatus.WheelMode == 1)
		printf(" Mode: wheel + external pedals.\n");
	else
		printf(" Mode: only wheel.\n");
	printf(" Change mode with \"ALT + Left | Right\"\n");

	if (ExternalPedalsDInputConnected)
		printf(" External pedals DInput connected.\n");
	if (ExternalPedalsArduinoConnected)
		printf(" External pedals Arduino connected.\n");

	printf_s(" Gamepad profile: %s.\n Change profiles with \"ALT + Up | Down\"\n", GamepadProfiles[ProfileIndex].c_str());

	printf(" Press ALT + F9 for get calibration data.\n");
}

int main(int argc, char **argv)
{
	SetConsoleTitle("Logitech WingMan 1.1");
	// Config parameters
	CIniReader IniFile("Config.ini");

	AppStatus.WheelMode = IniFile.ReadInteger("Wheel", "Mode", 0);
	int ButtonCount = IniFile.ReadInteger("Wheel", "ButtonCount", 6);
	int PedalsDeadZone = IniFile.ReadInteger("Wheel", "PedalsDeadZone", 0) * 2.55;
	float DeadZone = IniFile.ReadInteger("Wheel", "DeadZone", 0);
	int CalibMin = IniFile.ReadInteger("Wheel", "Min", 2816);
	int CalibMax = IniFile.ReadInteger("Wheel", "Max", 41090);
	int CalibCenter = IniFile.ReadInteger("Wheel", "Center", 32767);
	int CalibCenterExternalPedals = IniFile.ReadInteger("Wheel", "CenterExternalPedals", 41090);
	int WheelPercentCenterOffset = IniFile.ReadInteger("Gamepad", "WheelPercentCenterOffset", 0);
	float Sensitivity = IniFile.ReadInteger("Gamepad", "Sensitivity", 100) / 100.0f;

	int SleepTimeOut = IniFile.ReadInteger("Gamepad", "SleepTimeOut", 1);
	AppStatus.ExternalPedalsDInputSearch = IniFile.ReadBoolean("ExternalPedals", "DInput", false);
	ExternalPedalsJoyInfo.dwFlags = JOY_RETURNALL;
	ExternalPedalsJoyInfo.dwSize = sizeof(ExternalPedalsJoyInfo);
	int ExternalPedalsCOMPort = IniFile.ReadInteger("ExternalPedals", "COMPort", 0);

	if (AppStatus.ExternalPedalsDInputSearch) // Dinput in priority
		ExternalPedalsDInputSearch();
	else if (ExternalPedalsCOMPort != 0) {
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
					ExternalPedalsArduinoConnected = true;
					PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
					pArduinoReadThread = new std::thread(ArduinoRead);
				}
			}
		}
	}

	// Search profiles
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile("Profiles\\*.ini", &ffd);
	GamepadProfiles.push_back("Default.ini");
	std::string DefaultProfile = IniFile.ReadString("Gamepad", "DefaultProfile", "Default.ini");
	int ProfileCount = 0;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(ffd.cFileName, "Default.ini")) { // Already added to the top of the list
				GamepadProfiles.push_back(ffd.cFileName);
				ProfileCount++;
				if (strcmp(ffd.cFileName, DefaultProfile.c_str()) == 0)
					ProfileIndex = ProfileCount;
			}
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	LoadProfile(GamepadProfiles[ProfileIndex]);

	int SkipPollCount = 0;
	int TickCount = 0;

	const auto client = vigem_alloc();
	auto ret = vigem_connect(client);

	const auto x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);

	XUSB_REPORT report;

	JOYINFOEX joyInfo;
	joyInfo.dwFlags = JOY_RETURNALL;
	joyInfo.dwSize = sizeof(joyInfo);
	JOYCAPS JoyCaps;

	int JoyIndex = JOYSTICKID1;

	if (joyGetPosEx(JOYSTICKID1, &joyInfo) == JOYERR_NOERROR &&
		joyGetDevCaps(JOYSTICKID1, &JoyCaps, sizeof(JoyCaps)) == JOYERR_NOERROR && JoyCaps.wNumButtons == ButtonCount) {
		JoyIndex = JOYSTICKID1;
		AppStatus.WheelConneted = true;
	}
	else if (joyGetPosEx(JOYSTICKID2, &joyInfo) == JOYERR_NOERROR &&
		joyGetDevCaps(JOYSTICKID2, &JoyCaps, sizeof(JoyCaps)) == JOYERR_NOERROR && JoyCaps.wNumButtons == ButtonCount) {
		JoyIndex = JOYSTICKID2;
		AppStatus.WheelConneted = true;
	}
	
	MainTextUpdate();

	while (! ( GetAsyncKeyState(VK_LMENU) & 0x8000 && GetAsyncKeyState(VK_ESCAPE) & 0x8000 ) )
	{
		XUSB_REPORT_INIT(&report);
		
		AppStatus.WheelConneted = joyGetPosEx(JoyIndex, &joyInfo) == JOYERR_NOERROR;

		if (AppStatus.WheelConneted == false) {
			ret = vigem_target_x360_update(client, x360, report);
			Sleep(SleepTimeOut);
		}
		
		if (SkipPollCount == 0) {
			// Переключение профилей
			if ((IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_UP) || IsKeyPressed(VK_DOWN))) && GetConsoleWindow() == GetForegroundWindow())
				{
					SkipPollCount = 15;
					if (IsKeyPressed(VK_UP)) if (ProfileIndex > 0) ProfileIndex--; else ProfileIndex = GamepadProfiles.size() - 1;
					if (IsKeyPressed(VK_DOWN)) if (ProfileIndex < GamepadProfiles.size() - 1) ProfileIndex++; else ProfileIndex = 0;
					LoadProfile(GamepadProfiles[ProfileIndex]);
					MainTextUpdate();
				}

			// Смена режима
			if ((IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_LEFT) || IsKeyPressed(VK_RIGHT))))
			{
				SkipPollCount = 15;
				if (IsKeyPressed(VK_LEFT)) if (AppStatus.WheelMode > 0) AppStatus.WheelMode--; else AppStatus.WheelMode = 2;
				if (IsKeyPressed(VK_RIGHT)) if (AppStatus.WheelMode < 2) AppStatus.WheelMode++; else AppStatus.WheelMode = 0;
				MainTextUpdate();
			}

			// Калибровка
			if (((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) && (GetAsyncKeyState(VK_F9) & 0x8000) != 0)
			{
				SkipPollCount = 15;
				AppStatus.Calibration = !AppStatus.Calibration;
				if (AppStatus.Calibration) {
					CalibMax = 0;
					CalibMin = 65535;
					system("cls");
					printf("\n Turn the steering wheel to left and right, then the save the values to config and restart app.\n");
					TickCount = 300;
				} else
					MainTextUpdate();
			}

		}

		if (AppStatus.Calibration && TickCount == 0) {
			if (joyInfo.dwXpos < CalibMin)
				CalibMin = joyInfo.dwXpos;
			if (joyInfo.dwXpos > CalibMax)
				CalibMax = joyInfo.dwXpos;
			printf(" Center=%d Min=%d, Max=%d\n", joyInfo.dwXpos, CalibMin, CalibMax);
		}	

		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON1 ? ProfileButtons.Button1 : 0; // Бампер слева за рулём
		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON2 ? ProfileButtons.Button2 : 0; // Бампер справа за рулём
		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON3 ? ProfileButtons.Button3 : 0; // Сверху слева руля
		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON4 ? ProfileButtons.Button4 : 0; // Сверху справа руля
		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON5 ? ProfileButtons.Button5 : 0; // Слева спереди на руле
		report.wButtons |= joyInfo.dwButtons & JOY_BUTTON6 ? ProfileButtons.Button6 : 0; // Справа спереди на руле

		SHORT AxisX = 0;
		int AxisPercent = 0;

		// Режим руль + стандартные педали
		if (AppStatus.WheelMode == 0) {
			//int WheelAxis = clamp(joyInfo.dwXpos - 32767, -32767, 32767);

			//report.sThumbLX = WheelAxis;
			if (joyInfo.dwXpos > CalibCenter) {
				AxisPercent = (clamp(joyInfo.dwXpos - CalibCenter, 0, 65535)) * 100 / (65535 - CalibCenter);
				if (AxisPercent <= DeadZone) AxisPercent = 0;
				AxisX = clamp(AxisPercent * 327.67 * Sensitivity, 0, 32767);
				//printf("%d\t%d\t%d\n", joyInfo.dwXpos, AxisX, AxisPercent);
			} else {
				AxisPercent = 100 - (joyInfo.dwXpos * 100) / (65535 - CalibCenter);
				if (AxisPercent <= DeadZone) AxisPercent = 0;
				AxisX = clamp(AxisPercent * -327.67, -32767, 0);
				//printf("%d\t%d\t%d\n", joyInfo.dwXpos, AxisX, AxisPercent);
			}

			//printf("%d %d\n", joyInfo.dwXpos, WheelAxis);

			int LeftPedal = clamp((clamp(joyInfo.dwYpos, 32767, 65535) - 32767) / 128, 0, 255);
			int RightPedal = clamp((32767 - clamp(joyInfo.dwYpos, 0, 32767)) / 128, 0, 255);

			// Мертвая зона
			if (LeftPedal < PedalsDeadZone) LeftPedal = 0;
			if (RightPedal < PedalsDeadZone) RightPedal = 0;

			report.bLeftTrigger = LeftPedal;
			report.bRightTrigger = RightPedal;
		}

		// Режим руль + внешние педали
		else if (AppStatus.WheelMode == 1) {
			if (ExternalPedalsDInputConnected) {
				if (joyGetPosEx(ExternalPedalsJoyIndex, &ExternalPedalsJoyInfo) == JOYERR_NOERROR) {
					report.bLeftTrigger = ExternalPedalsJoyInfo.dwVpos / 256;
					report.bRightTrigger = ExternalPedalsJoyInfo.dwUpos / 256;
				}
			} else if (ExternalPedalsArduinoConnected) {
				report.bLeftTrigger = PedalsValues[0] * 255;
				report.bRightTrigger = PedalsValues[1] * 255;
			}
		}

		// Режим руль без каких-либо педалей
		else if (AppStatus.WheelMode == 2) {
			report.bLeftTrigger = joyInfo.dwButtons & ProfileButtons.LeftPedal ? 255 : 0;
			report.bRightTrigger = joyInfo.dwButtons & ProfileButtons.RightPedal ? 255 : 0;
		}

		//if (AppStatus.Calibration == false && AppStatus.ShowPercentWheel == false)
		if (AppStatus.WheelMode == 1 || AppStatus.WheelMode == 2)
		{
			if (joyInfo.dwXpos > CalibCenterExternalPedals) {
				AxisPercent = (clamp(joyInfo.dwXpos - CalibCenterExternalPedals, 0, 65535)) * 100 / (CalibMax - CalibCenterExternalPedals);
				if (AxisPercent <= DeadZone) AxisPercent = 0;
				AxisX = clamp(AxisPercent * 327.67 * Sensitivity, 0, 32767);
				//printf("%d\t%d\t%d\n", joyInfo.dwXpos, AxisX, AxisPercent);
			} else {
				AxisPercent = 100 - (clamp(joyInfo.dwXpos - CalibMin, 0, 65535) * 100) / (CalibCenterExternalPedals - CalibMin);
				if (AxisPercent <= DeadZone) AxisPercent = 0;
				AxisX = clamp(AxisPercent * -327.67, -32767, 0);
				//printf("%d\t%d\t%d\n", joyInfo.dwXpos, AxisX, AxisPercent);
			}
		}
		report.sThumbLX = AxisX;

		if (SkipPollCount > 0) SkipPollCount--;
		if (TickCount > 0) TickCount--;
		
		ret = vigem_target_x360_update(client, x360, report);

		Sleep(SleepTimeOut);
	}

	if (ExternalPedalsArduinoConnected) {
		ExternalPedalsArduinoConnected = false;
		pArduinoReadThread->join();
		delete pArduinoReadThread;
		pArduinoReadThread = nullptr;
		CloseHandle(hSerial);
	}

	vigem_target_x360_unregister_notification(x360);
	vigem_target_remove(client, x360);
	vigem_target_free(x360);
}
