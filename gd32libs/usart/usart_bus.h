/**
 * @file usart_bus.h
 * Библиотека для работы с шиной USART.
 */

#ifndef USART_BUS_H
#define	USART_BUS_H

#include <gd32f10x.h>
//#include "stm32f10x.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "errors/errors.h"
#include "defs/defs.h"
#include "dma/dma.h"
/*
typedef struct
{
  __IO uint16_t SR;
  uint16_t  RESERVED0;
  __IO uint16_t DR;
  uint16_t  RESERVED1;
  __IO uint16_t BRR;
  uint16_t  RESERVED2;
  __IO uint16_t CR1;
  uint16_t  RESERVED3;
  __IO uint16_t CR2;
  uint16_t  RESERVED4;
  __IO uint16_t CR3;
  uint16_t  RESERVED5;
  __IO uint16_t GTPR;
  uint16_t  RESERVED6;
} USART_TypeDef;
*/
/******************************************************************************/
/*                                                                            */
/*         Universal Synchronous Asynchronous Receiver Transmitter            */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for USART_SR register  *******************/
#define  USART_SR_PE                         ((uint16_t)0x0001)            /*!< Parity Error */
#define  USART_SR_FE                         ((uint16_t)0x0002)            /*!< Framing Error */
#define  USART_SR_NE                         ((uint16_t)0x0004)            /*!< Noise Error Flag */
#define  USART_SR_ORE                        ((uint16_t)0x0008)            /*!< OverRun Error */
#define  USART_SR_IDLE                       ((uint16_t)0x0010)            /*!< IDLE line detected */
#define  USART_SR_RXNE                       ((uint16_t)0x0020)            /*!< Read Data Register Not Empty */
#define  USART_SR_TC                         ((uint16_t)0x0040)            /*!< Transmission Complete */
#define  USART_SR_TXE                        ((uint16_t)0x0080)            /*!< Transmit Data Register Empty */
#define  USART_SR_LBD                        ((uint16_t)0x0100)            /*!< LIN Break Detection Flag */
#define  USART_SR_CTS                        ((uint16_t)0x0200)            /*!< CTS Flag */

/*******************  Bit definition for USART_DR register  *******************/
#define  USART_DR_DR                         ((uint16_t)0x01FF)            /*!< Data value */

/******************  Bit definition for USART_BRR register  *******************/
#define  USART_BRR_DIV_Fraction              ((uint16_t)0x000F)            /*!< Fraction of USARTDIV */
#define  USART_BRR_DIV_Mantissa              ((uint16_t)0xFFF0)            /*!< Mantissa of USARTDIV */

/******************  Bit definition for USART_CR1 register  *******************/
#define  USART_CR1_SBK                       ((uint16_t)0x0001)            /*!< Send Break */
#define  USART_CR1_RWU                       ((uint16_t)0x0002)            /*!< Receiver wakeup */
#define  USART_CR1_RE                        ((uint16_t)0x0004)            /*!< Receiver Enable */
#define  USART_CR1_TE                        ((uint16_t)0x0008)            /*!< Transmitter Enable */
#define  USART_CR1_IDLEIE                    ((uint16_t)0x0010)            /*!< IDLE Interrupt Enable */
#define  USART_CR1_RXNEIE                    ((uint16_t)0x0020)            /*!< RXNE Interrupt Enable */
#define  USART_CR1_TCIE                      ((uint16_t)0x0040)            /*!< Transmission Complete Interrupt Enable */
#define  USART_CR1_TXEIE                     ((uint16_t)0x0080)            /*!< PE Interrupt Enable */
#define  USART_CR1_PEIE                      ((uint16_t)0x0100)            /*!< PE Interrupt Enable */
#define  USART_CR1_PS                        ((uint16_t)0x0200)            /*!< Parity Selection */
#define  USART_CR1_PCE                       ((uint16_t)0x0400)            /*!< Parity Control Enable */
#define  USART_CR1_WAKE                      ((uint16_t)0x0800)            /*!< Wakeup method */
#define  USART_CR1_M                         ((uint16_t)0x1000)            /*!< Word length */
#define  USART_CR1_UE                        ((uint16_t)0x2000)            /*!< USART Enable */
#define  USART_CR1_OVER8                     ((uint16_t)0x8000)            /*!< USART Oversmapling 8-bits */

