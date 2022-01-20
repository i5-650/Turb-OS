#include <drivers/fs/vfs/turboVFS.hpp>
#include <lib/string.hpp>
#include <lib/lock.hpp>


namespace turbo::vfs{
    
    bool isInitialised = false;
    bool debug = false;
    tfs_node_t *tfs_root;

    DEFINE_LOCK(vfs_lock);
    
    uint64_t read_tfs(tfs_node_t *node,uint64_t offset,uint64_t size,char *buffer){
        if(node->fs->read != 0){
            return node->fs->read(node,offset,size,buffer);
        }
        return 0;
    }

    uint64_t write_tfs(tfs_node_t *node, uint64_t offset, uint64_t size,char *buffer){
        if(node->fs->write != 0){
            return node->fs->write(node,offset,size,buffer);
        }
        return 0;
    }

    void open_tfs(tfs_node_t *node , uint8_t read,uint8_t write){
        if(node->fs->open != 0){
            return node->fs->open(node,read,write);
        }
        return;
    }

    void close_tfs(tfs_node_t *node){
        if(node->fs->close != 0){
            return node->fs->close(node);
        }
        return;
    }

    tfs_node_t *getChild(tfs_node_t *parent, const char *path){
        tfs_node_t *parentNode;
        tfs_node_t *childNode;
        if(!parent){
            parentNode = tfs_root;
        }else{
            parentNode = parent;
        }

        if(*path == '\0'){
            return nullptr;
        }
        
        for(size_t i = 0 ; i < parentNode->childs.size() ; i++){
            childNode = parentNode->childs[i];
            if(!strcmp(childNode->name,path)){
                return childNode;
            }
        }
        return nullptr;
    }

    tfs_node_t *addChild(tfs_node_t *parent,const char *path){
        if(!parent){
            parent = tfs_root;
        }
    }
}