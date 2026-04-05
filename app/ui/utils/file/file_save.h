/* 文件持久化工具头文件
 * 提供针对任意结构体的读写接口，
 * 内部采用“先写临时文件再改名”的策略防止数据损坏。
 */
#ifndef _FILE_SAVE_H_
#define _FILE_SAVE_H_

/**
 * 将任意数据写入指定名称的文件。
 * 采用“写写临时文件 + fsync + rename”的原子替换，防止断电/崩溃导致文件损坏。
 * @param name  文件名（相对于资源目录）
 * @param data  要写入的数据指针
 * @param len   写入数据长度（字节）
 * @return  0 成功，-1 失败
 */
int file_param_write(const char *name,void *data,int len);

/**
 * 从指定名称的文件中读取数据。
 * 若主文件不存在但备份文件存在，自动从备份恢复。
 * @param name  文件名（相对于资源目录）
 * @param data  存储读取数据的缓冲区指针
 * @param len   期望读取的数据长度（字节）
 * @return  0 成功，-1 失败
 */
int file_param_read(const char *name,void *data,int len);

#endif
