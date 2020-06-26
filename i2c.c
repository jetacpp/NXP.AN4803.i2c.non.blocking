/*****************************************************************************
* (c) Copyright 2013, Freescale Semiconductor Inc.
* ALL RIGHTS RESERVED.
***************************************************************************/
/*!
* @file i2c.c
* @author B02785, plcm001
* @version 1.0.4.2
* @date July-24-2012
* @brief IIC driver implementation.
* @par Driver example
* @include i2c.h
******************************************************************************/
#include "i2c.h"
#include "derivative.h" // header file that implements peripheral memory map
// here is used MC56F82748.h
/******************************************************************************
* global variables *
******************************************************************************/
unsigned char iic_data[0x80];
tI2C_com_ctr sI2C_com_ctr; // I2C communication control structure
/******************************************************************************
* local variables *
******************************************************************************/
static volatile unsigned int timeout_cnt;
/******************************************************************************
* macro definitions *
******************************************************************************/

/******************************************************************************
* Public functions definitions *
******************************************************************************/
/******************************************************************************
* I2C peripheral module initialization function definition *
******************************************************************************/
void I2C_Init(void)
{
    I2C_F = 0x27; // clock divider 56, I2C frequency: 80 kHz
    I2C_C1 = I2C_C1_IICIE | I2C_C1_IICEN;
    // enable interrupt in INTC module
    INTC_IPR6 |= INTC_IPR6_IIC0_0 | INTC_IPR6_IIC0_1;
}
/******************************************************************************
* I2C peripheral module de-initialization function definition *
******************************************************************************/
void I2C_DeInit(void)
{
    I2C_C1 = 0;
}

/******************************************************************************
* I2C dealy function definition *
******************************************************************************/
void I2C_delay(void) // delay of 200 us @50MHz CPU clock (creates period of 5 kHz)
{
    unsigned int cnt;

    for (cnt = 0; cnt < 10000; cnt++)
    {
        asm(nop);
    };
}
/******************************************************************************
* I2C restore function definition *
******************************************************************************/

tI2C_fault I2C_Restore(void)
{
    unsigned char tmp = 0;
    I2C_STOP_SIGNAL;
    I2C_DeInit();
    I2C_MASTER_SDA_PIN_AS_GPIO;
    I2C_MASTER_SDA_PIN_GPIO_HIGH_DRIVE;
    I2C_MASTER_SCL_PIN_AS_GPIO;
    I2C_MASTER_SDA_PIN_AS_OUT;
    I2C_MASTER_SDA_PIN_0;
    I2C_MASTER_SCL_PIN_AS_OUT;
    for (tmp = 0; tmp < 9; tmp++) // nine clock for data
    {
        I2C_MASTER_SCL_PIN_0;
        I2C_delay();
        I2C_MASTER_SCL_PIN_1;
        I2C_delay();
    }
    I2C_MASTER_SCL_PIN_0;
    I2C_MASTER_SDA_PIN_AS_OUT; //SDA pin set to output
    I2C_MASTER_SDA_PIN_1;      //negative acknowledge
    I2C_delay();
    I2C_MASTER_SCL_PIN_1;
    I2C_delay();
    I2C_MASTER_SCL_PIN_0;
    I2C_delay();
    I2C_MASTER_SDA_PIN_0; //stop
    I2C_delay();
    I2C_MASTER_SCL_PIN_1;
    I2C_delay();

    I2C_MASTER_SDA_PIN_AS_IN;
    tmp = 0;
    if (!((GPIOC_DR >> 14) & 1)) // if still SDA is zero, try once again with
                                 // more SCL clocks
    {
        while (!((GPIOC_DR >> 14) & 1) && (tmp < 30))
        {
            I2C_MASTER_SCL_PIN_0;
            I2C_delay();
            I2C_MASTER_SCL_PIN_1;
            I2C_delay();
            tmp++;
        };
        if (tmp == 30)
            return I2C_PERMANENT_BUS_FAULT; // giving up, permanent
        // error, reset required
        I2C_MASTER_SCL_PIN_0;
        I2C_MASTER_SDA_PIN_AS_OUT; //SDA pin set to output
        I2C_MASTER_SDA_PIN_1;      //negative acknowledge
        I2C_delay();
        I2C_MASTER_SCL_PIN_1;
        I2C_delay();
        I2C_MASTER_SCL_PIN_0;
        I2C_delay();
        I2C_MASTER_SDA_PIN_0; //stop
        I2C_delay();
        I2C_MASTER_SCL_PIN_1;
        I2C_delay();
    }

    I2C_MASTER_SDA_PIN_AS_I2C;
    I2C_MASTER_SCL_PIN_AS_I2C;
    I2C_Init();
    return I2C_NO_FAULT;
}

