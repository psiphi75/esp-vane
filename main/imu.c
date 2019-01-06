#include "esp_log.h"

#include "driver/i2c.h"

#include "../components/ahrs/MadgwickAHRS.h"
#include "../components/mpu9250/mpu9250.h"
#include "../components/mpu9250/calibrate.h"
#include "../components/mpu9250/common.h"

#define I2C_MASTER_NUM I2C_NUM_0 /*!< I2C port number for master dev */

// static const char *TAG = "imu_run";

// BeagleBone IMU
// static calibration_t cal = {
//     .mag_offset = {.x = 25.183594, .y = 57.519531, .z = -62.648438},
//     .mag_scale = {.x = 1.513449, .y = 1.557811, .z = 1.434039},
//     .accel_offset = {.x = 0.020900, .y = 0.014688, .z = -0.002580},
//     .accel_scale_lo = {.x = -0.992052, .y = -0.990010, .z = -1.011147},
//     .accel_scale_hi = {.x = 1.013558, .y = 1.011903, .z = 1.019645},
//     .gyro_bias_offset = {.x = 0.303956, .y = -1.049768, .z = -0.403782}};

// Wood Board IMU
static calibration_t cal = {
    .mag_offset = {.x = -40.699219, .y = 26.630859, .z = -15.398438},
    .mag_scale = {.x = 1.513748, .y = 1.508752, .z = 1.478003},
    .accel_offset = {.x = 0.038291, .y = 0.023748, .z = 0.091626},
    .accel_scale_lo = {.x = -0.985539, .y = -0.987383, .z = -0.964251},
    .accel_scale_hi = {.x = 1.018004, .y = 1.014876, .z = 1.063251},
    .gyro_bias_offset = {.x = 2.465681, .y = 1.802849, .z = -0.764410}};

/**
 * Transformation:
 *  - Rotate around Z axis 180 degrees
 *  - Rotate around X axis -90 degrees
 * @param  {object} s {x,y,z} sensor
 * @return {object}   {x,y,z} transformed
 */
static void transform_accel_gyro(vector_t *v)
{
  float x = v->x;
  float y = v->y;
  float z = v->z;

  v->x = -x;
  v->y = -z;
  v->z = -y;
}

/**
 * Transformation: to get magnetometer aligned
 * @param  {object} s {x,y,z} sensor
 * @return {object}   {x,y,z} transformed
 */
static void transform_mag(vector_t *v)
{
  float x = v->x;
  float y = v->y;
  float z = v->z;

  v->x = -y;
  v->y = z;
  v->z = -x;
}

void imu_init(void)
{
  i2c_mpu9250_init(&cal);
  MadgwickAHRSinit(SAMPLE_FREQ_Hz, 0.8);
}

void imu_run(void)
{
  vector_t va, vg, vm;

  // Get the Accelerometer, Gyroscope and Magnetometer values.
  ESP_ERROR_CHECK(get_accel_gyro_mag(&va, &vg, &vm));

  // Transform these values to the orientation of our device.
  transform_accel_gyro(&va);
  transform_accel_gyro(&vg);
  transform_mag(&vm);

  // Apply the AHRS algorithm
  MadgwickAHRSupdate(DEG2RAD(vg.x), DEG2RAD(vg.y), DEG2RAD(vg.z),
                     va.x, va.y, va.z,
                     vm.x, vm.y, vm.z);
}

void imu_get_data(char *str)
{
  float temp;
  ESP_ERROR_CHECK(get_temperature_celsius(&temp));

  float heading, pitch, roll;
  MadgwickGetEulerAnglesDegrees(&heading, &pitch, &roll);
  sprintf(str, "{\"heading\":%2.3f,\"pitch\":%2.3f,\"roll\":%2.3f,\"temp\":%2.1f}", heading, pitch, roll, temp);
}

void close_imu(void)
{
  i2c_driver_delete(I2C_MASTER_NUM);
}
