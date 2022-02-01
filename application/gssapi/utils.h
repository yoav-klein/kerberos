
#ifndef __UTILS_H__
#define __UTILS_H__

void display_context_flags(int ret_flags);
void display_status(char *message, OM_uint32 major, OM_uint32 minor);

int send_token(int fd, gss_buffer_desc *token);
int recv_token(int fd, gss_buffer_desc *token);

#endif /* __UTILS_H__ */