/*
 * Copyright 2015
 * CZ.NIC z.s.p.o.
 * Martin Strbacka (martin.strbacka@nic.cz)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <atsha204_mac_read.h>

#define I2C_ATSHA204 0x64
#define BUFFSIZE_NI2C 64
#define ATSHA204_I2C_TIMEOUT 180000
#define POLYNOM 0x8005
#define CONFIG_SYS_I2C_SPEED 128000
#define ATSHA204_STATUS_WAKE_OK 0x11

unsigned char cmd_prefix[] =      { 0x03, 0x07, 0x02, 0x01, 0x03, 0x00, 0x12, 0xA7 };
unsigned char cmd_mac[] =         { 0x03, 0x07, 0x02, 0x01, 0x04, 0x00, 0x1E, 0xE7 };

void calculate_crc(uint16_t length, unsigned char *data, unsigned char *crc) {
        uint16_t counter;
        uint16_t crc_register = 0;
        uint16_t polynom = POLYNOM;
        unsigned char shift_register;
        unsigned char data_bit, crc_bit;

        for (counter = 0; counter < length; counter++) {
          for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
                 data_bit = (data[counter] & shift_register) ? 1 : 0;
                 crc_bit = crc_register >> 15;
                 crc_register <<= 1;
                 if (data_bit != crc_bit)
                        crc_register ^= polynom;
          }
        }
        crc[0] = (unsigned char) (crc_register & 0x00FF);
        crc[1] = (unsigned char) (crc_register >> 8);
}

bool check_crc(unsigned char length, unsigned char *data, unsigned char *crc) {
        unsigned char rcrc[2];
        calculate_crc(length, data, rcrc);
        if ((crc[0] != rcrc[0]) || (crc[1] != rcrc[1])) {
                return false;
        }

        return true;
}

int get_mac(uint8_t *tmp_mac, uint8_t *buffer) {

        i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
        udelay(50000);

        // Wake up
        i2c_write(I2C_ATSHA204, 0, 0, NULL, 1);
        udelay(ATSHA204_I2C_TIMEOUT);

        i2c_read(I2C_ATSHA204, 0, 0, buffer, BUFFSIZE_NI2C);
        if (!check_crc(buffer[0] - 2, buffer, (buffer + buffer[0] - 2))) {
            printf("CRC doesn't match.\n");
            return 1;
        }
        if (buffer[1] != ATSHA204_STATUS_WAKE_OK) return 1;

        // Read MAC prefix
        i2c_write(I2C_ATSHA204, 0, 0, cmd_prefix, 8);
        udelay(ATSHA204_I2C_TIMEOUT);

        i2c_read(I2C_ATSHA204, 0, 0, buffer, BUFFSIZE_NI2C);
        if (!check_crc(buffer[0] - 2, buffer, (buffer + buffer[0] - 2))){
            printf("CRC doesn't match.\n");
            return 1;
        }

        memcpy(tmp_mac, (buffer + 2), 3);

        // Read MAC suffix
        i2c_write(I2C_ATSHA204, 0, 0, cmd_mac, 8);
        udelay(ATSHA204_I2C_TIMEOUT);

        i2c_read(I2C_ATSHA204, 0, 0, buffer, BUFFSIZE_NI2C);
        if (!check_crc(buffer[0] - 2, buffer, (buffer + buffer[0] - 2))) {
                printf("CRC doesn't match.\n");
                return 1;
        }

        return 0;

}

int atsha204_mac_read()
{
    int i,ret = -1, try = 1;
    unsigned int mac_as_number = 0, mac_as_number_orig = 0;
    unsigned char tmp_mac[6];
    char mac[20], varname[10];
    uint8_t buffer[BUFFSIZE_NI2C];
    memset(buffer, 0, BUFFSIZE_NI2C);

    while(ret != 0 && try <= 3) {
        if(try > 1) {
            printf("try: %i\n", try);
            udelay(ATSHA204_I2C_TIMEOUT);
        }

        ret = get_mac(tmp_mac, buffer);
        try++;
    }
    if(ret != 0){
        printf("Error\n");
        return 1;
    }

    mac_as_number_orig |= (buffer[2] << 8*2);
    mac_as_number_orig |= (buffer[3] << 8*1);
    mac_as_number_orig |= buffer[4];

    for (i = 0; i < 3; i++) {
                mac_as_number = mac_as_number_orig;
                mac_as_number_orig++;

                tmp_mac[5] = mac_as_number & 0xFF; mac_as_number >>= 8;
                tmp_mac[4] = mac_as_number & 0xFF; mac_as_number >>= 8;
                tmp_mac[3] = mac_as_number & 0xFF; mac_as_number >>= 8;

                sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", tmp_mac[0], tmp_mac[1], tmp_mac[2], tmp_mac[3], tmp_mac[4], tmp_mac[5]);

		if(i == 0)
			sprintf(varname, "ethaddr");
		else
			sprintf(varname, "eth%iaddr", i);

                printf("MAC%i %s\n", i, mac);
                setenv(varname, mac);
    }

    return 0;
}

static int do_atsha204_mac_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    ret = atsha204_mac_read();
    return ret;
}

U_BOOT_CMD(
	atsha204_mac_read,	1,		1,	do_atsha204_mac_read,
	"read mac addresses from atsha204 chip",
	""
);
