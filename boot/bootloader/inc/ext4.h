/**
 * This immplementation of EXT4 is
 * not meant to be complete.
 * It is just good enough
 * to load the kernel, and then
 * the kernel will load a better version
 * of this driver.
 * Yes, there will be two versions of this driver...
 */

#ifndef EXT4_H
#define EXT4_H

#include <efi.h>
#include <efilib.h>

// --------------------
// Magnificent Macros
// --------------------
#define EXT4_LABEL_MAX 16

// Constants relative to the data blocks
#define EXT4_NDIR_BLOCKS 12
#define EXT4_IND_BLOCK EXT4_NDIR_BLOCKS
#define EXT4_DIND_BLOCK (EXT4_IND_BLOCK + 1)
#define EXT4_TIND_BLOCK (EXT4_DIND_BLOCK + 1)
#define EXT4_N_BLOCKS (EXT4_TIND_BLOCK + 1)

// --------------------
// Terrific Typedefs
// --------------------

// EXT4 inode
typedef struct
{
    UINT16 i_mode;        /* File mode */
    UINT16 i_uid;         /* Low 16 bits of Owner Uid */
    UINT32 i_size_lo;     /* Size in bytes */
    UINT32 i_atime;       /* Access time */
    UINT32 i_ctime;       /* Inode Change time */
    UINT32 i_mtime;       /* Modification time */
    UINT32 i_dtime;       /* Deletion Time */
    UINT16 i_gid;         /* Low 16 bits of Group Id */
    UINT16 i_links_count; /* Links count */
    UINT32 i_blocks_lo;   /* Blocks count */
    UINT32 i_flags;       /* File flags */
    union
    {
        struct
        {
            UINT32 l_i_version;
        } linux1;
        struct
        {
            UINT32 h_i_translator;
        } hurd1;
        struct
        {
            UINT32 m_i_reserved1;
        } masix1;
    } osd1;                        /* OS dependent 1 */
    UINT32 i_block[EXT4_N_BLOCKS]; /* Pointers to blocks */
    UINT32 i_generation;           /* File version (for NFS) */
    UINT32 i_file_acl_lo;          /* File ACL */
    UINT32 i_size_high;
    UINT32 i_obso_faddr; /* Obsoleted fragment address */
    union
    {
        struct
        {
            UINT16 l_i_blocks_high; /* were l_i_reserved1 */
            UINT16 l_i_file_acl_high;
            UINT16 l_i_uid_high;    /* these 2 fields */
            UINT16 l_i_gid_high;    /* were reserved2[0] */
            UINT16 l_i_checksum_lo; /* crc32c(uuid+inum+inode) LE */
            UINT16 l_i_reserved;
        } linux2;
        struct
        {
            UINT16 h_i_reserved1; /* Obsoleted fragment number/size which are removed in ext4 */
            UINT16 h_i_mode_high;
            UINT16 h_i_uid_high;
            UINT16 h_i_gid_high;
            UINT32 h_i_author;
        } hurd2;
        struct
        {
            UINT16 h_i_reserved1; /* Obsoleted fragment number/size which are removed in ext4 */
            UINT16 m_i_file_acl_high;
            UINT32 m_i_reserved2[2];
        } masix2;
    } osd2; /* OS dependent 2 */
    UINT16 i_extra_isize;
    UINT16 i_checksum_hi;  /* crc32c(uuid+inum+inode) BE */
    UINT32 i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
    UINT32 i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
    UINT32 i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
    UINT32 i_crtime;       /* File Creation time */
    UINT32 i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
    UINT32 i_version_hi;   /* high 32 bits for 64-bit version */
    UINT32 i_projid;       /* Project ID */
} __attribute__((packed)) ext4_inode_t;

