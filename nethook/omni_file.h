/*
 * =====================================================================================
 *
 *       Filename:  omni_file.h
 *
 *    Description:  file implement
 *
 *        Version:  1.0
 *        Created:  05/22/2013 10:20:05 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Changqing,Zhao ,changqing.1230@163.com
 *        Company:  None
 *
 * =====================================================================================
 */

#ifndef _OMNI_FILE_
#define _OMNI_FILE_

#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/security.h>
#include <linux/mount.h>

typedef struct file* omni_file_t;

static inline omni_file_t omni_file_open(const char *path, int open_mode, int creat_mode)
{
	return filp_open(path, open_mode, creat_mode);
}

static inline int omni_file_close(omni_file_t fd)
{
	if( NULL == fd )
		return -1;
	return filp_close(fd, NULL);
}

static inline int omni_file_read(omni_file_t fd, void *addr, unsigned long size)
{
	/* this is sys_read  + kernel_read impl */
	mm_segment_t old_fs;
	int result;
	/* get read pos */
	loff_t pos;

	if( NULL == fd )
		return -1;

	pos = fd->f_pos;
	old_fs = get_fs();
	set_fs(get_ds());
	result = vfs_read(fd, (void __user*)addr, size, &pos);
	set_fs(old_fs);

	/* writeback cur pos */
	fd->f_pos = pos;
	return result;
}

static inline int omni_file_write(omni_file_t fd, void *addr, unsigned long size)
{
	/* this is sys_write + kernel_read impl */
	mm_segment_t old_fs;
	int result;
	/* get read pos */
	loff_t pos;

	if( NULL == fd )
		return -1;

	pos = fd->f_pos;
	old_fs = get_fs();
	set_fs(get_ds());
	result = vfs_write(fd, (void __user*)addr, size, &pos);
	set_fs(old_fs);

	/* writeback cur pos */
	fd->f_pos = pos;
	return result;
}

static inline off_t omni_file_lseek(omni_file_t fd, off_t offset, unsigned int origin)
{
	if( NULL == fd )
		return -1;
	return vfs_llseek(fd, offset, origin);
}


/* internal */
static inline void _done_path_create(struct path *path, struct dentry *dentry)
{
	dput(dentry);
	mutex_unlock(&path->dentry->d_inode->i_mutex);
	mnt_drop_write(path->mnt);
	path_put(path);
}

static inline int omni_mkdir(const char *pathname, umode_t mode)
{
	struct dentry *dentry;
	struct path path;
	int error;

	dentry = kern_path_create(AT_FDCWD, pathname, &path, 1);
	if( IS_ERR(dentry) )
	{
		return -1;
	}

	if( !IS_POSIXACL(path.dentry->d_inode) )
	{
		mode &= ~current_umask();
	}

	error = security_path_mkdir(&path, dentry, mode);
	if( !error )
		error = vfs_mkdir(path.dentry->d_inode, dentry, mode);

	_done_path_create(&path, dentry);

	return 0;
}

#endif

