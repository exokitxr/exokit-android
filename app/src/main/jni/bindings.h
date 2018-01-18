#ifndef BINDINGS_H_
#define BINDINGS_H_

#include <v8.h>
#include <node.h>

v8::Local<v8::Object> makeGl(node::NodeService *service);

#endif
