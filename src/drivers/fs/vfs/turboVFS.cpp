#include <drivers/fs/vfs/turboVFS.hpp>
#include <drivers/display/serial/serial.hpp>
#include <lib/string.hpp>
#include <lib/lock.hpp>


namespace turbo::vfs{
    
    bool isInit = false;
    bool debug = false;
    tfs_node_t *tfs_root;

    DEFINE_LOCK(vfs_lock);

        void *notFound(){
        if(debug){ turbo::serial::log("[!]-Warning FILE NOT FOUND"); }
        vfs_lock.unlock();
        return nullptr;
        }

        tfs_node_t* openNext(tfs_node_t *parentNode,tfs_node_t *childNode,size_t items,size_t cleared,char** pathArray){
        if( items > 0){
            if((parentNode->flags & 0x07) == TFS_MOUNTPOINT || (parentNode->flags & 0x07) == TFS_SYMLINK){
                parentNode = parentNode->ptr;
                openNext(parentNode,childNode,items,cleared,pathArray);
            }
            if(!strcmp(pathArray[cleared],"..")){
                if(items > 1){ parentNode = parentNode->parent; }
                else{ childNode = parentNode->parent; }
                cleared++;
                items--;
                openNext(parentNode,childNode,items,cleared,pathArray);
            }
            if(!strcmp(pathArray[cleared],".")){
                if(items <= 1){ childNode = parentNode; }
                cleared++;
                items--;
                openNext(parentNode,childNode,items,cleared,pathArray);
            }
            for(size_t i=0;i<parentNode->childs.getLength();i++){
                if((parentNode->childs[i]->flags & 0x07) == TFS_MOUNTPOINT || (parentNode->childs[i]->flags & 0X07) == TFS_SYMLINK){
                    childNode = parentNode->childs[i]->ptr;
                }else{
                    childNode = parentNode->childs[i];
                }
                if(!strcmp(childNode->name,pathArray[cleared])){
                    parentNode = parentNode->childs[i];
                    cleared++;
                    items--;
                    openNext(parentNode,childNode,items,cleared,pathArray);
                }
            }
            notFound();
        }

        vfs_lock.unlock();
        return childNode;
    }

    tfs_node_t *createNext(tfs_node_t *parentNode,tfs_node_t *childNode, size_t items,size_t cleared, char **pathArray){
        if(items > 0){
            if(tfs_node_t *temp = getChild(parentNode, pathArray[cleared])){
                if(items > 1){ parentNode = temp; }
                else{ childNode = temp; }
                cleared++;
                items--;
                createNext(parentNode,childNode,items,cleared,pathArray);
            }
            if(!strcmp(pathArray[cleared],"..")){
                if(items > 1){ parentNode = parentNode->parent; }
                else{ childNode = parentNode->parent; }
                cleared++;
                items--;
                createNext(parentNode,childNode,items,cleared,pathArray);
            }
            if(!strcmp(pathArray[cleared],".")){
                if(items > 1){
                    parentNode = parentNode->parent;
                }else{
                    childNode = parentNode->parent;
                }
                cleared++;
                items--;
                createNext(parentNode,childNode,items,cleared,pathArray);
            }
            if(items > 1){ parentNode = addChild(parentNode,pathArray[cleared]);}
            else{ childNode = addChild(parentNode,pathArray[cleared]);}

            cleared++;
            items--;
        }
        vfs_lock.unlock();
        return childNode;
    }

    
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
        
