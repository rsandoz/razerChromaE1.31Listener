#include <set>
#include <tchar.h>
#include <wtypes.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>

#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "RzErrors.h"

#include "ChromaAPI.h"

#ifdef RAZERSDK

#ifdef _WIN64
#define CHROMASDKDLL        _T("RzChromaSDK64.dll")
#elif WIN32
#define CHROMASDKDLL        _T("RzChromaSDK.dll")
#endif

#else

#ifdef _WIN64
#define CHROMALINUXDLL        _T("ChromaDLL64.dll")
#elif WIN32
#define CHROMALINUXDLL        _T("ChromaDLL.dll")
#endif

#endif

using namespace ChromaSDK;
#ifdef RAZERSDK
using namespace ChromaSDK::Keyboard;
using namespace ChromaSDK::Keypad;
using namespace ChromaSDK::Mouse;
using namespace ChromaSDK::Mousepad;
using namespace ChromaSDK::Headset;

typedef RZRESULT(*INIT)(void);
typedef RZRESULT(*UNINIT)(void);
typedef RZRESULT(*CREATEEFFECT)(RZDEVICEID deviceId, ChromaSDK::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*SETEFFECT)(RZEFFECTID effectId);
typedef RZRESULT(*DELETEEFFECT)(RZEFFECTID effectId);
typedef RZRESULT(*REGISTEREVENTNOTIFICATION)(HWND hWnd);
typedef RZRESULT(*UNREGISTEREVENTNOTIFICATION)(void);
typedef RZRESULT(*QUERYDEVICE)(RZDEVICEID deviceId, ChromaSDK::DEVICE_INFO_TYPE &deviceInfo);

INIT init = nullptr;
UNINIT UnInit = nullptr;
CREATEEFFECT createEffect = nullptr;
SETEFFECT setEffect = nullptr;
DELETEEFFECT deleteEffect = nullptr;
QUERYDEVICE queryDevice = nullptr;
#else

typedef RZRESULT(*CREATEEFFECT)(RZDEVICEID deviceId, ChromaSDK::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);

CREATEEFFECT createEffect = nullptr;


