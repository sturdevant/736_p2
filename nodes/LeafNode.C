
#include "LeafNode.h"

/*
 * This file provides implementations for a leaf node. The
 * public functions of leaf nodes are described first.
 */

LeafNode::LeafNode(unsigned int nDims,
                   double epsilon,
                   double minPts,
                   double decay,
                   unsigned long decayRes,
                   unsigned int uniqueId,
                   double ptRemovalThresh,
                   double* mins,
                   double* maxes,
                   double* assignedMins,
                   double* assignedMaxes,
                   ReturnCode (*requestFunc)(unsigned int, Request*),
                   ReturnCode (*responseFunc)(Response*)) {

   this->dims = nDims;
   this->eps = epsilon;
   this->minPts = minPts;
   this->epsSqr = epsilon * epsilon;
   this->uniqueId = uniqueId;
   this->ptRemovalThreshold = ptRemovalThresh;
   this->mins = mins;
   this->decay = decay;
   this->requestFunc = requestFunc;
   this->responseFunc = responseFunc;
   //this->thread = (pthread_t*)malloc(sizeof(pthread_t));

   Point::setDecayFactor(decay, decayRes);
   Point::setEpsilon(eps);

   setLengths(maxes);
   setDimFactors();
   setMaxIndex();
   this->cells = (Cell**)malloc((maxIndex + 1) * sizeof(Cell*));
   for (unsigned int i = 0; i <= maxIndex; i++) {
      cells[i] = new Cell();
   }
   assignCells(assignedMins, assignedMaxes);

   // TODO: (maybe? maybe not) Need to thread out behavior here. We need to make
   // it so that this can run independently of the caller and can accept
   // asynchronous requests and responses.
}

LeafNode::~LeafNode() {
   //pthread_cancel(thread);
   //pthread_join(thread, NULL);
   //free(thread);
   for (unsigned int i = 0; i <= maxIndex; i++) {
      delete cells[i];
   }
   free(cells);
   free(dimFactors);
   free(lengths);
}

/*
 * When a request for data is sent, put it on a queue for the thread to get to.
 */
ReturnCode LeafNode::query(Request* req, Response* res) {
   RequestType t = req->getRequestType();
   Point* pt = req->getPoint();
   //std::cout << "Got query!\n";

   // We want to filter out all of the points not meaningful for our current
   // assignment.
   Cell* c = getPointCell(pt);
   //std::cout << "Got cell!\n";
   if (c == NULL || !c->isAssigned()) {
      //std::cout << "Filtered out point " << pt->getValue()[0] << ", " << pt->getValue()[1] << "\n";
      return RETURN_CODE_NO_ERROR;
   }

   switch(t) {
   case REQUEST_TYPE_ADD_POINT:
      //std::cout << "Calling handleAdd\n";
      return handleAdd(req, res);
   case REQUEST_TYPE_POINT_DATA:
      return handlePoint(req, res);
   case REQUEST_TYPE_POLYGON_DATA:
      return handlePolygon(req, res);
   case REQUEST_TYPE_CLUSTER_REPLACE:
      std::cout << "Got cluster replace request!\n";
      return handleClusterReplaceRequest(req, res);
   case REQUEST_TYPE_SHADOW_UPDATE:
      return handleShadowUpdate(req, res);
   }

   return RETURN_CODE_BAD_REQUEST_TYPE;
}

void LeafNode::snapshot(FILE* ptFile, FILE* assignmentFile) {

   double* d = (double*)malloc(dims * sizeof(double));
   for (unsigned int i = 0; i <= maxIndex; i++) { 

      indexToDArray(i, d);
      if (uniqueId == 10) {
         std::cout << "-->\tSNAPSHOTTING POINT (" << d[0] << ", " << d[1] << ")\n";
      }
      int flag = 0;
      if (cells[i]->isAssigned()) {
         flag = 1;
         if (cells[i]->isShadow()) {
            flag = 2;
         } else if (cells[i]->isFringe()) {
            flag = 3;
         }
      }
      fprintf(assignmentFile, "%lf,%lf,%d\n", d[0], d[1], flag);

      std::vector<Point*>* points = cells[i]->getPointVector();
      for (unsigned int j = 0; j < points->size(); j++) {
         Point* pt = (*points)[j];
         Cluster* clust = pt->getCluster();
         fprintf(ptFile, "%lu:%lf,%lf,%lu,%lf,%lf\n", pt->getTimestamp(), pt->getValue()[0], pt->getValue()[1], clust == NULL ? 0L : clust->getId(), pt->getWeight(pt->getTimestamp()), pt->getNWeight(pt->getTimestamp()));
      }
   }

   free(d);
}

