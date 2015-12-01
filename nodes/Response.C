
#include "Response.h"

Response::Response(ResponseType type, unsigned long id) {
   type = RESPONSE_TYPE_INVALID;
   this->id = id;
}

Response::~Response() {
}

void Response::setType(ResponseType t) {
   type = t;
}

void Response::setValue(double v) {
   value = v;
}

double Response::getValue() {
   return value;
}

unsigned long Response::getClusterId() {
   return clusterId;
}

void Response::setClusterId(unsigned long clustId) {
   clusterId = clustId;
}

void Response::setTime(unsigned long newTime) {
   time = newTime;
}

void Response::unpack(
      unsigned long* uId,
      unsigned long* uType,
      unsigned long* uTime,
      double* uValue,
      unsigned long* uClusterId) {
   
   *uType = type;
   *uId = id;
   *uTime = time;
   *uValue = value;
   *uClusterId = clusterId;
}

