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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINEDAG_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINEDAG_H_
//
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/List.h>
#include <utils/SortedVector.h>
#include <mtkcam/common.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * Type of Camera Pipeline Node Id.
 */
typedef MINTPTR                     Pipeline_NodeId_T;


/**
 * An interface of pipeline directed acyclic graph.
 */
class IPipelineDAG
    : public virtual android::RefBase
{
public:     ////                            Definitions.

    template<class _NodeId_T_, class _NodeVal_T_>
    struct TNodeObj
    {
        _NodeId_T_                          id;
        _NodeVal_T_                         val;
                                            //
                                            TNodeObj()
                                                : id(-1L)
                                                , val(-1L)
                                            {}
    };

    typedef ssize_t                         NodeVal_T;      //Node Value
    typedef Pipeline_NodeId_T               NodeId_T;       //Node Id
    typedef android::SortedVector<NodeId_T> NodeIdSet_T;    //Node Id Set
    typedef TNodeObj<NodeId_T, NodeVal_T>   NodeObj_T;      //Node Object
    typedef android::Vector<NodeObj_T>      NodeObjSet_T;   //Node Object Set

    struct Edge
    {
        NodeId_T                            src;
        NodeId_T                            dst;
                                            Edge(NodeId_T src = -1L, NodeId_T dst = -1L)
                                                : src(src)
                                                , dst(dst)
                                            {}
    };

public:     ////                            Operations.

    /**
     * Create the graph.
     *
     * @return
     *      A pointer to a newly-created instance.
     */
    static  IPipelineDAG*                   create();

    /**
     * Clone the graph.
     *
     * @return
     *      A pointer to a newly-cloned instance.
     */
    virtual IPipelineDAG*                   clone() const                   = 0;

    /**
     * Clone the graph, with a given set of nodes reserved.
     * Any node beyond the given set of nodes must be removed.
     *
     * @param[in] ids: a set of node ids.
     *
     * @return
     *      A pointer to a newly-cloned instance.
     */
    virtual IPipelineDAG*                   clone(
                                                NodeIdSet_T const& ids
                                            ) const                         = 0;

    /**
     * Add a node to the graph.
     *
     * @param[in] id: node id
     *
     * @param[in] val: node value
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          addNode(
                                                NodeId_T id,
                                                NodeVal_T const& val = -1L
                                            )                               = 0;

    /**
     * Remove a node and its associated edge.
     *
     * @param[in] id: node id
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          removeNode(
                                                NodeId_T id
                                            )                               = 0;

    /**
     * Add a directed edge to the graph, where a directed edge is from a given
     * source node to a given destination node.
     *
     * @param[in] id_src: the node id of source.
     *
     * @param[in] id_dst: the node id of destination.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          addEdge(
                                                NodeId_T id_src,
                                                NodeId_T id_dst
                                            )                               = 0;


    /**
     * Remove a directed edge from the graph, where a directed edge is from a
     * given source node to a given destination node.
     *
     * @param[in] id_src: the node id of source.
     *
     * @param[in] id_dst: the node id of destination.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          removeEdge(
                                                NodeId_T id_src,
                                                NodeId_T id_dst
                                            )                               = 0;

    /**
     * Set a node as the root of the graph.
     *
     * @param[in] id: node id
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          setRootNode(
                                                NodeId_T id
                                            )                               = 0;

    /**
     * Set the value of a specified node.
     *
     * @param[in] id: node id
     *
     * @param[in] val: node value
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          setNodeValue(
                                                NodeId_T id,
                                                NodeVal_T const& val
                                            )                               = 0;

    /**
     * Dump.
     */
    virtual MVOID                           dump() const                    = 0;
    virtual MVOID                           dump(
                                                android::Vector<android::String8>& rLogs
                                            ) const                         = 0;

public:     ////                            Attributes.

    /**
     * Get nodes and paths that are needed to make nodes inside the new DAG
     * reachable from root.
     *
     * @param[in]  orphanNodes: IDs of nodes that are not reachable from root
     *
     * @param[in]  checkList: Nodes that are reachable from root
     *
     * @param[out] newDAG: DAG with nodes and edges inserted
     *
     * @return
     *      0 indicates success; otherwise, errors.
     */
    virtual MERROR                          getNodesAndPathsForNewDAG(
                                                NodeIdSet_T                 &orphanNodes,
                                                NodeIdSet_T                 &checkList,
                                                android::sp<IPipelineDAG>    newDAG
                                            ) const                         = 0;

    /**
     * Get nodes that are not reachable from root.
     *
     * @param[out] orphanNodes: IDs of nodes that are not reachable from root
     *
     * @param[out] connectedNodes: IDs of nodes that are reachable from root
     *
     * @return
     *      0 indicates success; otherwise, errors.
     *
     */
    virtual MERROR                          getOrphanNodes(
                                                NodeIdSet_T &orphanNodes,
                                                NodeIdSet_T &connectedNodes
                                            ) const                         = 0;

    /**
     * Get the topological order list of the graph.
     *
     * @param[out] result: list of node obj in topological order
     *
     * @return
     *      0 indicates success; -1L indicates cycle exists in graph.
     */

    virtual MERROR                          getTopological(
                                                android::List<NodeObj_T>& result
                                            ) const                         = 0;

    /**
     * Get the topological sort of the graph.
     *
     * @return
     *      An empty vector indicates that the graph is cyclic.
     */
    virtual android::Vector<NodeObj_T>const&getToposort() const             = 0;

    /**
     * Get the root node of the graph.
     *
     * @return
     *      a node obj type indicates the root node
     */
    virtual NodeObj_T                       getRootNode() const             = 0;

    /**
     * Get node of certain id of the graph.
     *
     * @param[in] id: node id
     *
     * @return
     *      a node obj type of id node of the graph.
     */
    virtual NodeObj_T                       getNode(NodeId_T id) const      = 0;


    /**
     * Get edges of IDs of the graph.
     *
     * @param[in] ids: node id set
     *
     * @param[out] result: vector of edges related to those node
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          getEdges(
                                                android::Vector<Edge>& result
                                            ) const      = 0;


    /**
     * Get the number of nodes of the graph.
     *
     * @return
     *      size of the graph in size_t.
     */
    virtual size_t                          getNumOfNodes() const           = 0;

    /**
     * Get the in-coming nodes of node id.
     *
     * @param[in] id: node id
     *
     * @param[out] result: vector of node obj type of the in-coming nodes
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          getInAdjacentNodes(
                                                NodeId_T id,
                                                NodeObjSet_T& result
                                            ) const                         = 0;

    /**
     * Get the out-going nodes of node id.
     *
     * @param[in] id: node id
     *
     * @param[out] result: vector of node obj type of the out-going nodes
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                          getOutAdjacentNodes(
                                                NodeId_T id,
                                                NodeObjSet_T& result
                                            ) const                         = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINEDAG_H_

