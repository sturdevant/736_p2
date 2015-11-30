
#include "InternalNode.h"

/*
 * This file provides implementations for an internal node (non-leaf). The
 * public functions of internal nodes are described first.
 */

InternalNode::InternalNode(unsigned int nDims,
                           unsigned int nChildren,
                           double epsilon,
                           double minPts,
                           double removalThreshold,
                           double decayFactor,
                           unsigned long decayRes,
                           double* mins,
                           double* maxes,
                           double* aMins,
                           double* aMaxes,
                           ReturnCode (*requestFunc)(unsigned int, Request*),
                           ReturnCode (*responseFunc)(Response*),
                           double* caMins,
                           double* caMaxes) {
   this->dims = nDims;
   this->nChildren = nChildren;
   this->removalThreshold = removalThreshold;
   this->decayFactor = decayFactor;
   this->decayRes = decayRes;
   this->eps = epsilon;
   this->maxes = (double*)malloc(nDims * sizeof(double));
   this->mins = (double*)malloc(nDims * sizeof(double));
   bcopy(maxes, this->maxes, nDims * sizeof(double));
   bcopy(mins, this->mins, nDims * sizeof(double));
   this->requestFunc = requestFunc;
   this->responseFunc = responseFunc;
   //this.thread = (pthread_t*)malloc(sizeof(pthread_t));

   //std::cout << "Assigned " << aMins[0] << " to " << aMaxes[0] << " X\n";
   //std::cout << "Assigned " << aMins[1] << " to " << aMaxes[1] << " Y\n";

   setLengths();
   setDimFactors();
   setMaxIndex();
   this->childIndexes = 
      (std::priority_queue<QIndex, std::vector<QIndex>, QIndexCompare>**)malloc
      (nChildren * 
      sizeof(std::priority_queue<QIndex, std::vector<QIndex>, QIndexCompare>));

   this->childCenters = (Point**)malloc(nChildren * sizeof(Point**));
   for (unsigned int i = 0; i < nChildren; i++) {
      childIndexes[i] = new std::priority_queue<QIndex, std::vector<QIndex>, QIndexCompare>();
   }
   this->owners = (unsigned int*)malloc((maxIndex + 1) * sizeof(unsigned int));
   for (int i = 0; i <= maxIndex; i++) {
      owners[i] = nChildren;
   }
   //std::cout << "(" << aMins[0] << ", " << aMaxes[0] << ")\tPreset owner[" << 56 << "] " << owners[56] << "\n";
   //std::cout << "owners\n";
   assignOwners(aMins, aMaxes, caMins, caMaxes);
   //std::cout << "returning\n";

   // TODO: (maybe? maybe not) Need to thread out behavior here. We need to make
   // it so that this can run independently of the caller and can accept
   // asynchronous requests and responses.
   //for (int i = 0; i <= maxIndex; i++) {
      //std::cout << "(" << aMins[0] << ", " << aMaxes[0] << ")\tOwner[" << 56 << "] " << owners[56] << "\n";
   //}
}

InternalNode::~InternalNode() {
   //pthread_cancel(thread);
   //pthread_join(thread, NULL);
   //free(thread);
   free(dimFactors);
   free(lengths);
   free(owners);
}

// TODO: Filter properly for each child.
bool InternalNode::admitPoint(Point* pt) {
   /*std::cout << "\tPoint considered: " 
             << pt->getValue()[0] << ", " << pt->getValue()[1] 
             << " (owned by child " << getPointOwner(pt->getValue()) 
             << " / " << nChildren << ")\n";
   */return getPointOwner(pt->getValue()) != nChildren;
}

/*
 * When a point is added, we need to see if we should be aggregating data here
 * (in the case of multiple children having data about the point), or if we
 * should just send the point downstream because one of our children has all of
 * the data on its own.
 */
ReturnCode InternalNode::addPoint(Request* req) {
   /*
   // Verify that the request was actually the type that this function can
   // handle.
   RequestType t = req->getRequestType();
   if (t != REQUEST_TYPE_ADD_POINT) {
      return ERROR_CODE_BAD_REQUEST_TYPE;
   }

   // Get the point origin of the request.
   double* pt = req->getPoint();

   unsigned int ptOwner = getPointOwner(pt);
   return (*requestFunc)(ptOwner, req);*/
   return RETURN_CODE_NO_ERROR;
}

// TODO: Consider combining query and addPoint.
/*
 * When a request for data is sent, put it on a queue for the thread to get to.
 */
