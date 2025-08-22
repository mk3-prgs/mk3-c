/**
  ******************************************************************************
  * @file OptimizedI2Cexamples/src/I2CRoutines.c
  * @author  MCD Application Team
  * @version  V4.0.0
  * @date  06/18/2010
  * @brief  Contains the I2Cx slave/Master read and write routines.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c_bus.h"

/** @addtogroup Optimized I2C examples
  * @{
  */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t I2CDirection = I2C_DIRECTION_TX;
__IO uint32_t NumbOfBytes1;
__IO uint32_t NumbOfBytes2;
__IO uint8_t Address;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Reads buffer of bytes  from the slave.
  * @param pBuffer: Buffer of bytes to be read from the slave.
  * @param NumByteToRead: Number of bytes to be read by the Master.
  * @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
  * @param SlaveAddress: The address of the slave to be addressed by the Master.
  * @retval : None.
  */
Status I2C_Master_BufferRead(I2C_TypeDef* I2Cx, uint8_t* pBuffer,  uint32_t NumByteToRead, I2C_ProgrammingModel Mode, uint8_t addr)
{
    __IO uint32_t temp = 0;
    __IO uint32_t Timeout = 0;
    temp=temp;
    addr = (addr<<1)|1;
    /* Enable I2C errors interrupts (used in all modes: Polling, DMA and Interrupts */
    I2Cx->CR2 |= I2C_IT_ERR;
    if(Mode == Polling) { /* I2Cx Master Reception using Polling */
        if(NumByteToRead == 1) {
            Timeout = 0xFFFF;
            /* Send START condition */
            I2Cx->CR1 |= CR1_START_Set;
            /* Wait until SB flag is set: EV5  */
            while ((I2Cx->SR1&0x0001) != 0x0001) {
                if (Timeout-- == 0) return Error;
                }
            /* Send the slave address */
            I2Cx->DR = addr;
            /* Wait until ADDR is set: EV6_3, then program ACK = 0, clear ADDR
            and program the STOP just after ADDR is cleared. The EV6_3
            software sequence must complete before the current byte end of transfer.*/
            /* Wait until ADDR is set */
            Timeout = 0xFFFF;
            while ((I2Cx->SR1&0x0002) != 0x0002) {
                if (Timeout-- == 0) return Error;
                }
            /* Clear ACK bit */
            I2Cx->CR1 &= CR1_ACK_Reset;
            /* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
            software sequence must complete before the current byte end of transfer */
            __disable_irq();
            /* Clear ADDR flag */
            temp = I2Cx->SR2;
            /* Program the STOP */
            I2Cx->CR1 |= CR1_STOP_Set;
            /* Re-enable IRQs */
            __enable_irq();
            /* Wait until a data is received in DR register (RXNE = 1) EV7 */
            while ((I2Cx->SR1 & 0x00040) != 0x000040);
            /* Read the data */
            *pBuffer = I2Cx->DR;
            /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
            while ((I2Cx->CR1&0x200) == 0x200);
            /* Enable Acknowledgement to be ready for another reception */
            I2Cx->CR1 |= CR1_ACK_Set;
            }

        else if(NumByteToRead == 2) {
            /* Set POS bit */
            I2Cx->CR1 |= CR1_POS_Set;
            Timeout = 0xFFFF;
            /* Send START condition */
            I2Cx->CR1 |= CR1_START_Set;
            /* Wait until SB flag is set: EV5 */
            while((I2Cx->SR1&0x0001) != 0x0001) {
                if (Timeout-- == 0) return Error;
                }
            Timeout = 0xFFFF;
            /* Send slave address */
            I2Cx->DR = addr;
            /* Wait until ADDR is set: EV6 */
            while((I2Cx->SR1&0x0002) != 0x0002) {
                if(Timeout-- == 0) return Error;
                }
            /* EV6_1: The acknowledge disable should be done just after EV6,
            that is after ADDR is cleared, so disable all active IRQs around ADDR clearing and
            ACK clearing */
            __disable_irq();
            /* Clear ADDR by reading SR2 register  */
            temp = I2Cx->SR2;
            /* Clear ACK */
            I2Cx->CR1 &= CR1_ACK_Reset;
            /*Re-enable IRQs */
            __enable_irq();
            /* Wait until BTF is set */
            while ((I2Cx->SR1 & 0x00004) != 0x000004);
            /* Disable IRQs around STOP programming and data reading because of the limitation ?*/
            __disable_irq();
            /* Program the STOP */
            ///I2C_GenerateSTOP(I2Cx, ENABLE);
            i2c_stop_on_bus((uint32_t)I2Cx);
            /* Read first data */
            *pBuffer = I2Cx->DR;
            /* Re-enable IRQs */
            __enable_irq();
            /**/
            pBuffer++;
            /* Read second data */
            *pBuffer = I2Cx->DR;
            /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
            while ((I2Cx->CR1&0x200) == 0x200);
            /* Enable Acknowledgement to be ready for another reception */
            I2Cx->CR1  |= CR1_ACK_Set;
            /* Clear POS bit */
            I2Cx->CR1  &= CR1_POS_Reset;
            }

        else {
            Timeout = 0xFFFF;
            /* Send START condition */
            I2Cx->CR1 |= CR1_START_Set;
            /* Wait until SB flag is set: EV5 */
            while ((I2Cx->SR1&0x0001) != 0x0001) {
                if (Timeout-- == 0) return Error;
                }
            Timeout = 0xFFFF;
            /* Send slave address */
            I2Cx->DR = addr;
            /* Wait until ADDR is set: EV6 */
            while((I2Cx->SR1&0x0002) != 0x0002) {
                if(Timeout-- == 0) return Error;
                }
            /* Clear ADDR by reading SR2 status register */
            temp = I2Cx->SR2;
            /* While there is data to be read */
            while(NumByteToRead) {
                /* Receive bytes from first byte until byte N-3 */
                if(NumByteToRead != 3) {
                    /* Poll on BTF to receive data because in polling mode we can not guarantee the
                    EV7 software sequence is managed before the current byte transfer completes */
                    while ((I2Cx->SR1 & 0x00004) != 0x000004);
                    /* Read data */
                    *pBuffer = I2Cx->DR;
                    /* */
                    pBuffer++;
                    /* Decrement the read bytes counter */
                    NumByteToRead--;
                    }

                /* it remains to read three data: data N-2, data N-1, Data N */
                if(NumByteToRead == 3) {
                    /* Wait until BTF is set: Data N-2 in DR and data N -1 in shift register */
                    while ((I2Cx->SR1 & 0x00004) != 0x000004);
                    /* Clear ACK */
                    I2Cx->CR1 &= CR1_ACK_Reset;
                    /* Disable IRQs around data reading and STOP programming because of the limitation ? */
                    __disable_irq();
                    /* Read Data N-2 */
                    *pBuffer = I2Cx->DR;
                    /* Increment */
                    pBuffer++;
                    /* Program the STOP */
                    I2Cx->CR1 |= CR1_STOP_Set;
                    /* Read DataN-1 */
                    *pBuffer = I2Cx->DR;
                    /* Re-enable IRQs */
                    __enable_irq();
                    /* Increment */
                    pBuffer++;
                    /* Wait until RXNE is set (DR contains the last data) */
                    while((I2Cx->SR1 & 0x00040) != 0x000040);
                    /* Read DataN */
                    *pBuffer = I2Cx->DR;
                    /* Reset the number of bytes to be read by master */
                    NumByteToRead = 0;
                    }
                }
            /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
            while ((I2Cx->CR1&0x200) == 0x200);
            /* Enable Acknowledgement to be ready for another reception */
            I2Cx->CR1 |= CR1_ACK_Set;
            }
        }
    else { /* I2Cx Master Reception using Interrupts with highest priority in an application */
        /* Enable EVT IT*/
        I2Cx->CR2 |= I2C_IT_EVT;
        /* Enable BUF IT */
        I2Cx->CR2 |= I2C_IT_BUF;
        /* Set the I2C direction to reception */
        I2CDirection = I2C_DIRECTION_RX;
        Address = addr;
        if(I2Cx == I2C_1)    NumbOfBytes1 = NumByteToRead;
        else NumbOfBytes2 = NumByteToRead;
        /* Send START condition */
        I2Cx->CR1 |= CR1_START_Set;
        /* Wait until the START condition is generated on the bus: START bit is cleared by hardware */
        while ((I2Cx->CR1&0x100) == 0x100);
        /* Wait until BUSY flag is reset (until a STOP is generated) */
        while ((I2Cx->SR2 &0x0002) == 0x0002);
        /* Enable Acknowledgement to be ready for another reception */
        I2Cx->CR1 |= CR1_ACK_Set;
        }
    return Success;
}

