#pragma once
#ifndef _CHROMAAPIIMPL_H_
#define _CHROMAAPIIMPL_H_

//#define RAZERSDK 1

#ifndef DLL_COMPILED
#define DLL_INTERNAL __declspec( dllexport )
#endif 

#ifdef RAZERSDK
typedef RZRESULT(*CREATEKEYBOARDEFFECT)(ChromaSDK::Keyboard::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEMOUSEEFFECT)(ChromaSDK::Mouse::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEHEADSETEFFECT)(ChromaSDK::Headset::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEMOUSEPADEFFECT)(ChromaSDK::Mousepad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEKEYPADEFFECT)(ChromaSDK::Keypad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
#else
typedef long(*INITRAZERDRIVER)(struct hid_device** hdev);
typedef void(*CLOSE)(struct usb_dev_handle* deviceHandle);
#endif

class ChromaAPI {
public:
	ChromaAPI();
	~ChromaAPI();
	int initialize();

#ifdef RAZERSDK
	CREATEKEYBOARDEFFECT createKeyboardEffect = nullptr;
	CREATEMOUSEEFFECT createMouseEffect = nullptr;
	CREATEHEADSETEFFECT createHeadsetEffect = nullptr;
	CREATEMOUSEPADEFFECT createMousepadEffect = nullptr;
	CREATEKEYPADEFFECT createKeypadEffect = nullptr;
#else
	INITRAZERDRIVER init_razer_kbd_driver2 = nullptr;
	struct device_attribute* devkbd_attr_matrix_effect_custom2 = nullptr;
	struct device_attribute* devkbd_attr_matrix_custom_frame2 = nullptr;

	INITRAZERDRIVER init_razer_firefly_driver2 = nullptr;
	struct device_attribute* devfirefly_attr_matrix_effect_custom2 = nullptr;
	struct device_attribute* devfirefly_attr_matrix_custom_frame2 = nullptr;

	INITRAZERDRIVER init_razer_mouse_driver2 = nullptr;
	struct device_attribute* devmouse_attr_matrix_effect_custom2 = nullptr;
	struct device_attribute* devmouse_attr_matrix_custom_frame2 = nullptr;

	INITRAZERDRIVER init_razer_mug_driver2 = nullptr;
	struct device_attribute* devmug_attr_matrix_effect_custom2 = nullptr;
	struct device_attribute* devmug_attr_matrix_custom_frame2 = nullptr;

	INITRAZERDRIVER init_razer_kraken_driver2 = nullptr;
	//struct device_attribute* devkraken_attr_matrix_effect_custom2 = nullptr;
	//struct device_attribute* devkraken_attr_matrix_custom_frame2 = nullptr;
	
	INITRAZERDRIVER init_razer_core_driver2 = nullptr;
	struct device_attribute* devcore_attr_matrix_effect_custom2 = nullptr;
	struct device_attribute* devcore_attr_matrix_custom_frame2 = nullptr;

	CLOSE close = nullptr;

	void createEffect(ChromaSDK::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	void createKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	void createMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	void createHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	void createMousepadEffect(ChromaSDK::Mousepad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	void createKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
#endif

private:
#ifdef RAZERSDK
	HMODULE m_ChromaSDKModule;
	BOOL isDeviceConnected(RZDEVICEID deviceId);
#else
	HMODULE m_ChromaLinuxModule;

	std::set<struct device*> deviceMice;
	std::set<struct device*> deviceMice2;
	std::set<struct device*> deviceFireflies;
	std::set<struct device*> deviceHeadsets;
	std::set<struct device*> deviceCupholders;
	std::set<struct device*> deviceKeyboards;

	void openDevices();
	void closeDevice(struct usb_dev_handle* deviceHandle);
#endif
};

#endif
