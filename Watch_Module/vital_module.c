#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
#include "vital_module.h"

#define UDP_PORT 5005

void* watch_collector_run(void* arg) {
    int sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_size;
    char buffer[1024];
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(UDP_PORT);
    
    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("워치 모듈 바인딩 실패");
        return NULL;
    }
    
    printf("[Watch Module] 수집 시작 (Port: %d)\n", UDP_PORT);
    
    while (1) {
        clnt_size = sizeof(clnt_addr);
        int len = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&clnt_addr, &clnt_size);
        
        if (len > 0) {
            buffer[len] = '\0';
            	
            cJSON *root = cJSON_Parse(buffer);
            if (root == NULL) {
                printf("JSON 파싱 에러\n");
                continue;
            }

            WatchData data;
            cJSON *sen_id = cJSON_GetObjectItem(root, "sen_id");
            cJSON *wp_id = cJSON_GetObjectItem(root, "wp_id");
            cJSON *sk_temp = cJSON_GetObjectItem(root, "sk_temp");
            cJSON *hr = cJSON_GetObjectItem(root, "hr");
            cJSON *time = cJSON_GetObjectItem(root, "time");


            if (cJSON_IsString(sen_id)) strncpy(data.sen_id, sen_id->valuestring, 63);
            if (cJSON_IsNumber(wp_id)) strncpy(data.wp_id, wp_id->valuestring, 63);
            if (cJSON_IsNumber(sk_temp)) data.sk_temp = (float)sk_temp->valuedouble;
            if (cJSON_IsNumber(hr)) data.hr = (float)hr->valuedouble;
            if (cJSON_IsString(time)) strncpy(data.time, time->valuestring, 31);

            printf("[WATCH] ID: %s | HR: %.1f | Temp: %.1f | Time: %s\n", 
                    data.sen_id, data.hr, data.sk_temp, data.time);

            cJSON_Delete(root);
        }
    }
    close(sock);
    return NULL;
}
        
