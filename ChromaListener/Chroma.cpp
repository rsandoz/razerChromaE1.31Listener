#include <map>
#if defined(WIN32) || defined(_WIN64)
#include <process.h>
#include <wtypes.h>
#include <linux/init.h>
#else
#include <pthread.h>
#endif

#include "packet.h"
#include <vector>
#include <set>
#include "RzChromaSDKTypes.h"
#include "Chroma.h"
#include "tinyxml.h"
#include "ChromaAPI.h"

int bDebug = 0;
ChromaAPI m_ChromaAPI;

using namespace std;

static void chromathread(void *param) {
	Chroma* chroma = static_cast<Chroma*>(param);
	chroma->chromaThread();
}

static void effectthread(void *param) {
	Chroma* chroma = static_cast<Chroma*>(param);
	chroma->effectThread();
}

Chroma::Chroma() {
}

Chroma::~Chroma() {
}

int Chroma::mapPixels() {
	TiXmlDocument doc("mapping.xml");
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("map");
	if (root) {
		TiXmlElement* element = root->FirstChildElement("mapping");
		while (element) {
			//
			unsigned int universe = atoi(element->Attribute("universe"));
			universeSet.insert(universe);
			unsigned int propnum = atoi(element->Attribute("propertyNum"));

			mapping_t localElement;
			localElement.deviceType = atoi(element->Attribute("deviceType"));
			localElement.row = atoi(element->Attribute("row") == NULL ? "0" : element->Attribute("row"));
			localElement.col = atoi(element->Attribute("col") == NULL ? "0" : element->Attribute("col"));
			localElement.color = atoi(element->Attribute("color") == NULL ? "0" : element->Attribute("color"));
			localElement.name = strdup(element->GetText());

			pair<unsigned int, unsigned int> key = std::make_pair(universe, propnum);
			propertySet_t propertySet;
			if (propertyNumMap.count(key) > 0)
				propertySet = propertyNumMap[key];
			propertySet.push_back(localElement);
			propertyNumMap[key] = propertySet;

			element = element->NextSiblingElement("mapping");
		}
	}
	return 1;
}

int Chroma::initialize() {
	mapPixels();

	// clear all LEDs
	unsigned long defval = RGB(0, 0, 0);
	for (unsigned int row = 0; row < ChromaSDK::Keyboard::MAX_ROW; row++)
		for (unsigned int col = 0; col < ChromaSDK::Keyboard::MAX_COLUMN; col++)
			keyboard_effect.Color[row][col] = defval;
	for (unsigned int col = 0; col < ChromaSDK::Mousepad::MAX_LEDS; col++)
		mousemat_effect.Color[col] = defval;
	for (unsigned int col = 0; col < ChromaSDK::Mouse::MAX_LEDS; col++)
		mouse_effect.Color[col] = defval;
	for (unsigned int row = 0; row < ChromaSDK::Mouse::MAX_ROW; row++)
		for (unsigned int col = 0; col < ChromaSDK::Mouse::MAX_COLUMN; col++)
			mouse_effect2.Color[row][col] = defval;
	for (unsigned int col = 0; col < ChromaSDK::Headset::MAX_LEDS; col++)
		headset_effect.Color[col] = defval;
	for (unsigned int row = 0; row < ChromaSDK::Keypad::MAX_ROW; row++)
		for (unsigned int col = 0; col < ChromaSDK::Keypad::MAX_COLUMN; col++)
			keypad_effect.Color[row][col] = defval;

	m_ChromaAPI.initialize();

	return 1;
}

int Chroma::print(e131_packet_t* packet) {
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
	return 1;
}