void LeafNode::indexToDArray(unsigned int index, double* d) {
   for (int i = dims - 1; i >= 0; i--) {
      d[i] = mins[i] + eps * (double)(index / dimFactors[i]);
      index = index % dimFactors[i];
      //std::cout << "dim: " << i << " " << d[i] << " next index: " << index << "\n";
   }
}

// PRIVATE INTERFACE BEGINS HERE

// TODO: Thread starting function. (static?)

ReturnCode LeafNode::handleShadowUpdate(Request* req, Response* res) {
   Point* pt = req->getPoint();
   Cell* c = getPointCell(pt);
   
   if (!c->isShadow()) {
      return RETURN_CODE_NO_ERROR;
   }
   std::cout << "Shadow update received!\n";

   double* vals = pt->getValue();
   Cluster* clust = NULL;
   if (req->getLong1() != 0) {
      clust = lookupClusterId(req->getLong1());
      if (clust == NULL) {
         std::cout << "New cluster on boundry!\n";
         clust = makeNewClusterWithId(req->getLong1());
      }
   }
   
   unsigned long time = pt->getTimestamp();

   //pt = new Point(*pt);
   std::vector<Point*>* cellList = c->getPointVector();
   bool foundOriginal = false;
   for (unsigned int i = 0; i < cellList->size() && !foundOriginal; i++) {
      Point* curPt = (*cellList)[i];
      double* curVals = curPt->getValue();
      bool matches = true;
      for (unsigned int j = 0; matches && j < dims; j++) {
         matches &= (curVals[j] == vals[j]);
      }
      if (matches) {

         // If this point matches, we need to add in the proper neighbor
         // weight. If there was no cluster, that's all. Otherwise, we need to
         // check for cluster merging at this location.
         std::cout << "Setting shadow update weight to " << pt->getNWeight(time) << std::endl;
         curPt->setNWeight(pt->getNWeight(time));
         curPt->setTimestamp(time);
         if (clust != NULL) {
            std::cout << "Getting neighbor clusters!\n";
            std::vector<Cluster*> clusters = getNeighborClusters(pt, time);
            clust = pt->getCluster();
            std::cout << "Done getting neighbor clusters.\n";
            if (clusters.size() > 0) {
               // We found clusters to merge here.
               
               Cluster* minClust = clust;
               for (unsigned int j = 0; j < clusters.size(); j++) {
                  std::cout << clusters[j] << " " << minClust << "\n";
                  if (clusters[j]->getId() < minClust->getId()) {
                     minClust = clusters[j];
                  }
                  std::cout << "Checked for new min\n";
               }

               if (minClust != clust) {
                  std::cout << "Issuing replace request\n";
                  issueClusterReplaceRequest(clust, minClust, curPt);
               }

               std::cout << "Replacing locally... (min cluster = " << minClust->getId() << " at " << minClust << ")\n";
               for (unsigned int j = 0; j < clusters.size(); j++) {
                  if (clusters[j] != minClust) {
                     replaceClusterFromPoint(clusters[j], minClust, curPt);
                     std::cout << "(" << uniqueId << ")\thandleShadowUpdate: After replace\n";
                  }
               }

               std::cout << "(" << uniqueId << ")\thandleShadowUpdate: setting point cluster\n";
               setPointCluster(curPt, minClust);
            }
         }
         foundOriginal = true;
      }
   }

   // If we can't find the original point, just add this one in then. This
   // shouldn't really happen though.
   if (!foundOriginal) {
      std::cout << "WARNING: Could not find original point on shadow update!\n";
      handleAdd(req, res);
   }

   return RETURN_CODE_NO_ERROR;
}

void LeafNode::issueShadowUpdateRequest(Point* pt) {
   Request req(REQUEST_TYPE_SHADOW_UPDATE);
   //std::cout << "Creating shadow update request!\n";
   req.setPoint(pt);
   //std::cout << "Setting cluster!\n";
   if (pt->getCluster() != NULL) {
      req.setLong1(pt->getCluster()->getId());
   } else {
      req.setLong1(0);
   }
   //std::cout << "Sending request!\n";
   requestFunc(0, &req);
   //std::cout << "Request sent!\n";
}

