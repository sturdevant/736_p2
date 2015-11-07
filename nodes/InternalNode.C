/*
 * This file provides implementations for an internal node (non-leaf). The
 * public functions of internal nodes are described first.
 */

InternalNode::InternalNode(unsigned int nDims,
                           unsigned int nChildren
                           double scale,
                           double* mins,
                           double* maxes,
                           ReturnCode (*requestFunc)(unsigned int, Request*),
                           ReturnCode (*responseFunc)(Response*)) {
   this.dims = nDims;
   this.scale = scale;
   this.mins = mins;
   this.nOwners = nChildren;
   this.requestFunc = requestFunc;
   this.responseFunc = responseFunc;
   //this.thread = (pthread_t*)malloc(sizeof(pthread_t));

   setLengths(maxes);
   setDimFactors();
   setMaxIndex();
   assignOwners();

   // TODO: (maybe? maybe not) Need to thread out behavior here. We need to make it so that this can run
   // independently of the caller and can accept asynchronous requests and
   // responses.

   // TODO: Instantiate ResponseTable
}

InternalNode::~InternalNode() {
   //pthread_cancel(thread);
   //pthread_join(thread, NULL);
   //free(thread);
   free(dimFactors);
   free(lengths);
   free(owners);
}

/*
 * When a point is added, we need to see if we should be aggregating data here
 * (in the case of multiple children having data about the point), or if we
 * should just send the point downstream because one of our children has all of
 * the data on its own.
 *
 * ASSUMPTION: The forwarding function does NOT need the data after it is
 * called and all such pointers should be freed.
 */
ReturnCode InternalNode::addPoint(Request* req) {

   // Verify that the request was actually the type that this function can
   // handle.
   RequestType t = req->getRequestType();
   if (t != REQUEST_TYPE_ADD_POINT) {
      return ERROR_CODE_BAD_REQUEST_TYPE;
   }

   // Get the point origin of the request.
   double* pt = req->getPoint();

   // Get a list of all children with data about that point.
   std::vector<unsigned int> ptOwners();
   getPointOwnerGroup(pt, &ptOwners);

   // Set up the return value, hoping for no troubles.
   ReturnCode err = ERROR_CODE_NO_ERROR;

   // If there wasn't exactly 1 owner, then we need to request data from our
   // children and aggregate it here.
   if (ptOwners.size() > 1) {

      // We need an entry in our table so we know to wait for the incoming
      // point.
      childResponses.registerRequest(req.getID(), ptOwners.size());

      Request* newReq = new Request(REQUEST_TYPE_POINT_DATA);
      newReq.setPoint(pt);
      newReq.setID(req->getID());

      // Go through and send a data request to each of the lower nodes.
      for (size_type i = 0; i < ptOwners.size(); i++) {
         err = (*requestFunc)(newReq);
      }
      delete newReq;

   // If there was exactly 1 child with data about the point, just forward the
   // request.
   } else if (ptOwners.size() == 1) {
      err = (*requestFunc)(req);

      // If there was only one child with data, we don't need to store entries
      // in our table, so we should free the point.
      free(pt);
   }

   // TODO: Do I need to  delete the Request here? Idk, but I think so.
   delete req;

   return err;
}

// TODO: Consider combining query and addPoint.
/*
 * When a request for data is sent, put it on a queue for the thread to get to.
 */
ReturnCode InternalNode::query(Request* req) {
   // Verify that the request was actually the type that this function can
   // handle.
   RequestType t = req->getRequestType();
   if (t == REQUEST_TYPE_ADD_POINT || t == REQUEST_TYPE_INVALID) {
      return ERROR_CODE_BAD_REQUEST_TYPE;
   }

   // Get the point origin of the request.
   double* pt = req->getPoint();

   // Get a list of all children with data about that point.
   std::vector<unsigned int> ptOwners();
   getPointOwnerGroup(pt, &ptOwners);

   // Set up the return value, hoping for no troubles.
   ReturnCode err = ERROR_CODE_NO_ERROR;

   // If there wasn't exactly 1 owner, then we need to request data from our
   // children and aggregate it here.
   if (ptOwners.size() > 1) {

      // We need an entry in our table so we know to wait for the incoming
      // point.
      childResponses.registerRequest(req.getID(), ptOwners.size());

      RequestType newType = t;
      if (t == REQUEST_TYPE_POINT_QUERY) {
         newType = REQUEST_TYPE_POINT_DATA;
      else {
         newType = REQUEST_TYPE_POLYGON_DATA;
      }

      // TODO: Consider reusing req.
      Request* newReq = new Request(newType);
      newReq.setPoint(pt);
      newReq.setID(req->getID());

      // Go through and send a data request to each of the lower nodes.
      for (size_type i = 0; i < ptOwners.size(); i++) {
         err = (*requestFunc)(newReq);
      }
      delete newReq;

   // If there was exactly 1 child with data about the point, just forward the
   // request.
   } else if (ptOwners.size() == 1) {
      err = (*requestFunc)(req);

      // If there was only one child with data, we don't need to store entries
      // in our table, so we should free the point.
      free(pt);
   }

   // TODO: Do I need to  delete the Request here? Idk, but I think so.
   delete req;

   return err;
}

/*
 * When this node is set to recieve information from down stream about one of
 * its requests, this function is called with the response.
 */
void InternalNode::update(Response* res) {
   childResponses.update(res);
}

// PRIVATE INTERFACE BEGINS HERE

// TODO: Thread starting function. (static?)

// TODO: Callback function for the ResponseTable when it has data. (static?)
static void InternalNode::responseCallback(InternalNode* node, Response* responses, unsigned int nResponses) {
   // TODO: Handle point aggregation and what not.
}

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
         return nOwners;
      }

      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      return nOwners;
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
         if (curOwner != nOwners && 
         std::find(uniqueOwners.begin(), uniqueOwners.end(), curOwner) ==
             unqiueOwners.end()) {

            uniqueOwners.add(curOwner);
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
      cell[i] = (unsigned int)floor((pt[i] - mins[i]) * scale);
      whichDims[i] = i;
   }

   // Now, begin the recursive calls to find nearby cells.
   getCellOwnerGroup(whichDims, cell, uniqueOwners, dims);
   free(whichDims);
 free(cells);
}

