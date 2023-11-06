#pragma once

struct request_job {
    int client_socket;
};

int post_request_job(struct request_job job);
int get_request_job(struct request_job* job);
void free_all_request_jobs(void);
