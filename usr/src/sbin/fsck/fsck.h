/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)fsck.h	5.9 (Berkeley) %G%
 */

#define	MAXDUP		10	 /* limit on dup blks (per inode) */
#define	MAXBAD		10	 /* limit on bad blks (per inode) */
#define MAXBUFSPACE	128*1024 /* maximum space to allocate to buffers */

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

#define	USTATE	01		/* inode not allocated */
#define	FSTATE	02		/* inode is file */
#define	DSTATE	03		/* inode is directory */
#define	DFOUND	04		/* directory found during descent */
#define	DCLEAR	05		/* directory is to be cleared */
#define	FCLEAR	06		/* file is to be cleared */

/*
 * buffer cache structure.
 */
struct bufarea {
	struct bufarea	*b_next;		/* free list queue */
	struct bufarea	*b_prev;		/* free list queue */
	daddr_t	b_bno;
	int	b_size;
	int	b_errs;
	int	b_flags;
	union {
		char	*b_buf;			/* buffer space */
		daddr_t	*b_indir;		/* indirect block */
		struct	fs *b_fs;		/* super block */
		struct	cg *b_cg;		/* cylinder group */
		struct	dinode *b_dinode;	/* inode block */
	} b_un;
	char	b_dirty;
};

#define	B_INUSE 1

#define	MINBUFS		5	/* minimum number of buffers required */
struct bufarea bufhead;		/* head of list of other blks in filesys */
struct bufarea sblk;		/* file system superblock */
struct bufarea cgblk;		/* cylinder group blocks */
struct bufarea *getdatablk();

#define	dirty(bp)	(bp)->b_dirty = 1
#define	initbarea(bp) \
	(bp)->b_dirty = 0; \
	(bp)->b_bno = (daddr_t)-1; \
	(bp)->b_flags = 0;

#define	sbdirty()	sblk.b_dirty = 1
#define	cgdirty()	cgblk.b_dirty = 1
#define	sblock		(*sblk.b_un.b_fs)
#define	cgrp		(*cgblk.b_un.b_cg)

enum fixstate {DONTKNOW, NOFIX, FIX};

struct inodesc {
	enum fixstate id_fix;	/* policy on fixing errors */
	int (*id_func)();	/* function to be applied to blocks of inode */
	ino_t id_number;	/* inode number described */
	ino_t id_parent;	/* for DATA nodes, their parent */
	daddr_t id_blkno;	/* current block number being examined */
	int id_numfrags;	/* number of frags contained in block */
	long id_filesize;	/* for DATA nodes, the size of the directory */
	int id_loc;		/* for DATA nodes, current location in dir */
	int id_entryno;		/* for DATA nodes, current entry number */
	struct direct *id_dirp;	/* for DATA nodes, ptr to current entry */
	char *id_name;		/* for DATA nodes, name to find or enter */
	char id_type;		/* type of descriptor, DATA or ADDR */
};
/* file types */
#define	DATA	1
#define	ADDR	2

/*
 * Linked list of duplicate blocks.
 * 
 * The list is composed of two parts. The first part of the
 * list (from duplist through the node pointed to by muldup)
 * contains a single copy of each duplicate block that has been 
 * found. The second part of the list (from muldup to the end)
 * contains duplicate blocks that have been found more than once.
 * To check if a block has been found as a duplicate it is only
 * necessary to search from duplist through muldup. To find the 
 * total number of times that a block has been found as a duplicate
 * the entire list must be searched for occurences of the block
 * in question. The following diagram shows a sample list where
 * w (found twice), x (found once), y (found three times), and z
 * (found once) are duplicate block numbers:
 *
 *    w -> y -> x -> z -> y -> w -> y
 *    ^		     ^
 *    |		     |
 * duplist	  muldup
 */
struct dups {
	struct dups *next;
	daddr_t dup;
};
struct dups *duplist;		/* head of dup list */
struct dups *muldup;		/* end of unique duplicate dup block numbers */

/*
 * Linked list of inodes with zero link counts.
 */
struct zlncnt {
	struct zlncnt *next;
	ino_t zlncnt;
};
struct zlncnt *zlnhead;		/* head of zero link count list */

char	*devname;		/* name of device being checked */
long	dev_bsize;		/* computed value of DEV_BSIZE */
long	secsize;		/* actual disk sector size */
char	nflag;			/* assume a no response */
char	yflag;			/* assume a yes response */
int	bflag;			/* location of alternate super block */
int	debug;			/* output debugging info */
int	cvtflag;		/* convert to old file system format */
char	preen;			/* just fix normal inconsistencies */
char	hotroot;		/* checking root device */
char	havesb;			/* superblock has been read */
int	fsmodified;		/* 1 => write done to file system */
int	fsreadfd;		/* file descriptor for reading file system */
int	fswritefd;		/* file descriptor for writing file system */

daddr_t	maxfsblock;		/* number of blocks in the file system */
char	*blockmap;		/* ptr to primary blk allocation map */
ino_t	maxino;			/* number of inodes in file system */
ino_t	lastino;		/* last inode in use */
char	*statemap;		/* ptr to inode state table */
short	*lncntp;		/* ptr to link count table */

char	pathname[BUFSIZ];	/* current pathname */
char	*pathp;			/* ptr to current position in pathname */
char	*endpathname;		/* ptr to current end of pathname */

ino_t	lfdir;			/* lost & found directory inode number */
char	*lfname;		/* lost & found directory name */
int	lfmode;			/* lost & found directory creation mode */

daddr_t	n_blks;			/* number of blocks in use */
daddr_t	n_files;		/* number of files in use */

#define	clearinode(dp)	(*(dp) = zino)
struct	dinode zino;

#define	setbmap(blkno)	setbit(blockmap, blkno)
#define	testbmap(blkno)	isset(blockmap, blkno)
#define	clrbmap(blkno)	clrbit(blockmap, blkno)

#define	STOP	0x01
#define	SKIP	0x02
#define	KEEPON	0x04
#define	ALTERED	0x08
#define	FOUND	0x10

time_t time();
struct dinode *ginode();
struct bufarea *getblk();
ino_t allocino();
int findino();
