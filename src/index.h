#ifndef _INDEX_
#define _INDEX_

#include <stdint.h>
#define NUMDIMS	3	/* number of dimensions */
#define NDEBUG

typedef float RectReal;
typedef bool DeadFlag;

/*-----------------------------------------------------------------------------
| Global definitions.
-----------------------------------------------------------------------------*/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define NUMSIDES 2*NUMDIMS // Size = 6

struct Rect // Float(4byte) * 6
{
	RectReal boundary[NUMSIDES]; /* xmin,ymin,...,xmax,ymax,... */
};

struct Node;

#ifndef MULTIMETA
#define META 8
#define DUMMY 24
#define MAXCARD 55
#define NODECARD 55
#else
#ifdef NS2
#define META 16
#define DUMMY 16
#define MAXCARD 119
#define NODECARD 119
#elif NS3
#define META 24
#define DUMMY 8
#define MAXCARD 183
#define NODECARD 183
#elif NS4
#define META 32
#define DUMMY 0
#define MAXCARD 247
#define NODECARD 247
#endif
#endif
#include "bitmap.h" 
//
struct Branch // 24 + 8
{
	struct Rect rect; // Float(4byte) * 6
	struct Node *child; // Pointer =  8
};

/* max branching factor of a node */
struct Node
{
    MetaData<META> meta;
#if DUMMY != 0
    char dummy[DUMMY];
#endif
    struct Branch branch[MAXCARD]; //32*55 = 1760 = 27.5 
#ifdef SHARED
    mutable std::shared_mutex* mutex_;
#else
    mutable std::shared_mutex* mutex_;
#endif
};

#ifndef FULLLOG
struct splitLog{
    struct Node* parent; //+8
    struct Node* child; //+8
    struct Node* sibling; //+8
#ifdef MULTIMETA
    MetaData<META> meta;
#endif
};
#else
struct splitLog{
    struct Node parent; 
    struct Node* parentPoint; //+8
    struct Node child; 
    struct Node* childPoint; //+8
    struct Node sibling; 
    struct Node* siblingPoint; //+8
#ifdef MULTIMETA
    MetaData<META> meta;
#endif
};
#endif
#ifndef INPLACE 
struct cowLog{
  MetaData<META> meta;
  struct Node* parent;
};
#endif

struct ListNode
{
	struct ListNode *next; // 
	struct Node *node; // 
};

//#############################
#define METHODS 1
struct PartitionVars
{
    int partition[MAXCARD+1];
    int total, minfill;
    int taken[MAXCARD+1];
    int count[2];
    struct Rect cover[2];
    float area[2];
};

struct forSplit
{
    struct Branch BranchBuf[MAXCARD+1];
    int BranchCount;
    struct Rect CoverSplit;

    struct PartitionVars Partitions[METHODS];
};

/*
 * If passed to a tree search, this callback function will be called
 * with the ID of each data rect that overlaps the search rect
 * plus whatever user specific pointer was passed to the search.
 * It can terminate the search early by returning 0 in which case
 * the search will return the number of hits found up to that point.
 */


extern int RTreeSearch(struct Node*, struct Rect*);
extern int RTreeInsertRect(struct Rect*, int, struct Node**, int depth, struct splitLog *);
extern int RTreeDeleteRect(struct Rect*, int, struct Node**);
extern struct Node * RTreeNewIndex();
extern struct Node * RTreeNewNode();
extern void RTreeInitNode(struct Node*);
extern void RTreeFreeNode(struct Node *);
extern void RTreePrintNode(struct Node *, int);
extern void RTreeTabIn(int);
extern struct Rect RTreeNodeCover(struct Node *);
extern void RTreeInitRect(struct Rect*);
extern struct Rect RTreeNullRect();
extern RectReal RTreeRectArea(struct Rect*);
extern RectReal RTreeRectSphericalVolume(struct Rect *R);
extern RectReal RTreeRectVolume(struct Rect *R);
extern struct Rect RTreeCombineRect(struct Rect*, struct Rect*);
extern int RTreeOverlap(struct Rect*, struct Rect*);
extern void RTreePrintRect(struct Rect*, int);
extern int RTreeAddBranch(struct Branch *, struct Node *, struct Node **, struct Node *, struct splitLog *);
//extern int RTreeAddBranch(struct Branch *, struct Node *, struct Node **);
extern int RTreePickBranch(struct Rect *, struct Node *);
extern void RTreeDisconnectBranch(struct Node *, int);
extern void RTreeSplitNode(struct Node*, struct Branch*, struct Node**);

extern int RTreeSetNodeMax(int);
extern int RTreeSetLeafMax(int);
extern int RTreeGetNodeMax();
extern int RTreeGetLeafMax();

#endif /* _INDEX_ */
