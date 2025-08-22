/*************************************************************************************************
 * @file		includes.h
 *
 * @brief		Список подключаемых программных модулей
 *
 * @version		v1.0
 * @date		03.09.2013
 *
 *************************************************************************************************/
#ifndef INCLUDES_H_
#define INCLUDES_H_

// операционная система
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "xprintf.h"

// файловая система
#include "ff.h"

// стандартные модули
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// наши модули
#include "board.h"
#include "exceptions.h"
#include "fileInfo.h"

#endif /* INCLUDES_H_ */
