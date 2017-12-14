#include <string.h>
#include "eeprom.h"
#include "config.h"

#if !defined( WIN32 )
#include <lpc11xx.h>
#else
#define __NOP()
#endif

extern unsigned char crc8(unsigned char *block, int len);

int config_read(int param, uint8_t *value, int len) {
	config_unit_t unit;
	uint32_t addr = 2*param*sizeof(config_unit_t);
	if (param >= CONFIG_MAX_PARAM ||
		param < 0)
		return 1;
	// 1�� ������:
	ee_read(addr, (uint8_t*)&unit, sizeof(config_unit_t));
	if (unit.flag == 0xDE &&
		unit.len >= len &&
		unit.crc8 == crc8(unit.data, len)) {
		memcpy(value, unit.data, len);
		return 0;
	}
	// �� ���������. 2�� ������:
	ee_read(addr+sizeof(config_unit_t), (uint8_t*)&unit, sizeof(config_unit_t));
	if (unit.flag == 0xEA &&
		unit.len >= len &&
		unit.crc8 == crc8(unit.data, len)) {
		memcpy(value, unit.data, len);
		return 0;
	}
	return 2;
}

int config_save(int param, uint8_t *value, int len) {
	config_unit_t unit;
	uint32_t addr = 2*param*sizeof(config_unit_t);
	int n;
	if (param >= CONFIG_MAX_PARAM ||
		param < 0)
		return 1;
	if (len > CONFIG_MAX_SIZE)
		return 2;
	unit.crc8 = crc8(value, len);
	unit.len = len;
	memcpy(unit.data, value, len);
	// 1�� ������:
	unit.flag = 0xDE;
	ee_write(addr, (uint8_t*)&unit, sizeof(config_unit_t));
	// NOTE: ������ �������� ��������� ��� i2c-������, 
	// �.�. �� ����� ������ � �����������. ������ � ���� ����������� 
	// ������� �� ����������� ����.
	for (n=0; n<30000; n++) {
		__NOP();
	}
	// 2�� ������:
	unit.flag = 0xEA;
	ee_write(addr+sizeof(config_unit_t), (uint8_t*)&unit, sizeof(config_unit_t));
	// NOTE: ������ �������� ��������� ��� i2c-������, 
	// �.�. �� ����� ������ � �����������. ������ � ���� ����������� 
	// ������� �� ����������� ����.
	for (n=0; n<30000; n++) {
		__NOP();
	}
	return 0;
}