ReturnCode LeafNode::handleAdd(Request* req, Response* res) {

   //std::cout << "Getting point cell...\n";
   Cell* c = getPointCell(req->getPoint());
  // if (req->getPoint()->getTimestamp() % 50 == 0) {
      std::cout << "Time is " << req->getPoint()->getTimestamp() << "\n";
   //}
   if (c == NULL) {
      std::cout << "C is null!\n";
      exit(1);
      return RETURN_CODE_NO_ERROR; // TODO: Change this.
   }


   //std::cout << "Checking cell fringe...\n";
   // On the fringe, we only add a point to neighbors, we don't keep it around.
   if (c->isFringe()) {
      //std::cout << "Adding point to fringe...\n";
      addToNeighbors(req->getPoint(), req->getPoint()->getTimestamp());
      //std::cout << "Added point to fringe!\n";
      return RETURN_CODE_NO_ERROR;
   }

   //std::cout << "Adding point to cell...\n";
   Point* pt = new Point(*(req->getPoint()));
   unsigned long time = pt->getTimestamp();

   //std::cout << "Adding point to neighbors...\n";
   addToNeighbors(pt, time);

   //std::cout << "Setting nWeight...\n";
   pt->setNWeight(pt->getWeight(time));

   //std::cout << "Checking if shadow...\n";
   if (!c->isShadow()) {
      double nWeight = countNeighbors(pt, time) + pt->getWeight(time);
      //std::cout << "Counted neighbors!\n";
      pt->setNWeight(nWeight);
      //addToNeighbors(pt, time);

      // Get a list of all of the clusters nearby.
      if (nWeight >= minPts) {
         std::vector<Cluster*> clusters = getNeighborClusters(pt, time);
         if (clusters.size() == 0) {
            std::cout << "Making cluster...\n";
            makeCluster(pt);
         } else if (clusters.size() == 1) {
            std::cout << "Joining cluster...\n";
            addToCluster(clusters[0], pt);
         } else {
            std::cout << "Merging clusters...\n";
            addToCluster(mergeClusters(clusters, pt), pt);
         }
      }

      bool needsShadowUpdate = false;
      std::vector<unsigned int> nIndexes = getPointIndexGroup(pt);
      for (unsigned int i = 0; !needsShadowUpdate && i < nIndexes.size(); i++) {
         if (cells[nIndexes[i]]->isShadow()) {
            needsShadowUpdate = true;
         }
      }

      if (needsShadowUpdate) {
         std::cout << "Point needs shadow update " << pt->getValue()[0] << ", " << pt->getValue()[1] << " NWeight = " << pt->getNWeight(time) << " Cluster = " << (pt->getCluster() == NULL ? 0 : pt->getCluster()->getId()) << std::endl;
         issueShadowUpdateRequest(pt);
         //std::cout << "Done issuing request!\n";
      }
   }

   c->getPointVector()->push_back(pt);

   //std::cout << "Returning...\n";
   return RETURN_CODE_NO_ERROR;
}

ReturnCode LeafNode::handlePoint(Request* req, Response* res) {
   //std::cout << "setting response type...\n";
   res->setType(RESPONSE_TYPE_POINT_QUERY);
   //std::cout << "Setting response value...\n";
   Point* pt = req->getPoint();
   unsigned long time = pt->getTimestamp();
   std::vector<Cluster*> clusters = getNeighborClusters(pt, time);
   res->setValue(countNeighbors(pt, time));
   if (clusters.size() >= 1) {
      res->setClusterId(clusters[0]->getId());
   } else {
      res->setClusterId(0);
   }
   //std::cout << "Returning...\n";
   return RETURN_CODE_NO_ERROR;
}

ReturnCode LeafNode::handlePolygon(Request* req, Response* res) {
   return RETURN_CODE_NO_ERROR;
}

ReturnCode LeafNode::handleClusterReplaceRequest(Request* req, Response* res) {
   std::cout << "Handle cluster replace\n";
   /*
   
   Point* pt = req->getPoint();

   Cluster* oldClust = lookupClusterId(req->getLong1());
   if (oldClust == NULL) {
      std::cout << "Attempt to replace non-existant cluster!\n";
      return RETURN_CODE_NO_ERROR;
   }
   Cluster* newClust = lookupClusterId(req->getLong2());
   if (newClust == NULL) {
      newClust = makeNewClusterWithId(req->getLong2());
   }
   replaceClusterFromPoint(oldClust, newClust, pt);
   */return RETURN_CODE_NO_ERROR;
}

void LeafNode::setPointCluster(Point* pt, Cluster* clust) {
   Cluster* old = pt->getCluster();

   if (old == clust) {
      return;
   }

   if (clust != NULL) {
      clust->addPt(pt);
   }

   if (old != NULL) {
      if (old->removePt(pt) == 0) {
         deleteCluster(old);
      }
   }

   pt->setCluster(clust);
}