/******************  Bit definition for USART_CR2 register  *******************/
#define  USART_CR2_ADD                       ((uint16_t)0x000F)            /*!< Address of the USART node */
#define  USART_CR2_LBDL                      ((uint16_t)0x0020)            /*!< LIN Break Detection Length */
#define  USART_CR2_LBDIE                     ((uint16_t)0x0040)            /*!< LIN Break Detection Interrupt Enable */
#define  USART_CR2_LBCL                      ((uint16_t)0x0100)            /*!< Last Bit Clock pulse */
#define  USART_CR2_CPHA                      ((uint16_t)0x0200)            /*!< Clock Phase */
#define  USART_CR2_CPOL                      ((uint16_t)0x0400)            /*!< Clock Polarity */
#define  USART_CR2_CLKEN                     ((uint16_t)0x0800)            /*!< Clock Enable */

#define  USART_CR2_STOP                      ((uint16_t)0x3000)            /*!< STOP[1:0] bits (STOP bits) */
#define  USART_CR2_STOP_0                    ((uint16_t)0x1000)            /*!< Bit 0 */
#define  USART_CR2_STOP_1                    ((uint16_t)0x2000)            /*!< Bit 1 */

#define  USART_CR2_LINEN                     ((uint16_t)0x4000)            /*!< LIN mode enable */

/******************  Bit definition for USART_CR3 register  *******************/
#define  USART_CR3_EIE                       ((uint16_t)0x0001)            /*!< Error Interrupt Enable */
#define  USART_CR3_IREN                      ((uint16_t)0x0002)            /*!< IrDA mode Enable */
#define  USART_CR3_IRLP                      ((uint16_t)0x0004)            /*!< IrDA Low-Power */
#define  USART_CR3_HDSEL                     ((uint16_t)0x0008)            /*!< Half-Duplex Selection */
#define  USART_CR3_NACK                      ((uint16_t)0x0010)            /*!< Smartcard NACK enable */
#define  USART_CR3_SCEN                      ((uint16_t)0x0020)            /*!< Smartcard mode enable */
#define  USART_CR3_DMAR                      ((uint16_t)0x0040)            /*!< DMA Enable Receiver */
#define  USART_CR3_DMAT                      ((uint16_t)0x0080)            /*!< DMA Enable Transmitter */
#define  USART_CR3_RTSE                      ((uint16_t)0x0100)            /*!< RTS Enable */
#define  USART_CR3_CTSE                      ((uint16_t)0x0200)            /*!< CTS Enable */
#define  USART_CR3_CTSIE                     ((uint16_t)0x0400)            /*!< CTS Interrupt Enable */
#define  USART_CR3_ONEBIT                    ((uint16_t)0x0800)            /*!< One Bit method */

/******************  Bit definition for USART_GTPR register  ******************/
#define  USART_GTPR_PSC                      ((uint16_t)0x00FF)            /*!< PSC[7:0] bits (Prescaler value) */
#define  USART_GTPR_PSC_0                    ((uint16_t)0x0001)            /*!< Bit 0 */
#define  USART_GTPR_PSC_1                    ((uint16_t)0x0002)            /*!< Bit 1 */
#define  USART_GTPR_PSC_2                    ((uint16_t)0x0004)            /*!< Bit 2 */
#define  USART_GTPR_PSC_3                    ((uint16_t)0x0008)            /*!< Bit 3 */
#define  USART_GTPR_PSC_4                    ((uint16_t)0x0010)            /*!< Bit 4 */
#define  USART_GTPR_PSC_5                    ((uint16_t)0x0020)            /*!< Bit 5 */
#define  USART_GTPR_PSC_6                    ((uint16_t)0x0040)            /*!< Bit 6 */
#define  USART_GTPR_PSC_7                    ((uint16_t)0x0080)            /*!< Bit 7 */

#define  USART_GTPR_GT                       ((uint16_t)0xFF00)            /*!< Guard time value */

/**
 * Структура инициализации шины USART.
 */
typedef struct _UsartBusInit {
    USART_TypeDef* usart_device; //!< Устройство USART.
    DMA_Channel_TypeDef* dma_rx_channel; //!< Канал DMA для приёма.
    DMA_Channel_TypeDef* dma_tx_channel; //!< Канал DMA для передачи.
}usart_bus_init_t;

