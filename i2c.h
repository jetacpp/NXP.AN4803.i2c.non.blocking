i2c.h file
/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2013 Freescale Semiconductor
* ALL RIGHTS RESERVED.
*
*******************************************************************************/
/*
* @file i2c.h
* @author B02785, plcm001
* @version 1.0.2.0
* @date July-22-2013
* @brief I2C driver header file.
******************************************************************************/
#ifndef __I2C_H
#define __I2C_H
    /******************************************************************************
* definition of transmission control structure *
******************************************************************************/

    typedef enum {
        I2C_TRM_STAGE_NONE = 0,
        I2C_TRM_STAGE_WRITE_DATA,
        I2C_TRM_STAGE_WRITE_DEV_ADDRESS_W,
        I2C_TRM_STAGE_WRITE_DEV_ADDRESS_R,
        I2C_TRM_STAGE_WRITE_REG_ADDRESS,
        I2C_TRM_STAGE_READ_DUMMY_DATA,
        I2C_TRM_STAGE_READ_DATA,
        I2C_TRM_STAGE_NAK,
    } tI2C_trm_stage; // transmission stages
typedef enum
{
    I2C_MODE_READ = 0,
    I2C_MODE_WRITE,
} tI2C_mode;
typedef enum
{
    I2C_FLAG_NONE = 0,
    I2C_FLAG_TRANSMISSION_PROGRESS,
} tI2C_flag;
typedef enum
{
    I2C_NO_FAULT = 0,
    I2C_BUS_BUSY,
    I2C_TIMEOUT,
    I2C_PERMANENT_BUS_FAULT,
} tI2C_fault;

typedef struct
{
    tI2C_trm_stage eI2C_trm_stage;
    tI2C_flag eI2C_flag;
    tI2C_mode eI2C_mode;
    tI2C_fault eI2C_fault;
    unsigned char device_address_w;
    unsigned char device_address_r;
    unsigned char register_address;
    unsigned char data_size;
    unsigned char data_index;
} tI2C_com_ctr;

/******************************************************************************
* command definitions *
******************************************************************************/
/* I2C macro definitions */
#if defined (TOWER)
#define I2C_MASTER_SDA_PIN_1 GPIOF_DR |= GPIOF_DR_D_3
#define I2C_MASTER_SDA_PIN_0 GPIOF_DR &= ~GPIOF_DR_D_3
#define I2C_MASTER_SCL_PIN_1 GPIOF_DR |= GPIOF_DR_D_2
#define I2C_MASTER_SCL_PIN_0 GPIOF_DR &= ~GPIOF_DR_D_2
#define I2C_MASTER_SDA_PIN_AS_IN GPIOF_DDR &= ~GPIOF_DDR_DD_3
#define I2C_MASTER_SDA_PIN_AS_OUT GPIOF_DDR |= GPIOF_DDR_DD_3
#define I2C_MASTER_SCL_PIN_AS_IN GPIOF_DDR &= ~GPIOF_DDR_DD_2
#define I2C_MASTER_SCL_PIN_AS_OUT GPIOF_DDR |= GPIOF_DDR_DD_2
#define I2C_MASTER_SDA_PIN_AS_GPIO GPIOF_PER &= ~GPIOF_PER_PE_2
#define I2C_MASTER_SCL_PIN_AS_GPIO GPIOF_PER &= ~GPIOF_PER_PE_3
#define I2C_MASTER_SDA_PIN_AS_I2C GPIOF_PER |= GPIOF_PER_PE_2
#define I2C_MASTER_SCL_PIN_AS_I2C GPIOF_PER |= GPIOF_PER_PE_3
#endif
/***************************************************************************//*!
* @brief Macro generate Start I2C signal
* @param module - I2C0|I2C1
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_START_SIGNAL (I2C_C1 |= I2C_C1_MST)
/***************************************************************************//*!
* @brief Macro generate Stop I2C signal
* @param module - I2C0|I2C1
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_STOP_SIGNAL (I2C_C1 &= ~I2C_C1_MST)
/***************************************************************************//*!

* @brief Macro generate Repeat Start I2C signal
* @param module - I2C0|I2C1
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_REPEAT_START_SIGNAL (I2C_C1 |= I2C_C1_RSTA)
/***************************************************************************//*!
* @brief Macro Write data for transfer
* @param module - I2C0|I2C1
* @param data - Data for send
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_WRITE_BYTE(data) (I2C_D = data)
/***************************************************************************//*!
* @brief Macro Return data from last transfer
* @param module - I2C0|I2C1
* @return data - Data from last transfer
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_READ_BYTE (unsigned char)I2C_D

/***************************************************************************//*!
* @brief Macro Return Irq. flag
* @param module - I2C0|I2C1
* @return TRUE - Interrupt. pending (action finished)
* @return FALSE - No Interrupt. pending (action in proggress)
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_GET_IRQ_FLAG (I2C_S & I2C_S_IICIF)
/***************************************************************************//*!
* @brief Macro clear Irq. flag
* @param module - I2C0|I2C1
* @return NONE
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_CLEAR_IRQ_FLAG (I2C_S |= I2C_S_IICIF)
/***************************************************************************//*!
* @brief Macro change I2C mode RX
* @param module - I2C0|I2C1
* @return NONE
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_SET_RX_MODE (I2C_C1 &= ~I2C_C1_TX)

/***************************************************************************//*!
* @brief Macro change I2C mode TX
* @param module - I2C0|I2C1
* @return NONE
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_SET_TX_MODE (I2C_C1 |= I2C_C1_TX)
/***************************************************************************//*!
* @brief Macro change I2C mode NACK
* @param module - I2C0|I2C1
* @return NONE
* @note Implemented as inlined macro.
******************************************************************************/
#define I2C_SET_NACK_MODE (I2C_C1 |= I2C_C1_TXAK)
/***************************************************************************//*!
* @brief Macro change I2C mode back to ACK
* @param module - I2C0|I2C1
* @return NONE
* @note Implemented as inlined macro.
******************************************************************************/

#define I2C_CLEAR_NACK_MODE (I2C_C1 &= ~I2C_C1_TXAK)
/******************************************************************************
* public function prototypes *
******************************************************************************/
extern void I2C_Isr(void);
extern tI2C_fault I2C_write_data(tI2C_com_ctr *psI2C_tr_ctrl, unsigned char *data);
extern tI2C_fault I2C_read_data(tI2C_com_ctr *psI2C_tr_ctrl, unsigned char *data);
/******************************************************************************
* local function prototypes *
******************************************************************************/
void I2C_Init();
void I2C_DeInit();
tI2C_fault I2C_Restore();
void I2C_delay(void);
tI2C_fault I2C_isr_Callback (tI2C_com_ctr *psI2C_tr_ctrl,unsigned char *data);
#endif /* __I2C_H */
/******************************************************************************
* End of module *
******************************************************************************/