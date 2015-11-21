
#include "Response.h"

Response::Response() {
   type = RESPONSE_TYPE_INVALID;
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
