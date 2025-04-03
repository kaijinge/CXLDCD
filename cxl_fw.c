#include "include/cxl_fw.h"
#include "include/config_pl.h"
#include "include/common_mbox_function.h"
#include "include/cxl_log.h"
#include "include/resource_manager_io.h"


int transfer_tag = NOT_IN_PROGRESS_TAG;
int transfer_offset = 0;
struct fw_info g_fw_info = {2, 0, 0, {0}, {{0}, {0},{0},{0}}};




// The call has a prerequisite: allocate space from the 2MB region for transferring firmware versions
//, and the size of the firmware version will not exceed this space.
// tbd: Return codes - Aborted, FW Authentication Failed, FW Transfer Out of Order.
int mailbox_transfer_fw(int opcode, int *payload_length, char *payload)
{
    //The actual size of the firmware data can be obtained by subtracting 128 from *payload_length.
    if(TRANSFER_FW_SUP == 0){
        return UNSUPPOTED;
    }
    if(*payload_length <0 ||*payload_length >CXL_MAX_PAYLOAD_SIZE){
        return INVALID_PAYLOAD_LENGTH;
    }
    
    if(transfer_tag == IN_PROGRESS_TAG){
        return FW_TRANSFER_IN_PROGRESS;
    }
    char action = payload[0];
    char slot = payload[1];
    int *offset = (int *)(&(payload[4]));
    if(slot<=0 || slot >g_fw_info.fw_slots_supported){
        return INVALID_SLOT;
    }
    int data_length = *payload_length -128;
    *payload_length = 0;
    switch(action)
    {
        //full transmission, ensure that it can be transmitted in a single instance.
        case FULL_FW_TRANSFER_ACTION:{
            transfer_tag = IN_PROGRESS_TAG;
            //Write to the corresponding position of the slot in the input payload.
            memcpy((void*)(SLOT_BASE_ADDR + (slot-1) * SLOT_SIZE) , payload+128, data_length);
            transfer_tag = NOT_IN_PROGRESS_TAG;
            transfer_offset = 0;
            //cel
            char *command_effect = get_command_effect(0b01000001);
            cxl_gen_vdl(opcode, command_effect);
            return SUCCESS;
        }

        //First block in multiple transmissions; the logic for the following three cases is roughly the same.
        case INITIATE_FW_TRANSFER_ACTION:
        
        //continue
        case CONTINUE_FW_TRANSFER_ACTION: 
        
        //last
        case END_TRANSFER_ACTION:{
            if(transfer_offset != *offset){
                //Compare transfer_offset with the offset in the input payload; 
                //if inconsistent, return FW Transfer Out of Order.
                return FW_TRANSFER_OUT_OF_ORDER;
            }
            transfer_tag = IN_PROGRESS_TAG;
            memcpy((void*)(SLOT_BASE_ADDR + (slot-1) * SLOT_SIZE +(transfer_offset * 128)), payload + 128, data_length);
            //The data_length is ensured by the host to be in multiples of 128 bytes.
            transfer_offset += data_length / 128;
            transfer_tag = NOT_IN_PROGRESS_TAG;
            //cel
            char *command_effect;
            if(action == END_TRANSFER_ACTION){
                command_effect = get_command_effect(0b01000001);
            }
            else{
                 command_effect = get_command_effect(0b01000000);
            }
            cxl_gen_vdl(opcode, command_effect);
            return SUCCESS;
        }


        case ABORT_TRANSFER_ACTION:{
        transfer_tag = NOT_IN_PROGRESS_TAG;
        transfer_offset = 0;
        //cel
        char *command_effect = get_command_effect(NONE_COMMAND_EFFECT);
        cxl_gen_vdl(opcode, command_effect);
        return ABORTED;
        }

        default:
        break;
    }
    return 0;
}




//The get_fw_info command has no command effects, an input size of 0, and an output size of 80 bytes.
//possible return codes：SUCCESS、UNSUPOORTED、INTERNAL ERROR、RETRY Required、Invalid payload length
int mailbox_get_fw_info(int opcode, int *payload_length, char *payload)
{
    int i;
    char *slot_revison_offset;
    if(GET_FW_INFO_SUP == 0){
        return UNSUPPOTED;
    }
    if(*payload_length != 0){
        return INVALID_PAYLOAD_LENGTH;
    }
    //1.Generate a false fw_info that only assembles the package.
    struct fw_info fw_info = get_fw_info();
    //2.Fill the payload with information from the firmware, and update the payload length.
    
    payload[0] = fw_info.fw_slots_supported;
    payload[1] = fw_info.fw_slot_info;
    payload[2] = fw_info.fw_activation_cap;
    //rsvd填充，不能完全确保来自host的payload是不是空的，两端都要做校验
    for(i=3;i<16;++i){
        payload[i] = 0;
    }
    slot_revison_offset = payload + 16;
    //Fill in the reserved (rsvd) fields. It cannot be completely guaranteed that the payload from the host is not empty; 
    //both ends should perform validation.
    for(i=0;i<4;++i){
        memcpy(slot_revison_offset+(i*16),fw_info.slot[i].revision,16);
    }
    *payload_length = GET_FW_LEGNTH;
     //cel
    char *command_effect = get_command_effect(NONE_COMMAND_EFFECT);
    cxl_gen_vdl(opcode, command_effect);

    return SUCCESS;
}

struct fw_info get_fw_info()
{
    return g_fw_info;
}


//The identify command can invoke this function to obtain the fw_revision information.
char *get_active_fw_revison(void)
{
    char slot_info = g_fw_info.fw_slot_info;
    int index = slot_info & 0x7;
    return g_fw_info.slot[index].revision;
}