//! Состояние шины USART.
typedef enum _Usart_Status {
    USART_STATUS_IDLE = 0,//!< Бездействие.
    USART_STATUS_TRANSFERING,//!< Обмен данными.
    USART_STATUS_TRANSFERED,//!< Обмен данными завершён.
    USART_STATUS_ERROR//!< Ошибка.
} usart_status_t;

//! Ошибки шины USART.
typedef enum _Usart_Error {
    USART_ERROR_NONE    = 0, //!< Нет ошибки.
    USART_ERROR_PARITY  = 1, //!< Ошибка чётности.
    USART_ERROR_NOISE   = 2, //!< Шум.
    USART_ERROR_OVERRUN = 4, //!< Переполнение.
    USART_ERROR_FRAMING = 8, //!< Ошибка кадра.
    USART_ERROR_DMA     = 16//!< Ошибка DMA.
} usart_error_t;

//! Тип ошибок USART.
typedef uint8_t usart_errors_t;

//! Тип реакции на IDLE при приёме данных.
typedef enum _Usart_Idle_Mode {
    USART_IDLE_MODE_NONE = 0, //!< Ничего не делать.
    USART_IDLE_MODE_END_RX //!< Завершить приём данных.
} usart_idle_mode_t;

/**
 * Тип функции обратного вызова.
 * Вызывается при окончании приёма/передачи
 * даннх, или при возникновении ошибки.
 * @return true, если событие обработано, иначе false.
 */
typedef bool (*usart_bus_callback_t)(void);

/**
 * Тип функции обратного вызова приёма байта.
 * Вызывается при приёме байта.
 * @return true, если событие обработано, иначе false.
 */
typedef bool (*usart_bus_rx_byte_callback_t)(uint8_t byte);

/**
 * Тип идентификатора приёма/передачи.
 */
typedef uint8_t usart_transfer_id_t;

//! Идентификатор передачи по умолчанию.
#define USART_BUS_DEFAULT_TRANSFER_ID 0

/**
 * Структура буферизации USART.
 */
typedef struct _UsartBus {
    USART_TypeDef* usart_device; //!< Устройство USART.
    DMA_Channel_TypeDef* dma_rx_channel; //!< Канал DMA для приёма.
    DMA_Channel_TypeDef* dma_tx_channel; //!< Канал DMA для передачи.
    usart_bus_rx_byte_callback_t rx_byte_callback; //!< Каллбэк при приёме байта.
    usart_bus_callback_t rx_callback; //!< Каллбэк событий приёма данных шины USART.
    usart_bus_callback_t tx_callback; //!< Каллбэк событий передачи данных шины USART.
    usart_bus_callback_t tc_callback; //!< Каллбэк события окончания передачи данных шины USART.
    usart_bus_callback_t de_set_callback; //!< Каллбэк события ???
    usart_bus_callback_t de_clr_callback; //!< Каллбэк события ???
    bool dma_rx_locked;//!< Заблокирован канал получения.
    bool dma_tx_locked;//!< Заблокирован канал передачи.
    usart_transfer_id_t rx_transfer_id;//!< Идентификатор приёма.
    usart_transfer_id_t tx_transfer_id;//!< Идентификатор передачи.
    usart_status_t rx_status; //!< Состояние канала приёма.
    usart_status_t tx_status; //!< Состояние канала передачи.
    usart_errors_t rx_errors; //!< Ошибки канала приёма.
    usart_errors_t tx_errors; //!< Ошибки канала передачи.
    uint16_t rx_size; //!< Размер данных для приёма.
    uint16_t tx_size; //!< Размер данных для передачи.
    usart_idle_mode_t idle_mode; //!< Режим реакции на IDLE при приёме.
    uint8_t* rx_bf;
    uint8_t* tx_bf;
} usart_bus_t;

/**
 * Получает флаг разрешённости передатчика USART.
 * @param usart Устройство USART.
 * @return Флаг разрешённости передатчика USART.
 */
EXTERN FunctionalState usart_bus_transmitter_state(USART_TypeDef* usart);

/**
 * Получает флаг разрешённости приёмника USART.
 * @param usart Устройство USART.
 * @return Флаг разрешённости приёмника USART.
 */
EXTERN FunctionalState usart_bus_receiver_state(USART_TypeDef* usart);

/**
 * Получает флаг полудуплексного режима USART.
 * @param usart Устройство USART.
 * @return Флаг полудуплексного режима USART.
 */
EXTERN FunctionalState usart_bus_halfduplex_state(USART_TypeDef* usart);

