/* InvenSense ICM-20948 IMU Sensor Library
 *
 * This library provides access to scaled IMU measurement data through SPI. 
 * 
 * TODO: 
 * - calibration
 * - fixed update frequency?
 * - DMP support to return orientation quaternion
 * 
 * Note: This implementation only supports SPI interface
 *
 * Ported from Sparkfun's Arduino library: https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary
 * Datasheet: https://invensense.tdk.com/wp-content/uploads/2016/06/DS-000189-ICM-20948-v1.3.pdf
 *
 */

#ifndef _IMU_H_
#define _IMU_H_

#include "IMU_util.h"
#include "mbed.h"
#include <array>
#include <stdint.h>

#define MAX_SPI_FREQ                  7000000
#define PI                      3.14159265359
#define ACCELERATION_GRAVITY             9.81

#define ACCEL_SENSITIVITY_FACTOR_0      16384
#define ACCEL_SENSITIVITY_FACTOR_1       8192
#define ACCEL_SENSITIVITY_FACTOR_2       4096
#define ACCEL_SENSITIVITY_FACTOR_3       2048
#define GYRO_SENSITIVITY_FACTOR_0       131.0
#define GYRO_SENSITIVITY_FACTOR_1        65.5
#define GYRO_SENSITIVITY_FACTOR_2        32.8
#define GYRO_SENSITIVITY_FACTOR_3        16.4
#define MAG_SENSITIVITY_FACTOR           0.15


// Status codes
typedef enum {
    Status_Ok = 0x00,
    Status_Err,
    Status_NotImpl,       // returned by virtual functions that are not implemented
    Status_ParamErr,
    Status_WrongID,
    Status_InvalSensor,   // tried to apply a function to a sensor that does not support it
    Status_NoData,
    Status_SensorNotSupported,

    Status_NUM,
    Status_Unkonwn
} Status_e;

// Internal sensor IDs
typedef enum {
    Internal_Acc = (1 << 0),
    Internal_Gyr = (1 << 1),
    Internal_Mag = (1 << 2),
    Internal_Tmp = (1 << 3),
    Internal_Mst = (1 << 4)
} InternalSensorID_bm;

// Full-scale settings
typedef struct {
    uint8_t a : 2;
    uint8_t g : 2;
    uint8_t reserved_0 : 4;
} FSS_t;

// DLPF (digital low pass filter) settings
typedef struct {
    uint8_t a;
    uint8_t g;
} DLPF_cfg_t;

// Sample rate
typedef struct {
    uint8_t a;
    uint8_t g;
} SMPLRT_t;

// Interrupt enable
typedef struct {
    uint8_t I2C_MST_INT_EN : 1;
    uint8_t DMP_INT1_EN : 1;
    uint8_t PLL_RDY_EN : 1;
    uint8_t WOM_INT_EN : 1;
    uint8_t REG_WOF_EN : 1;
    uint8_t RAW_DATA_0_RDY_EN : 1;
    uint8_t FIFO_OVERFLOW_EN_4 : 1;
    uint8_t FIFO_OVERFLOW_EN_3 : 1;
    uint8_t FIFO_OVERFLOW_EN_2 : 1;
    uint8_t FIFO_OVERFLOW_EN_1 : 1;
    uint8_t FIFO_OVERFLOW_EN_0 : 1;
    uint8_t FIFO_WM_EN_4 : 1;
    uint8_t FIFO_WM_EN_3 : 1;
    uint8_t FIFO_WM_EN_2 : 1;
    uint8_t FIFO_WM_EN_1 : 1;
    uint8_t FIFO_WM_EN_0 : 1;
} INT_enable_t;

// Structure to store x, y, z axes
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} Axes;

// Structure to hold accel, gyro, and mag data
typedef struct {
    Axes acc;
    Axes gyr;
    Axes mag;
    uint8_t mag_stat_1;
    uint8_t mag_stat_2;
    FSS_t fss;  // full-scale range settings
} AGM_t;