/******************************************************************************
* I2C data write function definition *
******************************************************************************/
tI2C_fault I2C_write(tI2C_com_ctr *psI2C_tr_ctrl, unsigned char data[])
{
    // check if the bus is not busy while there is attempt to write device address
    //on the bus
    if ((I2C_S >> 5) & 1)
        return I2C_BUS_BUSY;
    // set the i2C communication mode, this flag will be evaluated in i2c isr
    psI2C_tr_ctrl->eI2C_mode = I2C_MODE_WRITE;
    psI2C_tr_ctrl->data_index = 0; // initialize data index
    psI2C_tr_ctrl->eI2C_flag = I2C_FLAG_TRANSMISSION_PROGRESS;
    // move to next byte
    psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_WRITE_REG_ADDRESS;
    I2C_SET_TX_MODE;
    I2C_START_SIGNAL;
    //initiate write message with device address
    I2C_WRITE_BYTE(psI2C_tr_ctrl->device_address_w);
    return I2C_NO_FAULT;
}

/******************************************************************************
* I2C data read function definition *
******************************************************************************/
tI2C_fault I2C_read(tI2C_com_ctr *psI2C_tr_ctrl, unsigned char data[])
{
    // check if the bus is not busy while there is attempt to write device address
    // on the bus
    if ((I2C_S >> 5) & 1)
        return I2C_BUS_BUSY;
    // set the i2C communication mode, this flag will be evaluated in i2c isr
    psI2C_tr_ctrl->eI2C_mode = I2C_MODE_READ;
    psI2C_tr_ctrl->data_index = 0; // initialise data index
    I2C_SET_TX_MODE;
    I2C_START_SIGNAL;
    //initiate read message with device address
    I2C_WRITE_BYTE(psI2C_tr_ctrl->device_address_w);
    psI2C_tr_ctrl->eI2C_flag = I2C_FLAG_TRANSMISSION_PROGRESS;
    // move to next byte
    psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_WRITE_REG_ADDRESS;
    return I2C_NO_FAULT;
}
/******************************************************************************
* write data function definition *
******************************************************************************/
tI2C_fault I2C_write_data(tI2C_com_ctr *psI2C_tr_ctrl, unsigned char data[])
{
    unsigned int cnt;
    timeout_cnt = 100000;
    while (((I2C_write(psI2C_tr_ctrl, data) == I2C_BUS_BUSY) &&
            (timeout_cnt != 0)))
    {
        timeout_cnt--;
    }
    if (timeout_cnt == 0)
    {
        psI2C_tr_ctrl->eI2C_fault = I2C_TIMEOUT;
        // the bus signals still busy and the timeout occurs, restore the I2C
        I2C_Restore();
        timeout_cnt = 100000;
        //second attempt to write the data to I2C bus
        while (((I2C_write(psI2C_tr_ctrl, data) == I2C_BUS_BUSY) &&
                (timeout_cnt != 0)))
        {
            timeout_cnt--;
        };
        if (timeout_cnt == 0)
        {
            // try to send the data anyway
            I2C_write(psI2C_tr_ctrl, data);
            // wait till the packet is sent
            for (cnt = 0; cnt < 100; cnt++)
                I2C_delay();
            // the bus signals still busy and the timeout occurs,
            // restore the I2C
            I2C_Restore();
            timeout_cnt = 100000;
            //third attempt to write the data to I2C bus
            while (((I2C_write(psI2C_tr_ctrl, data) ==
                     I2C_BUS_BUSY) &&
                    (timeout_cnt != 0)))
            {
                timeout_cnt--;
            };
            if (timeout_cnt == 0)
                return I2C_PERMANENT_BUS_FAULT;
        }
    }
    return I2C_NO_FAULT;
}
/******************************************************************************
* read data function definition *
******************************************************************************/
tI2C_fault I2C_read_data(tI2C_com_ctr *psI2C_tr_ctrl, unsigned char data[])
{
    timeout_cnt = 100000;
    while (((I2C_read(psI2C_tr_ctrl, data) == I2C_BUS_BUSY) &&
            (timeout_cnt != 0)))
    {
        timeout_cnt--;
    }
    if (timeout_cnt == 0)
    {
        // the bus is still busy and the timeout occurs, restore the I2C
        I2C_Restore();
        timeout_cnt = 100000;
        //second attempt to write the data to I2C bus
        while (((I2C_read(psI2C_tr_ctrl, data) == I2C_BUS_BUSY) &&
                (timeout_cnt != 0)))
        {
            timeout_cnt--;
        };
        if (timeout_cnt == 0)
            return I2C_PERMANENT_BUS_FAULT;
    }
    return I2C_NO_FAULT;
}
/******************************************************************************
* interrupt callback function definition *
******************************************************************************/
tI2C_fault I2C_isr_Callback(tI2C_com_ctr *psI2C_tr_ctrl, unsigned char *data)
{
    bool dummy;
    register unsigned char dummy_data;
    switch (psI2C_tr_ctrl->eI2C_mode)
    {
    case I2C_MODE_WRITE:
    {
        switch (psI2C_tr_ctrl->eI2C_trm_stage)
        {
        case I2C_TRM_STAGE_WRITE_REG_ADDRESS:
        {
            psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_WRITE_DATA;
            I2C_WRITE_BYTE(psI2C_tr_ctrl->register_address);
            break;
        };
        case I2C_TRM_STAGE_WRITE_DATA:
        {
            // if this is acknowledge after last byte in the message
            if ((psI2C_tr_ctrl->data_size) == psI2C_tr_ctrl->data_index)
            {
                // acknowledge after last data byte received,
                // so generate stop signal now
                I2C_STOP_SIGNAL;
                psI2C_tr_ctrl->eI2C_flag = I2C_FLAG_NONE; // write data sequence end
                // end of message transfer flag
                psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_NONE;
                return I2C_NO_FAULT;
            }
            else
            {
                I2C_WRITE_BYTE(data[psI2C_tr_ctrl->data_index]);
                // increase index to the next address of the data array
                psI2C_tr_ctrl->data_index++;
            }
            break;
        };
        };
        break;
    }
    case I2C_MODE_READ:
    {
        switch (psI2C_tr_ctrl->eI2C_trm_stage)
        {
        case I2C_TRM_STAGE_WRITE_REG_ADDRESS:
        {
            I2C_WRITE_BYTE(psI2C_tr_ctrl->register_address);
            psI2C_tr_ctrl->eI2C_trm_stage =
                I2C_TRM_STAGE_WRITE_DEV_ADDRESS_R; // move to next byte
            break;
        };
        case I2C_TRM_STAGE_WRITE_DEV_ADDRESS_R:
        {
            I2C_REPEAT_START_SIGNAL;
            I2C_WRITE_BYTE(psI2C_tr_ctrl->device_address_r);
            psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_READ_DUMMY_DATA;
            break;
        };
        case I2C_TRM_STAGE_READ_DUMMY_DATA: // post read dummy data action
        {
            I2C_SET_RX_MODE;
            // reading data register initiates
            // receiving of the next byte of data
            dummy_data = I2C_READ_BYTE;
            psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_READ_DATA;
            break;
        }
        case I2C_TRM_STAGE_READ_DATA:
        {
            // if this is acknowledge after last byte in the stream
            if ((psI2C_tr_ctrl->data_size - 1) == psI2C_tr_ctrl->data_index)
            {
                I2C_SET_NACK_MODE; // last read ends with NAK
                iic_data[psI2C_tr_ctrl->data_index] = I2C_READ_BYTE;
                psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_NAK;
            }
            else
            {
                iic_data[psI2C_tr_ctrl->data_index] = I2C_READ_BYTE;
                if (psI2C_tr_ctrl->data_index < (psI2C_tr_ctrl->data_size - 1))
                    // increase pointer to the next address of the data array
                    psI2C_tr_ctrl->data_index++;
            }
            break;
        };
        case I2C_TRM_STAGE_NAK:
        {
            I2C_STOP_SIGNAL;
            I2C_CLEAR_NACK_MODE;
            psI2C_tr_ctrl->eI2C_flag = I2C_FLAG_NONE; // read data sequence end
            psI2C_tr_ctrl->eI2C_trm_stage = I2C_TRM_STAGE_NONE;
            asm { nop}
            ;
            asm { nop}
            ;
            asm { nop}
            ;
            asm { nop}
            ;
            return I2C_NO_FAULT;
        }
        break;
        }
    }
    }
    return I2C_NO_FAULT;
}
/******************************************************************************
* interrupt function definition *
******************************************************************************/
void MC56F827xx_ISR_IIC0(void)
#pragma interrupt saveall
{
    if (I2C_S | I2C_S_TCF)
        if (I2C_isr_Callback(&sI2C_com_ctr, iic_data) == I2C_NO_FAULT)
        {
            if (sI2C_com_ctr.eI2C_flag == I2C_FLAG_NONE)
            {
                // the transfer of the whole message ended. If the read
                //message was completed, now the data are in the iic_data[]
                // Place a code that process received data.
            }
        }
    /* Clear Irq. request */
    I2C_CLEAR_IRQ_FLAG;
}
/******************************************************************************
* End of module *
******************************************************************************/