/**
 * Инициализирует шину USART.
 * Разрешает необходимые прерывания USART.
 * @param usart Шина USART.
 * @param usart_bus_init Структура инициализации USART.
 * @return Код ошибки.
 */
EXTERN err_t usart_bus_init(usart_bus_t* usart, usart_bus_init_t* usart_bus_is);

/**
 * Функция для обработки прерывания USART.
 * Должна вызываться из обработчика прерывания USART.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_irq_handler(usart_bus_t* usart);

/**
 * Обработчик прерывания канала передачи DMA.
 * @param usart Шина USART.
 * @return true, если канал использовался шиной usart, иначе false.
 */
EXTERN bool usart_bus_dma_rx_channel_irq_handler(usart_bus_t* usart);

/**
 * Обработчик прерывания канала приёма DMA.
 * @param usart Шина USART.
 * @return true, если канал использовался шиной usart, иначе false.
 */
EXTERN bool usart_bus_dma_tx_channel_irq_handler(usart_bus_t* usart);

/**
 * Получает флаг включения приёмника.
 * @param usart Шина USART.
 * @return true, если приёмник включен, иначе false.
 */
EXTERN bool usart_bus_receiver_enabled(usart_bus_t* usart);

/**
 * Включает приёмник.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_receiver_enable(usart_bus_t* usart);

/**
 * Выключает приёмник.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_receiver_disable(usart_bus_t* usart);

/**
 * Получает флаг включения передатчика.
 * @param usart Шина USART.
 * @return true, если передатчик включен, иначе false.
 */
EXTERN bool usart_bus_transmitter_enabled(usart_bus_t* usart);

/**
 * Включает передатчик.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_transmitter_enable(usart_bus_t* usart);

/**
 * Выключает передатчик.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_transmitter_disable(usart_bus_t* usart);

/**
 * Получает флаг занятости шины usart на приём.
 * @param usart Шина usart.
 * @return Флаг занятости шины usart.
 */
EXTERN bool usart_bus_rx_busy(usart_bus_t* usart);

/**
 * Получает флаг занятости шины usart на передачу.
 * @param usart Шина usart.
 * @return Флаг занятости шины usart.
 */
EXTERN bool usart_bus_tx_busy(usart_bus_t* usart);

/**
 * Ждёт завершения текущей операции usart на приём.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_rx_wait(usart_bus_t* usart);

/**
 * Ждёт завершения текущей операции usart на передачу.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_tx_wait(usart_bus_t* usart);

/**
 * Получает идентификатор приёма.
 * @param usart Шина USART.
 * @return Идентификатор передачи.
 */
EXTERN usart_transfer_id_t usart_bus_rx_transfer_id(usart_bus_t* usart);

/**
 * Устанавливает идентификатор приёма.
 * @param usart Шина USART.
 * @param id Идентификатор передачи.
 * @return true, если идентификатор передачи установлен, иначе false (шина занята).
 */
EXTERN bool usart_bus_set_rx_transfer_id(usart_bus_t* usart, usart_transfer_id_t id);

/**
 * Получает идентификатор передачи.
 * @param usart Шина USART.
 * @return Идентификатор передачи.
 */
EXTERN usart_transfer_id_t usart_bus_tx_transfer_id(usart_bus_t* usart);

/**
 * Устанавливает идентификатор передачи.
 * @param usart Шина USART.
 * @param id Идентификатор передачи.
 * @return true, если идентификатор передачи установлен, иначе false (шина занята).
 */
EXTERN bool usart_bus_set_tx_transfer_id(usart_bus_t* usart, usart_transfer_id_t id);

/**
 * Получает каллбэк приёма байта шины USART.
 * @param usart Шина USART.
 * @return Каллбэк приёма байта.
 */
EXTERN usart_bus_rx_byte_callback_t usart_bus_rx_byte_callback(usart_bus_t* usart);

/**
 * Устанавливает каллбэк приёма байта шины USART.
 * @param usart Шина USART.
 * @param callback Каллбэк приёма байта.
 */
EXTERN void usart_bus_set_rx_byte_callback(usart_bus_t* usart, usart_bus_rx_byte_callback_t callback);

/**
 * Получает каллбэк событий приёма данных шины USART.
 * @param usart Шина USART.
 * @return Каллбэк приёма байта.
 */
EXTERN usart_bus_callback_t usart_bus_rx_callback(usart_bus_t* usart);