/*
 * This function assigns an integer indentifying the owner of the cell to a
 * given location. A cell is an N-dimensional representation, while the list of
 * owners is 1-dimensional. This function performs the conversion.
 */
void InternalNode::setOwner(unsigned int* cell, unsigned int owner) {
   unsigned int index = 0;
   for (unsigned int i < dims; i++) {
      unsigned int cur = cell[i] * dimFactors[i];


      // Check to make sure individual components of cell are within bounds.
      if (cur >= lengths[i]) {
         std::cout << "ERROR: setOwner called on invalid point!" << std::endl;
         return;
      }

      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      std::cout << "ERROR: setOwner called on invalid point!" << std::endl;
      return;
   }

   // We have found the proper index into our 1-dimensional list, so assign the
   // owner there.
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
                                  unsigned int nDims) {

   // Identify which dimension is being fixed.
   curDim = whichDims[0];

   // If there are no more variable dimensions, begin setting owners.
   if (nDims = 0) {
      for (unsigned int i = 0; i < lengths[curDim]; i++) {
         cell[curDim] = i;
         setOwner(cell, owner);
      }

   // If there are any more variable dimensions, fix one at a time recursively.
   } else {
      for (unsigned int i = 0; i < lengths[curDim]; i++) {
         cell[curDim] = i;

         // Arg 1: a list of remaining dimensions (first one removed).
         // Arg 2: an integer array representing the current cell in N
         //    dimensions
         // Arg 3: the integer value of the owner
         // Arg 4: the number of dimensions that still need to be fixed.
         assignSubslice(&whichDims[1], cell, owner, nDims - 1);
      }
   }
}

void InternalNode::assignOwners() {
   unsigned int largestDim = max_element_index(lengths, dims);
   unsigned int dimSize = lengths[largestDim];
   unsigned int curPos = 0;
   double slicesPerOwner = (double)dimSize / (double)nOwners;
   double remainder = 0;

   // Create a null-terminated list of dimensions to var across.
   unsigned int* whichDims = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < dims; i++) {
      whichDims[i] = i;
   }

   // We don't want to vary across the dimension being split.
   whichDims[largestDim] = whichDims[dims - 1];
   whichDims[dims - 1] = NULL;

   unsigned int* cell = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < nOwners; i++) {
      // Assign slices to owner and assign cells too.
      unsigned int nSlices = (unsigned int)ceil(slicesPerOwner + remainder);
      remainder = slicesPerOwner + remainder - nSlices;
      for (unsigned int j = 0; j < nSlices; j++) {
         cell[largestDim] = curPos;
         assignSubslice(whichDims, cell, i, dims - 1);
         curPos++;
      }
   }
   free(whichDims);
   free(cell);
}

void InternalNode::setLengths(double* maxes) {
   lengths = (unsigned int*)malloc(dims * sizeof(unsigned int));
   for (unsigned int i = 0; i < dims; i++) {
      lengths[i] = ceil((maxes[i] - mins[i]) * scale);
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
   
   maxIndex = dimFactor[dims - 1] * lengths[dims - 1] - 1;

}

unsigned int InternalNode::getPointOwner(double* pt) {
   unsigned int index = 0;
   for (unsigned int i = 0; i < dims; i++) {
      unsigned int cur = ((unsigned int)floor((pt[i] - mins[i]) * scale)) * dimFactors[i];

      // Check to make sure individual components are within bounds.
      if (cur >= lengths[i]) {
         return nOwners;
      }
      index += cur;
   }

   // If the point wasn't within our box, return a bad value;
   if (index < 0 || index > maxIndex) {
      return nOwners;
   }
   return owners[index];
}
