
#include "LeafNode.h"

/*
 * This file provides implementations for a leaf node. The
 * public functions of leaf nodes are described first.
 */

LeafNode::LeafNode(unsigned int nDims,
                   double epsilon,
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
   this->epsSqr = epsilon * epsilon;
   this->uniqueId = uniqueId;
   this->ptRemovalThreshold = ptRemovalThresh;
   this->mins = mins;
   this->requestFunc = requestFunc;
   this->responseFunc = responseFunc;
   //this->thread = (pthread_t*)malloc(sizeof(pthread_t));

   setLengths(maxes);
   setDimFactors();
   setMaxIndex();
   //initPointLists();
   this->cells = (Cell**)malloc(maxIndex * sizeof(Cell*));
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
   free(dimFactors);
   free(lengths);
}

/*
 * When a request for data is sent, put it on a queue for the thread to get to.
 */
ReturnCode LeafNode::query(Request* req, Response* res) {
   RequestType t = req->getRequestType();
   Point* pt = req->getPoint();

   // We want to filter out all of the points not meaningful for our current
   // assignment.
   Cell* c = getPointCell(pt);
   if (c == NULL || !c->isAssigned()) {
      std::cout << "Filtered out point " << pt->getValue()[0] << ", " << pt->getValue()[1] << "\n";
      return RETURN_CODE_NO_ERROR;
   }

   switch(t) {
   case REQUEST_TYPE_UPDATE_POINT:
      return handleUpdate(req, res);
   case REQUEST_TYPE_ADD_POINT:
      return handleAdd(req, res);
   case REQUEST_TYPE_POINT_DATA:
      return handlePoint(req, res);
   case REQUEST_TYPE_POLYGON_DATA:
      return handlePolygon(req, res);
   }

   return RETURN_CODE_BAD_REQUEST_TYPE;
}

