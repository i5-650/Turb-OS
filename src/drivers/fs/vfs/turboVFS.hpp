#pragma once 

#include <lib/TurboVector/TurboVector.hpp>
#include <stdint.h>

namespace turbo::vfs{
    
    #define FILENAME_LENGTH 128
    #define ROOTNAME "[ROOT]"
    #define PATH_LENGTH 128

    enum filetypes{
        TFS_FILE = 0x01, //Flags to define it's a file
        TFS_FOLDER = 0x02, //Flags to define it's a folder
        TFS_PATHDEVICE = 0x03, //Reference to the path of the device
        TFS_BLOCKDEVICE = 0x04, //Reference to the memory allocated to this block
        TFS_PIPE = 0x05, //Reference to the maximum size of a pipe
        TFS_SYMLINK = 0x06, //Reference the link path with a symbolic link pointing to the original path
        TFS_MOUNTPOINT = 0x08 //Reference the parent directory accessible 
    };

    struct tfs_node_t;

    using read_t = size_t (*)(tfs_node_t*,size_t,size_t,char*);
    using write_t = size_t (*)(tfs_node_t*,size_t,size_t,char*);
    using open_t = void (*)(tfs_node_t*,uint8_t,uint8_t);
    using close_t = void (*)(tfs_node_t*);    

    struct tfs_t{
        char name[FILENAME_LENGTH];
        read_t read;
        write_t write;
        open_t open;
        close_t close;
    };

    struct tfs_node_t{
        char name[FILENAME_LENGTH];
        uint64_t mode; //Reference to the mode of the node
        uint64_t flags; //Reference for all the flags of the node
        tfs_node_t *ptr; //Self Pointer
        tfs_t *fs; //Pointer on the file-system
        tfs_node_t *parent; //Pointer on the parent node
        TurboVector<tfs_node_t*> childs; //Vector of all the listed child
        char path[PATH_LENGTH];
    };
    
    extern bool isInit;
    extern bool debug;
    extern tfs_node_t *tfs_root;

    size_t read_tfs(tfs_node_t *node, size_t offset, size_t size, char *buffer);
    size_t write_tfs(tfs_node_t *node, size_t offset, size_t size, char *buffer);
    void open_tfs(tfs_node_t *node, uint8_t read, uint8_t write);
    void close_tfs(tfs_node_t *node);

    tfs_node_t *getChild(tfs_node_t *parent,const char *path);
    tfs_node_t *addChild(tfs_node_t *parent,const char *name);
    tfs_node_t *addDir(tfs_node_t *parent,const char *name);
    tfs_node_t *addFile(tfs_node_t *parent,const char *name);
    void removeChild(tfs_node_t *parent,const char *name);

    tfs_node_t *open(tfs_node_t *parent,const char *path);
    tfs_node_t *create(tfs_node_t *parent,const char *path);
    tfs_node_t *open_root(tfs_node_t *parent,const char *path);

    tfs_node_t *mount_root(tfs_t *fileSystem);
    tfs_node_t *mount(tfs_t *fileSystem,tfs_node_t *parent,const char *path);

    void init();
}