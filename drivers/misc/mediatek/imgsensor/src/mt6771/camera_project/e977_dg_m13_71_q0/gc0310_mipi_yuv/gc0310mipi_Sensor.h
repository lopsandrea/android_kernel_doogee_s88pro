/*
 * GC0310 MIPI YUV -- the sizes, read off the binary.
 *
 * The six half-words GC0310MIPIGetResolution writes are all 632x474:
 * "mov x8, #0x278"@0xffffff80087173e8 builds 0x01DA_0278_01DA_0278, which
 * read as four u16 is 632, 474, 632, 474, and the w9 that follows adds two
 * more of the same.
 */
#ifndef _GC0310MIPI_SENSOR_H
#define _GC0310MIPI_SENSOR_H

#define GC0310MIPI_IMAGE_SENSOR_PV_WIDTH	632
#define GC0310MIPI_IMAGE_SENSOR_PV_HEIGHT	474
#define GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH	632
#define GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT	474
#define GC0310MIPI_IMAGE_SENSOR_VIDEO_WIDTH	632
#define GC0310MIPI_IMAGE_SENSOR_VIDEO_HEIGHT	474

extern void GC0310MIPI_write_reg(kal_uint8 addr, kal_uint8 para);
extern kal_uint16 GC0310MIPI_Read_Shutter(void);
extern int gc0310_i2c_write(u8 *pwrite_data, u16 write_length,
	u16 write_per_cycle, u16 id, int speed);
extern int gc0310_i2c_read(u8 *pwrite_data, u16 write_length,
	u8 *pread_data, u16 read_length, u16 id, int speed);
extern kal_uint16 GC0310MIPI_read_cmos_sensor(kal_uint8 addr);
extern kal_uint16 GC0310MIPI_read_reg(kal_uint8 addr);
extern UINT32 GC0310MIPIPreview(
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern UINT32 GC0310MIPICapture(
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern UINT32 GC0310MIPIControl(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern void GC0310MIPINightMode(kal_bool enable);
extern UINT32 GC0310ReadBV(void);
extern UINT32 GC0310MIPIOpen(void);
extern UINT32 GC0310MIPIGetInfo(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
	MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
extern UINT32 GC0310MIPI_set_param_wb(UINT16 para);
extern UINT32 GC0310MIPI_set_param_effect(UINT16 para);
extern UINT32 GC0310MIPIGetSensorID(UINT32 *sensorID);
extern UINT32 GC0310MIPIYUVSetVideoMode(UINT16 u2FrameRate);
extern void GC0310MIPI_Sensor_Init(void);
extern void GC0310MIPIGammaSelect(kal_uint8 index);
extern void GC0310MIPI_Set_Shutter(kal_uint16 iShutter);
extern void GC0310MIPI_config_window(kal_uint16 startx, kal_uint16 starty,
				     kal_uint16 width, kal_uint16 height);
extern void GC0310MIPI_SetGain(kal_uint16 iGain);
extern void GC0310MIPI_Write_More_Registers(void);
extern UINT32 GC0310MIPIClose(void);
extern UINT32 GC0310MIPIGetResolution(
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);

#endif
