

// Used to provide some common utility functions


//Global initialization related to the "mailbox" command. 
//This function needs to be called within the global main function
int global_init(void);


void gen_events(int num);

//init array by num
int init_u32_array(u32 array[], int length, u32 num);

//init array by num
int init_u64_array(u64 array[], int length, u64 num);

int samt_handler_adapter(void);
