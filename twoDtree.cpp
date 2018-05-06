
/**
 *
 * twoDtree (pa3)
 * slight modification of a Kd tree of dimension 2.
 * twoDtree.cpp
 * This file will be used for grading.
 *
 */

#include "twoDtree.h"
#include "math.h"
/* given */
twoDtree::Node::Node(pair<int,int> ul, pair<int,int> lr, RGBAPixel a)
	:upLeft(ul),lowRight(lr),avg(a),left(NULL),right(NULL)
	{}

/* given */
twoDtree::~twoDtree(){
	clear();
}

/* given */
twoDtree::twoDtree(const twoDtree & other) {
	copy(other);
}

/* given */
twoDtree & twoDtree::operator=(const twoDtree & rhs){
	if (this != &rhs) {
		clear();
		copy(rhs);
	}
	return *this;
}
  /**
    * Constructor that builds a twoDtree out of the given PNG.
    * Every leaf in the tree corresponds to a pixel in the PNG.
    * Every non-leaf node corresponds to a rectangle of pixels 
    * in the original PNG, represented by an (x,y) pair for the 
    * upper left corner of the rectangle and an (x,y) pair for 
    * lower right corner of the rectangle. In addition, the Node
    * stores a pixel representing the average color over the 
    * rectangle. 
    *
    * Every node's left and right children correspond to a partition
    * of the node's rectangle into two smaller rectangles. The node's
    * rectangle is split by the horizontal or vertical line that 
    * results in the two smaller rectangles whose sum of squared 
    * differences from their mean is as small as possible.
    *
    * The left child of the node will contain the upper left corner
    * of the node's rectangle, and the right child will contain the
    * lower right corner.
    *
   * This function will build the stats object used to score the 
   * splitting lines. It will also call helper function buildTree.
    */
twoDtree::twoDtree(PNG & imIn){ 
    stats imgstats = stats(imIn);
    this->root = buildTree(imgstats, pair<int,int>(0,0), pair<int,int>(imIn.width()-1,imIn.height()-1));
    this->width = imIn.width();
    this->height = imIn.height();
}
 /**
   * Private helper function for the constructor. Recursively builds
   * the tree according to the specification of the constructor.
   * @param s Contains the data used to split the rectangles
   * @param ul upper left point of current node's rectangle.
   * @param lr lower right point of current node's rectangle.
   */
twoDtree::Node * twoDtree::buildTree(stats & s, pair<int,int> ul, pair<int,int> lr) {
	Node * node = new Node(ul, lr, s.getAvg(ul,lr));
    
    pair<long, int> lowestScoreVR;
    pair<long, int> lowestScoreHR;
    int xSize = lr.first-ul.first;
    int ySize = lr.second-ul.second;
    // For 1xsomething and somethingx1 pixels
    if(xSize==0 && ySize==0){//1 by 1 rectangle (so a single pixel) TESTED
      
        return node;
    }
    else if((xSize==0&&ySize==1)||(ySize==0&&xSize==1)){ //1 by 2 rectangle, so just cut into 2 pixels TESTED
       
        partitionLeft(s, node, ul, ul);
        partitionRight(s, node, lr, lr);
    }
    else if(xSize==0 &&ySize>1){// 1 by 2+ rectangle
       
        for(int y=ul.second; y<lr.second; y++){
            long scoreUp   = s.getScore(ul, pair<int,int>(lr.first,y));
            long scoreDown = s.getScore(pair<int,int>(ul.first,y+1),lr);
            
            if((scoreUp+scoreDown<lowestScoreHR.first)||y==ul.second){
                lowestScoreHR.first = scoreUp+scoreDown;
                lowestScoreHR.second = y;
            }
        }
        partitionLeft(s, node, ul, pair<int,int>(lr.first, lowestScoreHR.second));
        partitionRight(s, node, pair<int,int>(ul.first,lowestScoreHR.second+1), lr);
    }
    else if(ySize==0 &&xSize>1){
      
        for(int x=ul.first; x<lr.first; x++){
            long scoreLeft  = s.getScore(ul, pair<int,int>(x,lr.second));
            long scoreRight = s.getScore(pair<int,int>(x+1,ul.second),lr);
       
            if((scoreLeft+scoreRight<lowestScoreVR.first)||x==ul.first){
                lowestScoreVR.first = scoreLeft+scoreRight;
                lowestScoreVR.second = x;
            }
        }
        partitionLeft(s, node, ul, pair<int,int>(lowestScoreVR.second,lr.second));
        partitionRight(s, node, pair<int,int>(lowestScoreVR.second+1,ul.second), lr);

    }
    
    // For 2+x2+ pixels
    else if(xSize>=1 &&ySize>=1){
        for(int x=ul.first; x<lr.first; x++){
            long scoreLeft  = s.getScore(ul, pair<int,int>(x,lr.second));
            long scoreRight = s.getScore(pair<int,int>(x+1,ul.second),lr);
       
            if((scoreLeft+scoreRight<lowestScoreVR.first)||x==ul.first){
                lowestScoreVR.first = scoreLeft+scoreRight;
                lowestScoreVR.second = x;
            }
        }

        for(int y=ul.second; y<lr.second; y++){
            long scoreUp   = s.getScore(ul, pair<int,int>(lr.first,y));
            long scoreDown = s.getScore(pair<int,int>(ul.first,y+1),lr);
        
            if((scoreUp+scoreDown<lowestScoreHR.first)||y==ul.second){
                lowestScoreHR.first = scoreUp+scoreDown;
                lowestScoreHR.second = y;
            }
        }

        if(lowestScoreVR.first<lowestScoreHR.first){ //VR is preferred
            partitionLeft(s, node, ul, pair<int,int>(lowestScoreVR.second,lr.second));
            partitionRight(s, node, pair<int,int>(lowestScoreVR.second+1,ul.second), lr);
        }else{                           //HR is preferred
            partitionLeft(s, node, ul, pair<int,int>(lr.first, lowestScoreHR.second));
            partitionRight(s, node, pair<int,int>(ul.first,lowestScoreHR.second+1), lr);
        }
    }
    
    
    return node;
}