// EXT4 super block
typedef struct
{
    UINT32 s_inodes_count;         /* Inodes count */
    UINT32 s_blocks_count_lo;      /* Blocks count */
    UINT32 s_r_blocks_count_lo;    /* Reserved blocks count */
    UINT32 s_free_blocks_count_lo; /* Free blocks count */
    UINT32 s_free_inodes_count;    /* Free inodes count */
    UINT32 s_first_data_block;     /* First Data Block */
    UINT32 s_log_block_size;       /* Block size */
    UINT32 s_log_cluster_size;     /* Allocation cluster size */
    UINT32 s_blocks_per_group;     /* # Blocks per group */
    UINT32 s_clusters_per_group;   /* # Clusters per group */
    UINT32 s_inodes_per_group;     /* # Inodes per group */
    UINT32 s_mtime;                /* Mount time */
    UINT32 s_wtime;                /* Write time */
    UINT16 s_mnt_count;            /* Mount count */
    UINT16 s_max_mnt_count;        /* Maximal mount count */
    UINT16 s_magic;                /* Magic signature */
    UINT16 s_state;                /* File system state */
    UINT16 s_errors;               /* Behaviour when detecting errors */
    UINT16 s_minor_rev_level;      /* minor revision level */
    UINT32 s_lastcheck;            /* time of last check */
    UINT32 s_checkinterval;        /* max. time between checks */
    UINT32 s_creator_os;           /* OS */
    UINT32 s_rev_level;            /* Revision level */
    UINT16 s_def_resuid;           /* Default uid for reserved blocks */
    UINT16 s_def_resgid;           /* Default gid for reserved blocks */
    /*
     * These fields are for EXT4_DYNAMIC_REV superblocks only.
     *
     * Note: the difference between the compatible feature set and
     * the incompatible feature set is that if there is a bit set
     * in the incompatible feature set that the kernel doesn't
     * know about, it should refuse to mount the filesystem.
     *
     * e2fsck's requirements are more strict; if it doesn't know
     * about a feature in either the compatible or incompatible
     * feature set, it must abort and not try to meddle with
     * things it doesn't understand...
     */
    UINT32 s_first_ino;                  /* First non-reserved inode */
    UINT16 s_inode_size;                 /* size of inode structure */
    UINT16 s_block_group_nr;             /* block group # of this superblock */
    UINT32 s_feature_compat;             /* compatible feature set */
    UINT32 s_feature_incompat;           /* incompatible feature set */
    UINT32 s_feature_ro_compat;          /* readonly-compatible feature set */
    UINT8 s_uuid[16];                    /* 128-bit uuid for volume */
    CHAR8 s_volume_name[EXT4_LABEL_MAX]; /* volume name */
    CHAR8 s_last_mounted[64];            /* directory where last mounted */
    UINT32 s_algorithm_usage_bitmap;     /* For compression */
    /*
     * Performance hints.  Directory preallocation should only
     * happen if the EXT4_FEATURE_COMPAT_DIR_PREALLOC flag is on.
     */
    UINT8 s_prealloc_blocks;      /* Nr of blocks to try to preallocate*/
    UINT8 s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
    UINT16 s_reserved_gdt_blocks; /* Per group desc for online growth */
                                  /*
                                   * Journaling support valid if EXT4_FEATURE_COMPAT_HAS_JOURNAL set.
                                   */
    UINT8 s_journal_uuid[16];     /* uuid of journal superblock */
    UINT32 s_journal_inum;        /* inode number of journal file */
    UINT32 s_journal_dev;         /* device number of journal file */
    UINT32 s_last_orphan;         /* start of list of inodes to delete */
    UINT32 s_hash_seed[4];        /* HTREE hash seed */
    UINT8 s_def_hash_version;     /* Default hash version to use */
    UINT8 s_jnl_backup_type;
    UINT16 s_desc_size; /* size of group descriptor */
    UINT32 s_default_mount_opts;
    UINT32 s_first_meta_bg;           /* First metablock block group */
    UINT32 s_mkfs_time;               /* When the filesystem was created */
    UINT32 s_jnl_blocks[17];          /* Backup of the journal inode */
                                      /* 64bit support valid if EXT4_FEATURE_INCOMPAT_64BIT */
    UINT32 s_blocks_count_hi;         /* Blocks count */
    UINT32 s_r_blocks_count_hi;       /* Reserved blocks count */
    UINT32 s_free_blocks_count_hi;    /* Free blocks count */
    UINT16 s_min_extra_isize;         /* All inodes have at least # bytes */
    UINT16 s_want_extra_isize;        /* New inodes should reserve # bytes */
    UINT32 s_flags;                   /* Miscellaneous flags */
    UINT16 s_raid_stride;             /* RAID stride */
    UINT16 s_mmp_update_interval;     /* # seconds to wait in MMP checking */
    UINT64 s_mmp_block;               /* Block for multi-mount protection */
    UINT32 s_raid_stripe_width;       /* blocks on all data disks (N*stride)*/
    UINT8 s_log_groups_per_flex;      /* FLEX_BG group size */
    UINT8 s_checksum_type;            /* metadata checksum algorithm used */
    UINT8 s_encryption_level;         /* versioning level for encryption */
    UINT8 s_reserved_pad;             /* Padding to next 32bits */
    UINT64 s_kbytes_written;          /* nr of lifetime kilobytes written */
    UINT32 s_snapshot_inum;           /* Inode number of active snapshot */
    UINT32 s_snapshot_id;             /* sequential ID of active snapshot */
    UINT64 s_snapshot_r_blocks_count; /* reserved blocks for active
                         snapshot's future use */
    UINT32 s_snapshot_list;           /* inode number of the head of the
                             on-disk snapshot list */
#define EXT4_S_ERR_START offsetof(struct ext4_super_block, s_error_count)
    UINT32 s_error_count;         /* number of fs errors */
    UINT32 s_first_error_time;    /* first time an error happened */
    UINT32 s_first_error_ino;     /* inode involved in first error */
    UINT64 s_first_error_block;   /* block involved of first error */
    UINT8 s_first_error_func[32]; /* function where the error happened */
    UINT32 s_first_error_line;    /* line number where error happened */
    UINT32 s_last_error_time;     /* most recent time of an error */
    UINT32 s_last_error_ino;      /* inode involved in last error */
    UINT32 s_last_error_line;     /* line number where error happened */
    UINT64 s_last_error_block;    /* block involved of last error */
    UINT8 s_last_error_func[32];  /* function where the error happened */
#define EXT4_S_ERR_END offsetof(struct ext4_super_block, s_mount_opts)
    UINT8 s_mount_opts[64];
    UINT32 s_usr_quota_inum;     /* inode for tracking user quota */
    UINT32 s_grp_quota_inum;     /* inode for tracking group quota */
    UINT32 s_overhead_clusters;  /* overhead blocks/clusters in fs */
    UINT32 s_backup_bgs[2];      /* groups with sparse_super2 SBs */
    UINT8 s_encrypt_algos[4];    /* Encryption algorithms in use  */
    UINT8 s_encrypt_pw_salt[16]; /* Salt used for string2key algorithm */
    UINT32 s_lpf_ino;            /* Location of the lost+found inode */
    UINT32 s_prj_quota_inum;     /* inode for tracking project quota */
    UINT32 s_checksum_seed;      /* crc32c(uuid) if csum_seed set */
    UINT8 s_wtime_hi;
    UINT8 s_mtime_hi;
    UINT8 s_mkfs_time_hi;
    UINT8 s_lastcheck_hi;
    UINT8 s_first_error_time_hi;
    UINT8 s_last_error_time_hi;
    UINT8 s_first_error_errcode;
    UINT8 s_last_error_errcode;
    UINT16 s_encoding;         /* Filename charset encoding */
    UINT16 s_encoding_flags;   /* Filename charset encoding flags */
    UINT32 s_orphan_file_inum; /* Inode for tracking orphan inodes */
    UINT32 s_reserved[94];     /* Padding to the end of the block */
    UINT32 s_checksum;         /* crc32c(superblock) */
} __attribute__((packed)) ext4_superblock_t;

#define EXT4_S_ERR_LEN (EXT4_S_ERR_END - EXT4_S_ERR_START)

#endif
