/*
 * GC032A MIPI YUV -- the sizes, read off the binary.
 *
 * The six half-words GC032AMIPIGetResolution writes are all 632x474:
 * "mov x8, #0x278"@0xffffff80087173e8 builds 0x01DA_0278_01DA_0278, which
 * read as four u16 is 632, 474, 632, 474, and the w9 that follows adds two
 * more of the same.
 */
#ifndef _GC032AMIPI_SENSOR_H
#define _GC032AMIPI_SENSOR_H

#define GC032A_IMAGE_SENSOR_PV_WIDTH	632
#define GC032A_IMAGE_SENSOR_PV_HEIGHT	474
#define GC032A_IMAGE_SENSOR_FULL_WIDTH	632
#define GC032A_IMAGE_SENSOR_FULL_HEIGHT	474
#define GC032A_IMAGE_SENSOR_VIDEO_WIDTH	632
#define GC032A_IMAGE_SENSOR_VIDEO_HEIGHT	474

extern void GC032A_write_reg(kal_uint8 addr, kal_uint8 para);
extern kal_uint16 GC032A_Read_Shutter(void);
extern int gc032a_i2c_write(u8 *pwrite_data, u16 write_length,
	u16 write_per_cycle, u16 id, int speed);
extern int gc032a_i2c_read(u8 *pwrite_data, u16 write_length,
	u8 *pread_data, u16 read_length, u16 id, int speed);
extern kal_uint16 GC032A_read_cmos_sensor(kal_uint8 addr);
extern kal_uint16 GC032A_read_reg(kal_uint8 addr);
extern UINT32 GC032APreview(
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern UINT32 GC032ACapture(
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern UINT32 GC032AMIPIControl(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern void GC032ANightMode(kal_bool enable);
extern UINT32 GC032AReadBV(void);
extern UINT32 GC032AMIPIOpen(void);
extern UINT32 GC032AMIPIGetInfo(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
	MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern UINT32 GC032A_set_param_wb(UINT16 para);
extern UINT32 GC032A_set_param_effect(UINT16 para);
extern UINT32 GC032AGetSensorID(UINT32 *sensorID);
extern UINT32 GC032AYUVSetVideoMode(UINT16 u2FrameRate);
extern void GC032A_Sensor_Init(void);
extern void GC032A_Set_Shutter(kal_uint16 iShutter);
extern void GC032A_config_window(kal_uint16 startx, kal_uint16 starty,
				     kal_uint16 width, kal_uint16 height);
extern void GC032A_SetGain(kal_uint16 iGain);
extern void GC032A_Write_More_Registers(void);
extern UINT32 GC032AMIPIClose(void);
extern UINT32 GC032AMIPIGetResolution(
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);

#endif