void twoDtree::partitionLeft(stats & s, Node* & node, pair<int,int> ul, pair<int,int> lr){
    
    node->left = buildTree(s, ul, lr);
}

void twoDtree::partitionRight(stats & s, Node* & node, pair<int,int> ul, pair<int,int> lr){
    
    node->right = buildTree(s, ul, lr);
}

/**
    * Render returns a PNG image consisting of the pixels
    * stored in the tree. may be used on pruned trees. Draws
    * every leaf node's rectangle onto a PNG canvas using the 
    * average color stored in the node.
    */
PNG twoDtree::render(){
	
    PNG ret = PNG(width, height);
    renderPic(ret, this->root);
    return ret;
}

void twoDtree::renderPic(PNG & png, Node* node){

    if(node==NULL){
        return;
    }
    else if(node->left==NULL && node->right==NULL){ //At leaf
        
        for(int y = node->upLeft.second; y<node->lowRight.second+1; y++){
            for(int x=node->upLeft.first; x<node->lowRight.first+1; x++){
                *(png.getPixel(x, y))=node->avg;
            }
        }
    }
    else{
        renderPic(png, node->left);
        renderPic(png, node->right);
    }
}
/*
    *  Prune function trims subtrees as high as possible in the tree.
    *  A subtree is pruned (cleared) if at least pct of its leaves are within 
    *  tol of the average color stored in the root of the subtree. 
    *  Pruning criteria should be evaluated on the original tree, not 
    *  on a pruned subtree. (we only expect that trees would be pruned once.)
    *  
   * You may want a recursive helper function for this one.
    */
void twoDtree::prune(double pct, int tol){
	
    pruneHelper(this->root, pct, tol);
   
}
void twoDtree::pruneHelper(Node* curNode, double pct, int tol){
    if(curNode==NULL){
        return;
    }
    if(pruneNode(curNode, pct, tol)){
        
        clearNodes(curNode->left);
        clearNodes(curNode->right);
        curNode->left = NULL;
        curNode->right = NULL;
    }else{
       
        pruneHelper(curNode->left, pct, tol);
        pruneHelper(curNode->right, pct, tol);
    }
}
bool twoDtree::pruneNode(Node* node, double pct, int tol){
    double numLeaf = 0;
    double numWithinTol = 0;
    findLeaves(node, numLeaf, numWithinTol, tol, node);
    if(numLeaf==0){
        return true;
    }
    if(numWithinTol/numLeaf>=pct){
        return true;
    }
    return false;
}

void twoDtree::findLeaves(Node* node, double & numLeaf, double & numWithinTol, int tol, Node* root){
    if(node->left == NULL && node->right == NULL){ //Leaf Reached
        numLeaf+=1;
        double distance = pow(root->avg.r-node->avg.r,2)+pow(root->avg.g-node->avg.g,2)+pow(root->avg.b-node->avg.b,2);
        if(distance<=tol){
            numWithinTol+=1;
        }
    }
    else{
        findLeaves(node->left, numLeaf, numWithinTol, tol, root);
        findLeaves(node->right, numLeaf, numWithinTol, tol, root);
    }
}

 /**
    * Destroys all dynamically allocated memory associated with the
    * current twoDtree class. Complete for PA3.
   * You may want a recursive helper function for this one.
    */
void twoDtree::clear() {
	clearNodes(root);
    this->height = 0;
    this->width = 0; 
}

void twoDtree::clearNodes(Node* node){
    if(node==NULL){
        return;
    }
    clearNodes(node->left);
    clearNodes(node->right);
    delete node;
}

 /**
   * Copies the parameter other twoDtree into the current twoDtree.
   * Does not free any memory. Called by copy constructor and op=.
   * You may want a recursive helper function for this one.
   * @param other The twoDtree to be copied.
   */

void twoDtree::copy(const twoDtree & orig){
	/* your code here */
    
    this->height = orig.height;
    this->width  = orig.width;
    this->root = new Node(orig.root->upLeft, orig.root->lowRight, orig.root->avg);
    copyTree(this->root, orig.root);
    
}

void twoDtree::copyTree(Node* & copy, Node* node){
    if(node==NULL){
        return;
    }
    addLeftTrees(copy, node->left);
    addRightTrees(copy, node->right);
}

void twoDtree::addLeftTrees(Node* & copy, Node* node){
    if(node==NULL){
        return;
    }
    copy->left = new Node(node->upLeft, node->lowRight, node->avg);
    copyTree(copy->left, node);
} 

void twoDtree::addRightTrees(Node* & copy, Node* node){
    if(node==NULL){
        return;
    }
    copy->right = new Node(node->upLeft, node->lowRight, node->avg);
    copyTree(copy->right, node);
} 
