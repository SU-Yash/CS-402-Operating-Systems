/******************************************************************************/
/* Important Fall 2019 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

/*
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.2 2018/05/27 03:57:26 cvsps Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/*
 * Syscalls for vfs. Refer to comments or man pages for implementation.
 * Do note that you don't need to set errno, you should just return the
 * negative error code.
 */

/* To read a file:
 *      o fget(fd)
 *      o call its virtual read vn_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
        //NOT_YET_IMPLEMENTED("VFS: do_read");
        if(fd < 0 || fd >= NFILES)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        file_t *file_obj;
        file_obj = fget(fd);
	if(file_obj==NULL){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -EBADF;
	}
	if(!(file_obj->f_mode & FMODE_READ)){
		fput(file_obj);
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -EBADF;
	}
        if(S_ISDIR(file_obj->f_vnode->vn_mode) || file_obj->f_vnode->vn_ops->read==NULL)
        {
		fput(file_obj);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EISDIR;
        }
        int res = file_obj->f_vnode->vn_ops->read(file_obj->f_vnode, file_obj->f_pos, buf, nbytes);
	if((size_t)res==nbytes){
        	file_obj->f_pos = (off_t) nbytes;
		dbg(DBG_PRINT,"(GRADING2B)\n");
	}
	else{
		file_obj->f_pos+=res;
		dbg(DBG_PRINT,"(GRADING2B)\n");
	}
        fput(file_obj);
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return res;
}

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * vn_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
        //NOT_YET_IMPLEMENTED("VFS: do_write");
        file_t *f;
	if(fd<0 || fd>=NFILES){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -EBADF;
	}
        f = fget(fd);
	if(f==NULL){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -EBADF;
	}
	if(S_ISDIR(f->f_vnode->vn_mode) || f->f_vnode->vn_ops->write==NULL)
        {
		fput(f);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EISDIR;
        }
	off_t new_pos=f->f_pos;
        if(f->f_mode & FMODE_WRITE)
        {
                if(f->f_mode & FMODE_APPEND)
                {
                        new_pos = do_lseek(fd, (off_t) 0, SEEK_END);
			dbg(DBG_PRINT,"(GRADING2B)\n");
                        
                }
		int res = f->f_vnode->vn_ops->write(f->f_vnode, new_pos, buf, nbytes);
		f->f_pos=new_pos+res;
                KASSERT((S_ISCHR(f->f_vnode->vn_mode)) ||
                        (S_ISBLK(f->f_vnode->vn_mode)) ||
                        ((S_ISREG(f->f_vnode->vn_mode)) && (f->f_pos <= f->f_vnode->vn_len)));
                dbg(DBG_PRINT,"(GRADING2A 3.a)\n");
                fput(f);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return res;
        }
        fput(f);
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return -EBADF;
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int
do_close(int fd)
{
        //NOT_YET_IMPLEMENTED("VFS: do_close");
        if(fd < 0 || fd >= NFILES)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        if(curproc->p_files[fd]==NULL){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -EBADF;
	}
        fput(curproc->p_files[fd]);
	curproc->p_files[fd]=NULL;
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return 0;
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
        //NOT_YET_IMPLEMENTED("VFS: do_dup");
        if(fd < 0 || fd >= NFILES)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        file_t *dup_file = fget(fd);
        if(dup_file == NULL)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        int nfd = get_empty_fd(curproc);
        /*if(nfd == -EMFILE)
        {
                fput(dup_file);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EMFILE;
        }*/
        curproc->p_files[nfd] = dup_file;
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return nfd;
}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
        //NOT_YET_IMPLEMENTED("VFS: do_dup2");
        if(ofd < 0 || ofd > NFILES)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        if(nfd < 0 || nfd > NFILES)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        file_t *dup_file = fget(ofd);
        if(dup_file == NULL)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        /*if(curproc->p_files[nfd] != NULL && nfd != ofd)
        {
                do_close(nfd);
		dbg(DBG_PRINT,"(GRADING2B)\n");
        }*/
        if(nfd == ofd)
        {
                fput(dup_file);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return nfd;
        }
        curproc->p_files[nfd] = dup_file;
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return nfd;
}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mknod(const char *path, int mode, unsigned devid)
{
        //NOT_YET_IMPLEMENTED("VFS: do_mknod");
        //TODO: Ask Professor about EEXIST for do_mknod
        /*if(mode != S_IFCHR && mode != S_IFBLK)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EINVAL;
        }*/
        size_t namelen;
        const char *name;
        vnode_t *dir_vnode;
        vnode_t *res_vnode_lookup;
        int res_dir = dir_namev(path, &namelen, &name, NULL, &dir_vnode);
        //if(namelen > (size_t)STR_MAX)
        //{
                //return -ENAMETOOLONG;
        //}
	/*if(res_dir!=0){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return res_dir;
	}*/
        int res = lookup(dir_vnode, name, namelen, &res_vnode_lookup);
        if(res == -ENOTDIR || dir_vnode->vn_ops->mknod==NULL || !S_ISDIR(dir_vnode->vn_mode))
        {
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return res;
        }
        //if(res==-ENOENT){
		KASSERT(NULL != dir_vnode->vn_ops->mknod);
                dbg(DBG_PRINT,"(GRADING2A 3.b)\n");
		int res_mknod= dir_vnode->vn_ops->mknod(dir_vnode, name, namelen, mode, devid);
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return res_mknod;

       // }
	//vput(res_vnode_lookup);
	//vput(dir_vnode);
	//dbg(DBG_PRINT,"(GRADING2B)\n");
        //return -EEXIST;
}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_mkdir(const char *path)
{
        //NOT_YET_IMPLEMENTED("VFS: do_mkdir");
        size_t namelen;
        const char *name;
        vnode_t *dir_vnode;
        vnode_t *res_vnode_lookup;
        int res_dir = dir_namev(path, &namelen, &name, NULL, &dir_vnode);
	if(res_dir!=0){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return res_dir;
	}
	if(namelen==0){
		vput(dir_vnode);
		return 0;
	}
	if(dir_vnode->vn_ops->mkdir==NULL){
		vput(dir_vnode);
		return -ENOTDIR;
	}
	if(namelen>(size_t)STR_MAX){
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -ENAMETOOLONG;
	}
        int res = lookup(dir_vnode, name, namelen, &res_vnode_lookup);
        if(res == -ENOENT)
        {	
		KASSERT(NULL != dir_vnode->vn_ops->mkdir);
                dbg(DBG_PRINT,"(GRADING2A 3.c)\n");
		int res_mkdir = dir_vnode->vn_ops->mkdir(dir_vnode, name, namelen);
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return res_mkdir;
        }
        /*else if(res == -ENOTDIR)
        {
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return res;
        }*/
	vput(res_vnode_lookup);
	vput(dir_vnode);
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return -EEXIST;
}