ReturnCode InternalNode::query(Request* req) {
   // Verify that the request was actually the type that this function can
   // handle.
   /*
   RequestType t = req->getRequestType();
   if (t == REQUEST_TYPE_ADD_POINT || t == REQUEST_TYPE_INVALID) {
      return ERROR_CODE_BAD_REQUEST_TYPE;
   }
   double* pt = req->getPoint();

   unsigned int ptOwner = getPointOwner(pt);
   return (*requestFunc)(ptOwner, req);*/
   return RETURN_CODE_NO_ERROR;
}

/*
 * When this node is set to recieve information from down stream about one of
 * its requests, this function is called with the response.
 */
ReturnCode InternalNode::update(Response* res) {
/*
   ResponseType t = res->getResponseType();
   if (t != RESPONSE_TYPE_ADD_POINT) {
      return RETURN_CODE_NO_ERROR;
   }

   // TODO: Send new info to lower nodes.
   unsigned long timestamp = res->getTimeStamp();
   double* pt = res->getPoint();
   unsigned long id = res->getID();

   unsigned int owner = getPointOwner(pt);
   std::vector<unsigned int> ptOwners;
   getPointOwnerGroup(pt, &ptOwners);

   // If there was more than one owner, we need to send this request down the
   // line to multiple nodes for storing in their shadow regions.
   if (ptOwners.size() > 1) {

      // Generate a new request, this time as an update, to send to all of the
      // nodes that need to know about this point (owners of it).
      Request* newReq = new Request(REQUEST_TYPE_UPDATE_POINT);
      newReq->setTimeStamp(timestamp);
      newReq->setPoint(pt);
      newReq->setID(res->getID());

      // Set up the return value, hoping for no trouble.
      ReturnCode err = RETURN_CODE_NO_ERROR;

      // Go through and send an update request to each of the lower nodes.
      for (unsigned int i = 0; i < ptOwners.size(); i++) {
         if (ptOwners[i] != owner) {
            err = (*requestFunc)(ptOwners[i], newReq);
            if (err != RETURN_CODE_NO_ERROR) {
               return err;
            }
         }
      }
      delete newReq;
   }
*/
   return RETURN_CODE_NO_ERROR;
}

// PRIVATE INTERFACE BEGINS HERE

// TODO: Thread starting function. (static?)

/*
 * This function returns the integer that identifies a child node (called an
 * "owner". Owners are child nodes that are known to possess data about
 * incoming point to a cell. For example, an owner of some point also contains
 * the nearby points and can cluster easily.
 */
unsigned int InternalNode::getCellOwner(unsigned int* cell) {
   unsigned int index = 0;
   for (unsigned int i = 0; i < dims; i++) {
      unsigned int cur = cell[i] * dimFactors[i];


      // Check to make sure individual components of cell are within bounds.
      if (cur >= lengths[i]) {
         return nChildren;
      }

      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      return nChildren;
   }
   return owners[index];
}

/*
 * A cell owner group is the vector of all integers identifying children with
 * data about NEARBY points. Basically, once you ask every owner in the group
 * about a point, you have collected ALL data about nearby points and you can
 * properly cluster. A cell is just an integer representation of boxes that
 * points fit in. They are effectively indexes into data about points.
*/

void InternalNode::snapshot(FILE* assignmentFile) {

   double* d = (double*)malloc(dims * sizeof(double));
   for (unsigned int i = 0; i <= maxIndex; i++) {

      indexToDArray(i, d);
      fprintf(assignmentFile, "%lf,%lf,%d\n", d[0], d[1], owners[i]);
   }
   free(d);
}

void InternalNode::indexToDArray(unsigned int index, double* d) {
   for (int i = dims - 1; i >= 0; i--) {
      d[i] = mins[i] + eps * (double)(index / dimFactors[i]);
      index = index % dimFactors[i];
   }
}

void InternalNode::getCellOwnerGroup(unsigned int* whichDims, 
                                 unsigned int* cell,
                                 std::vector<unsigned int>& uniqueOwners,
                                 unsigned int nDims) {

   // We need to check the cells below and above this one in the current
   // dimension.
   unsigned int curDim = whichDims[0];
   unsigned int low = cell[curDim - 1];
   unsigned int high = cell[curDim + 1];

   // If there are no other dimensions left to check, see who is owner of
   // nearby cells.
   if (nDims = 0) {
      for (int i = low; i <= high; i++) {
         cell[curDim] = i;
         unsigned int curOwner = getCellOwner(cell);
         
         // Check if the list of owners had the new one. If not, add it.
         if (curOwner != nChildren && 
         std::find(uniqueOwners.begin(), uniqueOwners.end(), curOwner) ==
             uniqueOwners.end()) {

            uniqueOwners.push_back(curOwner);
         }
      }

   // If there are more dimensions to check, recursively examine those
   // dimensions, building on the current owner vector.
   } else {
      for (unsigned int i = low; i <= high; i++) {
         cell[curDim] = i;
         getCellOwnerGroup(&whichDims[1], cell, uniqueOwners, nDims - 1);
      }
   }

   cell[curDim]--;
}

