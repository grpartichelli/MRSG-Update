/* Copyright (c) 2012-2014. MRSG Team. All rights reserved. */
/* This file is part of MRSG.

MRSG is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MRSG is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MRSG.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef MRSG_DFS_CODE
#define MRSG_DFS_CODE

#include <simgrid/msg.h>
#include "common_mrsg.hpp"
#include "worker_mrsg.h"
#include "dfs_mrsg.h"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY (msg_test);

char**  chunk_owner_mrsg;

static void send_mrsg_data (msg_task_t msg, Task_MRSG* task);

void distribute_data_mrsg (void)
{
    size_t  chunk;

    /* Allocate memory for the mapping matrix. */
    chunk_owner_mrsg = xbt_new (char*, config_mrsg.mrsg_chunk_count);
    for (chunk = 0; chunk < config_mrsg.mrsg_chunk_count; chunk++)
    {
	chunk_owner_mrsg[chunk] = xbt_new0 (char, config_mrsg.mrsg_number_of_workers);
    }

    /* Call the distribution function. */
    user_mrsg.dfs_f (chunk_owner_mrsg, config_mrsg.mrsg_chunk_count, config_mrsg.mrsg_number_of_workers, config_mrsg.mrsg_chunk_replicas);
}

void default_mrsg_dfs_f (char** dfs_matrix, size_t chunks, size_t workers_mrsg, int replicas)
{
    unsigned int r;
    size_t  chunk;
    size_t  owner;

    if (config_mrsg.mrsg_chunk_replicas >= config_mrsg.mrsg_number_of_workers)
    {
	/* All workers own every chunk. */
	for (chunk = 0; chunk < config_mrsg.mrsg_chunk_count; chunk++)
	{
	    for (owner = 0; owner < config_mrsg.mrsg_number_of_workers; owner++)
	    {
		chunk_owner_mrsg[chunk][owner] = 1;
	    }
	}
    }
    else
    {
	/* Ok, it's a typical distribution. */
	for (chunk = 0; chunk < config_mrsg.mrsg_chunk_count; chunk++)
	{
	    for (r = 0; r < config_mrsg.mrsg_chunk_replicas; r++)
	    {
		owner = ((chunk % config_mrsg.mrsg_number_of_workers)
			+ ((config_mrsg.mrsg_number_of_workers / config_mrsg.mrsg_chunk_replicas) * r)
			) % config_mrsg.mrsg_number_of_workers;

		chunk_owner_mrsg[chunk][owner] = 1;
	    }
	}
    }
}

size_t find_random_chunk_owner_mrsg (int cid)
{
    int     replica;
    long owner = NONE;
    size_t  mrsg_wid;

    replica = rand () % config_mrsg.mrsg_chunk_replicas;

    for (mrsg_wid = 0; mrsg_wid < config_mrsg.mrsg_number_of_workers; mrsg_wid++)
    {
	if (chunk_owner_mrsg[cid][mrsg_wid])
	{
	    owner = mrsg_wid;

	    if (replica == 0)
		break;
	    else
		replica--;
	}
    }

    xbt_assert (owner != NONE, "Aborted: chunk %d is missing.", cid);

    return owner;
}

int data_node_mrsg (int argc, char* argv[])
{
    char         mailbox[MAILBOX_ALIAS_SIZE];
    msg_error_t  status;
    msg_task_t   msg = NULL;
    Task_MRSG* task_ptr;
    TWO_TASKS* two_tasks;

    sprintf (mailbox, DATANODE_MRSG_MAILBOX, get_mrsg_worker_id (MSG_host_self ()));            //AQUI
    size_t wid =get_mrsg_worker_id (MSG_host_self ())+1 ;                                       //AQUI
    mrsg_task_pid.data_node[wid] = MSG_process_self_PID ();                                     //AQUI

    while (!job_mrsg.finished)
    {
        two_tasks = alt_receive(mailbox);
        msg = two_tasks->old_task;
        task_ptr = two_tasks->new_task;

        if (msg)
        {
            if (mrsg_message_is (msg, SMS_FINISH_MRSG))
            {
                MSG_task_destroy (msg);                                                          //AQUI
                break;
            }
            else
            {
                send_mrsg_data (msg, task_ptr);
            }
        }
    }

    return 0;
}

static void send_mrsg_data (msg_task_t msg, Task_MRSG* task)
{
    char         mailbox[MAILBOX_ALIAS_SIZE];
    double       data_size;
    size_t       my_id;
    mrsg_task_info_t  ti;
    mrsg_task_data_capsule_t t_data;
    
    //XBT_INFO("POINT 1");
    my_id = get_mrsg_worker_id (MSG_host_self ());                                                                              //AQUI

    sprintf (mailbox, TASK_MRSG_MAILBOX,
	    get_mrsg_worker_id (task->getSource()),                                                                         //AQUI
	    task->getSender()->getPid());                                                                       //AQUI


    if (mrsg_message_is (msg, SMS_GET_MRSG_CHUNK))
    {
        //XBT_INFO("POINT 2");
        alt_send("DATA-C", 0.0, config_mrsg.mrsg_chunk_size, NULL, mailbox); //async send?
        //XBT_INFO("POINT 3");
    }
    else if (mrsg_message_is (msg, SMS_GET_INTER_MRSG_PAIRS))
    {
        //XBT_INFO("POINT 4");
        ti = (mrsg_task_info_t) task->getData();                    
	    data_size = job_mrsg.map_output[my_id][ti->mrsg_tid] - ti->map_output_copied[my_id];
        alt_send("DATA-IP", 0.0, data_size, NULL, mailbox); //async send?
        //XBT_INFO("POINT 5");
    }

    MSG_task_destroy (msg);                                                                                                       //AQUI
}


#endif