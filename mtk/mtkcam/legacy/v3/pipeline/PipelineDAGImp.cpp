/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/pipeline"
//
#include "MyUtils.h"
#include <mtkcam/v3/pipeline/IPipelineDAG.h>
#include <utils/KeyedVector.h>
#include <utils/RWLock.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class PipelineDAGImp
    : public IPipelineDAG
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    friend  class IPipelineDAG;
public:     ////                            Definitions.
    enum
    {
        BAD_ID      = NodeId_T(-1L),
    };

public:     ////                            Definitions.
    struct NodeWithAdj_T
    {
        NodeObj_T                           mNode;
        NodeIdSet_T                         mInAdj;        //In adjacent node_id set
        NodeIdSet_T                         mOutAdj;    //Out adjacent node_id set
                                            NodeWithAdj_T(NodeObj_T const &node = NodeObj_T())
                                                : mNode(node)
                                                , mInAdj()
                                                , mOutAdj()
                                            {}
    };

    typedef KeyedVector<NodeId_T, NodeWithAdj_T>
                                            Map_T;

protected:  ////                            Data Members.
    mutable RWLock                          mMutex;
    NodeId_T                                mRootId;
    Map_T                                   mNodesVec;
    mutable Vector<NodeObj_T>               mToposort;      //topological sort

protected:  ////                            Operations.
                                            PipelineDAGImp();

private:    ////                            Operations.
            MERROR                          findPathBFS(
                                                NodeId_T         id,
                                                NodeIdSet_T     &checkList,
                                                sp<IPipelineDAG> newDAG
                                            ) const;

            MERROR                          checkListDFS(
                                                NodeId_T id,
                                                bool *checkList
                                            ) const;

    template <class T>
    static  MERROR                          evaluateToposort(
                                                Map_T const& dag,
                                                T& rToposort
                                            );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineDAG Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                            Operations.

    virtual IPipelineDAG*                   clone() const;

    virtual IPipelineDAG*                   clone(
                                                NodeIdSet_T const& ids
                                            ) const;

    virtual MERROR                          addNode(
                                                NodeId_T id,
                                                NodeVal_T const& val
                                            );

    virtual MERROR                          removeNode(
                                                NodeId_T id
                                            );

    virtual MERROR                          addEdge(
                                                NodeId_T id_src,
                                                NodeId_T id_dst
                                            );

    virtual MERROR                          removeEdge(
                                                NodeId_T id_src,
                                                NodeId_T id_dst
                                            );

    virtual MERROR                          setRootNode(
                                                NodeId_T id
                                            );

    virtual MERROR                          setNodeValue(
                                                NodeId_T id,
                                                NodeVal_T const& val
                                            );

    virtual MVOID                           dump() const;
    virtual MVOID                           dump(
                                                android::Vector<android::String8>& rLogs
                                            ) const;