/*
 * This is equivalent to getCellOwnerGroup, it just converts the double point
 * data into a nice integer cell where the point belongs.
 */
void InternalNode::getPointOwnerGroup(double* pt, std::vector<unsigned int>& uniqueOwners) {

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
   getCellOwnerGroup(whichDims, cell, uniqueOwners, dims);
   free(whichDims);
   free(cell);
}

/*
 * This function assigns an integer indentifying the owner of the cell to a
 * given location. A cell is an N-dimensional representation, while the list of
 * owners is 1-dimensional. This function performs the conversion.
 */
void InternalNode::setOwner(unsigned int* cell, unsigned int owner) {
   unsigned int index = 0;
   for (unsigned int i = 0; i < dims; i++) {
      unsigned int cur = cell[i] * dimFactors[i];
      //std::cout << " cur = " << cur << "\n";

      // Check to make sure individual components of cell are within bounds.
      if (cur >= lengths[i] * dimFactors[i]) {
         //std::cout << "ERROR: setOwner called on invalid point (" << cell[0] << ", " << cell[1] << ":" << i << ")!" << std::endl;
         return;
      }

      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      //std::cout << "ERROR: setOwner called on invalid point (off index)!" << std::endl;
      return;
   }

   // We have found the proper index into our 1-dimensional list, so assign the
   // owner there.
   //std::cout << "Setting owner at index " << index << "\n";
   owners[index] = owner;
}



/*
 * This function fixes one dimension, creating a hyper-plane, then recursively
 * cuts hyper-planes through all un-fixed dimensions. Once it is on the last
 * dimension, the function assigns owners to the true location in the owner
 * list.
 */
void InternalNode::assignSubslice(unsigned int* whichDims, 
                                  unsigned int* cell, 
                                  unsigned int owner,
                                  unsigned int nDims,
                                  unsigned int* aLengths) {

   // Identify which dimension is being fixed.
   unsigned int curDim = whichDims[0];
   unsigned int base = cell[curDim];

   // If there are no more variable dimensions, begin setting owners.
   if (nDims == 1) {
      for (unsigned int i = 0; i < aLengths[curDim]; i++) {
         cell[curDim] = base + i;
         //std::cout << "i = " << i << "\n";
         setOwner(cell, owner);
         //std::cout << "Owner set!\n";
      }

   // If there are any more variable dimensions, fix one at a time recursively.
   } else {
      for (unsigned int i = 0; i < aLengths[curDim]; i++) {
         cell[curDim] = base + i;

         // Arg 1: a list of remaining dimensions (first one removed).
         // Arg 2: an integer array representing the current cell in N
         //    dimensions
         // Arg 3: the integer value of the owner
         // Arg 4: the number of dimensions that still need to be fixed.
         assignSubslice(&whichDims[1], cell, owner, nDims - 1, aLengths);
      }
   }
   cell[curDim] = base;
}

unsigned int max_element_index(unsigned int* arr, unsigned int len) {
   unsigned int maxi = 0;
   unsigned int val = arr[0];
   for (unsigned int i = 1; i < len; i++) {
      if (arr[i] > val) {
         val = arr[i];
         maxi = i;
      }
   }
   return maxi;
}

