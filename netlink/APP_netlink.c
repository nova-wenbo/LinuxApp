/***************************************
* Author   : wenbo
* Versions : netlink_1.0
* Time     : 2017.10
*****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#define MSG_SIZE 2048
struct netlink_msg{
	char msg[MSG_SIZE];
	unsigned int count;
};
struct netlink_msg *i_msg = NULL;
/*
* Detection usb event
*/
int netlink_checkout_usb()
{
	
	struct sockaddr_nl client;
	const int netlink_buf = 1024;
	int netlink_fb = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
	i_msg = malloc(sizeof(*i_msg));
	memset(&client, 0, sizeof(client));
	client.nl_family = AF_NETLINK;
        client.nl_pid = getpid();
        client.nl_groups = 1; /* receive broadcast message*/
        setsockopt(netlink_fb, SOL_SOCKET, SO_RCVBUF, &netlink_buf, sizeof(netlink_buf));
        bind(netlink_fb, (struct sockaddr*)&client, sizeof(client));
	return netlink_fb;
}
void netlink_recv_msg(int netlink_fb)
{
        struct timeval tv;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(netlink_fb, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;
        select(netlink_fb + 1, &fds, NULL, NULL, &tv);
        i_msg->count = recv(netlink_fb, i_msg->msg, sizeof(i_msg->msg), 0);
    //    if(i_msg->count > 0) printf("%s\n",i_msg->msg);

}

void netlink_chars_handle()
{
	char cmd[32] = {0};
        char udir[4] = {0};
	char *pDeviceName = NULL;
    	struct dirent *ptr = NULL;
   	DIR *dir = NULL;		
	if(!(strncmp(i_msg->msg,"add",3))){
            pDeviceName = strstr(i_msg->msg,"block/sd");
            if(pDeviceName){
            	sprintf(cmd,"mount /dev/%s /mnt",pDeviceName+10);
           	sprintf(udir,"%s",pDeviceName+10);
                if(strncmp(udir,"sd",2) == 0){
                    system(cmd);
                    printf("插入U盘 : %s\n",pDeviceName+10);
                    dir = opendir("/mnt");
                    while((ptr = readdir(dir)) != NULL)
                    {
                     	pDeviceName = strstr(ptr->d_name,".txt");
                        if(pDeviceName)
                       	    printf("%s :文本文件\n",ptr->d_name);
                            pDeviceName = strstr(ptr->d_name,".img");
                            if(pDeviceName)
                                printf("%s :更新文件\n",ptr->d_name);
                     }

         	 }	
             }
       }
       else if(!(strncmp(i_msg->msg,"remove",6))){
           pDeviceName = strstr(i_msg->msg,"block/sd");
           if(pDeviceName){
           	sprintf(udir,"%s",pDeviceName+10);
                if(strncmp(udir,"sd",2) == 0){
                    closedir(dir);
                  //  system("umount /mnt");
                    printf("移除U盘\n");
                }
           }
       }
	
}
int main(int argc, char **argv)
{
	int netlink_fb = netlink_checkout_usb();
	for(;;){
		netlink_recv_msg(netlink_fb);
		netlink_chars_handle();
	}
	free(i_msg);
	return 0;
}
