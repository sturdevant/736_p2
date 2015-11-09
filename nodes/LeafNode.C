/*
 * This file provides implementations for an internal node (non-leaf). The
 * public functions of internal nodes are described first.
 */

LeafNode::LeafNode(unsigned int nDims,
                   double scale,
                   double* mins,
                   double* maxes,
                   ReturnCode (*requestFunc)(unsigned int, Request*),
                   ReturnCode (*responseFunc)(Response*)) {

   this.dims = nDims;
   this.scale = scale;
   this.mins = mins;
   this.requestFunc = requestFunc;
   this.responseFunc = responseFunc;
   //this.thread = (pthread_t*)malloc(sizeof(pthread_t));

   setLengths(maxes);
   setDimFactors();
   setMaxIndex();
   initPointLists();

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
Response* LeafNode::query(Request* req) {
   // Verify that the request was actually the type that this function can
   // handle.
   RequestType t = req->getRequestType();
   switch(t) {
   case REQUEST_TYPE_UPDATE_POINT:
      return handleUpdate(req);
   case REQUEST_TYPE_ADD_POINT:
      return handleAdd(req);
   case REQUEST_TYPE_POINT_DATA:
      return handlePoint(req);
   case REQUEST_TYPE_POLYGON_DATA:
      return handlePolygon(req);
   }

   return RETURN_CODE_BAD_REQUEST_TYPE;
}

// PRIVATE INTERFACE BEGINS HERE

// TODO: Thread starting function. (static?)

unsigned long LeafNode::countNeighbors(double* pt) {
   
}

/*
 * This is equivalent to getCellOwnerGroup, it just converts the double point
 * data into a nice integer cell where the point belongs.
 */
void LeafNode::getPointOwnerGroup(double* pt, std::vector<unsigned int>& uniqueOwners) {

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