/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_rmdir(const char *path)
{
        size_t namelen;
        int pathLen = strlen(path);
        const char *name;
        vnode_t *dir_vnode, *res_vnode;
        /*
        if(strlen(path) > (size_t)MAXPATHLEN)
        {
                return -ENAMETOOLONG;
        }*/

        int ret_dir_namev = dir_namev(path, &namelen, &name, NULL, &dir_vnode);
	if(ret_dir_namev!=0){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return ret_dir_namev;
	}
	if(dir_vnode->vn_ops->rmdir==NULL){
		vput(dir_vnode);
		return -ENOTDIR;
	}
	if(namelen == 1 && name[0] == '.'){
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EINVAL;
        }
	if(namelen==2 && name[0]=='.' && name[1]=='.'){
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -ENOTEMPTY;
	}
	int res_lookup=lookup(dir_vnode,name,namelen,&res_vnode);
	if(res_lookup==0){
		KASSERT(NULL != dir_vnode->vn_ops->rmdir);
		dbg(DBG_PRINT,"(GRADING2A 3.d)\n");
		int ret_rmdir = dir_vnode->vn_ops->rmdir(dir_vnode, name, namelen);
		dbg(DBG_PRINT,"(GRADING2B)\n"); 
		vput(dir_vnode);
		vput(res_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return ret_rmdir;
	} 
	vput(dir_vnode);
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return res_lookup;
        //NOT_YET_IMPLEMENTED("VFS: do_rmdir");
        //return -1;
}

/*
 * Similar to do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EPERM
 *        path refers to a directory.
 *      o ENOENT
 *        Any component in path does not exist, including the element at the
 *        very end.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int
do_unlink(const char *path)
{
        size_t namelen;
        int pathLen = strlen(path);
        const char *name;
        vnode_t *dir_vnode, *res_vnode_lookup; 
        int ret_dir_namev = dir_namev(path, &namelen, &name, NULL, &dir_vnode);
        if(ret_dir_namev == 0){
		if(dir_vnode->vn_ops->unlink==NULL){
			vput(dir_vnode);
			return -ENOTDIR;
		}
                int ret_lookup = lookup(dir_vnode, name, namelen, &res_vnode_lookup);
                if(ret_lookup == 0){
		        if(S_ISDIR(res_vnode_lookup->vn_mode)){
				vput(res_vnode_lookup);
				vput(dir_vnode);
				dbg(DBG_PRINT,"(GRADING2B)\n");
		                return -EPERM;
		        }
			KASSERT(NULL != dir_vnode->vn_ops->unlink);
			dbg(DBG_PRINT,"(GRADING2A 3.e)\n");
			int ret_rmdir = dir_vnode->vn_ops->unlink(dir_vnode, name, namelen);
			dbg(DBG_PRINT,"(GRADING2B)\n");
		        // TODO SUYASH maybe call put 
			vput(dir_vnode);
			vput(res_vnode_lookup);
			dbg(DBG_PRINT,"(GRADING2B)\n");
		        return ret_rmdir;
                }
                else{
			vput(dir_vnode);
			dbg(DBG_PRINT,"(GRADING2B)\n");
                        return ret_lookup;
                }
		//dbg(DBG_PRINT,"(GRADING2B)\n");
        }
        else {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return ret_dir_namev;
        }
	//dbg(DBG_PRINT,"(GRADING2B)\n");
        //NOT_YET_IMPLEMENTED("VFS: do_unlink");
        //return -1;
}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EPERM
 *        from is a directory.
 */
int
do_link(const char *from, const char *to)
{
        //NOT_YET_IMPLEMENTED("VFS: do_link");
	//TODO POOJA VPUT the nodes returned from open,dirnamev
	vnode_t *open_vnode, *dir_vnode, *res_vnode ;
	size_t namelen;
        const char *name;
	int retopen= open_namev(from,0,&open_vnode,NULL);
	if(retopen!=0){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return retopen;
	}
	else{
		/*if(S_ISDIR(open_vnode->vn_mode)){
			vput(open_vnode);
			dbg(DBG_PRINT,"(GRADING2B)\n");
                	return -EPERM;
        	}*/
		int retdir=dir_namev(to,&namelen,&name, NULL, &dir_vnode);
		/*if(retdir==0){
			int res_lookup=lookup(dir_vnode,name,namelen,&res_vnode);
			if(res_lookup==0){
				vput(dir_vnode);
				vput(open_vnode);
				vput(res_vnode);
				dbg(DBG_PRINT,"(GRADING2B)\n");
				return -EEXIST;	
			}
			if(res_lookup==-ENOTDIR){
				vput(dir_vnode);
				vput(open_vnode);
				dbg(DBG_PRINT,"(GRADING2B)\n");
				return res_lookup;
			}
			int ret= dir_vnode->vn_ops->link(open_vnode, dir_vnode, name, namelen);
			vput(dir_vnode);
			vput(open_vnode);
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return ret;
		}
		else{*/
			vput(open_vnode);
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return retdir;
		//}
		//dbg(DBG_PRINT,"(GRADING2B)\n");
	}
	//dbg(DBG_PRINT,"(GRADING2B)\n");
}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int
do_rename(const char *oldname, const char *newname)
{
        //NOT_YET_IMPLEMENTED("VFS: do_rename");
	int ret=do_link(oldname,newname);
	/*if(ret==0){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return do_unlink(oldname);
	}
	else{*/
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return ret;
	//}
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int
do_chdir(const char *path)
{
        //NOT_YET_IMPLEMENTED("VFS: do_chdir");
        size_t namelen;
        const char *name;
        vnode_t *dir_vnode;
        int res_dir = open_namev(path, 0, &dir_vnode, NULL);
        if(res_dir==0){
		if(!S_ISDIR(dir_vnode->vn_mode)){
			vput(dir_vnode);
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return -ENOTDIR;
		}
		//old=curproc->p_cwd;
		vput(curproc->p_cwd);
                curproc->p_cwd=dir_vnode;
		//dbg(DBG_PRINT,"do_chdir vget 1\n");
		dbg(DBG_PRINT,"(GRADING2B)\n");	
        }
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return res_dir;
}

/* Call the readdir vn_op on the given fd, filling in the given dirent_t*.
 * If the readdir vn_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
        //NOT_YET_IMPLEMENTED("VFS: do_getdent");
        if(fd < 0 || fd >= NFILES)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        file_t *file;
        file = fget(fd);
        if(file == NULL)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        if(!S_ISDIR(file->f_vnode->vn_mode))
        {
                fput(file);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -ENOTDIR;
        }
        if(file->f_vnode->vn_ops->readdir == NULL)
        {
                fput(file);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -ENOTDIR;
        }
        int bytesCount = file->f_vnode->vn_ops->readdir(file->f_vnode, file->f_pos, dirp);
        if(bytesCount == 0)
        {
		fput(file);
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return 0;
        }
        file->f_pos+=bytesCount;
	fput(file);
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return sizeof(*dirp);
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
        //NOT_YET_IMPLEMENTED("VFS: do_lseek");
        if(fd < 0 || fd >= NFILES)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EBADF;
        }
        if(whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END)
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EINVAL;
        }
        file_t* file_obj;
        file_obj = fget(fd);
	if(file_obj==NULL){
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return -EBADF;
	}
	
        switch (whence)
        {
        case SEEK_SET:
		if(offset<0){
			fput(file_obj);
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return -EINVAL;
		}
                file_obj->f_pos = offset;
		dbg(DBG_PRINT,"(GRADING2B)\n");
                break;

        case SEEK_CUR:
		if(file_obj->f_pos+offset<0){
			fput(file_obj);
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return -EINVAL;
		}
                file_obj->f_pos += offset;
		dbg(DBG_PRINT,"(GRADING2B)\n");
                break;
                
        case SEEK_END:
		if(offset<0 && -offset> file_obj->f_vnode->vn_len){
			fput(file_obj);
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return -EINVAL;
		}
                file_obj->f_pos = file_obj->f_vnode->vn_len + offset;
		dbg(DBG_PRINT,"(GRADING2B)\n");
                break;
        }
	int res=file_obj->f_pos;
	fput(file_obj);
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return res;
}

/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o EINVAL
 *        path is an empty string.
 */
int
do_stat(const char *path, struct stat *buf)
{
        //NOT_YET_IMPLEMENTED("VFS: do_stat");
        size_t namelen;
        const char *name;
        vnode_t *dir_vnode;
        if(strlen(path)==0){
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -EINVAL;
        }
        int res_dir = open_namev(path, 0, &dir_vnode, NULL);
        if(res_dir==0){
		if(dir_vnode->vn_ops->stat==NULL){
			vput(dir_vnode);
			return -ENOTDIR;
		}
		KASSERT(NULL != dir_vnode->vn_ops->stat);
		dbg(DBG_PRINT,"(GRADING2A 3.f)\n");
		int ret = dir_vnode->vn_ops->stat(dir_vnode, buf);
		dbg(DBG_PRINT,"(GRADING2B)\n");
		vput(dir_vnode);
		dbg(DBG_PRINT,"(GRADING2B)\n");
		return ret;       
        }
	dbg(DBG_PRINT,"(GRADING2B)\n");
        return res_dir;
}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
        return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
        NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
        return -EINVAL;
}
#endif