void LeafNode::deleteCluster(Cluster* clust) {
   for (unsigned int i = 0; i < clusterList.size(); i++) {
      if (clusterList[i] == clust) {
         clusterList.erase(clusterList.begin() + i);
         std::cout << "(" << uniqueId << ") Deleting cluster " << clust->getId() << " at " << clust << "\n";
         delete clust;
         return;
      }
   }
   delete clust;
   std::cout << "WARNING: TRIED TO DELETE UNREGISTERED CLUSTER!\n";
}

Cluster* LeafNode::makeNewCluster() {
   Cluster* result = new Cluster(getNewClusterId());
   clusterList.push_back(result);
   return result;
}

Cluster* LeafNode::makeNewClusterWithId(unsigned long id) {
   Cluster* result = new Cluster(id);
   clusterList.push_back(result);
   return result;
}

Cluster* LeafNode::lookupClusterId(unsigned long id) {
   for (unsigned int i = 0; i < clusterList.size(); i++) {
      if (clusterList[i]->getId() == id) {
         return clusterList[i];
      }
   }
   Cluster* newClust = new Cluster(id);
   clusterList.push_back(newClust);
   return newClust;
}

void LeafNode::issueClusterReplaceRequest(Cluster* oldClust, Cluster* newClust, Point* pt) {
   std::cout << "Issue cluster replace request!\n";
   Request req(REQUEST_TYPE_CLUSTER_REPLACE,
               pt,
               oldClust->getId(),
               newClust->getId());
   requestFunc(0, &req);
}

unsigned long LeafNode::getNewClusterId() {
   static unsigned int baseId = 0;
   baseId++;
   return (((unsigned long)baseId) << 32) + uniqueId;
}

void LeafNode::makeCluster(Point* pt) {
   //std::cout << "Making cluster at " << pt->getValue()[0] << ", " << pt->getValue()[1] << "\n";
   Cluster* clust = makeNewCluster();
   clust->addCheckPoint(pt);
   setPointCluster(pt, clust);
}

void LeafNode::addToCluster(Cluster* clust, Point* pt) {
   //std::cout << "Adding point to cluster at " << pt->getValue()[0] << ", " << pt->getValue()[1] << "\n";
   setPointCluster(pt, clust);
}

Cluster* LeafNode::mergeClusters(std::vector<Cluster*> clusters, Point* pt) {
   Cluster* clust = clusters[0];
   unsigned int i;
   for (i = 0; i < clusters.size(); i++) {
      if (clusters[i]->getId() < clust->getId()) {
         clust = clusters[i];
      }
   }
   for (i = 0; i < clusters.size(); i++) {
      if (clusters[i] == clust) {
         continue;
      }
      replaceClusterFromPoint(clusters[i], clust, pt);
   }
   return clust;
}

void LeafNode::removeFromCluster(Cluster* clust, Point* pt) {
   setPointCluster(pt, NULL);
}

struct QPointCompare : public std::binary_function<QPoint&, QPoint&, bool> {
   bool operator()(const QPoint& qp1, const QPoint& qp2) const {
      return qp1.dist < qp2.dist;
   }
};

Cluster* LeafNode::verifyCluster(Cluster* clust, Point* pt, unsigned long time) {
   static unsigned long curVisited = 0;
   curVisited++;

   if (pt->getNWeight(time) < minPts) {
      std::cout << "Point fell out of cluster.\n";
      return NULL;
   }

   std::priority_queue<QPoint, std::vector<QPoint>, QPointCompare> ptQueue;
   QPoint qp;// = (QPoint*)malloc(sizeof(QPoint));
   qp.pt = pt;
   std::cout << "Getting distance!\n";
   qp.dist = clust->getSqrDistToCheckPoint(pt, NULL);
   if (qp.dist < epsSqr) {
      return clust;
   }
   ptQueue.push(qp);
   bool done = false;

   while (!ptQueue.empty() && !done) {
      std::cout << "Considering point!\n";
      Point* curPt = ptQueue.top().pt;
      //free(ptQueue.top());
      ptQueue.pop();
      std::vector<std::vector<Point*>*> neighborLists;
      getPointNeighborList(curPt, neighborLists);

      for (unsigned int i = 0; i < neighborLists.size() && !done; i++) {
         std::vector<Point*>* ptList = neighborLists[i];

         for (unsigned int j = 0; j < ptList->size() && !done; j++) {
            
            if ((*ptList)[j]->getVisited() != curVisited && 
                curPt->isNeighbor((*ptList)[j]) && 
                curPt->getNWeight(time) >= minPts) {
               Cluster* curClust = (*ptList)[j]->getCluster();
               (*ptList)[j]->setVisited(curVisited);
               if (clust == curClust) {
                  //qp = (QPoint*)malloc(sizeof(QPoint));
                  qp.pt = (*ptList)[j];
                  std::cout << "Getting qp dist!\n";
                  qp.dist = clust->getSqrDistToCheckPoint(qp.pt, NULL);
                  if (done || qp.dist < epsSqr) {
                     done = true;
                     //free(qp);
                  } else {
                     ptQueue.push(qp);
                  }
               }
            }
         }
      }
   }

   if (done) {
      std::cout << "Returning confirmed cluster!\n";
      //while(!ptQueue.empty()) {
         //free(ptQueue.top());
         //ptQueue.pop();
      //}
      //std::cout << "Cluster verified.\n";
      return clust;
   }

   std::cout << "Cluster removed from point!\n";
   Cluster* newClust = makeNewCluster();
   setPointCluster(pt, newClust);
   std::cout << "New cluster created!\n";
   newClust->addCheckPoint(pt);
   std::cout << "Checkpoint added!\n";
   replaceClusterFromPoint(clust, newClust, pt);
   std::cout << "Returning from split!\n";
   //exit(0);
   
   return newClust;
}

void LeafNode::replaceClusterFromPoint(Cluster* oldClust, Cluster* newClust, Point* pt) {

   if (oldClust == newClust) {
      return;
   }

   std::cout << "Cluster replacement running (" << (oldClust == NULL ? 0 : oldClust->getId()) << " => " << newClust->getId() << ")...\n";
   std::queue<Point*> ptQueue;
   ptQueue.push(pt);

   while (!ptQueue.empty()) {
      Point* curPt = ptQueue.front();
      ptQueue.pop();
      std::vector<std::vector<Point*>*> neighborLists;
      getPointNeighborList(curPt, neighborLists);

      for (unsigned int i = 0; i < neighborLists.size(); i++) {
         std::vector<Point*>* ptList = neighborLists[i];
         //std::cout << "\tGoing through neighbor list!\n";
         
         for (unsigned int j = 0; j < ptList->size(); j++) {
            
            if (curPt->isNeighbor((*ptList)[j])) {
               Cluster* clust = (*ptList)[j]->getCluster();
               if (clust == oldClust) {
                  //std::cout << "\t\tClusters updated\n";
                  setPointCluster((*ptList)[j], newClust);
                  //std::cout << "\t\tPoint updated\n";
                  ptQueue.push((*ptList)[j]);
               }
            }
         }
      }
      //std::cout << "\tMoving to next point\n";
      //neighborLists.clear();
   }
   //std::cout << "Emptying point queue!\n";
   //while (!ptQueue.empty()) {
   //   ptQueue.pop();
   //}
   std::cout << "(" << uniqueId << ")\treplaceClusterFromPoint: return\n";
}

double LeafNode::addToNeighbors(Point* pt, unsigned long time) {
   double count = 0;
   double weight = pt->getWeight(time);
   std::vector<std::vector<Point*>*> neighborLists;
   std::cout << "\t(" << uniqueId << ") Adding point to neighbors\n";
   getPointNeighborList(pt, neighborLists);
   //std::cout << "Counting neighbor points from " << neighborLists.size() << " lists...\n";
   for (unsigned int i = 0; i < neighborLists.size(); i++) {
      //std::cout << "Iterating through point lists...\n";
      std::vector<Point*>* ptList = neighborLists[i];
      if (ptList == NULL) {
         std::cout << "NULL\n";
      }
      //std::cout << "Got point list " << i << " (" << ptList->size() << " points)...\n";
      for (unsigned int j = 0; j < ptList->size(); j++) {
         //std::cout << "Summing points...\n";
         Point* tempPt = (*ptList)[j];
         double w = tempPt->getWeight(time);
         if (w < ptRemovalThreshold) {
            if (tempPt->getCluster() != NULL) {
               tempPt->getCluster()->removePt(tempPt);
            }
            free(tempPt);
            ptList->erase(ptList->begin() + j);
            j--;
         } else if (pt->isNeighbor(tempPt)) {
            if (tempPt->getCluster() == NULL) {
               tempPt->addNWeight(weight, time);
               std::cout << "N weight = " << tempPt->getNWeight(time) << "\n";
               if (tempPt->getNWeight(time) >= minPts) {
                  std::cout << "Point joining cluster!\n";
                  std::vector<Cluster*> clusters = getNeighborClusters(tempPt, time);
                  std::cout << "Got " << clusters.size() << " neighbor clusters!\n";
                  if (clusters.size() == 0) {
                     makeCluster(tempPt);
                  } else if (clusters.size() == 1) {
                     addToCluster(clusters[0], tempPt);
                  } else {
                     addToCluster(mergeClusters(clusters, tempPt), tempPt);
                  }
               }
            } else {
               tempPt->addNWeight(weight, time);
               if (tempPt->getNWeight(time) < minPts) {
                  //std::cout << "Point left cluster!\n";
                  removeFromCluster(tempPt->getCluster(), tempPt);
                  //std::cout << "Point leaving cluster!\n";
               }
            }
            //count += (*ptList)[j]->getWeight(time);
         }
      }
   }
   //std::cout << "Going to return...\n";
   return count;
}

std::vector<Cluster*> LeafNode::getNeighborClusters(Point* pt, unsigned long time) {
   std::vector<Cluster*> result;

   std::vector<std::vector<Point*>*> neighborLists;
   getPointNeighborList(pt, neighborLists);

   //std::cout << "Getting neighboring clusters...\n";
   for (unsigned int i = 0; i < neighborLists.size(); i++) {
      std::vector<Point*>* ptList = neighborLists[i];

      for (unsigned int j = 0; j < ptList->size(); j++) {
         Point* tempPt = (*ptList)[j];
         double w = tempPt->getWeight(time);
         if (w < ptRemovalThreshold) {
            if (tempPt->getCluster() != NULL) {
               tempPt->getCluster()->removePt(tempPt);
            }
            free(tempPt);
            ptList->erase(ptList->begin() + j);
            j--;
         } else if (pt->isNeighbor(tempPt)) {
            Cluster* clust = tempPt->getCluster();
            std::cout << "Found cluster " << clust << "\n";
            if (clust != NULL &&
                std::find(result.begin(), 
                          result.end(), 
                          clust) == result.end()) {
               std::cout << "Verifying cluster...\n";
               Cluster* verifiedCluster = verifyCluster(clust, tempPt, time);
               std::cout << "Verified Cluster! (" << verifiedCluster << ")\n";
               if (verifiedCluster != NULL) {
                  result.push_back(verifiedCluster);
               }
            }
         }
      }
   }

   return result;
}

double LeafNode::countNeighbors(Point* pt, unsigned long time) {
   double count = 0;
   std::vector<std::vector<Point*>*> neighborLists;
   //std::cout << "Getting neighbor list...\n";
   getPointNeighborList(pt, neighborLists);
   //std::cout << "Counting neighbor points from " << neighborLists.size() << " lists...\n";
   for (unsigned int i = 0; i < neighborLists.size(); i++) {
      //std::cout << "Iterating through point lists...\n";
      std::vector<Point*>* ptList = neighborLists[i];
      if (ptList == NULL) {
         std::cout << "NULL\n";
      }
      //std::cout << "Got point list " << i << " (" << ptList->size() << " points)...\n";
      for (unsigned int j = 0; j < ptList->size(); j++) {
         double w = (*ptList)[j]->getWeight(time);
         if (w < ptRemovalThreshold) {
            //if ((*ptList)[j]->getCluster() != NULL) {
            //   (*ptList)[j]->getCluster()->removePt((*ptList)[j]);
            //}
            setPointCluster((*ptList)[j], NULL);
            free((*ptList)[j]);
            ptList->erase(ptList->begin() + j);
            j--;
         } else if (pt->isNeighbor((*ptList)[j])) {
            count += w;
         }
      }
   }
   //std::cout << "\t(countNeighbors): Returning\n";
   return count;
}

void LeafNode::getPointNeighborList(Point* point, std::vector<std::vector<Point*>*>& neighborList) {
   //std::cout << "Getting point cell group...\n";
   std::vector<Cell*> group = getPointCellGroup(point);
   //std::cout << "Filling neighbor vector (up to " << group.size() << ")...\n";

   for (unsigned int i = 0; i < group.size(); i++) {
      if (group[i] == NULL) {
         //std::cout << "ERROR: NULL CELL...\n";
      }
      std::vector<Point*>* list = group[i]->getPointVector();
      if (list != NULL) {
         //std::cout << "Group found...\n";
         neighborList.push_back(list);
      }
   }
}

unsigned int LeafNode::getCellIndex(unsigned int* cell) {
   unsigned int index = 0;
   for (unsigned int i = 0; i < dims; i++) {
      unsigned int cur = cell[i] * dimFactors[i];

      // Check to make sure individual components of cell are within bounds.
      if (cur >= lengths[i] * dimFactors[i]) {
         return UINT_MAX;
      }

      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      return UINT_MAX;
   }
   return index;
}

Cell* LeafNode::getCell(unsigned int* cell) {
   unsigned int index = 0;
   for (unsigned int i = 0; i < dims; i++) {
      unsigned int cur = cell[i] * dimFactors[i];

      // Check to make sure individual components of cell are within bounds.
      if (cur >= lengths[i] * dimFactors[i]) {
         return NULL;
      }

      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      return NULL;
   }
   return cells[index];
}

void LeafNode::getIndexGroupRecursive(unsigned int* cell,
                                      unsigned int nDims,
                                      std::vector<unsigned int>& result) {

   // We need to check the cells below and above this one in the current
   // dimension.
   unsigned int curDim = dims - nDims;
   unsigned int low = cell[curDim] - 1;
   unsigned int high = cell[curDim] + 1;

   if (low == UINT_MAX) {
      low = cell[curDim];
   }

   if (high == lengths[curDim]) {
      high = cell[curDim];
   }

   //std::cout << "Assessing dimension " << curDim << " nDims = " << nDims << " low = " << low << " high = " << high << std::endl;
   // If there are no other dimensions left to check, see who is owner of
   // nearby cells.
   if (nDims == 1) {
      for (int i = low; i <= high; i++) {
         cell[curDim] = i;
         result.push_back(getCellIndex(cell));
      }

   // If there are more dimensions to check, recursively examine those
   // dimensions, building on the current owner vector.
   } else {
      for (unsigned int i = low; i <= high; i++) {
         cell[curDim] = i;
         getIndexGroupRecursive(cell, nDims - 1, result);
      }
   }

   cell[curDim]--;
}

void LeafNode::getCellGroupRecursive(unsigned int* whichDims, 
                                     unsigned int* cell,
                                     std::vector<Cell*>& result,
                                     unsigned int nDims) {

   // We need to check the cells below and above this one in the current
   // dimension.
   unsigned int curDim = whichDims[0];
   unsigned int low = cell[curDim] - 1;
   unsigned int high = cell[curDim] + 1;

   if (low == UINT_MAX) {
      low = cell[curDim];
   }

   if (high == lengths[curDim]) {
      high = cell[curDim];
   }

   //std::cout << "Assessing dimension " << curDim << " nDims = " << nDims << " low = " << low << " high = " << high << std::endl;
   // If there are no other dimensions left to check, see who is owner of
   // nearby cells.
   if (nDims == 1) {
      for (int i = low; i <= high; i++) {
         cell[curDim] = i;
         result.push_back(getCell(cell));
      }

   // If there are more dimensions to check, recursively examine those
   // dimensions, building on the current owner vector.
   } else {
      for (unsigned int i = low; i <= high; i++) {
         cell[curDim] = i;
         getCellGroupRecursive(&whichDims[1], cell, result, nDims - 1);
      }
   }

   cell[curDim]--;
}

std::vector<unsigned int> LeafNode::getPointIndexGroup(Point* point) {

   std::vector<unsigned int> result;
   double* pt = point->getValue();
   unsigned int index = 0;

   // Construct an integer representation for the cell that the point is in.
   unsigned int* cell = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < dims; i++) {
      cell[i] = (unsigned int)floor((pt[i] - mins[i]) / eps);
   }

   // Now, begin the recursive calls to find nearby cells.
   getIndexGroupRecursive(cell, dims, result);
   free(cell);
   return result;
}

std::vector<Cell*> LeafNode::getPointCellGroup(Point* point) {

   std::vector<Cell*> result;
   double* pt = point->getValue();
   unsigned int index = 0;

   // Build a list of dimensions to recurse over.
   unsigned int* whichDims = (unsigned int*)malloc(dims * sizeof(unsigned int));

   // Construct an integer representation for the cell that the point is in.
   unsigned int* cell = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < dims; i++) {
      cell[i] = (unsigned int)floor((pt[i] - mins[i]) / eps);
      whichDims[i] = i;
   }

   // Now, begin the recursive calls to find nearby cells.
   getCellGroupRecursive(whichDims, cell, result, dims);
   free(whichDims);
   free(cell);
   return result;
}

void LeafNode::setLengths(double* maxes) {
   lengths = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < dims; i++) {
      lengths[i] = ceil((maxes[i] - mins[i]) / eps);
   }
}

