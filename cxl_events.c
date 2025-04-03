#include "include/common_mbox_function.h"
#include "include/cxl_interrupt.h"
#include "include/cxl_events.h"
#include "include/cxl_log.h"
#include "include/common_tool.h"
#include "include/ipc_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct common_event_record g_record = {{0},0,{0,0,0},{0,0},{0,0},{0},{0},{0}};
char intr_policy[4] = {0};

// Initialize software-maintained variables related to the event log.
Event_log event_log = {{0,0,0,0}, {0,0,0,0}, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {0,0,0,0}, {0,0,0,0}, {-1,-1,-1,-1}, 0};
u64 event_addr[4] = {INFO_EVENT_ADDR, WARNING_EVENT_ADDR, FAILURE_EVENT_ADDR, FATAL_EVENT_INTR_ADDR};


int gen_false_event_intr_policy(char *false_intrl_policy)
{
    int i;
    int ret = 0;
    for(i=0;i<EVENT_RECORD_SEVERITY_NUM;++i){
        intr_policy[i] = false_intrl_policy[i];
    }
    return ret;
}

int mailbox_set_event_intr_policy(int opcode, int *payload_length, char *payload)
{
    int i;
    if(*payload_length != 4){
        return INVALID_PAYLOAD_LENGTH;
    }
    for(i=0;i<EVENT_RECORD_SEVERITY_NUM;++i){
        intr_policy[i] = payload[i];
    }
    //no output
    *payload_length = 0;

    //cel
    char *command_effect = get_command_effect(SET_EVENT_INTR_POLICY_EFFECT);
    cxl_gen_vdl(opcode, command_effect);

    return SUCCESS;
}

//no input
int mailbox_get_event_intr_policy(int opcode, int *payload_length, char *payload)
{
    int i;
    int ret;
    if(*payload_length != 0){
        return INVALID_PAYLOAD_LENGTH;
    }
    for(i=0;i<EVENT_RECORD_SEVERITY_NUM;++i){
        intr_policy[i] |= 0x90;     //inttrupt number is 0x9
        payload[i] = intr_policy[i];
    }
    *payload_length = 4;

    //cel
    char *command_effect = get_command_effect(GET_EVENT_INTR_POLICY_EFFECT);
    cxl_gen_vdl(opcode, command_effect);

    return SUCCESS;
}


int mailbox_clear_event_records(int opcode, int *payload_length, char *payload)
{
    if(*payload_length < 6&&payload[1]!=1){
        return INVALID_PAYLOAD_LENGTH;
    }
    int severity = payload[0];
    int clear_flag = payload[1];
    int number = payload[2];
    int i;
    //test
    struct common_event_record *record;
    int temp = 0;

    int queue_size = queueSize(&event_log, severity);
    //check the number
    if((number<=0&&clear_flag!=1)||number>queue_size){
    	return INVALID_INPUT;
    }
    if(clear_flag==1){
    //Simply change the queue pointer; the element is not actually deleted from BRAM.
        event_log.front[severity] = event_log.rear[severity] = -1;
        event_log.handle_index[severity] = -1;
        xil_printf("now clear all events which severity is %d\r\n", severity);
    }
    else{
    //Here is a prerequisite: the host-side consumption of events always occurs in an ordered manner in the queue.
        for(i=0;i<number;++i){
        	record = dequeue(&event_log, severity);
        	temp = 0;
        	memcpy(&temp,record->event_record_handle,2);
        	xil_printf("now clear events and the handle is %d\r\n",temp);
        }
    }
    //no output
    *payload_length = 0;
    //cel
    char *command_effect = get_command_effect(CLEAR_EVENT_RECORDS_EFEECT);
    cxl_gen_vdl(opcode, command_effect);
    return SUCCESS;
}

