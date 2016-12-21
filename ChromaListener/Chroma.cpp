#include "stdafx.h"
#include "Chroma.h"
#include "tinyxml.h"

#ifdef _WIN64
#define CHROMASDKDLL        _T("RzChromaSDK64.dll")
#else
#define CHROMASDKDLL        _T("RzChromaSDK.dll")
#endif

using namespace ChromaSDK;
using namespace ChromaSDK::Keyboard;
using namespace ChromaSDK::Keypad;
using namespace ChromaSDK::Mouse;
using namespace ChromaSDK::Mousepad;
using namespace ChromaSDK::Headset;
using namespace std;


typedef RZRESULT(*INIT)(void);
typedef RZRESULT(*UNINIT)(void);
typedef RZRESULT(*CREATEEFFECT)(RZDEVICEID deviceId, ChromaSDK::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEKEYBOARDEFFECT)(ChromaSDK::Keyboard::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEHEADSETEFFECT)(ChromaSDK::Headset::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEMOUSEPADEFFECT)(ChromaSDK::Mousepad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEMOUSEEFFECT)(ChromaSDK::Mouse::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEKEYPADEFFECT)(ChromaSDK::Keypad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
typedef RZRESULT(*SETEFFECT)(RZEFFECTID effectId);
typedef RZRESULT(*DELETEEFFECT)(RZEFFECTID effectId);
typedef RZRESULT(*REGISTEREVENTNOTIFICATION)(HWND hWnd);
typedef RZRESULT(*UNREGISTEREVENTNOTIFICATION)(void);
typedef RZRESULT(*QUERYDEVICE)(RZDEVICEID deviceId, ChromaSDK::DEVICE_INFO_TYPE &deviceInfo);

INIT init = nullptr;
UNINIT UnInit = nullptr;
CREATEEFFECT createEffect = nullptr;
CREATEKEYBOARDEFFECT createKeyboardEffect = nullptr;
CREATEMOUSEEFFECT createMouseEffect = nullptr;
CREATEHEADSETEFFECT createHeadsetEffect = nullptr;
CREATEMOUSEPADEFFECT createMousepadEffect = nullptr;
CREATEKEYPADEFFECT createKeypadEffect = nullptr;
SETEFFECT setEffect = nullptr;
DELETEEFFECT deleteEffect = nullptr;
QUERYDEVICE queryDevice = nullptr;

static void chromathread(void *param) {
	Chroma* chroma = static_cast<Chroma*>(param);
	chroma->chromaThread();
}

static void effectthread(void *param) {
	Chroma* chroma = static_cast<Chroma*>(param);
	chroma->effectThread();
}

/*
BOOL Chroma::isDeviceConnected(RZDEVICEID deviceId) {
if (queryDevice != nullptr) {
ChromaSDK::DEVICE_INFO_TYPE deviceInfo = {};
RZRESULT result = queryDevice(deviceId, deviceInfo);
return deviceInfo.Connected;
}
return FALSE;
}
*/

Chroma::Chroma() :m_ChromaSDKModule(nullptr) {
}

Chroma::~Chroma() {
}

BOOL Chroma::mapPixels() {
	TiXmlDocument doc("mapping.xml");
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("map");
	if (root) {
		TiXmlElement* element = root->FirstChildElement("mapping");
		while (element) {
			//
			UINT universe = atoi(element->Attribute("universe"));
			universeSet.insert(universe);
			UINT propnum = atoi(element->Attribute("propertyNum"));

			mapping_t localElement;
			localElement.deviceType = atoi(element->Attribute("deviceType"));
			localElement.row = atoi(element->Attribute("row") == NULL ? "0" : element->Attribute("row"));
			localElement.col = atoi(element->Attribute("col") == NULL ? "0" : element->Attribute("col"));
			localElement.count = atoi(element->Attribute("count") == NULL ? "0" : element->Attribute("count"));
			localElement.color = atoi(element->Attribute("color") == NULL ? "0" : element->Attribute("color"));
			localElement.name = _strdup(element->GetText());

			pair<UINT, UINT> key = std::make_pair(universe, propnum);
			propertySet_t propertySet;
			if (propertyNumMap.count(key) > 0)
				propertySet = propertyNumMap[key];
			propertySet.push_back(localElement);
			propertyNumMap[key] = propertySet;

			element = element->NextSiblingElement("mapping");
		}
	}

	return TRUE;
}

BOOL Chroma::initialize() {
	mapPixels();

	// clear all LEDs
	COLORREF defval = RGB(0, 0, 0);
	for (UINT row = 0; row < ChromaSDK::Keyboard::MAX_ROW; row++)
		for (UINT col = 0; col < ChromaSDK::Keyboard::MAX_COLUMN; col++)
			keyboard_effect.Color[row][col] = defval;
	for (UINT count = 0; count < ChromaSDK::Mousepad::MAX_LEDS; count++)
		mousemat_effect.Color[count] = defval;
	for (UINT count = 0; count < ChromaSDK::Mouse::MAX_LEDS; count++)
		mouse_effect.Color[count] = defval;
	for (UINT count = 0; count < ChromaSDK::Headset::MAX_LEDS; count++)
		headset_effect.Color[count] = defval;
	for (UINT row = 0; row < ChromaSDK::Keypad::MAX_ROW; row++)
		for (UINT col = 0; col < ChromaSDK::Keypad::MAX_COLUMN; col++)
			keypad_effect.Color[row][col] = defval;


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

	return TRUE;
}

/*
void Chroma::resetEffects(UINT deviceType) {
switch (deviceType) {
case 6:
if (createKeyboardEffect) createKeyboardEffect(ChromaSDK::Keyboard::CHROMA_NONE, nullptr, nullptr);
if (createMousepadEffect) createMousepadEffect(ChromaSDK::Mousepad::CHROMA_NONE, nullptr, nullptr);
if (createMouseEffect)    createMouseEffect(ChromaSDK::Mouse::CHROMA_NONE, nullptr, nullptr);
if (createHeadsetEffect)  createHeadsetEffect(ChromaSDK::Headset::CHROMA_NONE, nullptr, nullptr);
if (createKeypadEffect)   createKeypadEffect(ChromaSDK::Keypad::CHROMA_NONE, nullptr, nullptr);
break;
case 1:	if (createKeyboardEffect) createKeyboardEffect(ChromaSDK::Keyboard::CHROMA_NONE, nullptr, nullptr);		break;
case 2:	if (createMousepadEffect) createMousepadEffect(ChromaSDK::Mousepad::CHROMA_NONE, nullptr, nullptr);		break;
case 3:	if (createMouseEffect)    createMouseEffect(ChromaSDK::Mouse::CHROMA_NONE, nullptr, nullptr);			break;
case 4:	if (createHeadsetEffect)  createHeadsetEffect(ChromaSDK::Headset::CHROMA_NONE, nullptr, nullptr);		break;
case 5:	if (createKeypadEffect)   createKeypadEffect(ChromaSDK::Keypad::CHROMA_NONE, nullptr, nullptr);			break;
}
}
*/

BOOL Chroma::print(e131_packet_t* packet) {
/*
	printf("Root(");
	printf("prs:%04X", packet->preamble_size);
	printf(" pos:%04X", packet->postamble_size);
	printf(" aid:");
	for(int i =0;i<12;i++)
	printf("%02hhX", packet->acn_id[i]);
	printf(" rf:%04X", packet->root_flength);
	printf(" rv:%08X", packet->root_vector);
	printf(" cid:");
	for (int i = 0; i<16; i++)
	printf("%02hhX", packet->cid[i]);
	printf(")");
*/
/*
	printf("Frame(");
	printf("ffl:%04X", packet->frame_flength);
	printf(" fv:%08X", packet->frame_vector);
	printf(" sn:%s", packet->source_name);
	printf(" p:%02X", packet->priority);
	printf(" r:%04X", packet->reserved);
	printf(" sn:%02X", packet->sequence_number);
	printf(" o:%02X", packet->options);
	printf(" u:%04X", packet->universe);
	printf(")");
*/
/*
	printf("DMPLayer(");
	printf("dfl:%04X", packet->dmp_flength);
	printf(" dv:%02X", packet->dmp_vector);
	printf(" t:%02X", packet->type);
	printf(" fa:%04X", packet->first_address);
	printf(" ai:%04X", packet->address_increment);
	printf(" pvc:%04X", packet->property_value_count);
	printf(" pv:");
	for (int i = 0; i<32; i++) //print first 32
		printf("%02hhX", packet->property_values[i]);
	printf(")");
*/
	printf("\n sn:%02X", packet->sequence_number);
	printf(" u:%04X", eflip(packet->universe));
	printf(" sn:%s", packet->source_name);
	printf(" pvc:%04X", eflip(packet->property_value_count));
	printf(" pv:");
	for (int i = 1; i<eflip(packet->property_value_count); i++) //print first 32
		printf("%02hhX", packet->property_values[i]);
	return TRUE;
}

void Chroma::updateColor(UINT colorComponent, UINT color, COLORREF* led) {
	switch (colorComponent) {
	case 0:
		*led = RGB(color, (*led & 0x00FF00) >> 8, (*led & 0xFF0000) >> 16);
		break;
	case 1:
		*led = RGB((*led & 0x0000FF), color, (*led & 0xFF0000) >> 16);
		break;
	case 2:
		*led = RGB((*led & 0x0000FF), (*led & 0x00FF00) >> 8, color);
		break;
	}
}

BOOL Chroma::command(e131_packet_t* packet) {
	if (bDebug)
		print(packet);
	uint16_t universe = eflip(packet->universe);
	uint16_t property_value_count = eflip(packet->property_value_count);
	for (const auto& entry : propertyNumMap) {
		if ((universe == entry.first.first) && (property_value_count> entry.first.second)) {
			propertySet_t propertySet = entry.second;
			for (mapping_t node : propertySet) {
				UINT color = (packet->property_values[entry.first.second]);
				switch (node.deviceType) {
					case 0: break;
					case 1: updateColor(node.color, color, &keyboard_effect.Color[node.row][node.col]);	break;
					case 2: updateColor(node.color, color, &mousemat_effect.Color[node.count]);			break;
					case 3: updateColor(node.color, color, &mouse_effect.Color[node.count]);			break;
					case 4: updateColor(node.color, color, &headset_effect.Color[node.count]);			break;
					case 5: updateColor(node.color, color, &keypad_effect.Color[node.row][node.col]);	break;
				}
			}
		}
	}


	return TRUE;
}

void Chroma::startThread() {
	_beginthread(chromathread, 0, this);
	_beginthread(effectthread, 0, this);
}

void Chroma::chromaThread()
{
	while (command(&packet)) {
		Sleep(5);
	}
}

void Chroma::effectThread()
{
	while (command(&packet)) {
		RZRESULT resultKeyboard = createKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &keyboard_effect, nullptr);
		RZRESULT resultMousemat = createMousepadEffect(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousemat_effect, nullptr);
		RZRESULT resultMouse = createMouseEffect(ChromaSDK::Mouse::CHROMA_CUSTOM, &mouse_effect, nullptr);
		RZRESULT resultHeadset = createHeadsetEffect(ChromaSDK::Headset::CHROMA_CUSTOM, &headset_effect, nullptr);
		RZRESULT resultKeypad = createKeypadEffect(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypad_effect, nullptr);
		Sleep(5);
	}
}
