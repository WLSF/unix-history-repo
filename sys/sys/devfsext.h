/* usual BSD style copyright here */
/* Written by Julian Elischer (julian@dialix.oz.au)*/
/*
 * $Id: devfsext.h,v 1.16 1997/09/16 14:23:35 bde Exp $
 */

#ifndef _SYS_DEVFSECT_H_
#define	_SYS_DEVFSECT_H_

/*
 * Make a device at a path, and get a cookie for it in return.
 * Specify the type, the minor number and the devsw entry to use,
 * and the initial default perms/ownerships.
 */
void	*devfs_add_devswf __P((void *devsw, int minor, int chrblk, uid_t uid,
			       gid_t gid, int perms, char *fmt, ...)); 
/*
 * Make a link to a device you already made, and have the cookie for 
 * We get another cookie, but for now, it can be discarded, as
 * at the moment there is nothing you can do with it that you couldn't do
 * with the original cookie. ( XXX this might be something I should change )
 */
void	*devfs_link __P((void *original, char *fmt, ...));

/*
 * Remove all instances of a device you have made. INCLUDING LINKS.
 * I.e. either the cookie from the original device or the cookie
 * from a link will have the effect of removing both entries.
 * Removing with BOTH an original cookie and one from a link is
 * likely to cause a panic.
 */
void	devfs_remove_dev __P((void *devnmp));

/*
 * Check if a device exists and is the type you need. Returns NULL or a
 * cookie that can be used to try 'open' the device. XXX This is a bit
 * of a duplication of devfs_lookup(). I might one day try merge them a bit.
 * Used for mountroot under DEVFS. Path is relative to the base of the devfs.
 */
struct vnode *devfs_open_device __P((char *path, int devtype));
void devfs_close_device __P((struct vnode *vn));

dev_t devfs_vntodev __P((struct vnode *vn)); /* extract dev_t from devfs vn */

#define DV_CHR 0
#define DV_BLK 1
#define DV_DEV 2

/* XXX */
#define	UID_ROOT	0
#define	UID_BIN		3
#define	UID_UUCP	66

/* XXX */
#define	GID_WHEEL	0
#define	GID_KMEM	2
#define	GID_OPERATOR	5
#define	GID_BIN		7
#define	GID_DIALER	68

#endif /* !_SYS_DEVFSECT_H_ */