/**
  * @brief  Writes buffer of bytes.
  * @param pBuffer: Buffer of bytes to be sent to the slave.
  * @param NumByteToWrite: Number of bytes to be sent by the Master.
  * @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
  * @param SlaveAddress: The address of the slave to be addressed by the Master.
  * @retval : None.
  */
Status I2C_Master_BufferWrite(I2C_TypeDef* I2Cx, uint8_t* pBuffer,  uint32_t NumByteToWrite, I2C_ProgrammingModel Mode, uint8_t addr)
{
    __IO uint32_t temp = 0;
    __IO uint32_t Timeout = 0;
    temp=temp;
    addr = (addr<<1);
    /* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
    I2Cx->CR2 |= I2C_IT_ERR;
    if(Mode == Polling) { /* I2Cx Master Transmission using Polling */
        Timeout = 0xFFFF;
        /* Send START condition */
        I2Cx->CR1 |= CR1_START_Set;
        /* Wait until SB flag is set: EV5 */
        while((I2Cx->SR1&0x0001) != 0x0001) {
            if(Timeout-- == 0) return Error;
            }
        /* Send slave address */
        I2Cx->DR = addr;
        Timeout = 0xFFFF;
        /* Wait until ADDR is set: EV6 */
        while ((I2Cx->SR1 &0x0002) != 0x0002) {
            if (Timeout-- == 0) return Error;
            }
        /* Clear ADDR flag by reading SR2 register */
        temp = I2Cx->SR2;
        /* Write the first data in DR register (EV8_1) */
        I2Cx->DR = *pBuffer;
        /* Increment */
        pBuffer++;
        /* Decrement the number of bytes to be written */
        NumByteToWrite--;
        /* While there is data to be written */
        while(NumByteToWrite--) {
            /* Poll on BTF to receive data because in polling mode we can not guarantee the
              EV8 software sequence is managed before the current byte transfer completes */
            while ((I2Cx->SR1 & 0x00004) != 0x000004);
            /* Send the current byte */
            I2Cx->DR = *pBuffer;
            /* Point to the next byte to be written */
            pBuffer++;
            }
        /* EV8_2: Wait until BTF is set before programming the STOP */
        while((I2Cx->SR1 & 0x00004) != 0x000004);
        /* Send STOP condition */
        I2Cx->CR1 |= CR1_STOP_Set;
        /* Make sure that the STOP bit is cleared by Hardware */
        while((I2Cx->CR1&0x200) == 0x200);
        }

    else { /* I2Cx Master Transmission using Interrupt with highest priority in the application */
        /* Enable EVT IT*/
        I2Cx->CR2 |= I2C_IT_EVT;
        /* Enable BUF IT */
        I2Cx->CR2 |= I2C_IT_BUF;
        /* Set the I2C direction to Transmission */
        I2CDirection = I2C_DIRECTION_TX;
        Address = addr;
        if (I2Cx == I2C_1)    NumbOfBytes1 = NumByteToWrite;
        else NumbOfBytes2 = NumByteToWrite;
        /* Send START condition */
        I2Cx->CR1 |= CR1_START_Set;
        /* Wait until the START condition is generated on the bus: the START bit is cleared by hardware */
        while((I2Cx->CR1&0x100) == 0x100);
        /* Wait until BUSY flag is reset: a STOP has been generated on the bus signaling the end of transmission */
        while ((I2Cx->SR2 &0x0002) == 0x0002);
        }
    //
    return Success;
}

