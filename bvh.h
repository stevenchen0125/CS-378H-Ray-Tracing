#pragma once
#include "bbox.h"
#include "scene.h"


class BVH {
    class BVHNode {
        private:
        BoundingBox bbox;
        bool leaf;
        unsigned int nObjsInNode;
        unsigned int index;

        public:
        void setBoundingBox(BoundingBox &bbox){}
        BoundingBox &getBoundingBox(){return bbox;}
        void makeLeaf(unsigned int index_, unsigned int nObjsInNode_){}
        void makeNode(unsigned int start){}
        bool isLeaf(){return leaf;}
        unsigned int getIndex(){return index;}
        unsigned int getNObjs(){return nObjsInNode;};
    };
public:
    void build(obj){};
    BVHNode getRoot() {return &root;};
    BVHNode setRoot(BVHNode &node) {root = node;};
private:
    BVHNode root;


    
};