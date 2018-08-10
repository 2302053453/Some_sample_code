#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
 
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "config.h"



#define MAX_LEN 1024 * 512
#define MAC_SIZE	18
#define IP_SIZE		16

// 根据域名获取ip
int get_ip_by_domain(const char *domain, char *ip)
{
	char **pptr;
	struct hostent *hptr;
 
	hptr = gethostbyname(domain);
	if(NULL == hptr)
	{
		printf("gethostbyname error for host:%s/n", domain);
		return -1;
	}
 
	for(pptr = hptr->h_addr_list ; *pptr != NULL; pptr++)
	{
		if (NULL != inet_ntop(hptr->h_addrtype, *pptr, ip, IP_SIZE) )
		{
			return 0; // 只获取第一个 ip
		}
	}
 
	return -1;
}
 
// 获取本机mac
int get_local_mac(const char *eth_inf, char *mac)
{
	struct ifreq ifr;
	int sd;
	
	bzero(&ifr, sizeof(struct ifreq));
	if( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("get %s mac address socket creat error\n", eth_inf);
		return -1;
	}
	
	strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);
 
	if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
	{
		printf("get %s mac address error\n", eth_inf);
		close(sd);
		return -1;
	}
 
	snprintf(mac, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
		(unsigned char)ifr.ifr_hwaddr.sa_data[0], 
		(unsigned char)ifr.ifr_hwaddr.sa_data[1],
		(unsigned char)ifr.ifr_hwaddr.sa_data[2], 
		(unsigned char)ifr.ifr_hwaddr.sa_data[3],
		(unsigned char)ifr.ifr_hwaddr.sa_data[4],
		(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
 
	close(sd);
	
	return 0;
}
 
 
// 获取本机ip
int get_local_ip(const char *eth_inf, char *ip)
{
	int sd;
	struct sockaddr_in sin;
	struct ifreq ifr;
 
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sd)
	{
		printf("socket error: %s\n", strerror(errno));
		return -1;		
	}
 
	strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	
	// if error: No such device
	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
	{
		printf("ioctl error: %s\n", strerror(errno));
		close(sd);
		return -1;
	}
 
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));
 
	close(sd);
	return 0;
}

//大小端判断程序
int isLittleEndian()
{
    union
    {
        short a;
        char  b;
    }test;
    test.a=0x0061;
    if(test.b==0x61) return 1;
    else return 0;
}



//�ж��Ƿ�ΪĿ¼
bool isDir(const char* path)
{
    struct stat st;
    lstat(path, &st);
    return 0 != S_ISDIR(st.st_mode);
}
 
 


//�����ļ���de�ݹ麯��
void __findFiles(const char *path, int recursive)
{
    DIR *pdir;
    struct dirent *pdirent;
    char temp[MAX_LEN];
	struct stat statbuf;
	int size = 0;
	FILE *inputfile = NULL;
	
    pdir = opendir(path);
    if(pdir)
    {
        while((pdirent = readdir(pdir)))
        {
            //����"."��".."
            if(strcmp(pdirent->d_name, ".") == 0
               || strcmp(pdirent->d_name, "..") == 0)
                continue;
            sprintf(temp, "%s/%s", path, pdirent->d_name);
            
            //��tempΪĿ¼����recursiveΪ1��ʱ��ݹ鴦����Ŀ¼
            if(isDir(temp) && recursive)
            {
                __findFiles(temp, recursive);
            }
            else
            {	
            	inputfile = fopen(temp, "r");
				if(inputfile){
	            	if(0 == fstat(fileno(inputfile), &statbuf)){
						size = statbuf.st_size;
	            	}
					fclose(inputfile);
				}else{
					printf("open file err = %s\n",strerror(errno));
				}
                printf("======%s size = %d\n", temp,size);
            }
        }
    }
    else
    {
        printf("opendir error:%s\n", path);
    }
    closedir(pdir);
}

//�����ļ��е���������
void findFiles(const char *path)
{
    unsigned long len;
    char temp[MAX_LEN];
    //ȥ��ĩβ��'/'
    len = strlen(path);
    strcpy(temp, path);
    if(temp[len - 1] == '/') temp[len -1] = '\0';
    
    if(isDir(temp))
    {
        //����Ŀ¼
        int recursive = 1;
        __findFiles(temp, recursive);
    }
    else   //����ļ�
    {
        printf("======%s\n", path);
    }
}
 

void scan_dir(char *dir, int depth)   // ����Ŀ¼ɨ�躯��  
{  
    DIR *dp;                      // ������Ŀ¼��ָ��  
    struct dirent *entry;         // ����dirent�ṹָ�뱣�����Ŀ¼  
    struct stat statbuf;          // ����statbuf�ṹ�����ļ�����  
    if((dp = opendir(dir)) == NULL) // ��Ŀ¼����ȡ��Ŀ¼��ָ�룬�жϲ����Ƿ�ɹ�  
    {  
        puts("can't open dir.");  
        return;  
    }  
    chdir (dir);                     // �л�����ǰĿ¼  
    while((entry = readdir(dp)) != NULL)  // ��ȡ��һ��Ŀ¼��Ϣ�����δ����ѭ��  
    {  
        lstat(entry->d_name, &statbuf); // ��ȡ��һ����Ա����  
        if(S_IFDIR &statbuf.st_mode)    // �ж���һ����Ա�Ƿ���Ŀ¼  
        {  
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)  
              continue;  
  
            printf("%*s%s/\n", depth, "", entry->d_name);  // ���Ŀ¼����  
            scan_dir(entry->d_name, depth+4);              // �ݹ��������ɨ����һ��Ŀ¼������  
        }  
        else  
            printf("%*s%s\n", depth, "", entry->d_name);  // ������Բ���Ŀ¼�ĳ�Ա  
    }  
    chdir("..");                                                  // �ص��ϼ�Ŀ¼  
    closedir(dp);                                                 // �ر���Ŀ¼��  
}  

static int get_volume_grade(int value)
{	
	if((value % 9) == 0){
		return value/9;
	}else{
		return value/9+1;
	}
}