public:     ////                            Attributes.

    virtual NodeObj_T                       getRootNode() const;

    virtual NodeObj_T                       getNode(NodeId_T id) const;

    virtual MERROR                          getEdges(
                                                Vector<Edge>& result
                                            ) const;

    virtual size_t                          getNumOfNodes() const;

    virtual MERROR                          getInAdjacentNodes(
                                                NodeId_T id,
                                                NodeObjSet_T& result
                                            ) const;

    virtual MERROR                          getOutAdjacentNodes(
                                                NodeId_T id,
                                                NodeObjSet_T& result
                                            ) const;

    virtual MERROR                          getNodesAndPathsForNewDAG(
                                                NodeIdSet_T     &orphanNodes,
                                                NodeIdSet_T     &checkList,
                                                sp<IPipelineDAG> newDAG
                                            ) const;

    virtual MERROR                          getOrphanNodes(
                                                NodeIdSet_T &orphanNodes,
                                                NodeIdSet_T &connectedNodes
                                            ) const;

    virtual MERROR                          getTopological(
                                                List<NodeObj_T>& result
                                            ) const;

    virtual Vector<NodeObj_T>const&         getToposort() const;

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
IPipelineDAG*
IPipelineDAG::
create()
{
    return new PipelineDAGImp;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineDAGImp::
PipelineDAGImp()
    : mMutex()
    , mRootId(BAD_ID)
    , mNodesVec()
    , mToposort()
{
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineDAG*
PipelineDAGImp::
clone() const
{
    RWLock::AutoRLock _(mMutex);
    PipelineDAGImp *tmp = new PipelineDAGImp;
    tmp->mRootId = mRootId;
    tmp->mNodesVec = mNodesVec;
    return tmp;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineDAG*
PipelineDAGImp::
clone(NodeIdSet_T const& ids) const
{
    PipelineDAGImp *ndag;
    Vector<NodeId_T> vDirty;
    {
        RWLock::AutoRLock _l(mMutex);
        //
        vDirty.setCapacity(mNodesVec.size()-ids.size());
        //
        //  Scan to determine the dirty set.
        for (size_t i = 0; i < mNodesVec.size(); i++)
        {
            NodeId_T const nodeId = mNodesVec.keyAt(i);
            ssize_t const index = ids.indexOf(nodeId);
            if  ( 0 > index ) {
                //Add this node to dirty if it is not specified within the given set.
                vDirty.add(nodeId);
                if  ( mRootId == nodeId ) {
                    MY_LOGE("RootId:%" PRIxPTR " is not specified within the given set", mRootId);
                    return NULL;
                }
            }
        }
        //
        //  Check the given set of nodes is a subset of the original DAG.
        if  ( mNodesVec.size() != vDirty.size() + ids.size() )
        {
            MY_LOGE(
                "The given set is not a subset of the original DAG..."
                "#Original:%zu #Dirty:%zu #Given:%zu",
                mNodesVec.size(), vDirty.size(), ids.size()
            );
            return NULL;
        }
        ndag = new PipelineDAGImp;
        ndag->mRootId = mRootId;
        ndag->mNodesVec = mNodesVec;
    }
    //
    if  ( ndag )
    {
        //  Remove all of un-specified nodes from the newly-cloned dag.
        for (size_t i = 0; i < vDirty.size(); i++)
        {
            ndag->removeNode(vDirty[i]);
        }
    }
    return ndag;
}


/******************************************************************************
 *Add a node into DAG; if memory is not enough, return failure
 ******************************************************************************/
MERROR
PipelineDAGImp::
addNode(NodeId_T id, NodeVal_T const& val)
{
    RWLock::AutoWLock _(mMutex);
    NodeObj_T tmpNode;
    tmpNode.id = id;
    tmpNode.val = val;
    ssize_t err = mNodesVec.add(id, NodeWithAdj_T(tmpNode));
    if (0 > err)
    {
        MY_LOGE("Not enough memory");
        return err;
    }
    return OK;
}


/******************************************************************************
 *Remove a node from DAG; if remove a non exist node, return failure
 ******************************************************************************/
MERROR
PipelineDAGImp::
removeNode(NodeId_T id)
{
    RWLock::AutoWLock _(mMutex);
    ssize_t rmIdx = mNodesVec.indexOfKey(id);
    if (0 > rmIdx)
    {
        MY_LOGE("The node of id %" PRIxPTR " does not exist",id);
        return rmIdx;
    }
    //
    //reset RootId if it is removed
    if  ( id == mRootId ) {
        mRootId = BAD_ID;
    }
    //
    PipelineDAGImp::NodeIdSet_T *rmSet;
    //remove in adjacent nodes of deleted node
    rmSet = &(mNodesVec.editValueAt(rmIdx).mInAdj);
    size_t upper = rmSet->size();
    for (size_t i = 0; i < upper; i++)
    {
        mNodesVec.editValueFor(rmSet->itemAt(i)).mOutAdj.remove(id);
    }
    //remove out adjacent nodes of deleted node
    rmSet = &(mNodesVec.editValueAt(rmIdx).mOutAdj);
    upper = rmSet->size();
    for (size_t i = 0; i < upper; i++)
    {
        mNodesVec.editValueFor(rmSet->itemAt(i)).mInAdj.remove(id);
    }
    mNodesVec.removeItemsAt(rmIdx);

    return OK;
}


/*******************************************************************************
 *Add an edge into DAG; if memory is not enough, return failure
 *******************************************************************************/
MERROR
PipelineDAGImp::
addEdge(NodeId_T id_src, NodeId_T id_dst)
{
    RWLock::AutoWLock _(mMutex);
    //Ensure both nodes exist
    ssize_t srcIdx = mNodesVec.indexOfKey(id_src);
    ssize_t dstIdx = mNodesVec.indexOfKey(id_dst);
    if (0 > srcIdx || 0 > dstIdx)
    {
        MY_LOGE("Node does not exist\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd\n",
        id_src, srcIdx, id_dst, dstIdx);
        return NAME_NOT_FOUND;
    }

    //Ensure that each edge only be added once
    ssize_t srcEdgeIdx = mNodesVec.valueAt(srcIdx).mOutAdj.indexOf(id_dst);
    ssize_t dstEdgeIdx = mNodesVec.valueAt(dstIdx).mInAdj.indexOf(id_src);
    if (0 > srcEdgeIdx || 0 > dstEdgeIdx)
    {
        ssize_t err_src = mNodesVec.editValueAt(srcIdx).mOutAdj.add(id_dst);
        ssize_t err_dst = mNodesVec.editValueAt(dstIdx).mInAdj.add(id_src);
        if (0 > err_src || 0 > err_dst)
        {
            MY_LOGE("Not enough memory\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd\n",
            id_src, err_src, id_dst, err_dst);
            return NO_MEMORY;
        }
    }

    return OK;
}


/******************************************************************************
 *Remove an edge from DAG; if the edge does not exist, return failure
 ******************************************************************************/
MERROR
PipelineDAGImp::
removeEdge(NodeId_T id_src, NodeId_T id_dst)
{
    RWLock::AutoWLock _(mMutex);
    ssize_t srcIdx = mNodesVec.indexOfKey(id_src);
    ssize_t dstIdx = mNodesVec.indexOfKey(id_dst);
    if (0 > srcIdx || 0 > dstIdx)
    {
        MY_LOGE("Node does not exist\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd\n",
        id_src, srcIdx, id_dst, dstIdx);
        return NAME_NOT_FOUND;
    }

    ssize_t srcEdgeIdx = mNodesVec.valueAt(srcIdx).mOutAdj.indexOf(id_dst);
    ssize_t dstEdgeIdx = mNodesVec.valueAt(dstIdx).mInAdj.indexOf(id_src);
    if (0 > srcEdgeIdx || 0 > dstEdgeIdx)
    {
        MY_LOGE("Edge does not exist\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd",
        id_src, srcEdgeIdx, id_dst, dstEdgeIdx);
        return NAME_NOT_FOUND;
    }

    mNodesVec.editValueAt(srcIdx).mOutAdj.removeAt(srcEdgeIdx);
    mNodesVec.editValueAt(dstIdx).mInAdj.removeAt(dstEdgeIdx);
    return OK;
}


/******************************************************************************
 *set certain node to root node; if the node does not exist, return failure
 ******************************************************************************/
MERROR
PipelineDAGImp::
setRootNode(NodeId_T id)
{
    RWLock::AutoWLock _(mMutex);
    ssize_t err = mNodesVec.indexOfKey(id);
    if (0 > err)
    {
        MY_LOGE("Node does not exist\nID:%" PRIxPTR "", id);
        return err;
    }
    mRootId = id;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDAGImp::
setNodeValue(
    NodeId_T id,
    NodeVal_T const& val
)
{
    RWLock::AutoWLock _l(mMutex);
    ssize_t const index = mNodesVec.indexOfKey(id);
    if (0 > index)
    {
        MY_LOGE("Node does not exist\nID:%" PRIxPTR "", id);
        return index;
    }
    //
    mNodesVec.editValueAt(index).mNode.val = val;
    return OK;
}


/******************************************************************************
 *get root node of a DAG; if node does not exist, return failure
 ******************************************************************************/
PipelineDAGImp::NodeObj_T
PipelineDAGImp::
getRootNode() const
{
    RWLock::AutoRLock _(mMutex);
    //Check if mRootId has been set or not
    if (mRootId == BAD_ID)
    {
        return NodeObj_T();
    }
    ssize_t idx = mNodesVec.indexOfKey(mRootId);
    if (0 > idx)
    {
        MY_LOGE("Node does not exist\nID:%" PRIxPTR "", mRootId);
        return NodeObj_T();
    }
    return mNodesVec.valueAt(idx).mNode;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDAGImp::
getEdges(Vector<PipelineDAGImp::Edge>& result) const
{
    RWLock::AutoRLock _(mMutex);

    result.clear();
    for (size_t i = 0; i < mNodesVec.size(); i++)
    {
        NodeWithAdj_T currentNode = mNodesVec.valueAt(i);
        for (size_t j = 0; j < currentNode.mOutAdj.size(); j++)
        {
            result.add(Edge(currentNode.mNode.id, currentNode.mOutAdj.itemAt(j)));
        }
    }
    return OK;
}
/******************************************************************************
 *get a node by its ID; if no such node exist, return a node of -1,-1 as failure
 ******************************************************************************/
PipelineDAGImp::NodeObj_T
PipelineDAGImp::
getNode(NodeId_T id) const
{
    RWLock::AutoRLock _(mMutex);
    ssize_t idx = mNodesVec.indexOfKey(id);
    if (0 > idx)
    {
        MY_LOGE("Node does not exist\nID:%" PRIxPTR "", id);
        return NodeObj_T();
    }
    return mNodesVec.valueAt(idx).mNode;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
PipelineDAGImp::
getNumOfNodes() const
{
    RWLock::AutoRLock _(mMutex);
    return mNodesVec.size();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDAGImp::
getInAdjacentNodes(NodeId_T id, NodeObjSet_T &result) const
{
    RWLock::AutoRLock _(mMutex);
    ssize_t idx = mNodesVec.indexOfKey(id);
    if (0 > idx)
    {
        MY_LOGE("Node does not exist\nID:%" PRIxPTR "", id);
        result.clear();
        return idx;
    }

    //let result be always empty before we put things inside
    result.clear();

    NodeWithAdj_T const &tmp = mNodesVec.valueAt(idx);
    NodeIdSet_T const &inAdj = tmp.mInAdj;
    size_t upperbound = inAdj.size();
    result.setCapacity(upperbound);
    for (size_t i = 0; i < upperbound; i++)
    {
        result.add(mNodesVec.valueFor(inAdj.itemAt(i)).mNode);
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDAGImp::
getOutAdjacentNodes(NodeId_T id, NodeObjSet_T &result) const
{
    RWLock::AutoRLock _(mMutex);
    ssize_t idx = mNodesVec.indexOfKey(id);
    if (0 > idx)
    {
        MY_LOGE("Node does not exist\nID:%" PRIxPTR "", id);
        result.clear();
        return idx;
    }

    //let result be always empty before we put things inside
    result.clear();

    NodeWithAdj_T const &tmp = mNodesVec.valueAt(idx);
    NodeIdSet_T const &outAdj = tmp.mOutAdj;
    size_t upperbound = outAdj.size();
    result.setCapacity(upperbound);
    for (size_t i = 0; i < upperbound; i++)
    {
        result.add(mNodesVec.valueFor(outAdj.itemAt(i)).mNode);
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class T>
MERROR
PipelineDAGImp::
evaluateToposort(
    Map_T const& dag,
    T& rToposort
)
{
    enum
    {
        NOT_VISIT,
        VISITED,
        SORTED,
    };

    struct DFS
    {
        static
        MERROR
        run(
            Map_T const& dag,
            NodeId_T const id,
            KeyedVector<NodeId_T, MINT>& visit,
            T& rToposort
        )
        {
            ssize_t const idx = visit.indexOfKey(id);
            if  ( idx < 0 ) {
                MY_LOGE("nodeId:%#"PRIxPTR" not found @ visit", id);
                return NO_INIT;
            }
            //
            switch  (visit.valueAt(idx))
            {
            case SORTED:
                return OK;
            case VISITED:
                MY_LOGE("CYCLE EXIST");
                return UNKNOWN_ERROR;
            default:
                break;
            }
            //
            visit.editValueAt(idx) = VISITED;
            ssize_t const indexDag = dag.indexOfKey(id);
            NodeIdSet_T const& outAdj = dag.valueAt(indexDag).mOutAdj;
            for (size_t i = 0; i < outAdj.size(); i++) {
                if  ( OK != DFS::run(dag, outAdj[i], visit, rToposort) ) {
                    return UNKNOWN_ERROR;
                }
            }
            //
            visit.editValueAt(idx) = SORTED;
            rToposort.push_front(dag.valueAt(indexDag).mNode);
            return OK;
        }
    };

    KeyedVector<NodeId_T, MINT> visit;
    visit.setCapacity(dag.size());
    for (size_t i = 0; i < dag.size(); i++) {
        NodeId_T const nodeId = dag.keyAt(i);
        visit.add(nodeId, NOT_VISIT);
    }
    //
    for (size_t i = 0; i < visit.size(); i++) {
        if ( NOT_VISIT == visit.valueAt(i) ) {
            NodeId_T const nodeId = visit.keyAt(i);
            if  ( OK != DFS::run(dag, nodeId, visit, rToposort) ) {
                rToposort.clear();
                return UNKNOWN_ERROR;
            }
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
Vector<PipelineDAGImp::NodeObj_T>const&
PipelineDAGImp::
getToposort() const
{
    RWLock::AutoRLock _(mMutex);
    //
    if  ( mToposort.isEmpty() ) {
        mToposort.setCapacity(mNodesVec.size());
        evaluateToposort(mNodesVec, mToposort);
    }
    return mToposort;
}


/******************************************************************************
 *result: a topological list of nodeObj_T of DAG, return -1L if CYCLE EXIST
 ******************************************************************************/
MERROR
PipelineDAGImp::
getTopological(List<PipelineDAGImp::NodeObj_T>& result) const
{
    RWLock::AutoRLock _(mMutex);
    //
    result.clear();
    return (OK == evaluateToposort(mNodesVec, result)) ? OK : MERROR(-1L);
}


/******************************************************************************
 *Get IDs of nodes that are not reachable from root, return 0 as normal
 ******************************************************************************/
MERROR
PipelineDAGImp::
getOrphanNodes(
    NodeIdSet_T &orphanNodes,
    NodeIdSet_T &connectedNodes
) const
{
    RWLock::AutoRLock _(mMutex);
    Vector<bool> checkList;
    checkList.setCapacity(mNodesVec.size());
    checkList.insertAt(0, 0, mNodesVec.size());
    if (0 > checkListDFS(mRootId, checkList.editArray()))
    {
        orphanNodes.clear();
        MY_LOGE("Accessing ID that does not exist");
        return -1L;
    }
    for (size_t i = 0; i < mNodesVec.size(); i++)
    {
        if (!checkList[i])
        {
            orphanNodes.add(mNodesVec.keyAt(i));
        }
        else
        {
            connectedNodes.add(mNodesVec.keyAt(i));
        }
    }
    return OK;
}


/******************************************************************************
 *DFS used only in getOrphanNodes()
 ******************************************************************************/
MERROR
PipelineDAGImp::
checkListDFS(NodeId_T id, bool *checkList) const
{
    ssize_t idx = mNodesVec.indexOfKey(id);
    if (0 > idx)
    {
        return -1L;
    }
    checkList[idx] = true;
    NodeWithAdj_T node = mNodesVec.valueAt(idx);
    for (size_t i = 0; i < node.mOutAdj.size(); i++)
    {
        if (0 > checkListDFS(node.mOutAdj.itemAt(i), checkList))
        {
            return -1L;
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
int
adjCompare(
    const PipelineDAGImp::NodeWithAdj_T *rhs,
    const PipelineDAGImp::NodeWithAdj_T *lhs
)
{
    if (rhs->mInAdj.size() > lhs->mInAdj.size())
    {
        return 1;
    }
    return 0;
}


/******************************************************************************
 * Use BFS to find path for node id, return -1L if no path found
 ******************************************************************************/
MERROR
PipelineDAGImp::
findPathBFS(
    NodeId_T            id,
    NodeIdSet_T        &checkList,
    sp<IPipelineDAG>    newDAG
) const
{
    //Used for BFS
    Vector<NodeId_T> queue;
    //
    //To keep track of the path
    KeyedVector<NodeId_T, NodeId_T> parent;
    parent.setCapacity(mNodesVec.size());
    queue.setCapacity(mNodesVec.size());

    //insert initial node
    queue.insertAt(id, 0);
    //BFS
    while(!queue.isEmpty())
    {
        NodeIdSet_T const & inAdj = mNodesVec.valueFor(queue.top()).mInAdj;
        for (size_t i = 0; i < inAdj.size(); i++)
        {
            //If it is inside checkList, it must be reachable from root
            if (0 <= checkList.indexOf(inAdj.itemAt(i)))
            {
                NodeId_T src = inAdj.itemAt(i);
                NodeId_T dst = queue.top();
                //
                //get path and node by traversing parant
                do
                {
                    NodeObj_T const & node = mNodesVec.valueFor(dst).mNode;
                    newDAG->addNode(node.id, node.val);
                    newDAG->addEdge(src, dst);
                    checkList.add(dst);
                    src = dst;
                    //
                    //test ending condition
                    if (dst != id) dst = parent.valueFor(dst);
                } while(src != id);
                return OK;
            }
            //If it is not in checkList, add into queue to do BFS
            parent.add(inAdj.itemAt(i), queue.top());
            queue.insertAt(inAdj.itemAt(i), 0);
        }
        //Finish this node
        queue.pop();
    }
    return -1L;
}


/******************************************************************************
 * Sort vector by inAdj and call BFS, return -1L if no path found
 ******************************************************************************/
MERROR
PipelineDAGImp::
getNodesAndPathsForNewDAG(
    NodeIdSet_T         &orphanNodes,
    NodeIdSet_T         &checkList,
    sp<IPipelineDAG>    newDAG
) const
{
    RWLock::AutoRLock _(mMutex);
    Vector<NodeWithAdj_T> nodesSortedByInAdj;
    nodesSortedByInAdj.setCapacity(orphanNodes.size());
    //
    //Sort orphanNodes by its in-coming nodes in ascending order.
    //In order to minimize number of nodes, we need to make all nodes
    //reachable from root in the new DAG.
    for (size_t i = 0; i < orphanNodes.size(); i++)
    {
        nodesSortedByInAdj.add(mNodesVec.valueFor(orphanNodes.itemAt(i)));
    }
    nodesSortedByInAdj.sort(adjCompare);

    //Do the algorithm with BFS
    for (size_t i = 0; i < nodesSortedByInAdj.size(); i++)
    {
        if (0 > findPathBFS(nodesSortedByInAdj.itemAt(i).mNode.id, checkList, newDAG))
        {
            MY_LOGE("No path found for node ID:%" PRIxPTR, nodesSortedByInAdj.itemAt(i).mNode.id);
            return -1L;
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDAGImp::
dump(android::Vector<android::String8>& rLogs) const
{
    RWLock::AutoRLock _(mMutex);
    //
    rLogs.resize(mNodesVec.size() + 2);
    android::Vector<android::String8>::iterator it = rLogs.begin();
    //
    {
        String8& str = *it++;
        str += String8::format("Toposort:");
        for (size_t i = 0; i < mToposort.size(); i++) {
            str += String8::format(" %#"PRIxPTR"", mToposort[i].id);
        }
    }
    //
    *it++ = String8::format("RootId:%#"PRIxPTR" Nodes:#%zu", mRootId, mNodesVec.size());
    for (size_t i = 0; i < mNodesVec.size(); i++) {
        NodeId_T const nodeId = mNodesVec.keyAt(i);
        NodeWithAdj_T const& node = mNodesVec.valueAt(i);
        //
        String8& str = *it++;
        str += String8::format("[%#"PRIxPTR"] inAdj: ", nodeId);
        for (size_t j = 0; j < node.mInAdj.size(); j++) {
            str += String8::format("%#"PRIxPTR" ", node.mInAdj[j]);
        }
        str += String8::format("outAdj: ");
        for (size_t j = 0; j < node.mOutAdj.size(); j++) {
            str += String8::format("%#"PRIxPTR" ", node.mOutAdj[j]);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDAGImp::
dump() const
{
    Vector<String8> logs;
    dump(logs);
    for (size_t i = 0; i < logs.size(); i++) {
        CAM_LOGD("%s", logs[i].string());
    }
}