void Chroma::updateColor(unsigned int colorComponent, unsigned int color, unsigned long* led) {
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

int Chroma::command(e131_packet_t* packet) {
	if (bDebug)
		print(packet);
	uint16_t universe = eflip(packet->universe);
	uint16_t property_value_count = eflip(packet->property_value_count);
	for (const auto& entry : propertyNumMap) {
		if ((universe == entry.first.first) && (property_value_count> entry.first.second)) {
			propertySet_t propertySet = entry.second;
			for (mapping_t node : propertySet) {
				unsigned int color = (packet->property_values[entry.first.second]);
				switch (node.deviceType) {
					case NO_DEVICES:		break;
					case KEYBOARD_DEVICES:	updateColor(node.color, color, &keyboard_effect.Color[node.row][node.col]);	break;
					case MOUSEMAT_DEVICES:	updateColor(node.color, color, &mousemat_effect.Color[node.col]);			break;
					case MOUSE_DEVICES:		updateColor(node.color, color, &mouse_effect.Color[node.col]);				break;
					case MOUSE_DEVICES2:	updateColor(node.color, color, &mouse_effect2.Color[node.row][node.col]);	break;
					case HEADSET_DEVICES:	updateColor(node.color, color, &headset_effect.Color[node.col]);			break;
					case ALL_DEVICES:		updateColor(node.color, color, &all_effect.Color[node.row][node.col]);	break;
				}
			}
		}
	}


	return 1;
}

void Chroma::startThread() {
#if defined(WIN32) || defined(WIN64)
	_beginthread(chromathread, 0, this);
	_beginthread(effectthread, 0, this);
#else
	//https://linux.die.net/man/3/pthread_create
	//http://www.ibm.com/developerworks/systems/library/es-MigratingWin32toLinux.html
	//int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
	pthread_t* threadId;
	pthread_attr_t  attr;
	pthread_create(threadId, &attr, (void*(*)(void*))chromaThread, this);
	pthread_create(threadId, &attr, (void*(*)(void*))effectthread, this);
#endif
}

void Chroma::chromaThread() {
	while (command(&packet))
		msleep(5000);
}

void Chroma::effectThread() {
	while (command(&packet)) {
		m_ChromaAPI.createEffect(ChromaSDK::CHROMA_CUSTOM, &all_effect, nullptr);
		m_ChromaAPI.createKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &keyboard_effect, nullptr);
		m_ChromaAPI.createMousepadEffect(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousemat_effect, nullptr);
		m_ChromaAPI.createMouseEffect(ChromaSDK::Mouse::CHROMA_CUSTOM, &mouse_effect, nullptr);
		m_ChromaAPI.createMouseEffect(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouse_effect2, nullptr);
		m_ChromaAPI.createHeadsetEffect(ChromaSDK::Headset::CHROMA_CUSTOM, &headset_effect, nullptr);
		m_ChromaAPI.createKeypadEffect(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypad_effect, nullptr);
	}
}


/*
void Chroma::resetEffects(unsigned int deviceType) {
	switch (deviceType) {
		case KEYBOARD_DEVICES:	if (createKeyboardEffect) createKeyboardEffect(ChromaSDK::Keyboard::CHROMA_NONE, nullptr, nullptr);		break;
		case MOUSEMAT_DEVICES:	if (createMousepadEffect) createMousepadEffect(ChromaSDK::Mousepad::CHROMA_NONE, nullptr, nullptr);		break;
		case MOUSE_DEVICES:		if (createMouseEffect)    createMouseEffect(ChromaSDK::Mouse::CHROMA_NONE, nullptr, nullptr);			break;
		case MOUSE_DEVICES2:	if (createMouseEffect)    createMouseEffect(ChromaSDK::Mouse::CHROMA_NONE, nullptr, nullptr);			break;
		case HEADSET_DEVICES:	if (createHeadsetEffect)  createHeadsetEffect(ChromaSDK::Headset::CHROMA_NONE, nullptr, nullptr);		break;
		case KEYPAD_DEVICES:	if (createKeypadEffect)   createKeypadEffect(ChromaSDK::Keypad::CHROMA_NONE, nullptr, nullptr);			break;
		case ALL_DEVICES:		if (createEffect)		  createEffect(ChromaSDK::CHROMA_NONE, nullptr, nullptr);						break;
	}
}
*/