/**
 * Устанавливает каллбэк событий приёма данных шины USART.
 * @param usart Шина USART.
 * @param callback Каллбэк приёма байта.
 */
EXTERN void usart_bus_set_rx_callback(usart_bus_t* usart, usart_bus_callback_t callback);

/**
 * Получает каллбэк событий передачи данных шины USART.
 * @param usart Шина USART.
 * @return Каллбэк.
 */
EXTERN usart_bus_callback_t usart_bus_tx_callback(usart_bus_t* usart);

/**
 * Устанавливает каллбэк событий передачи данных шины USART.
 * @param usart Шина USART.
 * @param callback Каллбэк.
 */
EXTERN void usart_bus_set_tx_callback(usart_bus_t* usart, usart_bus_callback_t callback);

/**
 * Получает каллбэк события окончания передачи данных шины USART.
 * @param usart Шина USART.
 * @return Каллбэк.
 */
EXTERN usart_bus_callback_t usart_bus_tc_callback(usart_bus_t* usart);

/**
 * Устанавливает каллбэк события окончания передачи данных шины USART.
 * @param usart Шина USART.
 * @param callback Каллбэк.
 */
EXTERN void usart_bus_set_tc_callback(usart_bus_t* usart, usart_bus_callback_t callback);

EXTERN void usart_bus_set_deset_callback(usart_bus_t* usart, usart_bus_callback_t callback);
EXTERN void usart_bus_set_declr_callback(usart_bus_t* usart, usart_bus_callback_t callback);

/**
 * Получает состояние канала приёма шины USART.
 * @param usart Шина USART.
 * @return Состояние канала приёма.
 */
EXTERN usart_status_t usart_bus_rx_status(usart_bus_t* usart);

/**
 * Получает состояние канала передачи шины USART.
 * @param usart Шина USART.
 * @return Состояние канала передачи.
 */
EXTERN usart_status_t usart_bus_tx_status(usart_bus_t* usart);

/**
 * Получает ошибки канала приёма шины USART.
 * @param usart Шина USART.
 * @return Ошибки канала приёма.
 */
EXTERN usart_errors_t usart_bus_rx_errors(usart_bus_t* usart);

/**
 * Получает ошибки канала передачи шины USART.
 * @param usart Шина USART.
 * @return Ошибки канала передачи.
 */
EXTERN usart_errors_t usart_bus_tx_errors(usart_bus_t* usart);

/**
 * Получает число полученных
 * после завершения приёма байт данных.
 * @param usart Шина USART.
 * @return Число полученных байт данных.
 */
EXTERN size_t usart_bus_bytes_received(usart_bus_t* usart);

/**
 * Получает число переданных
 * после завершения передачи байт данных.
 * @param usart Шина USART.
 * @return Число переданных байт данных.
 */
EXTERN size_t usart_bus_bytes_transmitted(usart_bus_t* usart);

/**
 * Получает вид реакции на IDLE при приёме.
 * @param usart Шина USART.
 * @return Вид реакции на IDLE.
 */
EXTERN usart_idle_mode_t usart_bus_idle_mode(usart_bus_t* usart);

/**
 * Устанавливает вид реакции на IDLE при приёме.
 * @param usart Шина USART.
 * @param mode Вид реакции на IDLE.
 */
EXTERN void usart_bus_set_idle_mode(usart_bus_t* usart, usart_idle_mode_t mode);

/**
 * Пропускает текущий поток данных
 * до метки IDLE.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_sleep(usart_bus_t* usart);

/**
 * Начинает принимать данные текущего потока
 * без ожидания метки IDLE.
 * @param usart Шина USART.
 */
EXTERN void usart_bus_wake(usart_bus_t* usart);

/**
 * Передаёт данные по шине USART.
 * Асинхронная операция.
 * @param usart Шина USART.
 * @param data Данные.
 * @param size Размер данных.
 * @return Код ошибки.
 */
EXTERN err_t usart_bus_send(usart_bus_t* usart, const void* data, size_t size);

/**
 * Направляет все последующие
 * полученные данные вплоть до
 * заданного размера в заданный буфер.
 * @param usart Шина USART.
 * @param data Буфер для данных.
 * @param size Размер буфера.
 * @return Код ошибки.
 */
EXTERN err_t usart_bus_recv(usart_bus_t* usart, void* data, size_t size);

#endif	/* USART_BUS_H */
