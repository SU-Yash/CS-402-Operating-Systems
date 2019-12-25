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

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{        
        KASSERT(NULL != dir);
        KASSERT(NULL != name);
        KASSERT(NULL != result);
        dbg(DBG_PRINT,"(GRADING2A 2.a)\n");
        if(!S_ISDIR(dir->vn_mode)){
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -ENOTDIR;
        }
        if(len > STR_MAX){
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -ENAMETOOLONG;
        }
	
        int res=dir->vn_ops->lookup(dir, name, len, result);
	dbg(DBG_PRINT,"(GRADING2B)\n");
	return res;
        //NOT_YET_IMPLEMENTED("VFS: lookup");
        //return 0;
}
char newpath[MAXPATHLEN];

/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
        KASSERT(NULL != pathname);
        KASSERT(NULL != namelen);
        KASSERT(NULL != name);
        KASSERT(NULL != res_vnode);
        dbg(DBG_PRINT,"(GRADING2A 2.b)\n");
        // Checks for pathname contaiing '/', '..', '.' 
        int pathLen = strlen(pathname); 
        int i=0,j=0,k,retv,slash=0;
        vnode_t *dir_vnode;
	/*if(pathLen > MAXPATHLEN){
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return -ENAMETOOLONG;
        }*/
	
	if(pathname[0]=='/'){
		while(j<pathLen){
		        if(pathname[j]=='/'){
		                if((i>0 && newpath[i-1]!='/' )|| i==0){
		                        newpath[i]=pathname[j];
		                        i++;
					dbg(DBG_PRINT,"(GRADING2B)\n");
		                }
		                j++;
		                /*for(k=j;k<pathLen;k++){
		                        if(pathname[k]=='/'){
						dbg(DBG_PRINT,"(GRADING2B)\n");
		                                continue;
		                        }
		                        else{	
						dbg(DBG_PRINT,"(GRADING2B)\n");
		                                break;
		                        }
		                }*/
		                //j=k;
				dbg(DBG_PRINT,"(GRADING2B)\n");
		        }
		        else{
		                if(pathname[j]=='.'){
		                        j++;
		                        if(j<pathLen){
		                                if(pathname[j]=='.'){
		                                        if(i!=0){
		                                                /*for(k=i-2;k>0;k--){
		                                                        if(newpath[k]=='/'){
										dbg(DBG_PRINT,"(GRADING2B)\n");
		                                                                break;
		                                                        }
									dbg(DBG_PRINT,"(GRADING2B)\n");
		                                                }*/
		                                                //if(k<0){
		                                                        i=1;
									//dbg(DBG_PRINT,"(GRADING2B)\n");
		                                                //}
		                                                /*else{
		                                                        i=k+1;
									dbg(DBG_PRINT,"(GRADING2B)\n");
		                                                }*/
								dbg(DBG_PRINT,"(GRADING2B)\n");
		                                        }
		                                        j=j+2;
							dbg(DBG_PRINT,"(GRADING2B)\n");
		                                }
		                                /*else{
		                                        j++;
							dbg(DBG_PRINT,"(GRADING2B)\n");
		                                }*/
						dbg(DBG_PRINT,"(GRADING2B)\n");
		                        }
					dbg(DBG_PRINT,"(GRADING2B)\n");
		                }
		                else{
		                        newpath[i]=pathname[j];
		                        i++;
		                        j++;
					dbg(DBG_PRINT,"(GRADING2B)\n");
		                }
				dbg(DBG_PRINT,"(GRADING2B)\n");
		        }
			dbg(DBG_PRINT,"(GRADING2B)\n");
		}
		dbg(DBG_PRINT,"(GRADING2B)\n");
	}
	else{
		while(j<pathLen){
		        if(pathname[j]=='/'){
		                if((i>0 && newpath[i-1]!='/' )|| i==0){
		                        newpath[i]=pathname[j];
		                        i++;
					dbg(DBG_PRINT,"(GRADING2B)\n");
		                }
		                j++;
		                for(k=j;k<pathLen;k++){
		                        if(pathname[k]=='/'){
						dbg(DBG_PRINT,"(GRADING2B)\n");
		                                continue;
		                        }
		                        else{
						dbg(DBG_PRINT,"(GRADING2B)\n");
		                                break;
		                        }
		                }
		                j=k;
				dbg(DBG_PRINT,"(GRADING2B)\n");
		        }
		        else{
	                        newpath[i]=pathname[j];
	                        i++;
	                        j++;
				dbg(DBG_PRINT,"(GRADING2B)\n");
		        }
			dbg(DBG_PRINT,"(GRADING2B)\n");
		}
		dbg(DBG_PRINT,"(GRADING2B)\n");
	}

	if(i>1 && newpath[i-1]=='/'){
		slash=1;
                newpath[i-1]='\0';
		dbg(DBG_PRINT,"(GRADING2B)\n");
        }
        else{
                newpath[i]='\0';
		dbg(DBG_PRINT,"(GRADING2B)\n");
        }
        pathLen = strlen(newpath);
        if(newpath[0] == '/'){
                i = 0;
                dir_vnode = vfs_root_vn;
		dbg(DBG_PRINT,"(GRADING2B)\n");
        }
        else{
                i=-1;
                //if(base == NULL){
                        dir_vnode = curproc->p_cwd;
			//dbg(DBG_PRINT,"(GRADING2B)\n");
                //}
                /*else{
                        dir_vnode=base;
			dbg(DBG_PRINT,"(GRADING2B)\n");
                }*/
		dbg(DBG_PRINT,"(GRADING2B)\n");
        }
        KASSERT(NULL != dir_vnode);
        dbg(DBG_PRINT,"(GRADING2A 2.b)\n");
	k=0;
        while(1){
                for(j = i+1; j < pathLen; j++){
                        if(newpath[j] == '/'){
				dbg(DBG_PRINT,"(GRADING2B)\n");
                                break;
                        } 
			dbg(DBG_PRINT,"(GRADING2B)\n");                      
                }
                newpath[j]='\0';
                pathname=newpath;
                *name=&pathname[i+1];
                *namelen = j-i-1;
                //dbg(DBG_PRINT,"pathName: %s pathLen:%d i:%d j:%d newpath:%s\n",*name, *namelen, i, j, newpath);
                i=j;
		if(*namelen==0){
			if(k==0){
				vget(dir_vnode->vn_fs,dir_vnode->vn_vno);
				dbg(DBG_PRINT,"(GRADING2B)\n");
			}
			*res_vnode=dir_vnode;
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return 0;
		}
                retv= lookup(dir_vnode, *name, *namelen, res_vnode);  
                //lookup is incrementing other path res, it should decrement???
                if(j == pathLen){
			if(retv==0 && !S_ISDIR((*res_vnode)->vn_mode) && slash){
				//vput(dir_vnode);
				//vput(*res_vnode);
				if(retv==0){
					vput(*res_vnode);
					dbg(DBG_PRINT,"(GRADING2B)\n");
				}                                         
				if(k!=0){
					vput(dir_vnode);
					dbg(DBG_PRINT,"(GRADING2B)\n");
				}
				dbg(DBG_PRINT,"(GRADING2B)\n");
				return -ENOTDIR;
			}
                        
			if(retv==-ENOTDIR){
				//vget(dir_vnode->vn_fs,dir_vnode->vn_vno);
				*res_vnode=dir_vnode;	
				if(k!=0){
					vput(dir_vnode);
					dbg(DBG_PRINT,"(GRADING2B)\n");
				}		
				dbg(DBG_PRINT,"(GRADING2B)\n");	
				return retv;
			}
			else{
				if(retv==0){
					vput(*res_vnode);
					dbg(DBG_PRINT,"(GRADING2B)\n");
				}
				if(k==0){
					vget(dir_vnode->vn_fs,dir_vnode->vn_vno);
					dbg(DBG_PRINT,"(GRADING2B)\n");	
				}
				*res_vnode=dir_vnode;
				dbg(DBG_PRINT,"(GRADING2B)\n");
				return 0;
			}
			//dbg(DBG_PRINT,"(GRADING2B)\n");
                }
                if(retv!=0){
                        //if(j != pathLen && retv == -ENOENT){
                                //return -ENOTDIR;
                        //}
                        //else{
                                //return retv;
                        //}
			//vput(dir_vnode);
			/*if(k!=0){
				vput(dir_vnode);
				dbg(DBG_PRINT,"(GRADING2B)\n");
			}*/
			dbg(DBG_PRINT,"(GRADING2B)\n");
			return retv;
                        
                } 
                //if(j == pathLen){
                        //return 0;
                //}
                else{
			if(k==0) {
				k=1;
				dbg(DBG_PRINT,"(GRADING2B)\n");
			}else{
				vput(dir_vnode);
				dbg(DBG_PRINT,"(GRADING2B)\n");
			}
                        dir_vnode = *res_vnode;
			dbg(DBG_PRINT,"(GRADING2B)\n");
                }
		dbg(DBG_PRINT,"(GRADING2B)\n");       
        }
        //NOT_YET_IMPLEMENTED("VFS: dir_namev");
	//dbg(DBG_PRINT,"(GRADING2B)\n");
        return 0; //TODO not required?
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified and the file does
 * not exist, call create() in the parent directory vnode. However, if the
 * parent directory itself does not exist, this function should fail - in all
 * cases, no files or directories other than the one at the very end of the path
 * should be created.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
        vnode_t *dir_vnode;
        size_t namelen;
        const char *name;
        int dret, lret;
        
        dret = dir_namev(pathname, &namelen, &name, base, &dir_vnode);
	*res_vnode=dir_vnode;
        if(dret == 0 ){
		if(namelen==0){
			*res_vnode=dir_vnode;
			return 0;
		}
		if(dir_vnode->vn_ops->create==NULL){
			vput(dir_vnode);
			return -EISDIR;
		}
		//if(namelen>0){
		        lret = lookup(dir_vnode, name, namelen, res_vnode);
		        if(lret == -ENOENT){
		                if(flag & O_CREAT){
					KASSERT(NULL != dir_vnode->vn_ops->create);
		                        dbg(DBG_PRINT,"(GRADING2A 2.c)\n");
					int res_create = dir_vnode->vn_ops->create(dir_vnode, name, namelen, res_vnode);
					dbg(DBG_PRINT,"(GRADING2B)\n");
					vput(dir_vnode);
					dbg(DBG_PRINT,"(GRADING2B)\n");
		                        return res_create;        
		                }
				vput(dir_vnode);
				dbg(DBG_PRINT,"(GRADING2B)\n");
		                return lret;
		        }
		        else{
				vput(dir_vnode);
				dbg(DBG_PRINT,"(GRADING2B)\n");
		                return lret;
		        }
		//}
		//vput(dir_vnode);
		//return 0;
		//dbg(DBG_PRINT,"(GRADING2B)\n");
        }
        else
        {
		dbg(DBG_PRINT,"(GRADING2B)\n");
                return dret;
        }
        //NOT_YET_IMPLEMENTED("VFS: open_namev");
	//dbg(DBG_PRINT,"(GRADING2B)\n");
        return 0;
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */
