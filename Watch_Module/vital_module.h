#ifndef VITAL_MODULE_H
#define VITAL_MODULE_H

typedef struct {
    char sen_id[64];   
    char wp_id[64];        
    float sk_temp;    
    float hr;          
    char time[32];
} WatchData;

void* vital_module(void* arg);

#endif