int mailbox_get_event_records(int opcode, int *payload_length, char *payload)
{
    int i;
    //Flags are set based on PAYLOAD_EVENT_RECORD_CAP
    if(*payload_length != 1){
        return INVALID_PAYLOAD_LENGTH;
    }
    int severity = payload[0];
    payload[0] = 0;
    //calculate how much elements there are from handle_index to the end of the queue 
    //and how much data still needs to be transmitted.
    int transfer_size = elementsAfterIndex(&event_log, severity, event_log.handle_index[severity]);
    if(transfer_size <= 0){
        return INTERNAL_ERROR;
    }
    //If it exceeds the payload's capacity.
    if(transfer_size > PAYLOAD_EVENT_RECORD_CAP){
        payload[0] |= 0x2;//still need transfer
        transfer_size = PAYLOAD_EVENT_RECORD_CAP;
    }

    //overflow bit
    if(event_log.overflow_flag[severity] == 1){
    payload[0] |= 0x1;//overflow bit
    payload[1] =0;//rsvd
    payload[2] = event_log.overflow_errcount[severity] & 0xFF;
    payload[3] = (event_log.overflow_errcount[severity]>>8) & 0xFF;

    memcpy(payload+4,&(event_log.overflow_first_time[severity]),8);
    memcpy(payload+12,&(event_log.overflow_last_time[severity]),8); 
    }

    //Write the data into the event records' relevant fields in the output payload
    payload[20] = transfer_size & 0xFF;
    payload[21] = (transfer_size >> 8) & 0xFF;
    xil_printf("now transfersize is %d and now handle_index is %d,begin to transfer\r\n",transfer_size,event_log.handle_index[severity]);
    //que_size
//    int queue_size = queueSize(&event_log,severity);
    int relative_index;
    int new_index;
    //Sequentially update the handle_index and write the data into the payload
    //tbd
    for(i=0;i<transfer_size;++i){
        // Write the record (pointed to by the current handle_index) into the payload
        memcpy(payload+32+(i*COMMON_EVENT_RECORD_SIZE),(void*)(event_addr[severity]+(event_log.handle_index[severity]*128)),COMMON_EVENT_RECORD_SIZE);
        // Move the handle_index while transferring data.
        relative_index = (event_log.handle_index[severity] - event_log.front[severity] + EVENT_OVERFLOW_CAP) % EVENT_OVERFLOW_CAP;
        new_index = (event_log.handle_index[severity] + 1) % EVENT_OVERFLOW_CAP;
        event_log.handle_index[severity] = (new_index + EVENT_OVERFLOW_CAP) % EVENT_OVERFLOW_CAP;
    }
    //output payload length： 32+ n*128
    *payload_length = 32 + transfer_size * COMMON_EVENT_RECORD_SIZE;
    //cel
    char *command_effect = get_command_effect(GET_EVENT_RECORDS_EFFECT);
    cxl_gen_vdl(opcode, command_effect);
    return SUCCESS;
}


//The upper-level logic ensures that the queue is non-full before calling.
//Precondition: If generation would result in overflow, it does not need to be generated.
struct common_event_record* gen_event_record(int event_record_type, char *flags, char *record_data)
{

    int severity;
    struct common_event_record record = {{0},0,{0,0,0},{0,0},{0,0},{0},{0},{0}};
    switch(event_record_type)
    {
        case GENERAL_MEDIA_TYPE:
        memcpy(record.event_record_uuid, general_media_event_uuid, EVENT_UUID_SIZE);
        record.event_record_length = GENERAL_MEDIA_SIZE;
        break;
        case DRAM_TYPE:
        memcpy(record.event_record_uuid, dram_event_uuid, EVENT_UUID_SIZE);
        record.event_record_length = DRAM_SIZE;
        break;
        case MEMORY_TYPE:
        memcpy(record.event_record_uuid, memory_module_event_uuid, EVENT_UUID_SIZE);
        record.event_record_length = MEMORY_MODULE_SIZE;
        break;
        default:
        break;
    }
    memcpy(record.event_record_flags, flags, EVENT_RECORD_FLAGS_BYTE_SIZE);

    //get handle
    severity = flags[0] & SEVERITY_MASK_IN_FLAGS;
    get_next_handle(record.event_record_handle, severity);

    memcpy(record.event_record_data, record_data, RECORD_DATA_SIZE);

    //tbd:get time_stamp
//    record.event_timestamp = ;
    //tbd:write data into bram
    enqueue(&event_log, severity, &record);

//    xil_printf("event gen success and event_record_type is %d .\r\n",event_record_type);
    memcpy((void*)(&g_record),(void*)(&record),128);
    return &g_record;
}

struct common_event_record* gen_err_rep_event_record(u64 physical_addr, char transaction_type)
{
    // If the current queue is full, it cannot be generated; modify the overflow information
    if(isFull(&event_log,FAILURE_EVENT_SEVERITY)){
        event_log.overflow_errcount[FAILURE_EVENT_SEVERITY] += 1;
        event_log.overflow_flag[FAILURE_EVENT_SEVERITY] = 1;
        xil_printf("que is full, gen failed overfolw count =%d\r\n",event_log.overflow_errcount[FAILURE_EVENT_SEVERITY]);
        return NULL;
    }
    char flags[EVENT_RECORD_FLAGS_BYTE_SIZE] = {2,0,0};

    char record_data[80]= {0};
    get_err_rep_record_data(record_data, physical_addr, transaction_type);

    struct common_event_record* record = gen_event_record(GENERAL_MEDIA_TYPE, flags, record_data);
    return record;
}


void get_err_rep_record_data(char *record_data, u64 physical_addr, char transaction_type)
{
    struct general_media_record_data data = {0, 0, 1, 0, {0,0}, 0, 0, {0,0,0}, {0}, {0}};
    data.physical_addr = physical_addr;
    data.transaction_type = transaction_type;
    memcpy(record_data,(void*)(&data),80);
}

