/*
 *  Software for Industrial Communication, Motion Control, and Automation
 *
 *  Copyright (C) 2002-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *
 *  This module is part of the SourceMedley repository.
 *
 *  This module is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this module.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        queue.h
 *
 *  @brief       Queue for intertask communication.
 *
 *  @remarks     A producer thread puts a data element of configurable size into
 *               the queue (FIFO), if it is not full.
 *               A consumer thread waits until at least one element is available
 *               in the queue and dequeues it, or returns when a time-out occurs.
 *
 *  @note        When the queue is full no further data element will be enqueued.
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 690 $
 *
 *  @defgroup    queue Waitable Queue
 *  @{
 */
#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */


/*  -----------  types  --------------------------------------------------
 */

typedef void *queue_t;                  /**< queue (opaque data type) */


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief       creates an instance of a waitable queue (constructor).
 *
 *  @param[in]   numElem   - maximum number of elements in the queue
 *  @param[in]   elemSize  - size of a queue element (number of bytes)
 *
 *  @returns     pointer to a queue instance if successful, or NULL on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      EINVAL   - invalid argument (numElem or elemSize)
 *  @retval      ENOMEM   - out of memory (insufficient storage space)
 *  @retval      'errno'  - error code from called system functions:
 *                          'pthread_mutex_init', 'pthread_cond_init'
 */
extern queue_t queue_create(size_t numElem, size_t elemSize);


/** @brief       destroys the queue instance (destructor).
 *
 *  @param[in]   queue  - pointer to a queue instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      EFAULT   - bad address (invalid queue instance)
 *  @retval      'errno'  - error code from called system functions:
 *                          'pthread_mutex_destroy', 'pthread_cond_destroy'
 */
extern int queue_destroy(queue_t queue);


/** @brief       removes all enqueued elements from the queue and reset the
 *               overflow indicator and the overflow counter.
 *
 *  @param[in]   queue  - pointer to a queue instance
 *
 *  @returns     the number of elements removed from the queue if successful, or
 *               a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      EFAULT   - bad address (invalid queue instance)
 */
extern int queue_clear(queue_t queue);


/** @brief       enqueues one element of n data bytes into the queue,
 *               if the queue is not full.
 *
 *  @remarks     The function copies not more data bytes than the size of the
 *               queue allows (see parameter 'elemSize' of 'queue_create').
 *               @see queue_create
 *
 *  @param[in]   queue    - pointer to a queue instance
 *  @param[in]   element  - data element to be enqueued
 *  @param[in]   nbytes   - number of bytes to be copied into the queue
 *
 *  @returns     the number of bytes copied into the queue if successful, or
 *               a negative value on error.
 *
 *  @retval      -20  - when the queue is full (CAN API compatible)
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      EFAULT  - bad address (invalid queue instance)
 *  @retval      EINVAL  - invalid argument (element or nbytes)
 *  @retval      ENSPC   - no space left (queue is full)
 */
extern int queue_enqueue(queue_t queue, const void *element, size_t nbytes);


/** @brief       dequeues one element from the queue, if any.
 *
 *  @param[in]   queue    - pointer to a queue instance
 *  @param[out]  element  - pointer to an array into which the element is copied
 *  @param[in]   maxbytes - maximum number of bytes to be copied from the queue
 *  @param[in]   timeout  - time to wait for elements available in the queue:
 *                               0 means the function returns immediately,
 *                               65535 means blocking read, and any other
 *                               value means the time to wait im milliseconds
 *
 *  @returns     the number of bytes copied from the queue if successful, or
 *               a negative value on error.
 *
 *  @retval      -30  - when the queue is empty (CAN API compatible)
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      EFAULT  - bad address (invalid queue instance)
 *  @retval      EINVAL  - invalid argument (element or maxbytes)
 *  @retval      ENOMSG  - no data available (queue empty)
 */
extern int queue_dequeue(queue_t queue, void *element, size_t maxbytes, uint16_t timeout);


/** @brief       returns true when an overflow has occurred.
 *
 *  @remarks     The overflow indicator can be reset by a call of 'queue_clear'.
 *               @see queue_clear
 *
 *  @param[in]   queue    - pointer to a queue instance
 *  @param[out]  counter  - number of overflows (optional)
 *
 *  @returns     0 when no overflow has occurred, or a none-zero value otherwise.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      EFAULT   - bad address (invalid queue instance)
 */
extern bool queue_overflow(queue_t queue, uint64_t *counter);


/** @brief       signals waiting objects, if any.
 *
 *  @param[in]   queue  - pointer to a queue instance
 *
 *  @returns     0 if successful, or a negative value on error.
 */
extern int queue_signal(queue_t queue);


#ifdef __cplusplus
}
#endif
#endif /* QUEUE_H_INCLUDED */

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