        for(size_t i = 0 ; i < parentNode->childs.getLength() ; i++){
            childNode = parentNode->childs[i];
            if(!strcmp(childNode->name,path)){
                return childNode;
            }
        }
        return nullptr;
    }

    tfs_node_t *addChild(tfs_node_t *parent,const char *name){
        if(!parent){
            parent = tfs_root;
        }
        tfs_node_t *node = static_cast<tfs_node_t*>(turbo::heap::calloc(1,sizeof(tfs_node_t)));
        strcpy(node->name,name);
        node->parent = parent;
        node->fs = parent->fs;
        parent->childs.push_back(node);
        return node;
    }

    tfs_node_t *addDir(tfs_node_t* parent, const char *name){
        if(!parent){
            parent = tfs_root;
        }
        tfs_node_t *node = static_cast<tfs_node_t*>(turbo::heap::calloc(1,sizeof(tfs_node_t)));
        strcpy(node->name,name);
        node->parent = parent;
        node->fs = parent->fs;
        node->flags = turbo::vfs::TFS_FOLDER;
        strcpy(node->path,node->parent->path);
        strcat(node->path,"/");
        strcat(node->path,node->name);
        parent->childs.push_back(node);
        return node;
    }

    tfs_node_t *addFile(tfs_node_t* parent, const char *name){
        if(!parent){
            parent = tfs_root;
        }
        tfs_node_t *node = static_cast<tfs_node_t*>(turbo::heap::calloc(1,sizeof(tfs_node_t)));
        strcpy(node->name,name);
        node->parent = parent;
        node->fs = parent->fs;
        node->flags = turbo::vfs::TFS_FILE;
        parent->childs.push_back(node);
        return node;
    }

    void removeChild(tfs_node_t * parent, const char *name){
        if(!parent){
            parent = tfs_root;
        }
        for(size_t i = 0;i < parent->childs.getLength();i++){
            tfs_node_t *node = parent->childs[i];
            if(!strcmp(node->name,name)){
                for(size_t i = 0;i < node->childs.getLength();i++){
                    turbo::heap::free(node->childs[i]);
                }
                node->childs.destroy();
                turbo::heap::free(node);
                parent->childs.remove(i);
                return;
            }
        }
    }



    tfs_node_t *open(tfs_node_t *parent,const char *path){
        if((parent == nullptr || parent == tfs_root) && !strcmp(path,"/")){
            return tfs_root->ptr;
        }
        vfs_lock.lock();
        if(!path){
            if(debug){
                turbo::serial::log("[!]-Warning INVALID PATH");
            }
            vfs_lock.unlock();
            return nullptr;
        }
        if(strchr(path,' ')){
            if(debug){
                turbo::serial::log("[!]-Warning PATH CAN'T CONTAIN SPACE");
            }
            vfs_lock.unlock();
            return nullptr;
        }

        tfs_node_t *parentNode;
        tfs_node_t *childNode;
        size_t items;
        size_t cleared = 0;

        if(parent == nullptr){parentNode = tfs_root->ptr;}
        else{parentNode = parent;}

        if(parentNode == nullptr){
            if(debug){ turbo::serial::log("[!]-Warning COULDN'T FIND DIRECTORY"); }
            vfs_lock.unlock();
            return nullptr;
        }
        if(!strcmp(path,"/")){ return parent; }

        char **pathArray = strsplit_count(path,"/",items);
        if(!pathArray){ return nullptr; }
        while(!strcmp(pathArray[0],"")){
            items--;
            pathArray++;
        }

        while(!strcmp(pathArray[items-1],"")){items--;}

        return openNext(parentNode,childNode,items,cleared,pathArray);
    }

    tfs_node_t *create(tfs_node_t *parent,const char *path){
        if((parent == nullptr || parent == tfs_root) && !strcmp(path,"/")){ return tfs_root->ptr; }
        vfs_lock.lock();
        if(!path){
            if(debug){ turbo::serial::log("[!]-Warning INVALID PATH"); }
            vfs_lock.unlock();
            return nullptr;
        }
        if(strchr(path,' ')){
            if(debug){ turbo::serial::log("[!]-Warning PATH CAN'T CONTAIN SPACE"); }
            vfs_lock.unlock();
            return nullptr;
        }

        tfs_node_t *parentNode;
        tfs_node_t *childNode;
        size_t items;
        size_t cleared = 0;

        if(!parent){ parentNode = tfs_root->ptr; }
        else{ parentNode = parent; }
        if(!parentNode){
            if(debug){
                turbo::serial::log("[!]-Warning CAN'T FIND DIRECTORY");
            }
            vfs_lock.unlock();
            return nullptr;
        }

        char **pathArray = strsplit_count(path,"/",items);
        if(!pathArray){ return nullptr; }
        while(!strcmp(pathArray[0],"")){
            items--;
            pathArray++;
        }
        while(!strcmp(pathArray[items-1],"")){ items--; }

        return createNext(parentNode,childNode,items,cleared,pathArray);
    }

    tfs_node_t *open_root(tfs_node_t *parent,const char *path){
        if((parent == nullptr || parent == tfs_root) && !strcmp(path,"/")){ return tfs_root->ptr; }
        tfs_node_t *node = open(parent,path);
        if(!node){ node = create(parent,path);}
        return node;
    }

    tfs_node_t *mount_root(tfs_t *fs){
        tfs_node_t *node = addChild(tfs_root,"/");
        tfs_root->ptr = node;
        node->flags = TFS_FOLDER;
        node->fs = fs;
        return node;
    }

    tfs_node_t *mount(tfs_t *fs, tfs_node_t *parent, const char *path){
    if (!tfs_root->ptr) mount_root(0);
    if (!parent) parent = tfs_root->ptr;
    if (!fs) fs = parent->fs;
    parent->ptr = create(parent, path);
    parent->flags = TFS_MOUNTPOINT;
    parent->ptr->flags = TFS_FOLDER;
    parent->ptr->fs = fs;
    return parent->ptr;
    }

    void init(){
        turbo::serial::log("[!]-Installing the VFS");
        if(isInit){
            turbo::serial::log("[!]-Warning VFS ALREADY INITIALISED");
            return;
        }

        tfs_root = new tfs_node_t;
        tfs_root->flags = filetypes::TFS_MOUNTPOINT;
        strcpy(tfs_root->name,ROOTNAME);
        tfs_root->fs = nullptr;

        turbo::serial::newline();
        isInit = true;
    }
}