//Circular queue-related content: encapsulate the handle as a char array of size 2
//, where the handle content represents the next index for inserting data into the queue 
// If the queue is already full, return -1.
int get_next_handle(char *handle, int severity)
{
    int temp = 0;
    if (isFull(&event_log,severity)) {
        handle[0] = -1;
        handle[1] = 0;
        return -1;
    }
    if (isEmpty(&event_log,severity)) {
        //handle_index == 0
    } else {
        // When the queue is not empty, move the rear one position backward for the next handle, 
        //considering circular behavior.
        temp = (event_log.rear[severity] + 1) % EVENT_OVERFLOW_CAP;
        handle[0] = temp & 0xFF;
        handle[1] = (temp >> 8) & 0xFF;
    }
    return SUCCESS;
}


//que is empty
int isEmpty(Event_log* event_log, int severity)
{
    return event_log->front[severity] == -1 && event_log->rear[severity] == -1;
}

//que is full
int isFull(Event_log* event_log, int severity)
{
    return (event_log->rear[severity] + 1) % EVENT_OVERFLOW_CAP == event_log->front[severity];
}

int samt_unset_dev_status(int severity)
{
    int value = pcie_mem_readl(PCIE_DEVICE_STATUS_BASE);
    value &= (~(0x1 << severity));

    pcie_mem_writel(PCIE_DEVICE_STATUS_BASE,value);
    return 0;
}

int samt_set_dev_status(int severity)
{
    int value = pcie_mem_readl(PCIE_DEVICE_STATUS_BASE);

    value |= (0x1 << severity);
    pcie_mem_writel(PCIE_DEVICE_STATUS_BASE,value);

    return 0;
}

// The upper-level logic ensures that the queue is non-full before calling.
//write the record data into bram
//tbd:when empty to non-empty then send a intr to host
void enqueue(Event_log* event_log, int severity, struct common_event_record* record)
{
	int intr_flag = 0;
    if (isEmpty(event_log,severity)) {
         event_log ->front[severity] = event_log ->rear[severity] = 0;
         //handle_check set 0
         event_log ->handle_index[severity] = 0;
         intr_flag = 1;
    } else {
        // change the rear pointer
        event_log->rear[severity] = (event_log->rear[severity] + 1) % EVENT_OVERFLOW_CAP;
    }
    //write the record data into bram
    memcpy((void*)(event_addr[severity] + event_log->rear[severity]*COMMON_EVENT_RECORD_SIZE), (void*)record, COMMON_EVENT_RECORD_SIZE);
    if(intr_flag)
    {
        samt_set_dev_status(severity);
    	send_intr_to_host();
    }
    xil_printf("event enque success and the addr is 0x%llx and quesize is %d. \r\n",event_addr[severity] + event_log->rear[severity]*COMMON_EVENT_RECORD_SIZE,queueSize(event_log,severity));
//    	xil_printf("count is %d\r\n",++count);
}

//The upper-level logic ensures that the queue is non-empty before calling.
struct common_event_record* dequeue(Event_log* event_log, int severity)
{
    memcpy((void*)(&g_record), (void*)(event_addr[severity] + event_log->front[severity]*COMMON_EVENT_RECORD_SIZE),COMMON_EVENT_RECORD_SIZE);
    //change the possible front,rear and handle_index pointer
    if (event_log->front[severity] == event_log->rear[severity]) {
        // only one element
        event_log->front[severity] = event_log->rear[severity] = -1;
        event_log ->handle_index[severity] = -1;
        samt_unset_dev_status(severity);
    } else {
         event_log->front[severity] = (event_log->front[severity] + 1) % EVENT_OVERFLOW_CAP;
    }
    return &g_record;
}

//Retrieve the number of elements in a severity queue.
int queueSize(Event_log* event_log, int severity) {
    if (isEmpty(event_log,severity)) {
        return 0;
    }
    if (event_log->front[severity] <= event_log->rear[severity]) {
        return event_log->rear[severity] - event_log->front[severity] + 1;
    } else {
        return EVENT_OVERFLOW_CAP - event_log->front[severity] + event_log->rear[severity] + 1;
    }
}


// Calculate the number of elements from the specified index to the end of the queue.
int elementsAfterIndex(Event_log* event_log, int severity, int startIndex) {
    if (isEmpty(event_log,severity) || startIndex < 0 || startIndex >= EVENT_OVERFLOW_CAP) {
        xil_printf("Invalid index or empty queue.\r\n");
        return -1;  // fail
    }

    int currentSize = queueSize(event_log, severity);
    int relativeIndex = (startIndex - event_log->front[severity] + EVENT_OVERFLOW_CAP) % EVENT_OVERFLOW_CAP;

    //  Ensure the relative index is valid.
    if (relativeIndex >= 0 && relativeIndex < currentSize) {
        return currentSize - relativeIndex;
    } else {
        xil_printf("Invalid index.\r\n");
        return -1; //fail
    }
}