void InternalNode::assignOwners(double* aMins, double* aMaxes, double* caMins, double* caMaxes) {
   //std::cout << "starting...\n";
   unsigned int* aLengths = (unsigned int*)malloc(dims * sizeof(unsigned int));
   //std::cout << "alengths allocated\n";
   for (unsigned int i = 0; i < dims; i++) {
      aLengths[i] = (unsigned int)ceil((aMaxes[i] - aMins[i]) / eps);
   }
   //std::cout << "alengths set\n";
   unsigned int largestDim = max_element_index(aLengths, dims);
   //std::cout << "large dim\n";
   unsigned int dimSize = aLengths[largestDim];//lengths[largestDim];
   //std::cout << "dimsize\n";
   unsigned int curPos = (unsigned int)floor((aMins[largestDim] - mins[largestDim]) / eps);
   //std::cout << "pos = " << curPos << "\n" ;
   double slicesPerOwner = (double)dimSize / (double)nChildren;
   //std::cout << "slices computed\n";
   double remainder = 0;

   // Create a null-terminated list of dimensions to var across.
   unsigned int* whichDims = (unsigned int*)malloc(dims * sizeof(unsigned int));
   //std::cout << "dims allocated\n";
   for (unsigned int i = 0; i < dims; i++) {
      whichDims[i] = i;
   }
   double min, max;
   Point pt(aMins, 1.0, 0);

   //std::cout << "simple dims set\n";
   // We don't want to vary across the dimension being split.
   whichDims[largestDim] = whichDims[dims - 1];
   whichDims[dims - 1] = NULL;

   //std::cout << "recur dims set\n";
   for (unsigned int i = 0; i < dims - 1; i++) {
      min = aMins[whichDims[i]];
      max = aMaxes[whichDims[i]];
      pt.getValue()[whichDims[i]] = min + (max - min) / 2.0;
      //std::cout << "Setting dim " << whichDims[i] << " value to " << pt.getValue()[whichDims[i]] << "\n";
      for (unsigned int j = 0; j < nChildren; j++) {
         //std::cout << i << " " << j << " " << nChildren << " " << caMins << " " << caMaxes << "\n";
         caMins[j * dims + whichDims[i]] = min;
         caMaxes[j * dims + whichDims[i]] = max;
      }
   }

   //std::cout << "non-split ca set\n";
   unsigned int* cell = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < dims; i++) {
      cell[i] = ((unsigned int)floor((aMins[i] - mins[i]) / eps)) * dimFactors[i];
      //std::cout << "(" << aMins[0] << ", " << aMaxes[0] << ") cell[" << i << "] = " << cell[i] << "\n";
   }
   
   //std::cout << "cells allocated\n";
   for (unsigned int i = 0; i < nChildren; i++) {
      min = aMins[largestDim];
      max = aMaxes[largestDim];
      pt.getValue()[largestDim] = (0.5 + (double)i) * (max - min) / nChildren + min;

      //std::cout << "Setting child center " << i << " to X: " << pt.getValue()[0] << " Y: " << pt.getValue()[1] << "\n";
      childCenters[i] = new Point(pt);
      //std::cout << "giving cells to child " << i << "\n";
      // Assign slices to owner and assign cells too.
      unsigned int nSlices = (unsigned int)ceil(slicesPerOwner + remainder);
      //std::cout << "num slices = " << nSlices << "\n";
      unsigned int child = i * dims + largestDim;
      caMins[child] = mins[largestDim] + curPos * eps;
      caMaxes[child] = caMins[child] + nSlices * eps;
      //std::cout << "bounds set = " << caMins[child] << " to " << caMaxes[child] << "\n";
      remainder = slicesPerOwner + remainder - nSlices;
      //std::cout << "remainder = " << remainder << "\n";
      for (unsigned int j = 0; j < nSlices; j++) {
         cell[largestDim] = curPos;
         
         //std::cout << "recursion\n";
         assignSubslice(whichDims, cell, i, dims - 1, aLengths);
         curPos++;
      }
   }
   free(aLengths);
   free(whichDims);
   free(cell);
}

void InternalNode::setLengths() {
   lengths = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < dims; i++) {
      lengths[i] = ceil((maxes[i] - mins[i]) / eps);
   }
}

void InternalNode::setDimFactors() {
   dimFactors = (unsigned int*)malloc(dims * sizeof(unsigned int));
   dimFactors[0] = 1;
   for (unsigned int i = 1; i < dims; i++) {
      dimFactors[i] = dimFactors[i - 1] * lengths[i - 1];
   }
}

void InternalNode::setMaxIndex() {
   
   maxIndex = dimFactors[dims - 1] * lengths[dims - 1] - 1;

}

unsigned int InternalNode::getPointOwner(double* pt) {
   unsigned int index = 0;
   for (unsigned int i = 0; i < dims; i++) {
      unsigned int cur = ((unsigned int)floor((pt[i] - mins[i]) / eps)) * dimFactors[i];

      //std::cout << "Dim value = " << cur << "(mins = " << mins[i] << ")\n";
      // Check to make sure individual components are within bounds.
      if (cur >= lengths[i] * dimFactors[i]) {
         //std::cout << "Point beyond dim " << i << "\n";
         return nChildren;
      }
      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      //std::cout << "Point over max index\n";
      return nChildren;
   }
   //std::cout << "Index = " << index << "\n";
   return owners[index];
}
