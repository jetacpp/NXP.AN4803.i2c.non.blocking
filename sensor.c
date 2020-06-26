#define SENSOR_I2C_ADDRESS 0x5A // as defined in sensor datasheet
#define IICWRITE(iicaddress) ((iicaddress << 1) & 0xFE)
#define IICREAD(iicaddress) ((iicaddress << 1) | 0x01)
#define SENSOR_I2C_ADDRESS_W IICWRITE(SENSOR_I2C_ADDRESS) //0xB4
#define SENSOR_I2C_ADDRESS_R IICREAD(SENSOR_I2C_ADDRESS)  //0xB5
typedef enum
{
    SENSOR_NO_FAULT = 0,
    SENSOR_INIT_FAILED,
    SENSOR_READ_FAILED,
} tSensor_fault;
typedef enum
{
    SENSOR_NO_TRANSFER = 0,
    SENSOR_TRANSFER_PROGRESS,
    SENSOR_TRANSFER_FINISHED,
} tSensor_data_transfer;
typedef struct
{
    tSensor_fault eSensor_fault;
    tSensor_data_transfer eSensor_data_transfer;
    unsigned char measured_data;
} tSensor_com_ctr;
// variable of “tSensor_com_ctr” type declaration:
tSensor_com_ctr sTMPR_com_ctr;

// array of data that are sent (received) during I2C message transfer. The size depends on
//the maximum size of the message that is sent over I2C bus (e.g. during initialization of
//the sensor
unsigned char iic_data[10];

//The following function serves for initialization of the I2C control structure:
void TemperatureSensingInit (void)
{
    // init I2C_com_ctr structure members that are the same for all application
    sI2C_com_ctr.device_address_w = SENSOR_I2C_ADDRESS_W;
    sI2C_com_ctr.device_address_r = SENSOR_I2C_ADDRESS_R;
    // init flags before first use
    sI2C_com_ctr.eI2C_flag = I2C_FLAG_NONE;
    sI2C_com_ctr.eI2C_fault = I2C_NO_FAULT;

    if (Sensor_Init() == SENSOR_INIT_FAILED)
    {
        // if the I2C bus error caused fail of the sensor init process, the error flag //is set. The
        // main application then should evaluate this condition
        sTMPR_com_ctr.eSensor_fault = SENSOR_INIT_FAILED;
        return;
    }
    ENABLE_TIMER_INTERRUPT;
}

tSensor_fault Sensor_Init(void)
{
    static unsigned int time_out_cnt = 1000;
    // clear the data array first
    for (i = 0; i < 10; i++)
        iic_data[i] = 0;
    //clear all registers
    sI2C_com_ctr.data_size = 10; // assuming the sensor has 10 registers
    sI2C_com_ctr.eI2C_trm_stage = I2C_TRM_STAGE_WRITE_DEV_ADDRESS_W;
    sI2C_com_ctr.register_address = 0x00;
    if (I2C_write_data(&sI2C_com_ctr, iic_data) != I2C_NO_FAULT)
        return SENSOR_INIT_FAILED;
    // Move to next command after previous message is sent
    // Here the blocking communication is used. The application has to wait anyway until
    // the sensor is initialized.
    while (((sI2C_com_ctr.eI2C_flag == I2C_FLAG_NONE) && (time_out_cnt != 0)))
    {
        time_out_cnt--
    }
    if (time_out_cnt == 0) // if the counter counts to zero
        return SENSOR_INIT_FAILED;
    // prepare data for next message transfer
    sI2C_com_ctr.data_size = 2;
    sI2C_com_ctr.eI2C_trm_stage = I2C_TRM_STAGE_WRITE_DEV_ADDRESS_W;
    sI2C_com_ctr.register_address = 0x00;
    iic_data[0] = 0x63; // some setting
    iic_data[1] = 0x10; // some setting
    if (I2C_write_data(&sI2C_com_ctr, iic_data) != I2C_NO_FAULT)
        return SENSOR_INIT_FAILED;
    // move to next command after previous "packet" sent
    time_out_cnt = 1000;
    while (!((sI2C_com_ctr.eI2C_flag == I2C_FLAG_NONE) && (time_out_cnt != 0)))
    {
        time_out_cnt--
    }
    if (time_out_cnt == 0) // if the counter counts to zero
        return SENSOR_INIT_FAILED;
    //… configuration continues by writing data to other registers. The initialization process
    // is divided to several messages, some registers might remained untouched, some might
    // require other registers to be written first.
    return SENSOR_NO_FAULT;
}

void Timer1_Isr(void)
{
    // initialization of I2C control structure
    sI2C_com_ctr.register_address = 0x04; // address of the register with actual measured
                                          // temperature
    sI2C_com_ctr.data_size = 1;
    sI2C_com_ctr.eI2C_flag = I2C_FLAG_NONE;
    sI2C_com_ctr.eI2C_trm_stage = I2C_TRM_STAGE_WRITE_DEV_ADDRESS_W;
    if (I2C_read_data(&sI2C_com_ctr, iic_data) != I2C_NO_FAULT)
        sTMPR_com_ctr.eSensor_fault = SENSOR_READ_FAILED;
    else
    {
        sTMPR_com_ctr.eSensor_data_transfer = SENSOR_TRANSFER_PROGRESS;
        // IMPORTANT: The SENSOR_TRANSFER_FINISHED flag has to be set in the I2C_isr_Callback
        // function, when all bytes of the I2C message are transferred. The main application should
        // evaluate eSensor_data_transfer flags in order to check the availability of actual
        // temperature data. Then the flag should be changed to SENSOR_NO_TRANSFER.
        // The value of actual temperature will be stored in first element of iic_data array
        sTMPR_com_ctr.eSensor_fault = SENSOR_NO_FAULT;
    }
    CLEAR_TIMER_INTERRUPT_FLAG;
}