void ChromaAPI::openDevices() {
	struct hid_device* hdev;
	unsigned int num;
	
	hdev = NULL;
	num = init_razer_kbd_driver2(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceKeyboards.insert(&hdev[i].dev);

	hdev = NULL;
	num = init_razer_firefly_driver2(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceFireflies.insert(&hdev[i].dev);

	hdev = NULL;
	num = init_razer_kraken_driver2(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceHeadsets.insert(&hdev[i].dev);

	hdev = NULL;
	num = init_razer_mug_driver2(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceCupholders.insert(&hdev[i].dev);

	hdev = NULL;
	num = init_razer_mouse_driver2(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceMice.insert(&hdev[i].dev);
}

void ChromaAPI::closeDevice(struct usb_dev_handle* deviceHandle) {
	if (deviceHandle) {
		close(deviceHandle);
		deviceHandle = 0;
	}
}

void ChromaAPI::createEffect(ChromaSDK::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
}

void ChromaAPI::createKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	//createKeyboardEffect - Keyboard USB drivers actually handle keypad calls 
}

void ChromaAPI::createKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	for (struct device* device : deviceKeyboards) {
		if (device) {
			//printf("kbd=%s %d\n", device->init_name, strcmp("razerkbd", device->init_name));
			switch (effect) {
			case ChromaSDK::Keyboard::CHROMA_CUSTOM: {
					const unsigned int maxCol = ChromaSDK::Keyboard::MAX_COLUMN;
					char buf[3 * maxCol + 4] = "\x00\x00";
					buf[2] = maxCol - 1;
					for (unsigned int row = 0; row < ChromaSDK::Keyboard::MAX_ROW; row++) {
						buf[0] = row;
						for (unsigned int col = 0; col < maxCol; col++) {
							unsigned long color = ((ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE*)pParam)->Color[row][col];
							buf[3 * col + 3] = (char)(color & 0x0000FF);
							buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
							buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
						}
						devkbd_attr_matrix_custom_frame2->store(device, NULL, (const char*)&buf[0], sizeof(buf) - 1);
					}
					devkbd_attr_matrix_effect_custom2->store(device, NULL, 0, 0);
				}
				break;
			default:
				printf("Keyboard::TBD(%d)\n", effect);
				break;
			}
		}
	}
}

void ChromaAPI::createMousepadEffect(ChromaSDK::Mousepad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	for (struct device* device : deviceFireflies) {
		if (device) {
			//printf("ff=%s %d\n", device->init_name, strcmp("razerfirefly", device->init_name));
			switch (effect) {
			case ChromaSDK::Mousepad::CHROMA_CUSTOM: {
					const unsigned int maxCol = ChromaSDK::Mousepad::MAX_LEDS;
					char buf[3 * maxCol + 4] = "\x00\x00";
					buf[2] = maxCol - 1;
					for (unsigned int col = 0; col < maxCol; col++) {
						unsigned long color = ((ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE*)pParam)->Color[col];
						buf[3 * col + 3] = (char)(color & 0x0000FF);
						buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
						buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
					}
					devfirefly_attr_matrix_custom_frame2->store(device, NULL, (const char*)&buf[0], sizeof(buf) - 1);
					devfirefly_attr_matrix_effect_custom2->store(device, NULL, 0, 0);
				}
				break;
			default:
				printf("Mousepad::TBD(%d)\n", effect);
				break;
			}
		}
	}
	for (struct device* device : deviceCupholders) {
		if (device) {
			switch (effect) {
			case ChromaSDK::Mousepad::CHROMA_CUSTOM: {
					const unsigned int maxCol = ChromaSDK::Mousepad::MAX_LEDS;
					char buf[3 * maxCol + 4] = "\x00\x00";
					buf[2] = maxCol - 1;
					for (unsigned int col = 0; col < maxCol; col++) {
						unsigned long color = ((ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE*)pParam)->Color[col];
						buf[3 * col + 3] = (char)(color & 0x0000FF);
						buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
						buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
					}
					//TBD (assumed to be supported in future, no namespace specifically for mug, but looks alot like a mousepad, check future rev of SDK)
					devmug_attr_matrix_custom_frame2->store(device, NULL, (const char*)&buf[0], sizeof(buf) - 1);
					devmug_attr_matrix_effect_custom2->store(device, NULL, 0, 0);
				}
				break;
			default:
				printf("Mug::TBD(%d)\n", effect);
				break;
			}
		}
	}
}

void ChromaAPI::createMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	for (struct device* device : deviceMice) {
		if (device) {
			switch (effect) {
				case ChromaSDK::Mouse::CHROMA_CUSTOM: {
					const unsigned int maxCol = 25;//can't use ChromaSDK::Mouse::MAX_LEDS as it is 30, must be 25 or lower or breaches razer_report.arguments
					char buf[3 * maxCol + 4] = "\x00\x00";
					buf[2] = maxCol - 1;
					for (unsigned int col = 0; col < maxCol; col++) {
						unsigned long color = ((ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE*)pParam)->Color[col];
						buf[3 * col + 3] = (char)(color & 0x0000FF);
						buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
						buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
					}
					devmouse_attr_matrix_custom_frame2->store(device, NULL, (const char*)&buf[0], sizeof(buf) - 1);
					devmouse_attr_matrix_effect_custom2->store(device, NULL, 0, 0);
				}
				break;
			}
		}
	}
	for (struct device* device : deviceMice2) {
		if (device) {
			switch (effect) {
			case ChromaSDK::Mouse::CHROMA_CUSTOM2: {
					const unsigned int maxCol = ChromaSDK::Mouse::MAX_COLUMN;
					char buf[3 * maxCol + 4] = "\x00\x00";
					buf[2] = maxCol - 1;
					for (unsigned int row = 0; row < ChromaSDK::Keyboard::MAX_ROW; row++) {
						buf[0] = row;
						for (unsigned int col = 0; col < maxCol; col++) {
							unsigned long color = ((ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2*)pParam)->Color[row][col];
							buf[3 * col + 3] = (char)(color & 0x0000FF);
							buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
							buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
						}
						devmouse_attr_matrix_custom_frame2->store(device, NULL, (const char*)&buf[0], sizeof(buf) - 1);
					}
					devmouse_attr_matrix_effect_custom2->store(device, NULL, 0, 0);
				}
				break;
			}
		}
	}
}


void ChromaAPI::createHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	for (struct device* device : deviceHeadsets) {
		if (device) {
			switch (effect) {
			case ChromaSDK::Headset::CHROMA_CUSTOM: {
					const unsigned int maxCol = ChromaSDK::Headset::MAX_LEDS;
					char buf[3 * maxCol + 4] = "\x00\x00";
					buf[2] = maxCol - 1;
					for (unsigned int col = 0; col < maxCol; col++) {
						unsigned long color = ((ChromaSDK::Headset::CUSTOM_EFFECT_TYPE*)pParam)->Color[col];
						buf[3 * col + 3] = (char)(color & 0x0000FF);
						buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
						buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
					}
					//TBD (RazerSDK namespace, headset, indicates a custom mode should be available) 
					//devkraken_attr_matrix_custom_frame2->store(device, NULL, (const char*)&buf[0], sizeof(buf) - 1);
					//devkraken_attr_matrix_effect_custom2->store(device, NULL, 0, 0);
				}
				break;
			default:
				printf("Mousepad::TBD(%d)\n", effect);
				break;
			}
		}
	}
}

#endif

#ifdef RAZERSDK
BOOL ChromaAPI::isDeviceConnected(RZDEVICEID deviceId) {
	if (queryDevice != nullptr) {
		ChromaSDK::DEVICE_INFO_TYPE deviceInfo = {};
		RZRESULT result = queryDevice(deviceId, deviceInfo);
		return deviceInfo.Connected;
	}
	return FALSE;
}
#endif

ChromaAPI::ChromaAPI() {
#ifdef RAZERSDK
	m_ChromaSDKModule = nullptr;
#else
	m_ChromaLinuxModule = nullptr;
#endif
}

ChromaAPI::~ChromaAPI() {
#ifndef RAZERSDK
	for (struct device* device : deviceMice) closeDevice(device->p.udev);
	for (struct device* device : deviceMice2) closeDevice(device->p.udev);
	for (struct device* device : deviceFireflies) closeDevice(device->p.udev);
	for (struct device* device : deviceHeadsets) closeDevice(device->p.udev);
	for (struct device* device : deviceCupholders) closeDevice(device->p.udev);
	for (struct device* device : deviceKeyboards) closeDevice(device->p.udev);
#endif
}

BOOL ChromaAPI::initialize() {

#ifdef RAZERSDK
	if (m_ChromaSDKModule == nullptr) {
		m_ChromaSDKModule = LoadLibrary(CHROMASDKDLL);
		if (m_ChromaSDKModule == nullptr)
			return FALSE;
	}

	if (init == nullptr) {
		RZRESULT result = RZRESULT_INVALID;
		init = reinterpret_cast<INIT>(GetProcAddress(m_ChromaSDKModule, "Init"));
		if (init) {
			result = init();
			if (result == RZRESULT_SUCCESS) {
				createEffect = reinterpret_cast<CREATEEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateEffect"));
				createKeyboardEffect = reinterpret_cast<CREATEKEYBOARDEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateKeyboardEffect"));
				createMouseEffect = reinterpret_cast<CREATEMOUSEEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateMouseEffect"));
				createHeadsetEffect = reinterpret_cast<CREATEHEADSETEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateHeadsetEffect"));
				createMousepadEffect = reinterpret_cast<CREATEMOUSEPADEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateMousepadEffect"));
				createKeypadEffect = reinterpret_cast<CREATEKEYPADEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateKeypadEffect"));
				setEffect = reinterpret_cast<SETEFFECT>(GetProcAddress(m_ChromaSDKModule, "SetEffect"));
				deleteEffect = reinterpret_cast<DELETEEFFECT>(GetProcAddress(m_ChromaSDKModule, "DeleteEffect"));
				queryDevice = reinterpret_cast<QUERYDEVICE>(GetProcAddress(m_ChromaSDKModule, "QueryDevice"));

				if (createEffect
					&& createKeyboardEffect
					&& createMouseEffect
					&& createHeadsetEffect
					&& createMousepadEffect
					&& createKeypadEffect
					&& setEffect
					&& deleteEffect
					&& queryDevice)
					return TRUE;
				else
					return FALSE;
			}
		}
	}

	GUID guidArray[] =
	{ BLACKWIDOW_CHROMA
		, BLACKWIDOW_CHROMA_TE
		, DEATHSTALKER_CHROMA
		, OVERWATCH_KEYBOARD
		, BLACKWIDOW_X_CHROMA
		, BLACKWIDOW_X_TE_CHROMA
		, ORNATA_CHROMA
		, BLADE_STEALTH
		, BLADE
		, DEATHADDER_CHROMA
		, MAMBA_CHROMA_TE
		, DIAMONDBACK_CHROMA
		, MAMBA_CHROMA
		, NAGA_EPIC_CHROMA
		, NAGA_CHROMA
		, OROCHI_CHROMA
		, NAGA_HEX_CHROMA
		, KRAKEN71_CHROMA
		, MANOWAR_CHROMA
		, FIREFLY_CHROMA
		, TARTARUS_CHROMA
		, ORBWEAVER_CHROMA
		, LENOVO_Y900
		, LENOVO_Y27
		, CORE_CHROMA };
	int len = _countof(guidArray);
	//int len = sizeof(guidArray) / sizeof(*guidArray);
	for (int i = 0; i < len; i++)
		if (isDeviceConnected(guidArray[i]))
			printf("Found #%d - {0x%8x, 0x%4x, 0x%4x, { 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x } }\n", i, guidArray[i].Data1, guidArray[i].Data2, guidArray[i].Data3, guidArray[i].Data4[0], guidArray[i].Data4[1], guidArray[i].Data4[2], guidArray[i].Data4[3], guidArray[i].Data4[4], guidArray[i].Data4[5], guidArray[i].Data4[6], guidArray[i].Data4[7]);
#else
	if (m_ChromaLinuxModule == nullptr) {
		m_ChromaLinuxModule = LoadLibrary(CHROMALINUXDLL);
		if (m_ChromaLinuxModule == nullptr)
			return FALSE;
	}

	init_razer_kbd_driver2 = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(m_ChromaLinuxModule, "init_razer_kbd_driver"));
	devkbd_attr_matrix_effect_custom2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devkbd_attr_matrix_effect_custom"));
	devkbd_attr_matrix_custom_frame2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devkbd_attr_matrix_custom_frame"));

	init_razer_firefly_driver2 = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(m_ChromaLinuxModule, "init_razer_firefly_driver"));
	devfirefly_attr_matrix_effect_custom2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devfirefly_attr_matrix_effect_custom"));
	devfirefly_attr_matrix_custom_frame2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devfirefly_attr_matrix_custom_frame"));

	init_razer_mouse_driver2 = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(m_ChromaLinuxModule, "init_razer_mouse_driver"));
	devmouse_attr_matrix_effect_custom2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devmouse_attr_matrix_effect_custom"));
	devmouse_attr_matrix_custom_frame2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devmouse_attr_matrix_custom_frame"));

	init_razer_mug_driver2 = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(m_ChromaLinuxModule, "init_razer_mug_driver"));
	devmug_attr_matrix_effect_custom2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devmug_attr_matrix_effect_custom"));
	devmug_attr_matrix_custom_frame2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devmug_attr_matrix_custom_frame"));

	init_razer_kraken_driver2 = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(m_ChromaLinuxModule, "init_razer_kraken_driver"));
	//devkraken_attr_matrix_effect_custom2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devkraken_attr_matrix_effect_custom"));
	//devkraken_attr_matrix_custom_frame2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devkraken_attr_matrix_custom_frame"));

	init_razer_core_driver2 = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(m_ChromaLinuxModule, "init_razer_core_driver"));
	devcore_attr_matrix_effect_custom2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devcore_attr_matrix_effect_custom"));
	devcore_attr_matrix_custom_frame2 = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(m_ChromaLinuxModule, "devcore_attr_matrix_custom_frame"));

	close = reinterpret_cast<CLOSE>(GetProcAddress(m_ChromaLinuxModule, "close"));

	typedef void(*INIT)(void);
	INIT init = reinterpret_cast<INIT>(GetProcAddress(m_ChromaLinuxModule, "init"));
	if (init)
		init();
	openDevices();
#endif

	//printf("Press enter to start E1.31 listener..");
	//getc(stdin);
	//printf("\n");

	return TRUE;
}