void LeafNode::snapshot(FILE* ptFile, FILE* assignmentFile) {

   double* d = (double*)malloc(dims * sizeof(double));
   for (unsigned int i = 0; i <= maxIndex; i++) { 

      indexToDArray(i, d);
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
         fprintf(ptFile, "%lu:%lf,%lf,%lu,%lf\n", pt->getTimestamp(), pt->getValue()[0], pt->getValue()[1], clust == NULL ? 0L : clust->getId(), pt->getWeight(pt->getTimestamp()));
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

ReturnCode LeafNode::handleUpdate(Request* req, Response* res) {
   return RETURN_CODE_NO_ERROR;
}

ReturnCode LeafNode::handleAdd(Request* req, Response* res) {

   //std::cout << "Getting point cell...\n";
   Cell* c = getPointCell(req->getPoint());

   if (c == NULL) {
      std::cout << "C is null!\n";
      exit(1);
      return RETURN_CODE_NO_ERROR; // TODO: Change this.
   }

   // On the fringe, we only add a point to neighbors, we don't keep it around.
   if (c->isFringe()) {
      addToNeighbors(req->getPoint(), req->getPoint()->getTimestamp());
      return RETURN_CODE_NO_ERROR;
   }

   //std::cout << "Adding point to cell...\n";
   Point* pt = new Point(*(req->getPoint()));
   unsigned long time = pt->getTimestamp();
   double nWeight = countNeighbors(pt, time);
   pt->setNWeight(nWeight + pt->getWeight(time));
   addToNeighbors(pt, time);

   // Get a list of all of the clusters nearby.
   if (nWeight > MIN_PTS) {
      std::vector<Cluster*> clusters = getNeighborClusters(pt, time);
      if (clusters.size() == 0) {
         makeCluster(pt);
      } else if (clusters.size() == 1) {
         addToCluster(clusters[0], pt);
      } else {
         pt->setCluster(mergeClusters(clusters, pt));
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
   res->setValue(countNeighbors(req->getPoint(), req->getPoint()->getTimestamp()));
   //std::cout << "Returning...\n";
   return RETURN_CODE_NO_ERROR;
}

ReturnCode LeafNode::handlePolygon(Request* req, Response* res) {
   return RETURN_CODE_NO_ERROR;
}

unsigned long LeafNode::getNewClusterId() {
   static unsigned int baseId = 0;
   baseId++;
   return ((unsigned long)uniqueId) << 32 + baseId;
}

void LeafNode::makeCluster(Point* pt) {
   std::cout << "Making cluster at " << pt->getValue()[0] << ", " << pt->getValue()[1] << "\n";
   Cluster* clust = new Cluster(pt, getNewClusterId());
   pt->setCluster(clust);
}

void LeafNode::addToCluster(Cluster* clust, Point* pt) {
   std::cout << "Adding point to cluster at " << pt->getValue()[0] << ", " << pt->getValue()[1] << "\n";
   clust->addPt(pt);
   pt->setCluster(clust);
}

Cluster* LeafNode::mergeClusters(std::vector<Cluster*> clusters, Point* pt) {
   unsigned int maxCount = 0;
   unsigned int index = -1;
   for (unsigned int i = 0; i < clusters.size(); i++) {
      if (clusters[i]->getPtCount() > maxCount) {
         index = i;
         maxCount = clusters[i]->getPtCount();
      }
   }
   for (unsigned int i = 0; i < clusters.size(); i++) {
      if (i == index) {
         continue;
      }
      replaceClusterFromPoint(clusters[i], clusters[index], pt);
   }
   return clusters[index];
}

void LeafNode::removeFromCluster(Cluster* clust, Point* pt) {
   clust->removePt(pt);
   pt->setCluster(NULL);
}

struct QPointCompare : public std::binary_function<QPoint&, QPoint&, bool> {
   bool operator()(const QPoint& qp1, const QPoint& qp2) const {
      return qp1.dist < qp2.dist;
   }
};

bool LeafNode::verifyCluster(Cluster* clust, Point* pt) {
   static unsigned long curVisited = 0;
   curVisited++;

   // TODO: Implement body and use priority queue with distance to base.
   std::priority_queue<QPoint, std::vector<QPoint>, QPointCompare> ptQueue;
   QPoint qp;// = (QPoint*)malloc(sizeof(QPoint));
   qp.pt = pt;
   qp.dist = clust->getSqrDistToCheckPoint(pt, NULL);
   ptQueue.push(qp);
   bool done = false;

   while (!ptQueue.empty() && !done) {
      Point* curPt = ptQueue.top().pt;
      //free(ptQueue.top());
      ptQueue.pop();
      std::vector<std::vector<Point*>*> neighborLists;
      getPointNeighborList(curPt, neighborLists);

      for (unsigned int i = 0; i < neighborLists.size() && !done; i++) {
         std::vector<Point*>* ptList = neighborLists[i];

         for (unsigned int j = 0; j < ptList->size() && !done; j++) {
            
            if ((*ptList)[j]->getVisited() != curVisited && curPt->isNeighbor((*ptList)[j])) {
               Cluster* curClust = (*ptList)[j]->getCluster();
               (*ptList)[j]->setVisited(curVisited);
               if (clust == curClust) {
                  //qp = (QPoint*)malloc(sizeof(QPoint));
                  qp.pt = (*ptList)[j];
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
      //while(!ptQueue.empty()) {
         //free(ptQueue.top());
         //ptQueue.pop();
      //}
      return true;
   }
   return false;
}

void LeafNode::replaceClusterFromPoint(Cluster* oldClust, Cluster* newClust, Point* pt) {

   if (oldClust == newClust) {
      return;
   }

   std::queue<Point*> ptQueue;
   ptQueue.push(pt);

   while (!ptQueue.empty()) {
      Point* curPt = ptQueue.front();
      ptQueue.pop();
      std::vector<std::vector<Point*>*> neighborLists;
      getPointNeighborList(curPt, neighborLists);

      for (unsigned int i = 0; i < neighborLists.size(); i++) {
         std::vector<Point*>* ptList = neighborLists[i];

         for (unsigned int j = 0; j < ptList->size(); j++) {
            
            if (curPt->isNeighbor((*ptList)[j])) {
               Cluster* clust = (*ptList)[j]->getCluster();
               if (clust == oldClust) {
                  oldClust->removePt((*ptList)[j]);
                  newClust->addPt((*ptList)[j]);
                  (*ptList)[j]->setCluster(newClust);
                  ptQueue.push((*ptList)[j]);
               }
            }
         }
      }
   }
}

double LeafNode::addToNeighbors(Point* pt, unsigned long time) {
   double count = 0;
   double weight = pt->getWeight(time);
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
               if (tempPt->getNWeight(time) > MIN_PTS) {
                  std::cout << "Point joining cluster!\n";
                  std::vector<Cluster*> clusters = getNeighborClusters(tempPt, time);
                  if (clusters.size() == 0) {
                     makeCluster(tempPt);
                  } else if (clusters.size() == 1) {
                     addToCluster(clusters[0], tempPt);
                  } else {
                     tempPt->setCluster(mergeClusters(clusters, tempPt));
                  }
               }
            } else {
               tempPt->addNWeight(weight, time);
               if (tempPt->getNWeight(time) < MIN_PTS) {
                  removeFromCluster(tempPt->getCluster(), tempPt);
                  std::cout << "Point leaving cluster!\n";
               }
            }
            //count += (*ptList)[j]->getWeight(time);
         }
      }
   }
   return count;
}

std::vector<Cluster*> LeafNode::getNeighborClusters(Point* pt, unsigned long time) {
   std::vector<Cluster*> result;

   std::vector<std::vector<Point*>*> neighborLists;
   getPointNeighborList(pt, neighborLists);

   std::cout << "Getting neighboring clusters...\n";
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
               if (verifyCluster(clust, tempPt)) {
                  std::cout << "Cluster verified " << clust << "\n";
                  result.push_back(clust);
               } else {
                  std::cout << "Could not verify cluster " << clust << "\n";
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
            if ((*ptList)[j]->getCluster() != NULL) {
               (*ptList)[j]->getCluster()->removePt((*ptList)[j]);
            }
            free((*ptList)[j]);
            ptList->erase(ptList->begin() + j);
            j--;
         } else if (pt->isNeighbor((*ptList)[j])) {
            count += w;
         }
      }
   }
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
   
   std::vector<unsigned int> group = getPointIndexGroup(pt);
   for (unsigned int i = 0; i < group.size(); i++) {
      unsigned int index = group[i];
      double* subPt = (double*)malloc(dims * sizeof(double));
      indexToDArray(index, subPt);
      Point pt2(subPt, 0, 0);
      std::vector<Cell*> subGroup = getPointCellGroup(&pt2);
      for (unsigned int j = 0; j < subGroup.size(); j++) {
         if (!subGroup[j]->isAssigned()) {
            subGroup[j]->setFringe(true);
         }
      }
      if (!cells[index]->isAssigned() || cells[index]->isFringe()) {
         cells[index]->setShadow(true);
      }
   }
   Cell* c = getPointCell(pt);
   c->setAssigned(true);
   c->setShadow(false);
   c->setFringe(false);
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
