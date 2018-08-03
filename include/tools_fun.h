#ifndef __TOOL_FUN_H__
#define __TOOL_FUN_H__

// 根据域名获取ip
int get_ip_by_domain(const char *domain, char *ip);
// 获取本机mac
int get_local_mac(const char *eth_inf, char *mac);
// 获取本机ip
int get_local_ip(const char *eth_inf, char *ip);

//�ж��Ƿ�Ϊ�ļ���
bool isDir(const char* path);
 
//�����ļ��е���������
void findFiles(const char *path);
 
//�����ļ���de�ݹ麯��
void __findFiles(const char *path, int recursive);
#endif