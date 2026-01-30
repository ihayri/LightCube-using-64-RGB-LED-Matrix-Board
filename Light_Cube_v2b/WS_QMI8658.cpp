#include "WS_QMI8658.h"

#define I2C_SDA       11
#define I2C_SCL       12

SensorQMI8658 QMI;
IMUdata Accel;
IMUdata Gyro;

void QMI8658_Init()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  
  if (!QMI.begin(Wire, QMI8658_L_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    printf("Failed to find QMI8658 - check your wiring!\r\n");
    while (1) {
      delay(1000);
    }
  }

  // Standard 6DOF configuration
  QMI.configAccelerometer(
    SensorQMI8658::ACC_RANGE_4G,
    SensorQMI8658::ACC_ODR_1000Hz,
    SensorQMI8658::LPF_MODE_3);

  QMI.configGyroscope(
    SensorQMI8658::GYR_RANGE_64DPS,
    SensorQMI8658::GYR_ODR_896_8Hz,
    SensorQMI8658::LPF_MODE_3);

  QMI.enableGyroscope();
  QMI.enableAccelerometer();

  // We are NOT calling any WakeOnMotion functions here.
  // This keeps the sensor in standard streaming mode.

  printf("Read data now...\r\n");
}

void QMI8658_Loop()
{
  if (QMI.getDataReady()) {
    QMI.getAccelerometer(Accel.x, Accel.y, Accel.z);
    QMI.getGyroscope(Gyro.x, Gyro.y, Gyro.z);
  }
}