// --------------------------------------------------------------------------------

class IMU {

    protected:
        Status_e read_register(uint8_t regaddr, uint8_t *pdata, uint32_t len);
        Status_e write_register(uint8_t regaddr, uint8_t *pdata, uint32_t len);

    public:

        IMU();

        // SPI
        SPI spi;
        DigitalOut cs;

        Status_e init_SPI(PinName mosi, PinName miso, PinName sclk, PinName cs, uint8_t SPI_freq);
        Status_e init_IMU(void);

        // Measurement data
        AGM_t agm;
        Status_e update_AGM(void);
        Status_e data_ready(void);  

        std::array<double, 3> get_IMU_lin_accel(void);  // m/s^2
        std::array<double, 3> get_IMU_ang_vel(void);    // rad/s
        std::array<double, 3> get_IMU_mag_field(void);  // Tesla

        // ID
        Status_e get_whoami(uint8_t *whoami);
        bool is_connected(void);
        Status_e check_ID(void);

        // Magnetometer specific
        Status_e init_mag(void);
        Status_e check_mag_ID(void);
        Status_e read_mag(AK09916_Reg_Addr_e reg, uint8_t *pdata);
        Status_e write_mag(AK09916_Reg_Addr_e reg, uint8_t *pdata);

        // Status
        Status_e status;
        const char* status_string(Status_e = Status_NUM);   // converts status to human readable string

        // Device configurations
        Status_e set_bank(uint8_t bank);
        Status_e sw_reset(void);
        Status_e sleep(bool on);
        Status_e low_power(bool on);

        Status_e set_sample_mode(InternalSensorID_bm sensors, ICM_20948_LP_CONFIG_CYCLE_e mode);
        Status_e set_full_scale_mode(InternalSensorID_bm sensors, FSS_t fss);
        Status_e set_DLPF_cfc(InternalSensorID_bm sensors, DLPF_cfg_t cfg);
        Status_e enable_DLPF(InternalSensorID_bm sensors, bool enable);
        Status_e set_sample_rate(InternalSensorID_bm sensors, SMPLRT_t smplrt);
        
        // Interrupts
        Status_e clear_interrupts(void);
        Status_e cfg_int(ICM_20948_INT_PIN_CFG_t *write, ICM_20948_INT_PIN_CFG_t *read);
        Status_e cfg_int_active_low(bool active_low);
        Status_e cfg_int_open_drain(bool open_drain);
        Status_e cfg_int_latch(bool latching);
        Status_e cfg_int_any_read_to_clear(bool enabled);
        Status_e cfg_fsync_active_low(bool active_low);
        Status_e cfg_fsync_int_mode(bool interrupt_mode);

        Status_e int_enable(INT_enable_t *write, INT_enable_t *read);
        Status_e int_enable_i2c(bool enable);
        Status_e int_enable_DMP(bool enable);
        Status_e int_enable_PLL(bool enable);
        Status_e int_enable_WOM(bool enable);
        Status_e int_enable_WOF(bool enable);
        Status_e int_enable_raw_data_ready(bool enable);
        Status_e int_enable_overflow_FIFO(uint8_t bm_enable);
        Status_e int_enable_watermark_FIFO(uint8_t bm_enable);

        Status_e set_clock_source(ICM_20948_PWR_MGMT_1_CLKSEL_e source);

        // Interface (auxilliary I2C lines used for magnetometer)
        Status_e i2c_master_pass_through(bool passthrough);
        Status_e i2c_master_enable(bool enable);
        Status_e i2c_master_reset(void);

        Status_e i2c_master_slv4_txn(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len, bool Rw, bool send_reg_addr);
        Status_e i2c_master_configure_slave(uint8_t slave, uint8_t addr, uint8_t reg, uint8_t len, bool Rw, bool enable, bool data_only, bool grp, bool swap);
};

#endif