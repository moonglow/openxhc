#pragma once

#define WHBxx_MAGIC 0xFDFE 
#define DEV_UNKNOW 0

#define DEV_WHB03  sizeof( struct whb03_out_data )
#define DEV_WHB04  sizeof( struct whb04_out_data )

#define WHBxx_VID 0x10CE
#define WHB03_PID 0xEB6E
#define WHB04_PID 0xEB70

#pragma pack( push, 1 )

struct whb03_out_data
{
	/* header of our packet */
	uint16_t	magic; 
	/* day of the month .. funny i know*/
	uint8_t		day;
        
        struct
        {
          uint16_t	p_int;
          uint8_t	p_frac;
        }pos[6];
	/* speed */
	uint16_t	feedrate_ovr;
	uint16_t	sspeed_ovr;
	uint16_t	feedrate;
	uint16_t	sspeed;

	uint8_t    step_mul;
	uint8_t    state;

};

struct whb04_out_data
{
	/* header of our packet */
	uint16_t	magic; 
	/* day of the month .. funny i know*/
	uint8_t		day;
        struct
        {
          uint16_t	p_int;
          uint16_t	p_frac;
        }pos[6];

	/* speed */
	uint16_t	feedrate_ovr;
	uint16_t	sspeed_ovr;
	uint16_t	feedrate;
	uint16_t	sspeed;

	uint8_t    step_mul;
	uint8_t    state;

};

#if 0
struct whb03_out_data
{
	/* header of our packet */
	uint16_t	magic; 
	/* day of the month .. funny i know*/
	uint8_t		day;
	/* work pos */
	uint16_t	x_wc_int;
	uint8_t		x_wc_frac;
	uint16_t	y_wc_int;
	uint8_t		y_wc_frac;
	uint16_t	z_wc_int;
	uint8_t		z_wc_frac;
	/* machine pos */
	uint16_t	x_mc_int;
	uint8_t		x_mc_frac;
	uint16_t	y_mc_int;
	uint8_t		y_mc_frac;
	uint16_t	z_mc_int;
	uint8_t		z_mc_frac;

	/* speed */
	uint16_t	feedrate_ovr;
	uint16_t	sspeed_ovr;
	uint16_t	feedrate;
	uint16_t	sspeed;

	uint8_t    step_mul;
	uint8_t    state;

};

struct whb04_out_data
{
	/* header of our packet */
	uint16_t	magic; 
	/* day of the month .. funny i know*/
	uint8_t		day;
	/* work pos */
	uint16_t	x_wc_int;
	uint16_t	x_wc_frac;
	uint16_t	y_wc_int;
	uint16_t	y_wc_frac;
	uint16_t	z_wc_int;
	uint16_t	z_wc_frac;
	/* machine pos */
	uint16_t	x_mc_int;
	uint16_t	x_mc_frac;
	uint16_t	y_mc_int;
	uint16_t	y_mc_frac;
	uint16_t	z_mc_int;
	uint16_t	z_mc_frac;

	/* speed */
	uint16_t	feedrate_ovr;
	uint16_t	sspeed_ovr;
	uint16_t	feedrate;
	uint16_t	sspeed;

	uint8_t    step_mul;
	uint8_t    state;

};
#endif



struct whb0x_in_data
{
	uint8_t		id;
	uint8_t		btn_1;
	uint8_t		btn_2;
	uint8_t		wheel_mode;
	int8_t		wheel;
	uint8_t		xor_day;
};
#pragma pack( pop )

extern __IO uint8_t HwType;