/**
  * @brief Prepares the I2Cx slave for transmission.
  * @param I2Cx: I2C1 or I2C2.
  * @param Mode: DMA or Interrupt having the highest priority in the application.
  * @retval : None.
  */

void I2C_Slave_BufferReadWrite(I2C_TypeDef* I2Cx, I2C_ProgrammingModel Mode)

{
    Mode=Mode;
    /* Enable Event IT needed for ADDR and STOPF events ITs */
    I2Cx->CR2 |= I2C_IT_EVT ;
    /* Enable Error IT */
    I2Cx->CR2 |= I2C_IT_ERR;

    I2Cx->CR2 |= I2C_IT_BUF;
}

/**
* @brief  Initializes peripherals: I2Cx, GPIO, DMA channels .
  * @param  None
  * @retval None
  */
void I2C_LowLevel_Init(int bus)
{
    ///GPIO_InitTypeDef  GPIO_InitStructure;
    ///I2C_InitTypeDef  I2C_InitStructure;

    /* GPIOB clock enable */
    ///RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    rcu_periph_clock_enable(RCU_GPIOB);
    /* Enable the DMA1 clock */
    //
    if(bus == 0) {
        /* I2C1 clock enable */
        ///RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
        rcu_periph_clock_enable(RCU_I2C0);
        /* I2C1 SDA and SCL configuration */
        ///GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        ///GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        ///GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        ///GPIO_Init(GPIOB, &GPIO_InitStructure);

        ///GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        ///GPIO_Init(GPIOB, &GPIO_InitStructure);
        //
        gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
        gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_7);

        /* Enable I2C1 reset state */
        ///RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
        rcu_periph_reset_enable(RCU_I2C0RST);
        /* Release I2C1 from reset state */
        ///RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
        rcu_periph_reset_disable(RCU_I2C0RST);

            /* I2C clock configure */
        i2c_clock_config(I2C0, 20000, I2C_DTCY_2);
        /* I2C address configure */
        i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, OwnAddress1);
        /* enable I2C0 */
        i2c_enable(I2C0);
        /* enable acknowledge */
        i2c_ack_config(I2C0, I2C_ACK_ENABLE);
        }

    else { /* I2Cx = I2C2 */
        /* I2C2 clock enable */
        ///RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
        rcu_periph_clock_enable(RCU_I2C1);
        /* I2C1 SDA and SCL configuration */
        ///GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        ///GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        ///GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        ///GPIO_Init(GPIOB, &GPIO_InitStructure);

        ///GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        ///GPIO_Init(GPIOB, &GPIO_InitStructure);
        //
        gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
        gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

        /* Enable I2C2 reset state */
        ///RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
        rcu_periph_reset_enable(RCU_I2C1RST);
        /* Release I2C2 from reset state */
        ///RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
        rcu_periph_reset_disable(RCU_I2C1RST);

        /* I2C clock configure */
        i2c_clock_config(I2C1, 20000, I2C_DTCY_2);
        /* I2C address configure */
        i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, OwnAddress2);
        /* enable I2C0 */
        i2c_enable(I2C1);
        /* enable acknowledge */
        i2c_ack_config(I2C1, I2C_ACK_ENABLE);
        }
}

