#ifndef CXL_IDENTIFY_H
#define CXL_IDENTIFY_H
#include <xil_io.h>
#define IDENTIFY_OUT_PAYLOAD_LENGTH 67

struct identify_info{
    unsigned char fw_revision[16];
    u64 total_cap;
    u64 volatile_only_cap;
    u64 persistent_only_cap;
    u64 partition_alignment;
    char info_event_size[2];
    char warning_event_size[2];
    char failure_event_size[2];
    char fatal_event_size[2];
    char lsa_size[4];
    char poison_max_err_records[3];
    char poison_inject_limit[2];
    char poison_handling_cap;
    char qos_tel_cap;
};

int mailbox_identify_mem_dev(int opcode, int *payload_length, char *payload);

struct identify_info get_identify_info(void);

#endif