void LeafNode::setDimFactors() {
   dimFactors = (unsigned int*)malloc(dims * sizeof(unsigned int));
   dimFactors[0] = 1;
   for (unsigned int i = 1; i < dims; i++) {
      dimFactors[i] = dimFactors[i - 1] * lengths[i - 1];
   }
}

void LeafNode::setMaxIndex() {
   maxIndex = dimFactors[dims - 1] * lengths[dims - 1] - 1;
}

Cell* LeafNode::getPointCell(Point* point) {
   double* pt = point->getValue();
   unsigned int index = 0;


   for (unsigned int i = 0; i < dims; i++) {
      unsigned int cur = ((unsigned int)floor((pt[i] - mins[i]) / eps))
                         * dimFactors[i];

      // Check to make sure individual components are within bounds.
      if (cur >= lengths[i] * dimFactors[i]) {
         return NULL;
      }
      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      return NULL;
   }
   return cells[index];
}

void LeafNode::assignPointCell(Point* pt) {
   //std::cout << "Assigning (" << pt->getValue()[0] << ", " << pt->getValue()[1] << ")\n";
   std::vector<unsigned int> group = getPointIndexGroup(pt);
   //std::cout << "\t\tFound " << group.size() << " points\n";
   unsigned int gsize = group.size();
   for (unsigned int i = 0; i < gsize; i++) {
      //std::cout << "\t\tHandling point " << i << " of " << gsize << "\n";
      unsigned int index = group[i];
      double* subPt = (double*)malloc(dims * sizeof(double));
      indexToDArray(index, subPt);
      Point pt2(subPt, 0, 0);
      std::vector<unsigned int> subGroup = getPointIndexGroup(&pt2);
      for (unsigned int j = 0; j < subGroup.size(); j++) {
         //std::cout << "\t\t\tSubgroup pt #" << j << " of " << subGroup.size() << "\n";
         if (!cells[subGroup[j]]->isAssigned()) {
            cells[subGroup[j]]->setFringe(true);
         }
      }
      if (!cells[index]->isAssigned() || cells[index]->isFringe()) {
         cells[index]->setShadow(true);
      }
      free(subPt);
      //std::cout << "\t\tStarting next round of points (" << i + 1 << " of " << gsize << ")!\n";
   }
   //std::cout << "\t\tFINISHED POINT CELL\n";
   Cell* c = getPointCell(pt);
   c->setAssigned(true);
   c->setShadow(false);
   c->setFringe(false);
   if (uniqueId == 10) {
      if (getPointCell(pt)->isAssigned()) {
         std::cout << "-->\tPoint assigned!\n";
      }
   }
}

void LeafNode::recursiveAssignSubslice(double* low, double* high, double* cur, unsigned int nDims) {
   unsigned int curDim = dims - nDims;
   if (nDims == 1) {
      for (cur[curDim] = low[curDim]; cur[curDim] < high[curDim]; cur[curDim] += eps) {
         // TODO: When the point goes out of scope, it dies! careful!
         Point pt(cur, 0, 0);
         //std::cout << "Point at: " << cur[0] << ", " << cur[1] << "\n";
         /*
         std::vector<Cell*> group = getPointCellGroup(&pt);
         for (unsigned int i = 0; i < group.size(); i++) {
            if (!group[i]->isAssigned() || group[i]->isFringe()) {
               //group[i]->setAssigned(true);
               group[i]->setShadow(true);
            }
         }
         */
         if (uniqueId == 10) {
            std::cout << "-->\tASSINGING POINT TO NODE 10 (" << cur[0] << ", " << cur[1] << ")!\n";
         }
         assignPointCell(&pt);/*
         std::vector<unsigned int> group = getPointIndexGroup(&pt);
         for (unsigned int i = 0; i < group.size(); i++) {
            unsigned int index = group[i];
            double* subPt
            Point p2();
         Cell* c = getPointCell(&pt);
         c->setAssigned(true);
         c->setShadow(false);
         c->setFringe(false);
         */
      }
   } else {
      for (cur[curDim] = low[curDim]; cur[curDim] < high[curDim]; cur[curDim] += eps) {
         recursiveAssignSubslice(low, high, cur, nDims - 1);
      }
   }
}

void LeafNode::assignCells(double* low, double* high) {
   double* tempPt = (double*)malloc(dims * sizeof(double));
   std::cout << "Assigning cells...\n";
   recursiveAssignSubslice(low, high, tempPt, dims);
   std::cout << "Done assigning cells...\n";
   free(tempPt);
}
