/*************************************************************************************************
 * @file		mp3Task.c
 *
 * @brief		Заголовок задачи декодера MP3
 *
 * @version		v1.0
 * @date		10.09.2013
 * @author		Mike Smith
 *
 ************************************************************************************************/
#ifndef MP3TASK_H_
#define MP3TASK_H_


// инициализация/запуск декодера
void * mp3_player_init(decoder_param_t * decoder_params);
extern QueueHandle_t mp3_Cmd;

#endif /* MP3TASK_H_ */
